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
#include <MidiEndpoint.h>
#include <MidiProducer.h>
// ---------------------------------------------------------------------------
// Constants

const int32			cTrackAdvance_Real	= Ticks_Per_QtrNote / 2,
					cTrackAdvance_Metered= 250,
					cEventAdvance_Real	= Ticks_Per_QtrNote,
					cEventAdvance_Metered= 500;

// REM: We need to initialize this thing...ports should be NULL

// ---------------------------------------------------------------------------
// CMIDIPlayer controls the execution thread of the player task

class CMIDIPlayer {

	// rather incestuous, aren't we?
	friend class	StPlayerLock;
	friend class	CPlaybackTaskGroup;
	friend class	CPlaybackTask;
	friend class	CEventTask;
	friend class	CRealTimeEventTask;
	friend class	CMeteredTimeEventTask;
	friend class	CMasterEventTask;
	friend class	CPlayerControl;

private:

	struct ChannelState {
		uint16	pitchBendState;		// current pitch bend value
		uint16	pitchBendTarget;		// target of interpolated pitch bend
		uint8	program;
		uint8	channelAfterTouch;
		uint16	ctlStates[ 128 ];		// Controller states
	};

	struct PortInfo {
		char		portName[ 64 ];
		char		devString[ 32 ];
		
			// MIDI State information for each channel.
		ChannelState	channelStates[ 16 ];
	};

	BMidiLocalProducer	*m_ports[ Max_MidiPorts ];		// List of virtual midi ports
	PortInfo		m_portInfo[ Max_MidiPorts ];
	
//	BSynth		*synth;
//	int32		synthUseCount;
	
	BLocker		m_lock;							// Lock for concurrent access

	port_id		m_port;							// Port for sending player commands to
	thread_id	m_thread;						// Realtime playback thread

		// low-level clock functions
	// +++++ shouldn't be exposed +++++
	// (CPlaybackTaskGroup uses)
	long		m_internalTimerTick;			// the current time value

		// external synchronization variables
// long			lastExternalTimeVal,			// time value of last external tick
// 			lastExternalTimeTick,			// timer value at time of external tick
// 			externalTickPeriod,			// timer ticks between external tick
// 			externalOrigin;				// origin of external time

		// Management of playback contexts
	DList		m_groupList;					// list of playback contexts

//	long			defaultTempoPeriod;			// default period of tempo
	
	// thread control
	static status_t ControlThreadEntry(void *user);
	void ControlThread();
	status_t StopControlThread();

	CPlaybackTaskGroup
				*songGroup,					// context for playing songs
				*wildGroup;					// context for playing seperate data
				
	CPlaybackTaskGroup						// Find group associated with a document
				*FindGroup( CMeVDoc *doc );
				
	BMidiSynth *NewMidiSynth();
	void DeleteMidiSynth( BMidiSynth *inSynth );

public:

		// Constructor and destructor
	CMIDIPlayer();
	~CMIDIPlayer();

		// Start all tasks and threads
	void Initialize();

		// Initialize all channel state records...
	void InitChannelStates();

		// Various command functions (start, stop, etc.)
		// Event stacking functions
		// State query functions
	void SetExternalTime( int32 time );

		// Queue events
	bool QueueEvents( Event *eventList, uint32 count, int32 startTime );
	bool QueueImmediate( Event *eventList, uint32 count );
	
		// Send an event directly to a hardware port. You probably shouldn't call
		// this directly. (no locking, for one thing)
	void SendEvent(	const Event &inEvent, uint8 inPort, uint8 inActualChannel, bigtime_t inTime  );

		// Fetch the player's command port ID
	port_id Port() const;

		// Launch a new track
// BOOL launch( CTrack *track, long time, int clockType );

	// locking
	bool Lock();
	void Unlock();
	bool IsLocked();
	
	/**	Assert that the player is in fact locked.
	    +++++ deprecated
	*/
	void CheckLock() { /* ASSERT( m_lock.IsLocked() ); */ }
};

	// Global instance of the player
extern CMIDIPlayer		thePlayer;

	// Stack-based locker for player
class StPlayerLock {
public:
	StPlayerLock() { thePlayer.Lock(); }
	~StPlayerLock() { thePlayer.Unlock(); }
};

#define LOCK_PLAYER		StPlayerLock		_stlock

#endif /* __C_Player_H__ */