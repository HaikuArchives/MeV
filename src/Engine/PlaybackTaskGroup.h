/* ===================================================================== *
 * PlaybackTaskGroup.h (MeV/Engine)
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

#ifndef __C_PlaybackTaskGroup_H__
#define __C_PlaybackTaskGroup_H__
 
#include "MeV.h"
#include "MeVDoc.h"
#include "EventTrack.h"
#include "EventStack.h"
#include "PlayerControl.h"
#include "TempoMap.h"

/**
 *	A playback task group represents a set of related tasks, such as a song.
 *	@author Talin, Christopher Lenz
 *	@todo	Clean-up the interpolation model, move most of the calculation from
 *			CDestination::Interpolate() into here or CEventTask.
 */
class CPlaybackTaskGroup
	:	public DNode
{
	friend class	CPlayer;
	friend class	CPlaybackTask;
	friend class	CEventTask;
	friend class	CRealClockEventTask;
	friend class	CMeteredClockEventTask;
	friend class	CMasterEventTask;
	friend class	CPlayerControl;

public:							// Types & Constants

	/** There is one of these for each clock type. */
	struct TimeState
	{
		/** Stack of items pending to be played (real or metered)
		 *	it's public (for now) because it has it's own protections.
		 */
		CEventStack				stack;

		/** Current effective time (real or metered). */
		long					time;

		/** Target time to seek to. */
		long					seekTime;

		/** Expansion of time due to repeats. */
		long					expansion;

		/** Restart time if looping... */
		long					start;

		/** Time to stop (or -1 if no stop). */
		long					end;
	};

	enum EGroupFlags
	{
		/** Clock halted because group is stopped. */
		Clock_Stopped			= (1<<0),

		/** Clock halted because group is paused. */
		Clock_Paused			= (1<<1),

		/** Clock halted because awaiting sync. */
		Clock_AwaitingSync		= (1<<2),

		/** Clock halted because still locating. */
		Clock_Locating			= (1<<3),

		/** Tells player to relocate sequence. */
		Locator_Find			= (1<<4),

		/** Tells player to relocate from time zero. */
		Locator_Reset			= (1<<5),

		// This sequence never stops
		Clock_Continuous		= (1<<6),
		
		Clock_Halted =
			(Clock_Stopped | Clock_Paused | Clock_AwaitingSync | Clock_Locating)
	};

public:							// Constructor/Destructor

	/** Constructor. */
								CPlaybackTaskGroup(
									CMeVDoc *document = NULL);

private:

	/** Private destructor for playback context.
	 *	Only friends can delete.
	 */
	virtual 					~CPlaybackTaskGroup();

public:							// Accessors

	bool						ClockRunning() const
								{ return (flags & Clock_Halted) == 0; }
	
	/** Calculate the tempo period for the given clock time. */
	int32						CurrentTempoPeriod() const;

	CMeVDoc *					Document() const
								{ return doc; }

	uint16						Flags() const
								{ return flags; }

public:							// Operations

	/** Kill all events in progress. */
	void						FlushEvents();

	/** Kill all notes on the stack. */
	void						FlushNotes();
	
	/** Pause the sequence. */
	void						Pause(
									bool pause);

	/** Start playing. */
	void						Start(
									CTrack *track1,
									CTrack *track2,
									int32 locTime,
									enum ELocateTarget locTarget = LocateTarget_Real,
									int32 duration = -1,
									enum ESyncType syncType = SyncType_SongInternal,
									int32 optionFlags = 0);

	/** Halt playing and flush all pending note-offs. */
	void						Stop();

private:						// Internal Operations

	// Note that nonoe of the private functions have any locking,
	// since it is assumed that the caller will locks them.

	/** Execute a tempo change. */
	void						_changeTempo(
									long newRate,
									long start,
									long duration,
									long clockType);

	/** Takes a MeV event sends it out to the MIDI stream.
	 *	Handles the actual playing of an event. This is where events
	 *	are sent to AFTER they have been pulled of the player stack, 
	 *	i.e. events do not get here until after they have been remapped 
	 *	and are absolutely ready to go.
	 */
	void						_executeEvent(
									CEvent &ev,
									TimeState &);
								
	/**	Kill all tasks. */
	void						_flushTasks();

	void						_flushNotes(
									CEventStack &stack);

	/** Set all tasks belonging to a particular parent as
	 *	having expired.
	 */
	void						_killChildTasks(
									CPlaybackTask *parent);

	/** Called by player task to do the locate chores. */
	void						_locate();

	/** Locate a small batch of events. */
	void						_locateNextChunk(
									TimeState &tState);

	/** Hook function for spawned task for locator task. */
	static int32				_locatorTaskFunc(
									void *data);

	/** Restarts the track when an auto-loop happens. */
	void						_restart();

	/** Update the local time of the playback context. */
	void						_update(
									long internalTicks);

private:						// Instance Data

	/** List of active tasks for this musical set. */
	DList						tasks;

	/** Pointer to document (only one document per context, or NULL). */
	CMeVDoc *					doc;

	/** The start time of this group, in absolute real time. */
	int32						origin;

	/** Real time of next event, relative to the origin. */
	int32						nextEventTime;

	/** Real-time clock state. */
	TimeState					real;

	/** Metered-time clock state. */
	TimeState					metered;

	/** Various clock flags. */
	uint16						flags;

	/** Playback options. */
	uint16						pbOptions;

	/** Type of external sync. */
	enum ESyncType				syncType;

	/** Type of locate. */
	enum ELocateTarget 			locateType;

	/** The source tracks from which. */
	CTrack *					mainTracks[2];

	/** Seperate thread for locating. */
	thread_id					locatorThread;

	/** Current tempo state. */
	CTempoMapEntry				tempo;
};

#endif /* __C_PlaybackTaskGroup_H__ */
