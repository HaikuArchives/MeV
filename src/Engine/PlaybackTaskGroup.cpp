/* ===================================================================== *
 * PlaybackTaskGroup.cpp (MeV/Engine)
 * ===================================================================== */
 
#include "PlaybackTask.h"
#include "PlaybackTaskGroup.h"
#include "Player.h"
#include "EventTask.h"
#include "Idents.h"

//#include <stdio.h>
// Support Kit
#include <Debug.h>

#define D_ALLOC(x) //PRINT(x)			// Constructor/Destructor
#define D_EXECUTE(x) //PRINT(x)
#define D_INTERNAL(x) //PRINT(x)		// Internal Operations

#define LOCATE_MAX 200

// ---------------------------------------------------------------------------
// Constructor/Destructor

CPlaybackTaskGroup::CPlaybackTaskGroup(
	CMeVDoc *inDocument)
{
	LOCK_PLAYER;
	
	// Initialize what fields need it
	doc = inDocument;
	flags = Clock_Stopped;
	origin = thePlayer.m_internalTimerTick; // +++++ REMOVE THIS DEPENDANCY +++++
	real.time = real.start = 0;
	metered.time = metered.start = 0;
	syncType = SyncType_FreeRunning;
	locateType = LocateTarget_Continue;
	locatorThread = -1;
	mainTracks[0] = mainTracks[ 1 ] = NULL;
	pbOptions = 0;

	// Setup the initial tempo variables
	tempo.SetInitialTempo(RateToPeriod(doc ? doc->InitialTempo()
										   : CMeVDoc::DEFAULT_TEMPO));

	// Add ourselves to the list of playback contexts
	thePlayer.m_groupList.AddTail(this);
}

CPlaybackTaskGroup::~CPlaybackTaskGroup()
{
	LOCK_PLAYER;

	if (locatorThread >= B_NO_ERROR)
		kill_thread(locatorThread);

	// delete all active tasks
	_flushTasks();

	// Remove from list of playback contexts
	Remove();
}

// ---------------------------------------------------------------------------
// Accessors

int32
CPlaybackTaskGroup::CurrentTempoPeriod() const
{
	return (int32)tempo.CalcPeriodAtTime(metered.time, ClockType_Metered);
}

void
CPlaybackTaskGroup::FlushEvents()
{
	LOCK_PLAYER;

	// Pop all events off of the stack!
	// But only execute the note-offs.
	Event ev;
	while (real.stack.Pop(ev))
	{
		if (ev.Command() == EvtType_NoteOff)
			_executeEvent(ev, real);
	}
	while (metered.stack.Pop(ev))
	{
		if (ev.Command() == EvtType_NoteOff)
			_executeEvent(ev, metered);
	}
}

void
CPlaybackTaskGroup::FlushNotes()
{
	LOCK_PLAYER;
	
	_flushNotes(real.stack);
	_flushNotes(metered.stack);
}

void CPlaybackTaskGroup::Start(
	CTrack *inTrack1,
	CTrack *inTrack2,
	int32 inLocTime,
	enum ELocateTarget inLocTarget,
	int32 inDuration,
	enum ESyncType inSyncType,
	int32 inOptionFlags )
{
	int16 prevFlags = flags;

	if (inLocTarget == LocateTarget_Continue)
	{
		if (		mainTracks[ 0 ] == inTrack1
			&&	mainTracks[ 1 ] == inTrack2
			&&	inSyncType == syncType
			&&	!tasks.Empty() )
		{
			flags &= ~(Clock_Stopped | Clock_Paused);
			return;
		}
		
		inLocTarget = locateType;		// REM: Correct?

		if (tasks.Empty()) inLocTime = 0;
	}
	
	LOCK_PLAYER;

		// Set the "locating" flags to effectively stop the clock for this track
	flags |= Clock_Locating;
	flags &= ~(Clock_Stopped | Clock_Paused);
	pbOptions = inOptionFlags;
	
	real.end = metered.end = -1;
	real.expansion = metered.expansion = 0;

		// Set one of the two clocks
	if (inLocTarget == LocateTarget_Real)
	{
			// Set real time and convert to metered
		real.time = real.start = inLocTime;
		metered.start = doc->TempoMap().ConvertRealToMetered( real.start );
		
		if (inDuration >= 0)
		{
			real.end = real.time + inDuration;
			metered.end = doc->TempoMap().ConvertRealToMetered( real.end );
		}
	}
	else
	{
			// Set metered time and convert to real
		metered.time = metered.start = inLocTime;
		real.start = doc->TempoMap().ConvertMeteredToReal( metered.start );

		if (inDuration >= 0)
		{
			metered.end = metered.time + inDuration;
			real.end = doc->TempoMap().ConvertMeteredToReal( metered.end );
		}
	}

	origin = thePlayer.m_internalTimerTick - real.start; // +++++ REMOVE THIS DEPENDANCY +++++

		// If "paused" flag, then set sequence to paused
	if (inOptionFlags & PB_Paused) flags |= Clock_Paused;

		// See if we really need to relocate from the beginning
		// (if the track or time changed)
	if (		inSyncType != syncType
		||	inTrack1 != mainTracks[ 0 ]
		||	inTrack2 != mainTracks[ 1 ])
	{
		flags |= (Locator_Find|Locator_Reset);
	}

		// REM: This bit about always relocating if the clock was stopped, is really
		// just a temporary kludge. What we really want to do is do a complete relocate
		// whenever there's been a "significant" edit.
	if (inLocTime < 500 || (prevFlags & Clock_Stopped))
	{
			// If we're relocating to the first half-second, then there's no reason
			// not to relocate.
		flags |= (Locator_Find|Locator_Reset);
	}
	else if (inLocTarget == LocateTarget_Real)
	{
		int32		timeDiff = real.seekTime - real.time;

			// If we're .5 seconds ahead of the clock, then relocate to that point;
			// otherwise, if the clock actually went backwards then do a total
			// relocate.
		if (timeDiff < -100)			flags |= Locator_Find;
		else if (timeDiff > 0)		flags |= (Locator_Find|Locator_Reset);
	}
	else
	{
		int32		timeDiff = metered.seekTime - metered.time;

			// If we're 1 qnote ahead of the clock, then relocate to that point;
			// otherwise, if the clock actually went backwards then do a total
			// relocate.
		if (timeDiff < -100)			flags |= Locator_Find;
		else if (timeDiff > 0)		flags |= (Locator_Find|Locator_Reset);
	}

	mainTracks[ 0 ]	= inTrack1;
	mainTracks[ 1 ]	= inTrack2;
	syncType			= inSyncType;
	locateType		= inLocTarget;

		// If external sync, then disable playback until 1st sync pulse comes in
	if ((flags & Locator_Find) && inSyncType > SyncType_SongInternal)
		flags |= Clock_AwaitingSync;

		// Set the default tempo
	tempo.SetInitialTempo( RateToPeriod( doc ? doc->InitialTempo() : 100.0 ) ); 

	if (flags & (Locator_Reset | Locator_Find))
	{
		if (locatorThread >= B_NO_ERROR)
		{
			status_t		result;

			wait_for_thread( locatorThread, &result );
		}
		locatorThread = spawn_thread(_locatorTaskFunc, "LocatorTask",
									 B_LOW_PRIORITY, this);
		if (locatorThread >= 0)
			resume_thread(locatorThread);
	}
}

void
CPlaybackTaskGroup::Pause(
	bool pause)
{
	if (pause)
		flags |= Clock_Paused;
	else
		flags &= ~Clock_Paused;
}

void
CPlaybackTaskGroup::Stop()
{
	flags |= Clock_Stopped;
	FlushNotes();
}

// ---------------------------------------------------------------------------
// Internal Operations

void
CPlaybackTaskGroup::_changeTempo(
	long newRate,
	long start,
	long duration,
	long clockType)
{
	D_INTERNAL(("CPlaybackTaskGroup::_changeTempo(%ld, %ld, %ld)\n",
				newRate, start, duration));

	tempo.SetTempo(tempo, newRate, start, duration, (TClockType)clockType);
}

void
CPlaybackTaskGroup::_executeEvent(
	Event &ev,
	TimeState &tState)
{
	D_INTERNAL(("CPlaybackTaskGroup::_executeEvent(%s)\n",
				ev.NameText()));

	uint8 cmd = ev.Command();
	if (ev.HasProperty( Event::Prop_MIDI ))
	{
		if ((cmd == EvtType_Note) && (flags & Clock_Locating))
			// no notes while locating
			return;

		thePlayer.SendEvent(ev, ev.stack.actualPort, ev.stack.actualChannel,
							system_time());
	}
	else
	{
		CPlayer::ChannelState *chState;
		switch (cmd)
		{
			case EvtType_TaskMarker:
			{
				if (ev.task.taskPtr->flags & CPlaybackTask::Task_Finished)
				{
					delete ev.task.taskPtr;
					if (tasks.Empty())
					{
						BMessage message(Player_ChangeTransportState);
						be_app->PostMessage(&message);
					}
				}
				else
				{
					ev.task.taskPtr->Play();
				}

				break;
			}		
			case EvtType_StartInterpolate:
			{	
				chState = &thePlayer.m_portInfo[0].channelStates[ev.stack.actualChannel];
				
				switch (ev.startInterpolate.interpolationType)
				{
					case Interpolation_PitchBend:
					{
						// You know, one thing we COULD do is to search through the playback
						// stack looking for interpolations to kill....do that at the time of the
						// push I think.
		
						chState->pitchBendTarget = ev.startInterpolate.targetValue;
		
						ev.pitchBend.command = EvtType_PitchBend;
						ev.pitchBend.targetBend = ev.startInterpolate.startValue;
						thePlayer.SendEvent( ev, ev.stack.actualPort, ev.stack.actualChannel, system_time());
					}
				}
				break;
			}
			case EvtType_Interpolate:
			{
				chState = &thePlayer.m_portInfo[0].channelStates[ev.stack.actualChannel];
				
				switch (ev.startInterpolate.interpolationType)
				{
					case Interpolation_PitchBend:
					{
						int32		elapsed;
						int32		delta;
						int32		newValue;
		
						// I was originally supposed to have executed at ev.Start() + timeStep,
						// but I may be a bit later than that -- take the difference into account.
						// Here's how much time has elapsed since I was dispatched...
						elapsed = tState.time - ev.Start() + ev.interpolate.timeStep;
		
						// If we went past the end, then clip the time.
						if ((unsigned long)elapsed > ev.interpolate.duration)
							elapsed = ev.interpolate.duration;
		
						// Here's the amount by which the value would have changed in that time.
						delta = ((int32)chState->pitchBendTarget - (int32)chState->pitchBendState)
							* elapsed / (int32)ev.interpolate.duration;
							
						// If pitch bend has never been set, then center it.
						if (chState->pitchBendState > 0x3fff)
							chState->pitchBendState = 0x2000;
		
						// Calculate the current state
						newValue = (int32)chState->pitchBendState + (int32)delta;
							
						// Now, we're going to modify the interpolation event so that it
						// is shorter, and later... and we're going to push that back onto
						// the execution stack so that it gets sent back to us later.
						ev.interpolate.start += elapsed;
						ev.interpolate.duration -= elapsed;
		
						// If there's more left to do in this interpolation, then push the event
						// on the stack for the next iteration
						if ((unsigned long)elapsed < ev.interpolate.duration)
							tState.stack.Push(ev);
						else
							newValue = chState->pitchBendTarget;
		
						// Now, we need to construct a pitch bend event...
						if (newValue != chState->pitchBendState)
						{
							ev.pitchBend.command = EvtType_PitchBend;
							ev.pitchBend.targetBend = newValue;
							thePlayer.SendEvent( ev, ev.stack.actualPort, ev.stack.actualChannel, system_time());
						}
					}
					break;
				}
			}
		}
	}
}

void
CPlaybackTaskGroup::_flushNotes(
	CEventStack &stack)
{
	CEventStackIterator iter(stack);
	for (;;)
	{
		Event *ev = iter.Current();
		if (ev == NULL)
			break;
		if (ev->Command() == EvtType_NoteOff)
			_executeEvent(*ev, real);
		iter.Next();
	}
}

void
CPlaybackTaskGroup::_flushTasks()
{
	// Delete all tasks belonging to this context.
	CPlaybackTask *th;
	while ((th = (CPlaybackTask *)tasks.First()) != NULL)
		delete th;
}

void
CPlaybackTaskGroup::_killChildTasks(
	CPlaybackTask *parent)
{
	// Delete all tasks belonging to this context.
	CPlaybackTask *th;
	for (th = (CPlaybackTask *)tasks.First();
		 th != NULL;
		 th = (CPlaybackTask *)th->Next())
	{
		if (th->parent == parent)
		{
			_killChildTasks(th);
			delete th;
		}
	}
}

void
CPlaybackTaskGroup::_locate()
{
	D_INTERNAL(("CPlaybackTaskGroup::_locate()\n"));

	CPlaybackTask *th[2];
	th[0] = th[1] = NULL;

	// If the locator has to seek around in the sequence, then
	// kill all of the stuff that's playing now and re-launch
	// the master track.
	if (flags & (Locator_Reset | Locator_Find))
	{
		LOCK_PLAYER;

		// Stop all notes for this song.
		FlushEvents();

		if (flags & Locator_Reset)
		{
			// Stop all playback tasks for this song.
			_flushTasks();

			real.seekTime = metered.seekTime = 0;

			// Launch each of the two main tracks
			for (int i = 0; i < 2; i++)
			{
				CEventTrack	*tr = (CEventTrack *)mainTracks[i];
				if (tr == NULL)
					continue;
				
				// REM: This use of "track duration" is incorrect if
				// both the master sequences are playing.
				CReadLock lock(tr);
				if (!tr->Events().IsEmpty())
				{
					// Start the new tasks at time 0 with no parent task.
					if (tr->ClockType() == ClockType_Real)
					{
						int32 endTime = pbOptions & PB_Loop ? LONG_MAX
															: real.end;
						th[0] = new CRealClockEventTask(*this,
														(CEventTrack *)tr,
														NULL, 0, endTime);
					}
					else
					{
						int32 endTime = pbOptions & PB_Loop ? LONG_MAX
															: metered.end;
						th[ 1 ] = new CMeteredClockEventTask(*this,
															 (CEventTrack *)tr,
															 NULL, 0,
															 endTime);
					}
				}
			}
		}

		flags &= ~(Locator_Reset | Locator_Find);
	}

	if (locateType == LocateTarget_Real)
	{
		// While seek time has not caught up to actual time,
		// locate through some number of events, except
		// if we're near the start of the song there's no need
		// to do any locating.
		while ((real.seekTime < real.time) && (real.time > 100))
		{
			// Check once in a while to see if we have spent too much
			// time locating and didn't give other tasks a chance to run.
			// Also, check to see if the locate should be abandoned.
			if (flags & (Locator_Reset | Locator_Find))
				return;

			LOCK_PLAYER;

			// Pop events off of the stack which are ready to go
			_locateNextChunk(real);
			metered.seekTime = tempo.ConvertRealToMetered(real.seekTime);
			_locateNextChunk(metered);

			if (!real.stack.NextTime(&real.seekTime))
				real.seekTime = real.time;
		}
	}
	else
	{
		// While seek time has not caught up to actual time,
		// locate through some number of events, except
		// if we're near the start of the song there's no need
		// to do any locating.
		if (metered.time > 10)
		{
			while (metered.seekTime < metered.time)
			{
				// Check once in a while to see if we have spent too much
				// time locating and didn't give other tasks a chance to run.
				// Also, check to see if the locate should be abandoned.
				if (flags & (Locator_Reset | Locator_Find))
					return;

				// Lock the player for another batch of events we are seeking.
				LOCK_PLAYER;
	
				_locateNextChunk(metered);
				real.seekTime = tempo.ConvertMeteredToReal(metered.seekTime);
				_locateNextChunk(real);
	
				int32 target = metered.time;
				if (pbOptions & PB_Folded)
					target = metered.time + metered.expansion;
				else
					target = metered.time;

				int32 nextTime;
				if (metered.stack.NextTime(&nextTime))
					metered.seekTime = MIN(nextTime + cTrackAdvance_Metered,
										   target);
				else
					metered.seekTime = target;
			}
		}

		// REM: I'm not sure this is right, but it works for now...
		real.time = tempo.ConvertMeteredToReal(metered.seekTime);
	}

	// Push back task origin so that lTime is correct.
	// REM: Is this correct for synced sequences???
	// +++++ REMOVE THIS DEPENDANCY +++++
	origin = thePlayer.m_internalTimerTick - real.time;
	flags &= ~Clock_Locating;

	// Poke the main player task to make sure it handles the first
	// event promptly. It doesn't have to actually do anything with
	// this command.
	write_port(thePlayer.Port(), Command_Attention, "", 0);
}

void
CPlaybackTaskGroup::_locateNextChunk(
	TimeState &tState)
{
	D_INTERNAL(("CPlaybackTaskGroup::_locateNextChunk()\n"));

	// Pop events off of the stack which are ready to go. Note that even
	// though the player will push events onto the stack in advance of when
	// they are needed, this code will not pop them until they are ready.
	// this means that when we start the piece, there may be stuff already
	// waiting on the stack. That's OK.
	Event ev;
	int32 count = 0;
	while (tState.stack.Pop(ev, tState.seekTime) && (count < LOCATE_MAX))
	{
		_executeEvent(ev, tState);
		count++;
	}
}

int32
CPlaybackTaskGroup::_locatorTaskFunc(
	void *data)
{
	((CPlaybackTaskGroup *)data)->_locate();
	return 0;
}

void
CPlaybackTaskGroup::_restart()
{
		// Cannot restart if not internally sync'd
	if (syncType > SyncType_SongInternal) return;

	LOCK_PLAYER;

		// Set the "locating" flags to effectively stop the clock for this track
	flags |= Clock_Locating |  Locator_Find | Locator_Reset;

	real.time		= real.start;
	metered.time	= metered.start;

	origin += real.end - real.start + real.expansion;
	real.expansion = metered.expansion = 0;

	if (locatorThread >= 0)
	{
		status_t result;
		wait_for_thread(locatorThread, &result);
	}

	locatorThread = spawn_thread(_locatorTaskFunc, "LocatorTask",
								 B_LOW_PRIORITY, this);
	if (locatorThread >= 0)
		resume_thread(locatorThread);
}

void
CPlaybackTaskGroup::_update(
	long internalTicks)
{
	Event ev;

	switch (syncType)
	{
		case SyncType_FreeRunning:
		case SyncType_SongInternal:
		{
			if (flags & Clock_Halted)
			{	
				// Push back origin so that real.time stays the same
				origin = internalTicks - real.time;
				return;
			}
			else
			{
				// Compute new values for real and metered time.
				real.time = internalTicks - origin;
				metered.time = tempo.ConvertRealToMetered(real.time);
			}
			break;
		}
		case SyncType_SongMTC:
		{
			// I think this is where the external sync code would go...
			break;
		}
		case SyncType_SongMidiClocks:
		{
			// I think this is where the external sync code would go...
			break;
		}
	}

	// If we are not in the middle of locating, then go ahead and
	// play some events.
	if (ClockRunning())
	{
		// Record what time our track is positioned to.
		real.seekTime = real.time;
		metered.seekTime = metered.time;

		// Pop events off of the stack which are ready to go
		while (real.stack.Pop(ev, real.seekTime))
			_executeEvent(ev, real);
		while (metered.stack.Pop(ev, metered.seekTime))
			_executeEvent(ev, metered);

		long next;
		bool done = true;

		// Compute next event time as min of all track times
		if (real.stack.NextTime(&next))
		{
			done = false;
			if (IsTimeGreater(next, nextEventTime))
				nextEventTime = next;
		}

		// Same computation for metered stack, but with a conversion
		// to real time
		if (metered.stack.NextTime(&next))
		{
			done = false;
			next = tempo.ConvertMeteredToReal(next);
			if (IsTimeGreater(next, nextEventTime))
				nextEventTime = next;
		}
		
		// Looping options for entire task!
		if (pbOptions & PB_Loop)
		{
			if (done)
			{
				if (real.end < 0)
					real.end = real.seekTime;
				_restart();
				nextEventTime = real.time;
			}
		}
		else
		{
			// If both stacks are empty, then stop
			if (done && !(flags & Clock_Continuous))
			{
				flags |= Clock_Stopped;

				// Notify the UI that we've changed state.
				BMessage message(Player_ChangeTransportState);
				be_app->PostMessage(&message);
			}
		}
	}
}

// END -- PlaybackTaskGroup.cpp
