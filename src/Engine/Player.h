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
 *	The thread playback pointer normally plays somewhat ahead of the actual clock
 *	value. This is so that an event who's start time is moved earlier by a real-time
 *	filter function can still play on time.
 *
 *	The reason for the two different values is this: Each time a track is checked
 *	for events which are ready, the track must be locked, which takes time. So to avoid
 *	too much locking, we allow each thread to stack up a number of events in a "burst",
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
 *
 * ===================================================================== */

#ifndef __C_Player_H__
#define __C_Player_H__

#include "MeV.h"
#include "PlaybackThread.h"
#include "PlaybackThreadTeam.h"
#include "PlayerControl.h"
#include <midi/Midi.h>

// ---------------------------------------------------------------------------
// Constants

const int32			cTrackAdvance_Real	= Ticks_Per_QtrNote / 2,
					cTrackAdvance_Metered= 250,
					cEventAdvance_Real	= Ticks_Per_QtrNote,
					cEventAdvance_Metered= 500;

// REM: We need to initialize this thing...ports should be NULL

// ---------------------------------------------------------------------------
// CMIDIPlayer controls the execution thread of the player task

class CMIDIPlayer : public BMidi {

	friend class StPlayerLock;
	friend class	CPlaybackThreadTeam;
	friend class	CPlaybackThread;
	friend class	CEventThread;
	friend class	CRealTimeEventThread;
	friend class	CMeteredTimeEventThread;
	friend class	CMasterEventThread;
	friend class CPlayerControl;

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

	BMidi		*ports[ Max_MidiPorts ];		// List of open midi ports
	PortInfo		portInfo[ Max_MidiPorts ];
	
	BSynth		*synth;
	int32		synthUseCount;
	
	BLocker		lock;						// Lock for concurrent access

	port_id		cmdPort;						// Port for sending player commands to

		// low-level clock functions
	long			internalTimerTick;			// the current time value

		// external synchronization variables
// long			lastExternalTimeVal,			// time value of last external tick
// 			lastExternalTimeTick,			// timer value at time of external tick
// 			externalTickPeriod,			// timer ticks between external tick
// 			externalOrigin;				// origin of external time

	long			nextEventTime;				// task wake-up time

		// Management of playback contexts
	DList		teamList;					// list of playback contexts

//	long			defaultTempoPeriod;			// default period of tempo
	
		// BMidi hook function
	void Run();

	CPlaybackThreadTeam
				*songTeam,					// context for playing songs
				*wildTeam;					// context for playing seperate data
				
	CPlaybackThreadTeam						// Find team associated with a document
				*FindTeam( CMeVDoc *doc );
				
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

		// Queue events for immediate execution.
	bool QueueEvents( Event *eventList, uint32 count, int32 startTime );
	bool QueueImmediate( Event *eventList, uint32 count )
	{
		return QueueEvents( eventList, count, B_NOW );
	}
	
		// Send an event directly to a hardware port. You probably shouldn't call
		// this directly. (no locking, for one thing)
	void SendEvent(	const Event &inEvent, uint8 inPort, uint8 inActualChannel, uint32 inTime );

		// Launch a new track
// BOOL launch( CTrack *track, long time, int clockType );

		/**	Assert that the player is in fact locked. */
	void CheckLock() { /* ASSERT( lock.IsLocked() ); */ }
};

	// Global instance of the player
extern CMIDIPlayer		thePlayer;

	// Stack-based locker for player
class StPlayerLock {
public:
	StPlayerLock() { thePlayer.lock.Lock(); }
	~StPlayerLock() { thePlayer.lock.Unlock(); }
};

#define LOCK_PLAYER		StPlayerLock		_stlock

#endif /* __C_Player_H__ */