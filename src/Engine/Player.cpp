/* ===================================================================== *
 * Player.cpp (MeV/Engine)
 * ===================================================================== */

#include "Player.h"

#include "EventStack.h"
#include "Idents.h"
#include "MidiDeviceInfo.h"
#include "PlaybackTask.h"
#include "PlaybackTaskGroup.h"
#include "TimeUnits.h"
#include "Track.h"

// Gnu C Library
#include <stdio.h>
// Midi Kit
#include <MidiProducer.h>
// Support Kit
#include <Debug.h>

#define D_CONTROL(x) //PRINT(x)
#define D_EVENT(x) //PRINT(x)
#define D_WARNING(x) //PRINT(x)

const int32			maxSleep = 30;

CPlayer				thePlayer;

// ---------------------------------------------------------------------------
// Constructor/Destructor

CPlayer::CPlayer()
	:	m_songGroup(NULL),
		m_wildGroup(NULL)
{
}

CPlayer::~CPlayer()
{
	StopControlThread();

	CPlaybackTaskGroup	*group;

	// Delete song contexts
	while ((group = (CPlaybackTaskGroup *)m_groupList.First()))
		delete group;
	m_songGroup = m_wildGroup = NULL;
}

// ---------------------------------------------------------------------------
// Accessors

CPlaybackTaskGroup *
CPlayer::FindGroup(
	CMeVDoc *doc)
{
	if ((doc != NULL) && (m_songGroup->doc == doc))
		return m_songGroup;
	return NULL;
}

// ---------------------------------------------------------------------------
// Operations

void
CPlayer::Initialize()
{
	m_thread = spawn_thread(
		&ControlThreadEntry,
		"CPlayer",
		B_REAL_TIME_PRIORITY,
		this);

	m_port = create_port( 1024, "MeVPlayer Command Port" );

	// Set up a context for playing songs
	// REM: Should pass the document pointer as parameter
	m_songGroup = new CPlaybackTaskGroup( NULL );
	D_CONTROL((">>>>> m_songGroup: %p\n", m_songGroup));

	// Set up a context for miscellaneous note-playing
	// REM: wildContext->setTime( m_internalTimerTick, 0 );
	m_wildGroup = new CPlaybackTaskGroup( NULL );
	D_CONTROL((">>>>> m_wildGroup: %p\n", m_wildGroup));

	// Set up wildContext with current time, and start it.
	m_wildGroup->origin = 0;
	m_wildGroup->real.time = 0;
	m_wildGroup->metered.time = 0;
	// set context to "running always"
	m_wildGroup->flags = CPlaybackTaskGroup::Clock_Continuous;

	resume_thread(m_thread);
}

bool
CPlayer::QueueEvents(
	CEvent *eventList,
	uint32 count,
	long startTime)
{
	// Note: Locking not needed since stack has it's own lock.
	if (m_wildGroup->real.stack.PushList(eventList, count, startTime))
		return true;

	return false;
}

bool 
CPlayer::QueueImmediate(
	CEvent *eventList,
	uint32 count)
{
	return QueueEvents(eventList, count, uint32(system_time() / 1000LL));
}

// ---------------------------------------------------------------------------
// The main player loop

status_t 
CPlayer::ControlThreadEntry(
	void *user)
{
	(static_cast<CPlayer*>(user))->ControlThread();
	return B_OK;
}

status_t 
CPlayer::StopControlThread()
{
	thread_id tid = m_thread;
	if(tid < 0)
	{
		D_WARNING(("CPlayer::StopControlThread(): thread already stopped\n"));
		return B_NOT_ALLOWED;
	}
	status_t err = write_port_etc(m_port, Command_Quit, 0, 0, B_TIMEOUT, 250000LL);
	if(err < B_OK)
	{
		D_WARNING(("CPlayer::StopControlThread(): write_port_etc(): %s\n",
			strerror(err)));
		return err;
	}
	while(wait_for_thread(tid, &err) == B_INTERRUPTED) {}
	m_thread = 0;
	return B_OK;
}


void
CPlayer::ControlThread()
{
	D_CONTROL(("CPlayer::Run()\n"));

	// Initialize time vars
	m_internalTimerTick = system_time() / 1000;

	// +++++ convert to bigtime_t
	long nextEventTime = m_internalTimerTick;
	
	bool keepRunning = true;
	while (keepRunning)
	{
		bigtime_t wakeUp = (bigtime_t)nextEventTime * 1000 - system_time();
		if (wakeUp < 0)
		{
			D_CONTROL(("late: next event %Ld, now %Ld\n",
					   bigtime_t(nextEventTime) * 1000,
					   system_time()));
			wakeUp = 0;
		}
		
		int32 command;
		CommandArgs args;
		if (read_port_etc(m_port, &command, &args, sizeof(args),
						  B_TIMEOUT, wakeUp) >= 0)
		{
			switch (command)
			{
				case Command_Start:
				{
					D_CONTROL(("CPlayer: Command_Start\n"));

					if (args.document == NULL)
						break;
					m_songGroup->doc = args.document;
	
					if ((args.trackID >= 0) && (args.trackID <= 1))
					{
						CTrack *meterTrack = args.document->FindTrack((int32)0);
						CTrack *realTrack = args.document->FindTrack(1);
						if ((meterTrack != NULL) && (realTrack != NULL))
						{	
							m_songGroup->Start(meterTrack, realTrack,
											   args.locTime,
											   (enum ELocateTarget)args.locateTarget,
											   args.duration,
											   (enum ESyncType)args.syncType,
											   args.options);
						}
					}
					else
					{
						CTrack	*track = args.document->FindTrack(args.trackID);
						if (track != NULL)
						{	
							m_songGroup->Start(track, NULL,
											   args.locTime,
											   (enum ELocateTarget)args.locateTarget,
											   args.duration,
											   (enum ESyncType)args.syncType,
											   args.options);
						}
					}
					break;
				}	
				case Command_Stop:
				{
					D_CONTROL(("CPlayer: Command_Stop\n"));

					if (args.document == NULL)
						break;
					m_songGroup->flags |= CPlaybackTaskGroup::Clock_Stopped;
					m_songGroup->FlushNotes();
					break;
				}	
				case Command_Pause:
				case Command_Continue:
				{
					break;
				}
				case Command_Quit:
				{
					D_CONTROL(("CPlayer: Command_Quit\n"));

					keepRunning = false;
					break;
				}
			}
			
			if (command != Command_Attention)
			{
				BMessage message(Player_ChangeTransportState);
				be_app->PostMessage(&message);
			}
		}
		
		// And then process any waiting events.
		m_internalTimerTick = system_time() / 1000;
		nextEventTime = m_internalTimerTick + maxSleep;
		StPlayerLock lock;

		CPlaybackTaskGroup *group;
		for (group = (CPlaybackTaskGroup *)m_groupList.First();
			 group;
			 group = (CPlaybackTaskGroup *)group->Next())
		{
			group->nextEventTime = LONG_MAX;
			group->_update(m_internalTimerTick);

			// Compute next event time as min of all track times
			int32 nextGroupEvent = group->nextEventTime + group->origin;
			if (IsTimeGreater(nextGroupEvent, nextEventTime))
				nextEventTime = nextGroupEvent;
		}
	}
}

/*	1. How are the timestamps going to be interpreted from the input device?

	Launch task at moment's notice (example: key mapping)

	Relocate at moment's notice
		MIDI start
		MIDI continue
		Cue buttons during playback.
		Cue buttons at other times.
		MTC discontinuities

	Negative start times:
		Count-in
		MTC with offset
*/

// END - Player.cpp
