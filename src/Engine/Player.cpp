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

#define D_CONTROL(x) //PRINT (x)
#define D_SYNTH(x) //PRINT (x)
#define D_EVENT(x) //PRINT (x)
#define D_WARNING(x) //PRINT (x)

const int32			maxSleep = 30;

CPlayer				thePlayer;

// ---------------------------------------------------------------------------
// Constructor/Destructor

CPlayer::CPlayer()
{
	for (uint32 i = 0; i < Max_MidiPorts; i++)
	{
		m_ports[i] = NULL;
		strcpy(m_portInfo[i].portName, "");
		strcpy(m_portInfo[i].devString, "");
	}

	InitChannelStates();

	songGroup = NULL;
	wildGroup = NULL;
}

CPlayer::~CPlayer()
{
	StopControlThread();

	CPlaybackTaskGroup	*group;

	// Delete song contexts
	while ((group = (CPlaybackTaskGroup *)m_groupList.First()))
		delete group;
	songGroup = wildGroup = NULL;

	// release ports
	// REM: Commented out because code crashes...!
	for (uint32 i = 0; i < Max_MidiPorts; i++)
	{
		m_ports[i]->Release();
	}
}

// ---------------------------------------------------------------------------
// Accessors

CPlaybackTaskGroup *
CPlayer::FindGroup(
	CMeVDoc *doc)
{
	if ((doc != NULL) && (songGroup->doc == doc))
		return songGroup;
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
	songGroup = new CPlaybackTaskGroup( NULL );
	D_CONTROL((">>>>> songGroup: %p\n", songGroup));

	// Set up a context for miscellaneous note-playing
	// REM: wildContext->setTime( m_internalTimerTick, 0 );
	wildGroup = new CPlaybackTaskGroup( NULL );
	D_CONTROL((">>>>> wildGroup: %p\n", wildGroup));

		// Set up wildContext with current time, and start it.
	wildGroup->origin = 0; // system_time() / 1000;
	wildGroup->real.time = 0;
	wildGroup->metered.time = 0;
		// set context to "running always"
	wildGroup->flags = CPlaybackTaskGroup::Clock_Continuous;

	resume_thread(m_thread);
}

void
CPlayer::InitChannelStates()
{
	// Initialize all channel states...
	for (uint32 i = 0; i < Max_MidiPorts; i++)
	{
		for (int j = 0; j < 16; j++)
		{
			memset(&m_portInfo[i].channelStates[j], 0xff, sizeof(ChannelState));
		}
	}
}

bool
CPlayer::QueueEvents(
	Event *eventList,
	uint32 count,
	long startTime)
{
	// Note: Locking not needed since stack has it's own lock.
	if (wildGroup->real.stack.PushList(eventList, count, startTime))
		return true;

	return false;
}

bool 
CPlayer::QueueImmediate(
	Event *eventList,
	uint32 count)
{
	return QueueEvents(eventList, count, uint32(system_time() / 1000LL));
}

void
CPlayer::SendEvent(
	const Event &ev,
	BMidiLocalProducer *port,
	uchar channel,
	bigtime_t time)
{
	if (port == NULL)
		return;

	// Attempt to send the event.
	switch (ev.Command())
	{
		case EvtType_Note:
		{
			D_EVENT(("SendEvent(NoteOn)\n"));
			port->SprayNoteOn(channel, ev.note.pitch,
							  ev.note.attackVelocity, time);
			break;
		}
		case EvtType_NoteOff:
		{
			D_EVENT(("SendEvent(NoteOff)\n"));
			port->SprayNoteOff(channel, ev.note.pitch,
							   ev.note.releaseVelocity, time);
			break;
		}
		case EvtType_ChannelATouch:
		{
			D_EVENT(("SendEvent(ChannelATouch)\n"));

			ChannelState *chState = &m_portInfo[0].channelStates[0];
			if (chState->channelAfterTouch != ev.aTouch.value)
			{
				port->SprayChannelPressure(channel, ev.aTouch.value,
										   time);
				chState->channelAfterTouch = ev.aTouch.value;
			}
			break;
		}
		case EvtType_PolyATouch:
		{
			D_EVENT(("SendEvent(PolyATouch)\n"));
			port->SprayKeyPressure(channel, ev.aTouch.pitch,
								   ev.aTouch.value, time);
			break;
		}
		case EvtType_Controller:
		{
			D_EVENT(("SendEvent(Controller)\n"));
	
			ChannelState *chState = &m_portInfo[0].channelStates[0];
			// Check if it's a 16-bit controller
			uint8 lsbIndex = controllerInfoTable[ev.controlChange.controller].LSBNumber;
			if ((lsbIndex == ev.controlChange.controller)
			 || (lsbIndex > 127))
			{
				// It's an 8-bit controller.
				if (ev.controlChange.MSB != chState->ctlStates[ev.controlChange.controller])
				{
					port->SprayControlChange(channel, ev.controlChange.controller,
											 ev.controlChange.MSB, time);
					chState->ctlStates[ev.controlChange.controller] = ev.controlChange.MSB;
				}
			}
			else
			{
				// Handle the MSB first...if the MSB changed, then update LSB state as well
				if ((ev.controlChange.MSB != chState->ctlStates[ev.controlChange.controller])
				 && (ev.controlChange.MSB < 128))
				{
					port->SprayControlChange(channel,
											 ev.controlChange.controller,
											 ev.controlChange.MSB, time);
					chState->ctlStates[ev.controlChange.controller] = ev.controlChange.MSB;
					chState->ctlStates[lsbIndex] = 0;
				}

				// Now deal with the LSB.
				if ((ev.controlChange.LSB != chState->ctlStates[lsbIndex])
				 && (ev.controlChange.LSB < 128))
				{
					port->SprayControlChange(channel, lsbIndex,
											 ev.controlChange.LSB, time);
					chState->ctlStates[lsbIndex] = ev.controlChange.LSB;
				}
			}
			break;
		}
		case EvtType_ProgramChange:
		{
			D_EVENT(("SendEvent(ProgramChange)\n"));

			ChannelState *chState = &m_portInfo[0].channelStates[0];
			// Only send bank changes if the instrument understands such...
			// And only if this event has a valid bank number
			MIDIDeviceInfo *mdi = NULL;
			if (mdi != NULL && mdi->SupportsProgramBanks())
			{
				// Don't send a bank change if the instrument is already
				// set to that bank
				if ((ev.programChange.bankMSB < 128)
				 && (ev.programChange.bankLSB < 128)
				 && ((chState->ctlStates[0] != ev.programChange.bankMSB)
				  || (chState->ctlStates[32] != ev.programChange.bankLSB)))
				{
					// Both bank bytes must always be sent...
					// Send a bank change MSB message
					port->SprayControlChange(channel, 0x00,
											 ev.programChange.bankMSB,
											 time);
					// Send a bank change LSB message
					port->SprayControlChange(channel, 0x20,
											 ev.programChange.bankLSB,
											 time);
					// Update the channel state
					chState->ctlStates[0] = ev.programChange.bankMSB;
					chState->ctlStates[32] = ev.programChange.bankLSB;
				}
			}
			
			// Always send a program change, regardless...
			port->SprayProgramChange(channel, ev.programChange.program,
									 time);
			chState->program = ev.programChange.program;
			break;
		}	
		case EvtType_PitchBend:
		{
			D_EVENT(("SendEvent(PitchBend)\n"));
	
			ChannelState *chState = &m_portInfo[0].channelStates[0];
			// Don't send un-needed pitch-bends
			if (chState->pitchBendState != ev.pitchBend.targetBend)
			{
				port->SprayPitchBend(channel, ev.pitchBend.targetBend & 0x7f,
									 ev.pitchBend.targetBend >> 7, time);
				chState->pitchBendState = ev.pitchBend.targetBend;
			}
			break;
		}
		case EvtType_SysEx:
		{
			D_EVENT(("SendEvent(SysEx)\n"));
	
			void *data = ev.ExtendedData();
			int32 size = ev.ExtendedDataSize();	
			if ((data != NULL) && (size > 0))
			{
				port->SpraySystemExclusive(data, size, time);
			}
			break;
		}
	}
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
					CRefCountObject::Release(songGroup->doc);
					songGroup->doc = args.document;
					songGroup->m_destlist = args.document->GetDestinationList(  );
					// Reset all channel state records...
					InitChannelStates();
	
					if ((args.trackID >= 0) && (args.trackID <= 1))
					{
						CTrack *meterTrack = args.document->FindTrack((int32)0);
						CTrack *realTrack = args.document->FindTrack(1);
						if ((meterTrack != NULL) && (realTrack != NULL))
						{	
							songGroup->Start(meterTrack, realTrack,
											 args.locTime,
											 (enum ELocateTarget)args.locateTarget,
											 args.duration,
											 (enum ESyncType)args.syncType,
											 args.options);
						}
						else
						{
							if (meterTrack)
								CRefCountObject::Release(meterTrack);
							if (realTrack)
								CRefCountObject::Release(realTrack);
						}
					}
					else
					{
						CTrack	*track = args.document->FindTrack(args.trackID);
						if (track != NULL)
						{	
							songGroup->Start(track, NULL,
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
					CRefCountObject::Release(args.document);
					songGroup->flags |= CPlaybackTaskGroup::Clock_Stopped;
					songGroup->FlushNotes();
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
			group->Update(m_internalTimerTick);

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
