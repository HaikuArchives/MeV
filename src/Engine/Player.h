/* ===================================================================== *
 * Player.h (MeV/Engine)
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
 *  Defines characteristics of the player task
 *
 *	The task playback pointer normally plays somewhat ahead of the actual clock
 *	value. This is so that an event who's start time is moved earlier by a real-time
 *	filter function can still play on time.
 *
 *	The reason for the two different values is this: Each time a track is checked
 *	for events which are ready, the track must be locked, which takes time. So to avoid
 *	too much locking, we allow each task to stack up a number of events in a "burst",
 *	placing a copy of these events on the event stack, which will then be emitted at the
 *	correct time. 
 *
 *	For metered tracks, the playback pointer looks for events which are up to one
 *	quarter note ahead of the current metered playback time. Any events which are in
 *	this range are filtered and queued. When there are no more events to process,
 *	the track itself is requeued at a time equal to the time of the next event minus
 *	one eighth note.
 *	
 *	So, the time of the last note (t) must be greater than (current time + wnote/4).
 *	The next time the track will be checked is (t - wnote/8), which must be greater
 *	than (current time + wnote/8). Therefore, this track will be checked _at most_ once
 *	per eighth note.
 * ---------------------------------------------------------------------
 * History:
 *	1997		Talin
 *		Original implementation
 *	04/08/2000	cell
 *		General cleanup in preparation for initial SourceForge checkin
 * ---------------------------------------------------------------------
 * To Do:
 * -The communication between CPlayerControl and Player is based on the ports setup by
 * the original midi kit.  This should be replaced with threads or blooper.
 * -Player should not be a BMidi object.
 * ===================================================================== */

#ifndef __C_Player_H__
#define __C_Player_H__

#include "MeV.h"
#include "PlaybackTask.h"
#include "PlaybackTaskGroup.h"
#include "PlayerControl.h"

// Support Kit
#include <Locker.h>

namespace Midi
{
	class CMidiOutput;
}

// ---------------------------------------------------------------------------
// Constants

const int32			cTrackAdvance_Real	= Ticks_Per_QtrNote / 2,
					cTrackAdvance_Metered= 250,
					cEventAdvance_Real	= Ticks_Per_QtrNote,
					cEventAdvance_Metered= 500;

// REM: We need to initialize this thing...ports should be NULL

// ---------------------------------------------------------------------------
// CPlayer controls the execution thread of the player task

class CPlayer
{
	// rather incestuous, aren't we?
	friend class StPlayerLock;
	friend class CPlaybackTaskGroup;
	friend class CPlaybackTask;
	friend class CEventTask;
	friend class CRealTimeEventTask;
	friend class CMeteredTimeEventTask;
	friend class CMasterEventTask;
	friend class CPlayerControl;

public:							// Constructor/Destructor

	/** Standard constructor. Initializes the player task. */
								CPlayer();

	/** Destructor. Cleans up the player task. */
								~CPlayer();

public:							// Accessors

	/** Fetch the player's command port ID */
	port_id						Port() const
								{ return m_port; }

	/** Locking */
	bool						IsLocked() const
								{ return m_lock.IsLocked(); }
	bool						Lock()
								{ return m_lock.Lock(); }
	void						Unlock()
								{ m_lock.Unlock(); }
	
	/**	Assert that the player is in fact locked.
	    +++++ deprecated
	*/
	void						CheckLock()
								{ /* ASSERT( m_lock.IsLocked() ); */ }

public:							// Operations

	// Start all tasks and threads
	void						Initialize();

	void						SetExternalTime(
									int32 time);

	/** push some events on the the linear time stack, and then
		check to see if it's time to wake up.
	*/
	bool						QueueEvents(
									CEvent *eventList,
									uint32 count,
									int32 startTime);

	/** Queue given events for immediate execution. */
	bool						QueueImmediate(
									CEvent *eventList,
									uint32 count);

private:						// Internal Operations

	/** Find group associated with a document */
	CPlaybackTaskGroup *		FindGroup(
									CMeVDoc *doc);
				
	/** thread control */
	static status_t				ControlThreadEntry(
									void *user);

	void						ControlThread();

	status_t					StopControlThread();

private:						// Instance Data

	/** Lock for concurrent access */
	BLocker						m_lock;

	/** Port for sending player commands to */
	port_id						m_port;

	/** Realtime playback thread */
	thread_id					m_thread;

	/** low-level clock functions
		+++++ shouldn't be exposed +++++
		(CPlaybackTaskGroup uses)
		the current time value
	*/
	long						m_internalTimerTick;

	/**	Management of playback contexts
		list of playback contexts
	*/
	DList						m_groupList;

	/** context for playing songs */
	CPlaybackTaskGroup *		m_songGroup;

	/** context for playing seperate data */
	CPlaybackTaskGroup *		m_wildGroup;
};

// Global instance of the player
extern CPlayer thePlayer;

// Stack-based locker for player
class StPlayerLock
{

public:

								StPlayerLock()
								{ thePlayer.Lock(); }

								~StPlayerLock()
								{ thePlayer.Unlock(); }
};

// ++++++++ deprecated
#define LOCK_PLAYER		StPlayerLock		_stlock

#endif /* __C_Player_H__ */
