/* ===================================================================== *
 * EventThread.h (MeV/Engine)
 * ---------------------------------------------------------------------
 * License:
 *  The contents of this file are subject to the Mozilla Public
 *  License Version 1.1 (the "License"); you may not use this file
 *  except in compliance with the License. You may obtain a copy of
 *  the License at http://www.mozilla.org/MPL/
 *
 *  Software distributed under the License is distributed on an "AS
 *  IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 *  implied. See the License for the specific language governing
 *  rights and limitations under the License.
 *
 *  The Original Code is MeV (Musical Environment) code.
 *
 *  The Initial Developer of the Original Code is Sylvan Technical 
 *  Arts. Portions created by Sylvan are Copyright (C) 1997 Sylvan 
 *  Technical Arts. All Rights Reserved.
 *
 *  Contributor(s): 
 *		Christopher Lenz (cell)
 *
 * ---------------------------------------------------------------------
 * Purpose:
 *  
 * ---------------------------------------------------------------------
 * History:
 *	1997		Talin
 *		Original implementation
 *	04/08/2000	cell
 *		General cleanup in preparation for initial SourceForge checkin
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __C_EventThread_H__
#define __C_EventThread_H__

#include "PlaybackThread.h"
#include "PlaybackThreadTeam.h"

// ---------------------------------------------------------------------------
// CEventThread -- subclass which is used for event-based tracks

#define maxRepeatNest		4

typedef CPlaybackThreadTeam::TimeState		TState;

class CEventThread : public CPlaybackThread {

// friend class CPlaybackThreadTeam;

	struct RepeatState {
		RepeatState		*next;				// next enclosing repeat
		EventMarker		pos;					// where to jump back to
		long				endTime,				// when to jump back
						timeOffset;			// time offset for repeat
		uint32			repeatCount;			// repeat coundown

		// REM: For master tracks, we might want to record both time
		// clock states...

		RepeatState( EventMarker &em ) : pos( em ) {}
	};

protected:
	TState				&timeBase;
	EventMarker			playPos;				// Playback position
	int8					transposition;		// key transposition of thread
	uint8				clockType;			// clock type for this thread
	int32				trackAdvance,			// track playback buffering time
						eventAdvance;			// event playback buffering time
	int32				nextRepeatTime;		// time of next repeat
	int32				trackEndTime;			// end time of track, compressed
	int32				threadDuration;		// desired duration of thread, expanded.
	bool					interruptable;			// thread can end at any time;
											// false means ends only at the end.

		// Variables pertaining to Repeat events
	RepeatState			*repeatStack;
	
		// Variables pertaining to implicit repeat
//	EventMarker			implicitRepeatPos;
//	int32				implicitRepeatStart;

	bool Repeat();
	void Play();

public:
		// constructor
	CEventThread(	CPlaybackThreadTeam	&team,
					CEventTrack			*track,
					TState				&inTimeBase,
					CPlaybackThread		*parent,
					int32				start,
					int32				end );

		// copy constructor
	CEventThread(	CPlaybackThreadTeam	&team,
					CEventThread			&thread );

		// destructor
	~CEventThread();

		// Perform a single event
	void PlayEvent( const Event &ev, CEventStack &stack, long time );

	void BeginRepeat( int32 inRepeatStart, int32 inRepeatDuration, int32 inRepeatCount );

	int32 CurrentTime();

//	void RecalcImplicitLoop( bool inAtStart, int32 inRepeatEnd );
};

// ---------------------------------------------------------------------------
// CRealTimeEventThread -- play events in real time

class CRealClockEventThread : public CEventThread {

		// playback routine
//	void Play();

public:
		// constructor
	CRealClockEventThread(	CPlaybackThreadTeam &team,
							CEventTrack		*track,
							CPlaybackThread	*parent,
							int32			start,
							int32			end );
};

// ---------------------------------------------------------------------------
// CMeteredTimeEventThread -- play events in real time

class CMeteredClockEventThread : public CEventThread {

		// playback routine
//	void Play();

public:
		// constructor
	CMeteredClockEventThread(		CPlaybackThreadTeam &team,
								CEventTrack		*track,
								CPlaybackThread	*parent,
								int32			start,
								int32			end );
};

#if 0
// ---------------------------------------------------------------------------
// CRealTimeEventThread -- play events in real time

class CMasterEventThread : public CEventThread {

	EventMarker			mPlayPos;				// Playback position

		// playback routine
	void Play();

public:
		// constructor
	CMasterEventThread(	CPlaybackThreadTeam &team,
						CEventTrack		*track,
						CTrack			*parent,
						long				start );

	int32 CurrentTime() { return 0; }
};
#endif

#endif /* __C_EventThread_H__ */
