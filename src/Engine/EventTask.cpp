/* ===================================================================== *
 * EventTask.cpp (MeV/Engine)
 * ===================================================================== */
 
#include "EventTask.h"

#include "MeVApp.h"
#include "MidiDestination.h"
#include "MidiDeviceInfo.h"
#include "PlaybackTaskGroup.h"
#include "Player.h"

// Support Kit
#include <Debug.h>

#define D_ALLOC(x) //PRINT(x)			// Constructor/Destructor
#define D_HOOK(x) //PRINT(x)			// CPlaybackTask Implementation
#define D_INTERNAL(x) //PRINT(x)		// Internal Operations

// ---------------------------------------------------------------------------
// Constructor/Destructor

CEventTask::CEventTask(
	CPlaybackTaskGroup &group,
	CEventTrack		*tr,
	TState			&inTimeBase,
	CPlaybackTask	*par,
	int32			start,
	int32			end )
		: CPlaybackTask(group, tr, par, start),
		  timeBase(inTimeBase),
		  playPos(tr->Events())
{
	transposition		= 0;
	clockType			= ClockType_Real;
	repeatStack		= NULL;
	trackEndTime		= tr->LogicalLength();
	taskDuration		= end >= 0 ? end : LONG_MAX;
	nextRepeatTime	= trackEndTime;
	interruptable		= true;

	playPos.First();
}

CEventTask::CEventTask( CPlaybackTaskGroup &group, CEventTask &th )
		: CPlaybackTask(group, th),
		  timeBase(th.timeBase),
		  playPos(th.playPos)
{
	transposition		= th.transposition;
	clockType			= th.clockType;
	nextRepeatTime	= th.nextRepeatTime;
	trackEndTime		= th.trackEndTime;
	taskDuration		= th.taskDuration;
	interruptable		= th.interruptable;
}

CEventTask::~CEventTask()
{
	// REM: Should also search stacks and kill

		// Delete all pending repeats
	while (repeatStack)
	{
		RepeatState	*rps = repeatStack->next;
		delete repeatStack;
		repeatStack = rps;
	}
}

// ---------------------------------------------------------------------------
// CPlaybackTask Implementation

int32
CEventTask::CurrentTime() const
{
	return timeBase.seekTime - originTime;
}

void
CEventTask::Play()
{
	D_HOOK(("CEventTask::Play()\n"));

	int32 targetTime;
	bool locating = (group.flags & CPlaybackTaskGroup::Clock_Locating);

	// If we're locating, then we want to lock for certain
	if (locating)
	{
		targetTime = timeBase.seekTime;
		track->ReadLock();
	}
	else
	{
		if (!track->ReadLock(500))
		{
			// Attempt to lock the track; if we fail, then just continue
			// and play something else.
			ReQueue(timeBase.stack, timeBase.seekTime + 10);
			return;
		}
		targetTime = timeBase.seekTime + eventAdvance;
	}

	int32 actualEndTime = originTime + taskDuration - 1;	
	currentTime = targetTime - originTime;

	// If there's a repeat scheduled before the playback time.
	while (nextRepeatTime <= currentTime)
	{
		// Play all of the events before the repeat.
		for (const CEvent *ev = (const CEvent *)playPos; ev != NULL; )
		{
			if (ev->Start() >= nextRepeatTime)
				break;
			if (IsTimeGreater(taskDuration-1, ev->Start()))
				break;

			_stackEvent(*ev, timeBase.stack, originTime);
			ev = playPos.Seek(1);
		}

		// Process the repeat. If there was no repeat to process, then...
		if (!_repeat())
		{
			// Check to see if we've run out of track
			if (currentTime >= trackEndTime || (flags & Task_Finished))
			{
				// If we've reached the end, then exit this routine...
				flags |= Task_Finished;
				ReQueue(timeBase.stack, trackEndTime + originTime - 1);
			}
			else
			{
				ReQueue(timeBase.stack, nextRepeatTime + originTime - 1);
			}
			track->ReadUnlock();
			return;
		}

		targetTime = timeBase.seekTime;
	}

	for (const CEvent *ev = (const CEvent *)playPos; ev != NULL; )
	{
		long t = ev->Start() + originTime;

		if (IsTimeGreater(actualEndTime, t))
		{
			// past end of task
			flags |= Task_Finished;
			track->ReadUnlock();
			return;
		}

		if (IsTimeGreater(targetTime, t))
		{
			// done with this chunk
			ReQueue(timeBase.stack, locating ? t : t - trackAdvance);
			track->ReadUnlock();
			return;
		}

		_stackEvent(*ev, timeBase.stack, originTime);
		ev = playPos.Seek(1);
	}

	if ((repeatStack != NULL) || (currentTime < taskDuration))
	{
		ReQueue(timeBase.stack, nextRepeatTime + originTime);
		track->ReadUnlock();
		return;
	}

	// REM: Is this incorrect for the master track?
	track->ReadUnlock();
	flags |= Task_Finished;
	ReQueue(timeBase.stack, trackEndTime + originTime);
}

// ---------------------------------------------------------------------------
// Internal Operations

void
CEventTask::_beginRepeat(
	int32 inRepeatStart,
	int32 inRepeatDuration,
	int32 inRepeatCount)
{
	if ((inRepeatCount == 1)
	 || (inRepeatDuration <= 0))
	 	return;

	// Ignore overlapped repeat events
	if ((repeatStack == NULL)
	 || (repeatStack->endTime >= inRepeatStart + inRepeatDuration))
	{
		// Make a new repeat state, starting at the next event
		RepeatState	*rps = new RepeatState(playPos);
		rps->pos.Seek(1);
		rps->endTime = inRepeatStart + inRepeatDuration;
		rps->timeOffset = inRepeatDuration;
		rps->repeatCount = inRepeatCount;
		rps->next = repeatStack;

		// If this is a master track, then adjust something or other...
		if ((track == group.mainTracks[0])
		 || (track == group.mainTracks[1]))
		{
			if (inRepeatCount > 0)
				timeBase.expansion += (inRepeatCount - 1) * rps->timeOffset;
		}

		// Set the time of the next repeat concern
		repeatStack = rps;
		nextRepeatTime = MIN(trackEndTime, rps->endTime);
	}
}

bool
CEventTask::_repeat()
{
	bool result = false;

	// Handle repeat events.
	while (	repeatStack
			&& timeBase.seekTime - originTime >= repeatStack->endTime - 5)
	{
		playPos = repeatStack->pos;
		originTime += repeatStack->timeOffset;
		currentTime = timeBase.seekTime - originTime;
		result = true;

		// Now, back-seek to find any events which got skipper over.
		EventMarker	sPos( playPos );
		const CEvent	*s;
		int32		count = 0;

		// Back up in sequence, until we reach either the beginning of
		// the track, or we find an event which occured before the
		// start of the repeat event.
		while ((s = sPos.Peek(-1)))
		{
			if (s->Start() < repeatStack->endTime - repeatStack->timeOffset)
				break;
				
			sPos.Seek( -1 );
			count++;
		}
		
			// Now, play forward again, and queue for playback all of the
			// events which we skipped over, except for any repeat events.
		while (count--)
		{
			s = sPos.Peek( 0 );
			if (		s != NULL
				&&	s->Command() != EvtType_Repeat)
			{
				_stackEvent( *s, timeBase.stack, originTime );
				
			}
			sPos.Seek( 1 );
		}

		if (repeatStack == NULL) break;

		if (repeatStack->repeatCount > 0)		// Zero repeats forever
		{
			repeatStack->repeatCount--;
			if (repeatStack->repeatCount <= 1)
			{
				RepeatState	*rps = repeatStack->next;
				delete repeatStack;
				repeatStack = rps;

				if (repeatStack)
					nextRepeatTime = repeatStack->endTime;
				else nextRepeatTime = trackEndTime;
			}
		}
	}
	

		// OK, here's another go at it. If the sequence failed to repeat, and we're seeking to
		// a time which is beyond the end of the track, and we're doing auto-looping, then
		// let's attempt to re-start the sequence.
	if (		result == false
		&&	currentTime >= trackEndTime
		&& 	(flags & Task_ImplicitLoop))
	{
			// Delete all pending repeats
		while (repeatStack != NULL)
		{
			RepeatState	*rps = repeatStack->next;
			delete repeatStack;
			repeatStack = rps;
		}
		
		if (taskDuration != LONG_MAX) taskDuration -= track->LogicalLength();

			// Set play position to start of track, and adjust origin and current time.
		playPos.First();
		originTime += track->LogicalLength();
		currentTime = timeBase.seekTime - originTime;
		
		//	REM: If this is the master track[ 0 ], then also loop the absolute track at the
		//	same time...
		result = true;
	}

	if (interruptable && trackEndTime > taskDuration) trackEndTime = taskDuration;

	return result;
}

void
CEventTask::_stackEvent(
	const CEvent &ev,
	CEventStack &stack,
	long origin)
{
	D_INTERNAL(("CEventTask::_stackEvent([%ld] %s)\n",
				ev.Command(), ev.NameText()));

	CEvent stackedEvent(ev);

	// Make a copy of the duration field before we kick it to death (below).
	int32 duration = stackedEvent.common.duration - 1;

	// Process a copy of the event event through the filters...
	((CEventTrack *)track)->FilterEvent(stackedEvent);

	// filter event though virtual channel table
	if (ev.HasProperty(CEvent::Prop_MIDI))
	{
		CDestination *dest = group.Document()->FindDestination(ev.common.vChannel);
		stackedEvent.stack.start += origin;
		stackedEvent.stack.task = taskID;
		if (dest->ReadLock(500))
		{
			dest->Stack(stackedEvent, *this, stack, duration);
			dest->ReadUnlock();
		}
		return;
	}
	else
	{	
		// Modify the stack
		stackedEvent.stack.start += origin;
		stackedEvent.stack.task = taskID;
		stackedEvent.stack.destination = NULL;

		switch (ev.Command())
		{
			case EvtType_Repeat:
			{
				_beginRepeat(ev.Start(), ev.Duration(), ev.repeat.repeatCount);
				break;
			}
			case EvtType_Sequence:
			{
				CTrack *tr = group.doc->FindTrack(ev.sequence.sequence);
				CPlaybackTask *p;
	
				if (tr == NULL)
					break;
	
				StSubjectLock lock(*tr, Lock_Shared);
				for (p = this; p != NULL; p = p->Parent())
				{
					// Don't allow tracks to call each other recursively.
					if (p->Track() == tr)
						return;
				}
				
				CEventTrack	*tk = dynamic_cast<CEventTrack *>(tr);
				
				// If it's an event track and not empty or muted
				if ((tk != NULL) && !tk->Events().IsEmpty()
				 && !tk->Muted() && !tk->MutedFromSolo())
				{
					CEventTask *th;
					int32 start = stackedEvent.stack.start;
					int32 stop = start + duration;
					
					// Launch based on clock type.
					if (tr->ClockType() == ClockType_Real)
					{
						if (track->ClockType() == ClockType_Metered)
						{
							start = group.tempo.ConvertMeteredToReal(start);
							stop = group.doc->TempoMap().ConvertMeteredToReal(stop);
							duration = stop - start;
						}
						
						th = new CRealClockEventTask(group, tk, this, start,
													 duration);
						th->interruptable = (ev.sequence.flags & CEvent::Seq_Interruptable);
					}
					else
					{
						if (track->ClockType() == ClockType_Real)
						{
							start = group.tempo.ConvertRealToMetered(start);
							stop = group.doc->TempoMap().ConvertRealToMetered(stop);
							duration = stop - start;
						}
					
						th = new CMeteredClockEventTask(group, tk, this, start,
														duration);
						th->interruptable = (ev.sequence.flags & CEvent::Seq_Interruptable);
					}
					th->transposition = ev.sequence.transposition;
				}
				break;
			}
		}
	}
}

// ---------------------------------------------------------------------------
// CRealTimeEventTask constructor

CRealClockEventTask::CRealClockEventTask(
	CPlaybackTaskGroup &group,
	CEventTrack		*tr,
	CPlaybackTask	*par,
	int32			start,
	int32			end )
		: CEventTask( group, tr, group.real, par, start, end )
{
	trackAdvance = cTrackAdvance_Real;
	eventAdvance = cEventAdvance_Real;
	clockType = ClockType_Real;
	ReQueue( timeBase.stack, start );
}

// ---------------------------------------------------------------------------
// CRealTimeEventTask constructor

CMeteredClockEventTask::CMeteredClockEventTask(
	CPlaybackTaskGroup &group,
	CEventTrack		*tr,
	CPlaybackTask	*par,
	int32			start,
	int32			end )
		: CEventTask( group, tr, group.metered, par, start, end )
{
	trackAdvance = cTrackAdvance_Metered;
	eventAdvance = cEventAdvance_Metered;
	clockType		= ClockType_Metered;
	ReQueue( timeBase.stack, start );
}

// END -- EventTask.cpp
