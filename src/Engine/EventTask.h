/* ===================================================================== *
 * EventTask.h (MeV/Engine)
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

#ifndef __C_EventTask_H__
#define __C_EventTask_H__

#include "PlaybackTask.h"
#include "PlaybackTaskGroup.h"

// ---------------------------------------------------------------------------
// CEventTask -- subclass which is used for event-based tracks

#define maxRepeatNest		4

typedef CPlaybackTaskGroup::TimeState		TState;

class CEventTask : public CPlaybackTask {

// friend class CPlaybackTaskGroup;

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
	int8					transposition;		// key transposition of task
	uint8				clockType;			// clock type for this task
	int32				trackAdvance,			// track playback buffering time
						eventAdvance;			// event playback buffering time
	int32				nextRepeatTime;		// time of next repeat
	int32				trackEndTime;			// end time of track, compressed
	int32				taskDuration;		// desired duration of task, expanded.
	bool					interruptable;			// task can end at any time;
											// false means ends only at the end.

		// Variables pertaining to Repeat events
	RepeatState			*repeatStack;
	
	bool Repeat();
	void Play();

public:
		// constructor
	CEventTask(	CPlaybackTaskGroup	&group,
					CEventTrack			*track,
					TState				&inTimeBase,
					CPlaybackTask		*parent,
					int32				start,
					int32				end );

		// copy constructor
	CEventTask(	CPlaybackTaskGroup	&group,
					CEventTask			&task );

		// destructor
	~CEventTask();

		// Perform a single event
	void PlayEvent( const Event &ev, CEventStack &stack, long time );

	void BeginRepeat( int32 inRepeatStart, int32 inRepeatDuration, int32 inRepeatCount );

	int32 CurrentTime();
};

// ---------------------------------------------------------------------------
// CRealTimeEventTask -- play events in real time

class CRealClockEventTask : public CEventTask {

public:
		// constructor
	CRealClockEventTask(	CPlaybackTaskGroup &group,
							CEventTrack		*track,
							CPlaybackTask	*parent,
							int32			start,
							int32			end );
};

// ---------------------------------------------------------------------------
// CMeteredTimeEventTask -- play events in real time

class CMeteredClockEventTask : public CEventTask {

public:
		// constructor
	CMeteredClockEventTask(		CPlaybackTaskGroup &group,
								CEventTrack		*track,
								CPlaybackTask	*parent,
								int32			start,
								int32			end );
};

#endif /* __C_EventTask_H__ */
