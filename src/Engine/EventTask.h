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
 * History:
 *	1997		Talin
 *		Original implementation
 *	04/08/2000	cell
 *		General cleanup in preparation for initial SourceForge checkin
 * ===================================================================== */

#ifndef __C_EventTask_H__
#define __C_EventTask_H__

#include "PlaybackTask.h"
#include "PlaybackTaskGroup.h"

#define maxRepeatNest		4

typedef CPlaybackTaskGroup::TimeState		TState;

/**
 *	CPlaybackTask subclass which is used for event-based tracks.
 *	@author	Talin, Christopher Lenz
 */
class CEventTask
	:	public CPlaybackTask
{

	struct RepeatState
	{
		RepeatState		*next;				// next enclosing repeat
		EventMarker		pos;					// where to jump back to
		long				endTime,				// when to jump back
						timeOffset;			// time offset for repeat
		uint32			repeatCount;			// repeat coundown

		// REM: For master tracks, we might want to record both time
		// clock states...

		RepeatState( EventMarker &em ) : pos( em ) {}
	};

public:							// Constructor/Destructor

	/** Constructor. */
								CEventTask(
									CPlaybackTaskGroup &group,
									CEventTrack *track,
									TState &inTimeBase,
									CPlaybackTask *parent,
									int32 start,
									int32 end);

	/** Copy constructor. */
								CEventTask(
									CPlaybackTaskGroup &group,
									CEventTask &task);

	/** Destructor. */
								~CEventTask();

protected:						// CPlaybackTask Implementation

	/**	Returns the current time of this track. */
	int32						CurrentTime() const;

	/** Playback routine. */
	void						Play();

private:						// Internal Operations

	/** Force a repeat event at the current point in the sequence. */
	void						_beginRepeat(
									int32 start,
									int32 duration,
									int32 count);

	/** Handle repeats and other scheduled discontinuities. */
	bool						_repeat();

	/** Perform a single event. */
	void						_stackEvent(
									const Event &ev,
									CEventStack &stack,
									long time);

protected:						// Instance Data

	TState &					timeBase;

	/** Playback position. */
	EventMarker					playPos;

	/** Key transposition of task. */
	int8						transposition;

	/** Clock type for this task. */
	uint8						clockType;

	/** Track playback buffering time. */
	int32						trackAdvance;

	/** Event playback buffering time. */
	int32						eventAdvance;

	/** Time of next repeat. */
	int32						nextRepeatTime;

	/** End time of track, compressed. */
	int32						trackEndTime;

	/** Desired duration of task, expanded. */
	int32						taskDuration;

	/** Whether task can end at any time.
	 *	false means ends only at the end.
	 */
	bool						interruptable;

	/** Variables pertaining to Repeat events. */
	RepeatState *				repeatStack;
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
