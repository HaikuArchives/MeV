/* ===================================================================== *
 * EventThread.cpp (MeV/Engine)
 * ===================================================================== */
 
#include "EventThread.h"
#include "PlaybackThreadTeam.h"
#include "Player.h"
#include "MidiDeviceInfo.h"
#include "MeVApp.h"

// ---------------------------------------------------------------------------
// CEventThread constructor

CEventThread::CEventThread(
	CPlaybackThreadTeam &team,
	CEventTrack		*tr,
	TState			&inTimeBase,
	CPlaybackThread	*par,
	int32			start,
	int32			end )
		: CPlaybackThread(team, tr, par, start),
		  timeBase(inTimeBase),
		  playPos(tr->Events())
{
	transposition		= 0;
	clockType			= ClockType_Real;
	repeatStack		= NULL;
	trackEndTime		= tr->LogicalLength();
	threadDuration		= end >= 0 ? end : LONG_MAX;
	nextRepeatTime	= trackEndTime;
	interruptable		= true;

	playPos.First();
//	implicitRepeatPos = playPos;
//	implicitRepeatStart = 0;
}

// ---------------------------------------------------------------------------
// copy constructor

CEventThread::CEventThread( CPlaybackThreadTeam &team, CEventThread &th )
		: CPlaybackThread(team, th),
		  timeBase(th.timeBase),
		  playPos(th.playPos)
{
	transposition		= th.transposition;
	clockType			= th.clockType;
	nextRepeatTime	= th.nextRepeatTime;
	trackEndTime		= th.trackEndTime;
	threadDuration		= th.threadDuration;
	interruptable		= th.interruptable;

//	implicitRepeatPos	= th.implicitRepeatPos;
//	implicitRepeatStart	= th.implicitRepeatStart;

// RepeatState			repeatStack[ maxRepeatNest ];
}

/* ---------------------------------------------------------------------------
	This instructs the track that the implicit loop feature is going to be used.
	If the "inAtStart" is true, it means the loop point is at the start of the
	sequence; If it is false, then it means that the loop point is at the
	current point to which the sequence is located.
*/

#if 0
void CEventThread::RecalcImplicitLoop( bool inAtStart, int32 inImplicitRepeatEnd )
{
	if (inAtStart)
	{
		implicitRepeatPos.First();
		implicitRepeatStart = 0;
	}
	else
	{
		implicitRepeatPos = playPos;
		implicitRepeatStart = timeBase.seekTime - originTime;
	}
	trackEndTime = inImplicitRepeatEnd;
	flags |= Thread_ImplicitLoop;
}
#endif

// ---------------------------------------------------------------------------
// destructor

CEventThread::~CEventThread()
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
// playback routine for a single event

void CEventThread::PlayEvent( const Event &ev, CEventStack &stack, long origin )
{
		// filter event though virtual channel table
	VChannelEntry			*vc = team.vChannelTable + ev.note.vChannel;
	CMIDIPlayer::ChannelState	*chState = &thePlayer.portInfo[ vc->port ].channelStates[ vc->channel ];
	int32			duration;
	Event			stackedEvent( ev );
	
		// Process a copy of the event event through the filters...
	((CEventTrack *)track)->FilterEvent( stackedEvent );

		// Make a copy of the duration field before we kick it to death (below).
	duration = stackedEvent.common.duration;

		// Modify the stack
	stackedEvent.stack.start			+= origin;
	stackedEvent.stack.actualPort		= vc->port;
	stackedEvent.stack.actualChannel	= vc->channel;
	stackedEvent.stack.thread			= threadID;

		// REM: Do we also want to filter on the VChannel? I think so...
		
	switch (ev.Command()) {

		// MIDI channel events

	case EvtType_Note:							// note-on event

			// Ignore the note event if locating
		if (team.flags & CPlaybackThreadTeam::Clock_Locating) break;
		if (vc->flags & (VChannelEntry::mute | VChannelEntry::muteFromSolo)) break;
		
			// Apply thread-specific transposition.
		if (transposition != 0 && vc->flags & VChannelEntry::transposable)
		{
			stackedEvent.note.pitch += transposition;

				// If pitch went out of bounds, then don't play the note.
			if (stackedEvent.note.pitch & 0x80) break;
		}
		
			// REM: Here we would apply velocity contour.
			// REM: Here we would do the VU meter code...
			
			// If there was room on the stack to push the note-off, then
			// play the note-on
		stackedEvent.stack.start		+= duration;
		stackedEvent.stack.command	= EvtType_NoteOff;

		if (stack.Push( stackedEvent ))
		{
			stackedEvent.stack.start		-= duration;
			stackedEvent.stack.command	= EvtType_Note;
			stack.Push( stackedEvent );
		}
		break;

	case EvtType_PitchBend:						// pitch bend
	
			// Play nothing if muted
		if (vc->flags & (VChannelEntry::mute | VChannelEntry::muteFromSolo)) break;
		
			// If locating, update channel state table but don't stack the event
		if (team.flags & CPlaybackThreadTeam::Clock_Locating)
		{
			chState->pitchBendState = ev.pitchBend.targetBend;
			break;
		}

		if (duration > 0 && ev.pitchBend.updatePeriod > 0)
		{
				// Push a "start interpolating" event
			stackedEvent.startInterpolate.command = EvtType_StartInterpolate;
			stackedEvent.startInterpolate.interpolationType = Interpolation_PitchBend;
			stackedEvent.startInterpolate.startValue = ev.pitchBend.startBend;
			stackedEvent.startInterpolate.targetValue = ev.pitchBend.targetBend;
			stackedEvent.SetStart( stackedEvent.stack.start );
			stack.Push( stackedEvent );

				// Push an "interpolation" event
			stackedEvent.interpolate.command = EvtType_Interpolate;
			stackedEvent.interpolate.interpolationType = Interpolation_PitchBend;
			stackedEvent.interpolate.duration = duration;
			stackedEvent.interpolate.timeStep = ev.pitchBend.updatePeriod;
			stackedEvent.SetStart( stackedEvent.stack.start + ev.pitchBend.updatePeriod );
			stack.Push( stackedEvent );
		}
		else
		{
				// Just push an ordinary pitch bend event...
			stack.Push( stackedEvent );
		}
		break;

	case EvtType_ProgramChange:					// program change

			// Play nothing if muted
		if (vc->flags & (VChannelEntry::mute | VChannelEntry::muteFromSolo)) break;

			// If locating, update channel state table but don't stack the event
		if (team.flags & CPlaybackThreadTeam::Clock_Locating)
		{
			MIDIDeviceInfo	*mdi = ((CMeVApp *)be_app)->LookupInstrument( vc->port, vc->channel );

				// (Only update the channel bank state if this device supports banks)
			if (	mdi != NULL
				&& mdi->SupportsProgramBanks()
				&& ev.programChange.bankMSB < 128
				&& ev.programChange.bankLSB < 128)
			{
					// Update the channel state
				chState->ctlStates[ 0 ] = ev.programChange.bankMSB;
				chState->ctlStates[ 32 ] = ev.programChange.bankLSB;
			}

			chState->program = ev.programChange.program;
			break;
		}
		
		stack.Push( stackedEvent );
		break;

	case EvtType_ChannelATouch:					// channel aftertouch

			// Play nothing if muted
		if (vc->flags & (VChannelEntry::mute | VChannelEntry::muteFromSolo)) break;

			// If locating, update channel state table but don't stack the event
		if (team.flags & CPlaybackThreadTeam::Clock_Locating)
		{
			chState->channelAfterTouch = ev.aTouch.value;
			break;
		}

		stack.Push( stackedEvent );
		break;

	case EvtType_Controller:						// controller change

			// Play nothing if muted
		if (vc->flags & (VChannelEntry::mute | VChannelEntry::muteFromSolo)) break;
		
			// REM: Data entry controls should probably be passed through, since they
			// can't be summarized in a simple way.
			// (Actually, the ideal would be to aggregate data entry -- but that's a job for
			// another time and another event type.)

			// If locating, update channel state table but don't stack the event
		if (team.flags & CPlaybackThreadTeam::Clock_Locating)
		{
			uint8			lsbIndex;

				// Check if it's a 16-bit controller
			lsbIndex = controllerInfoTable[ ev.controlChange.controller ].LSBNumber;
	
				// Otherwise, it's an 8-bit controller
			if (lsbIndex == ev.controlChange.controller || lsbIndex > 127)
			{
				chState->ctlStates[ ev.controlChange.controller ] = ev.controlChange.MSB;
			}
			else
			{
				if (	ev.controlChange.MSB < 128)
				{
					chState->ctlStates[ ev.controlChange.controller ] = ev.controlChange.MSB;
					chState->ctlStates[ lsbIndex ] = 0;
				}
	
				if (	ev.controlChange.LSB < 128)
				{
					chState->ctlStates[ lsbIndex] = ev.controlChange.LSB;
				}
			}
			break;
		}

		stack.Push( stackedEvent );
		break;

	case EvtType_PolyATouch:						// polyphonic aftertouch

			// Ignore the event if locating
		if (team.flags & CPlaybackThreadTeam::Clock_Locating) break;
		if (vc->flags & (VChannelEntry::mute | VChannelEntry::muteFromSolo)) break;

		stack.Push( stackedEvent );
		break;

	case EvtType_SysEx:							// system exclusive

		stack.Push( stackedEvent );
		break;

		// Global control events

// case evtTypeStop:							// stop the sequencer
// case evtTypeGo:								// start the sequencer
// case evtTypeLocate:							// locate to "duration" time
// case evtTypeCue:							// trigger a cue point
// case evtTypeMTCCue:							// trigger an MTC cue point
// case evtTypeMuteVChannel:					// mute a vChannel
// case evtTypeMuteTrack:						// mute a track
// case evtTypeSpliceIn:						// a "splice" event for overdub
// case evtTypeSpliceOut:						// a "splice" event for overdub
// 	break;

			// Track control events
	case EvtType_Repeat:							// repeat a section

		BeginRepeat( ev.Start(), ev.Duration(), ev.repeat.repeatCount );
		break;

	case EvtType_Sequence:							// play another track

		CTrack			*tr = team.doc->FindTrack( ev.sequence.sequence );
		CPlaybackThread	*p;

		if (tr == NULL) break;
#if USE_SHARED_LOCKS
		tr->Lock( Lock_Shared );
#else
		tr->Lock();
#endif
		for (p = this; p != NULL; p = p->Parent())
		{
				// Don't allow tracks to call each other recursively.
			if (p->Track() == tr)
			{
#if USE_SHARED_LOCKS
				tr->Unlock( Lock_Shared );
#else
				tr->Unlock();
#endif
				return;
			}
		}
		
		CEventTrack	*tk = dynamic_cast<CEventTrack *>(tr);
		
			// If it's an event track and not empty
		if (tk != NULL && tk->Events().IsEmpty() == false)
		{
			CEventThread		*th;
			int32			start = stackedEvent.stack.start,
							stop = start + duration;

				// Launch based on clock type.
			if (tr->ClockType() == ClockType_Real)
			{
				if (track->ClockType() == ClockType_Metered)
				{
					start = team.tempo.ConvertMeteredToReal( start );
					stop = team.doc->TempoMap().ConvertMeteredToReal( stop );
					duration = stop - start;
				}
				
					// This code is wrong...need flags, need auto-repeats
//				if (ev.sequence.flags & Event::Seq_Interruptable)
//				{
//					if (duration > tk->LogicalLength())
//						duration = tk->LogicalLength();
//				}
//				else
//				{
//					int32	reps = (duration + tk->LogicalLength() - 1) / + tk->LogicalLength();
//					duration = tk->LogicalLength() * reps;
//				}
				
				th = new CRealClockEventThread(
					team, tk, this, start, duration );
					
				th->interruptable = ((ev.sequence.flags & Event::Seq_Interruptable) ? true : false);
			}
			else
			{
				if (track->ClockType() == ClockType_Real)
				{
					start = team.tempo.ConvertRealToMetered( start );
					stop = team.doc->TempoMap().ConvertRealToMetered( stop );
					duration = stop - start;
				}
			
					// This code is wrong...need flags, need auto-repeats
//				if (ev.sequence.flags & Event::Seq_Interruptable)
//				{
//					if (duration > tk->LogicalLength())
//						duration = tk->LogicalLength();
//				}
//				else
//				{
//					int32	reps = (duration + tk->LogicalLength() - 1) / + tk->LogicalLength();
//					duration = tk->LogicalLength() * reps;
//				}
				
				th = new CMeteredClockEventThread(
					team, tk, this, start, duration );

				th->interruptable = ((ev.sequence.flags & Event::Seq_Interruptable) ? true : false);
			}
			th->transposition = ev.sequence.transposition;
		}
#if USE_SHARED_LOCKS
		tr->Unlock( Lock_Shared );
#else
		tr->Unlock();
#endif
		break;
// case evtTypeBranch:							// conditional branch to track
// case evtTypeErase:							// erase notes on channel
// case evtTypePunch:							// punch in over track
// 	break;

		// Clock control events

// 	case evtTypeTempo:							// change tempo event
// 	case evtTypeTimeSig:						// change time signature event
// 		break;

// 	case evtTypeVelocityContour:				// velocity contour event
// 	case evtTypeTranspose:						// transposition for vChannel
		break;
	}
}

// ---------------------------------------------------------------------------
// Force a repeat event at the current point in the sequence.

void CEventThread::BeginRepeat( int32 inRepeatStart, int32 inRepeatDuration, int32 inRepeatCount )
{
	if (		inRepeatCount == 1
		||	inRepeatDuration <= 0) return;
	
		// Ignore overlapped repeat events
	if (		repeatStack == NULL
		||	repeatStack->endTime >= inRepeatStart + inRepeatDuration)
	{
		RepeatState		*rps;

			// Make a new repeat state, starting at the next event
		rps = new RepeatState( playPos );
		rps->pos.Seek( 1 );
		rps->endTime = inRepeatStart + inRepeatDuration;
		rps->timeOffset = inRepeatDuration;
		rps->repeatCount = inRepeatCount;
		rps->next = repeatStack;

			// If this is a master track, then adjust something or other...
		if (		track == team.mainTracks[ 0 ]
			||	track == team.mainTracks[ 1 ])
		{
			if (inRepeatCount > 0)
				timeBase.expansion += (inRepeatCount - 1) * rps->timeOffset;
		}

			// Set the time of the next repeat concern
		repeatStack = rps;
		nextRepeatTime = MIN( trackEndTime, rps->endTime );
	}
}

// ---------------------------------------------------------------------------
// Code to handle repeats and other scheduled discontinuities

bool CEventThread::Repeat()
{
	bool			result = false;

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
		const Event	*s;
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
				PlayEvent( *s, timeBase.stack, originTime );
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
		&&	currentTime < threadDuration
		&& 	(flags & Thread_ImplicitLoop))
	{
			// Delete all pending repeats
		while (repeatStack != NULL)
		{
			RepeatState	*rps = repeatStack->next;
			delete repeatStack;
			repeatStack = rps;
		}
		
		if (threadDuration != LONG_MAX) threadDuration -= track->LogicalLength();

			// Set play position to start of track, and adjust origin and current time.
//		playPos = implicitRepeatPos;
		playPos.First();
//		originTime += track->LogicalLength() - implicitRepeatStart;
		originTime += track->LogicalLength();
		currentTime = timeBase.seekTime - originTime;
		
		//	REM: If this is the master track[ 0 ], then also loop the absolute track at the
		//	same time...
		result = true;
	}

	if (interruptable && trackEndTime > threadDuration) trackEndTime = threadDuration;

	return result;
}

// ---------------------------------------------------------------------------
// playback routine

void CEventThread::Play()
{
	int32		targetTime;
	bool			locating = (team.flags & CPlaybackThreadTeam::Clock_Locating) != false;

		// If we're locating, then we want to lock for certain
	if (locating)
	{
		targetTime = timeBase.seekTime;
#if USE_SHARED_LOCKS
		track->Lock( Lock_Shared );
#else
		track->Lock();
#endif
	}
	else
	{
#if USE_SHARED_LOCKS
		if (track->Lock( Lock_Shared, 5000 ) == false)
#else
		if (track->Lock( 5000 ) == false)
#endif
		{
				// Attempt to lock the track; if we fail, then just continue
				// and play something else.
			ReQueue( timeBase.stack, timeBase.seekTime + 10 );
			return;
		}
		targetTime = timeBase.seekTime + eventAdvance;
	}

	currentTime = targetTime - originTime;

		// If there's a repeat scheduled before the playback time.
	while (nextRepeatTime <= currentTime)
	{
			// Play all of the events before the repeat.
		for (const Event *ev = (const Event *)playPos; ev != NULL; )
		{
			if (ev->Start() >= nextRepeatTime) break;

			PlayEvent( *ev, timeBase.stack, originTime );
			ev = playPos.Seek( 1 );
		}
		
			// Process the repeat. If there was no repeat to process, then...
		if (Repeat() == false)
		{
				// Check to see if we've run out of track
			if (currentTime >= trackEndTime || (flags & Thread_Finished))
			{
					// If we've reached the end, then exit this routine...
				flags |= Thread_Finished;
				ReQueue( timeBase.stack, trackEndTime + originTime - 1 );
			}
			else
			{
				ReQueue( timeBase.stack, nextRepeatTime + originTime - 1 );
			}
#if USE_SHARED_LOCKS
			track->Unlock( Lock_Shared );
#else
			track->Unlock();
#endif
			return;
		}

		targetTime = timeBase.seekTime;
	}

	for (const Event *ev = (const Event *)playPos; ev != NULL; )
	{
/*		if (ev->Start() >= nextRepeatTime)
		{
			if (Repeat( timeBase )) continue;

			if (ev->Start() >= trackEndTime)
			{
				flags |= Thread_Finished;
				ReQueue( timeBase.stack, trackEndTime + originTime - 1 );
			}
			else ReQueue( timeBase.stack, nextRepeatTime + originTime );
			track->Unlock();
			return;
		} */

		long		t = ev->Start() + originTime;

		if (IsTimeGreater( targetTime, t ))
		{
			ReQueue( timeBase.stack, locating ? t : t - trackAdvance );
#if USE_SHARED_LOCKS
			track->Unlock( Lock_Shared );
#else
			track->Unlock();
#endif
			return;
		}

		PlayEvent( *ev, timeBase.stack, originTime );
		ev = playPos.Seek( 1 );
	}

	if (repeatStack != NULL || currentTime < threadDuration)
	{
		ReQueue( timeBase.stack, nextRepeatTime + originTime );
#if USE_SHARED_LOCKS
		track->Unlock( Lock_Shared );
#else
		track->Unlock();
#endif
		return;
	}
	
		// REM: Is this incorrect for the master track?
#if USE_SHARED_LOCKS
	track->Unlock( Lock_Shared );
#else
	track->Unlock();
#endif
	flags |= Thread_Finished;
	ReQueue( timeBase.stack, trackEndTime + originTime );
}

// ---------------------------------------------------------------------------
// Returns the current time in the thread

int32 CEventThread::CurrentTime()
{
	return timeBase.seekTime - originTime;
}

// ---------------------------------------------------------------------------
// CRealTimeEventThread constructor

CRealClockEventThread::CRealClockEventThread(
	CPlaybackThreadTeam &team,
	CEventTrack		*tr,
	CPlaybackThread	*par,
	int32			start,
	int32			end )
		: CEventThread( team, tr, team.real, par, start, end )
{
	trackAdvance = cTrackAdvance_Real;
	eventAdvance = cEventAdvance_Real;
	clockType = ClockType_Real;
	ReQueue( timeBase.stack, start );
}

// ---------------------------------------------------------------------------
// playback routine

#if 0
void CRealClockEventThread::Play()
{
	int32		targetTime;
	bool			locating = (team.flags & CPlaybackThreadTeam::Clock_Locating) != false;

		// If we're locating, then we want to lock for certain
	if (locating)
	{
		targetTime = timeBase.seekTime;
#if USE_SHARED_LOCKS
		track->Lock( Lock_Shared );
#else
		track->Lock();
#endif
	}
	else
	{
#if USE_SHARED_LOCKS
		if (track->Lock( Lock_Shared, 5000 ) == false)
#else
		if (track->Lock( 5000 ) == false)
#endif
		{
				// Attempt to lock the track; if we fail, then just continue
				// and play something else.
			ReQueue( timeBase.stack, timeBase.seekTime + 10 );
			return;
		}
		targetTime = timeBase.seekTime + cEventAdvance_Real;
	}

	currentTime = timeBase.seekTime - originTime;
	Repeat();
	
	for (const Event *ev = (const Event *)playPos; ev != NULL; )
	{
		if (ev->Start() >= nextRepeatTime)
		{
			if (Repeat()) continue;

			if (ev->Start() >= trackEndTime)
			{
				flags |= Thread_Finished;
				ReQueue( timeBase.stack, trackEndTime + originTime - 1 );
			}
			else ReQueue( timeBase.stack, nextRepeatTime + originTime );
#if USE_SHARED_LOCKS
			track->Unlock( Lock_Shared );
#else
			track->Unlock();
#endif
			return;
		}
		
		long		t = ev->Start() + originTime;

		if (IsTimeGreater( targetTime, t ))
		{
			ReQueue( timeBase.stack, locating ? t : t - cTrackAdvance_Real );
#if USE_SHARED_LOCKS
			track->Unlock( Lock_Shared );
#else
			track->Unlock();
#endif
			return;
		}

		PlayEvent( *ev, timeBase.stack, originTime );
		ev = playPos.Seek( 1 );
	}

	if (repeatStack != NULL || currentTime < threadDuration)
	{
		ReQueue( timeBase.stack, nextRepeatTime + originTime );
#if USE_SHARED_LOCKS
		track->Unlock( Lock_Shared );
#else
		track->Unlock();
#endif
		return;
	}
	
		// REM: Is this incorrect for the master track?
#if USE_SHARED_LOCKS
	track->Unlock( Lock_Shared );
#else
	track->Unlock();
#endif
	flags |= Thread_Finished;
	ReQueue( timeBase.stack, trackEndTime + originTime - 1 );
}
#endif

// ---------------------------------------------------------------------------
// CRealTimeEventThread constructor

CMeteredClockEventThread::CMeteredClockEventThread(
	CPlaybackThreadTeam &team,
	CEventTrack		*tr,
	CPlaybackThread	*par,
	int32			start,
	int32			end )
		: CEventThread( team, tr, team.metered, par, start, end )
{
	trackAdvance = cTrackAdvance_Metered;
	eventAdvance = cEventAdvance_Metered;
	clockType		= ClockType_Metered;
	ReQueue( timeBase.stack, start );
}

// ---------------------------------------------------------------------------
// playback routine

#if 0
void CMeteredClockEventThread::Play()
{
	int32		targetTime;
	bool			locating = (team.flags & CPlaybackThreadTeam::Clock_Locating) != false;

		// If we're locating, then we want to lock for certain
	if (locating)
	{
		targetTime = timeBase.seekTime;
#if USE_SHARED_LOCKS
		track->Lock( Lock_Shared );
#else
		track->Lock();
#endif
	}
	else
	{
#if USE_SHARED_LOCKS
		if (track->Lock( Lock_Shared, 5000 ) == false)
#else
		if (track->Lock( 5000 ) == false)
#endif
		{
				// Attempt to lock the track; if we fail, then just continue
				// and play something else.
			ReQueue( timeBase.stack, timeBase.seekTime + 10 );
			return;
		}
		targetTime = timeBase.seekTime + cEventAdvance_Metered;
	}

	currentTime = targetTime - originTime;

		// If there's a repeat scheduled before the playback time.
	while (nextRepeatTime <= currentTime)
	{
			// Play all of the events before the repeat.
		for (const Event *ev = (const Event *)playPos; ev != NULL; )
		{
			if (ev->Start() >= nextRepeatTime) break;

			PlayEvent( *ev, timeBase.stack, originTime );
			ev = playPos.Seek( 1 );
		}
		
			// Process the repeat. If there was no repeat to process, then...
		if (Repeat() == false)
		{
				// Check to see if we've run out of track
			if (currentTime >= trackEndTime || (flags & Thread_Finished))
			{
					// If we've reached the end, then exit this routine...
				flags |= Thread_Finished;
				ReQueue( timeBase.stack, trackEndTime + originTime - 1 );
			}
			else
			{
				ReQueue( timeBase.stack, nextRepeatTime + originTime - 1 );
			}
#if USE_SHARED_LOCKS
			track->Unlock( Lock_Shared );
#else
			track->Unlock();
#endif
			return;
		}

		targetTime = timeBase.seekTime;
	}

	for (const Event *ev = (const Event *)playPos; ev != NULL; )
	{
/*		if (ev->Start() >= nextRepeatTime)
		{
			if (Repeat( timeBase )) continue;

			if (ev->Start() >= trackEndTime)
			{
				flags |= Thread_Finished;
				ReQueue( timeBase.stack, trackEndTime + originTime - 1 );
			}
			else ReQueue( timeBase.stack, nextRepeatTime + originTime );
			track->Unlock();
			return;
		} */

		long		t = ev->Start() + originTime;

		if (IsTimeGreater( targetTime, t ))
		{
			ReQueue( timeBase.stack, locating ? t : t - cTrackAdvance_Metered );
#if USE_SHARED_LOCKS
			track->Unlock( Lock_Shared );
#else
			track->Unlock();
#endif
			return;
		}

		PlayEvent( *ev, timeBase.stack, originTime );
		ev = playPos.Seek( 1 );
	}

	if (repeatStack != NULL || currentTime < threadDuration)
	{
		ReQueue( timeBase.stack, nextRepeatTime + originTime );
#if USE_SHARED_LOCKS
		track->Unlock( Lock_Shared );
#else
		track->Unlock();
#endif
		return;
	}
	
		// REM: Is this incorrect for the master track?
#if USE_SHARED_LOCKS
	track->Unlock( Lock_Shared );
#else
	track->Unlock();
#endif
	flags |= Thread_Finished;
	ReQueue( timeBase.stack, trackEndTime + originTime );
}
#endif

#if 0
// ---------------------------------------------------------------------------
// CRealTimeEventThread constructor

CMasterEventThread::CMasterEventThread(
	CPlaybackThreadTeam &team,
	CEventTrack		*tr,
	CTrack			*par,
	long			start )
		: CEventThread( team, tr, par, start ),
			mPlayPos( tr->Events() )
{
	clockType		= ClockType_Real;
	ReQueue( team.real.stack, start );
	mPlayPos.First();
}

// ---------------------------------------------------------------------------
// playback routine

void CMasterEventThread::Play()
{
	currentTime = team.real.seekTime - originTime;

// reQueue( &pl->meteredTimeStack, t );

		// Attempt to lock the track; if we fail, then just continue as
		// and play something else.
	if (track->Lock( 1000 ) == false) return;
	
		// REM: Should we handle repeats? Or not?

	for (const Event *ev = (const Event *)playPos; ev != NULL; )
	{
		long		t = ev->Start() + originTime;

		if (IsTimeGreater( team.real.seekTime, t )) break;

			// Ummm, we need to convert the time to absolute before sending it to PlayEvent...
		PlayEvent( *ev, team.real.stack, originTime );
		ev = playPos.Seek( 1 );
	}
	
	for (const Event *ev = (const Event *)mPlayPos; ev != NULL; )
	{
			// REM: Is this incorrect for the master track?
			// REM: We need to calc the absolute time of the event...
		long		t = ev->Start() /* + originTime */;

		if (IsTimeGreater( team.metered.seekTime, t )) break;

			// Ummm, we need to convert the time to absolute before sending it to PlayEvent...
		PlayEvent( *ev, team.metered.stack, 0 );
		ev = mPlayPos.Seek( 1 );
	}
	
	track->Unlock();
	
	// If we broke out of the loop, it simply means that we've gone ahead...
	// Should we "re-queue" this?

		// REM: Is this incorrect for the master track?
// flags |= Thread_Finished;
}
#endif
