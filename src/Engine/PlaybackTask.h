/* ===================================================================== *
 * PlaybackTask.h (MeV/Engine)
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
 *  Defines an instance of a track being played
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

#ifndef __C_PlaybackTask_H__
#define __C_PlaybackTask_H__

#include "MeV.h"
#include "EventTrack.h"
#include "EventStack.h"

#define MAX_NORMAL_TASKS 	250
#define MAX_FEEDBACK_TASKS	251

// ---------------------------------------------------------------------------
// CPlaybackTask -- an instance of a track being played

class CPlaybackTask
	:	public DNode
{
	friend class CPlayer;
	friend class CPlaybackTaskGroup;
	friend class CPlayerControl;

	static int32			GetPlaybackMarkerTimes( CTrack *track, int32 *resultBuf, int32 bufSize );

public:							// Constants

	enum taskFlags {
								/** Turns off sound on track. */
								Task_Muted		= (1<<0),

								/** Mute all other tracks. */
								Task_Solo		= (1<<1),

								/** Use internal tick, not master time. */
								Task_Wild		= (1<<2),

								/** No more playback for this task. */
								Task_Finished	= (1<<3),

								/** Task has implicit looping. */
								Task_ImplicitLoop = (1<<4)
	};

public:							// Constructor/Destructor

	/** Standard constructor. */
								CPlaybackTask(
									CPlaybackTaskGroup &group,
									CTrack *track,
									CPlaybackTask *parent,
									long start);

	/** Copy constructor -- can copy to a different group. */
								CPlaybackTask(
									CPlaybackTaskGroup &group,
									CPlaybackTask &th);

	/** Destructor. */
	virtual						~CPlaybackTask();

public:							// Hook Functions

	/**	Returns the current time of this track. */
	virtual int32				CurrentTime() const = 0;

	/** Play next event if event is earlier than time 't'; either way,
		return the time that the task wants to be woken up next.
	*/
	virtual void				Play() = 0;

public:							// Accessors

	/**	Returns pointer to parent track, if any. */
	CPlaybackTask *				Parent() const
								{ return parent; }

	/**	Returns pointer to track. */
	CTrack *					Track() const
								{ return track; }
	
public:							// Operations

	/** Re-queue this task on the list of tasks to do. */
	void						ReQueue(
									CEventStack &stack,
									long time);

protected:						// Instance Data

	/** unique ID number of task */
	uint8						taskID;

	/** task flags (see above) */
	uint8						flags;

	/** longword-align */
	uint16						__pad__;

	/** which song or context is this in. */
	CPlaybackTaskGroup &		group;

	/** pointer to track being played */
	CTrack *					track;

	/** pointer to parent track */
	CPlaybackTask *				parent;

	/** The start of this sequence, relative to the start of the song.
		The origin may be different due to repeat events and other non-
		linear playback features. */
	int32						startTime;
	int32						originTime;

	/** Current time, relative to beginning of track. */
	int32						currentTime;
};

#endif /* __C_PlaybackTask_H__ */
