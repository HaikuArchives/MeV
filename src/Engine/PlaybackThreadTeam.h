/* ===================================================================== *
 * PlaybackThreadTeam.h (MeV/Engine)
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

#ifndef __C_PlaybackThreadTeam_H__
#define __C_PlaybackThreadTeam_H__
 
#include "MeV.h"
#include "MeVDoc.h"
#include "EventTrack.h"
#include "EventStack.h"
#include "PlayerControl.h"
#include "TempoMap.h"
#include "VCTableManager.h"
// ---------------------------------------------------------------------------
// A playback thread team represents a "group" of related threads, such as a song.

class CPlaybackThreadTeam : public DNode {

	friend class	CMIDIPlayer;
	friend class	CPlaybackThread;
	friend class	CEventThread;
	friend class	CRealClockEventThread;
	friend class	CMeteredClockEventThread;
	friend class	CMasterEventThread;
	friend class CPlayerControl;

public:

		// There is one of these for each clock type
	struct TimeState {
			// Stack of items pending to be played (real or metered)
			// it's public (for now) because it has it's own protections.
		CEventStack			stack;

		int32				time,		// Current effective time (real or metered)
							seekTime,	// Target time to seek to.
							expansion,	// Expansion of time due to repeats
							start,		// restart time if looping...
							end;			// Time to stop (or -1 if no stop)
	};

	enum ETeamFlags {
			// Clock states
		Clock_Stopped		= (1<<0),	// Clock halted because team is stopped
		Clock_Paused			= (1<<1),	// Clock halted because team is paused
		Clock_AwaitingSync	= (1<<2),	// Clock halted because awaiting sync
		Clock_Locating		= (1<<3),	// Clock halted because still locating

			// tells player to relocate sequence
		Locator_Find			= (1<<4),

			// tells player to relocate from time zero
		Locator_Reset		= (1<<5),

			// This sequence never stops
		Clock_Continuous		= (1<<6),
		
			// Other clock attributes
// 	externalSync	= (1<<4),

		Clock_Halted =
			(Clock_Stopped | Clock_Paused | Clock_AwaitingSync | Clock_Locating)
	};

private:

		// list of active threads for this musical set
	DList			threads;

		// Pointer to document (only one document per context, or NULL)
	CMeVDoc			*doc;

		// Pointer to document's virtual channel table
		CVCTableManager	*vChannelTable;
	//VChannelEntry	*vChannelTable;
	
		// The start time of this team, in absolute real time
	int32			origin;

		// The relative start and end time of this team (in locate-type terms). Mainly used for repeating
//	int32			sectionStart,
//					sectionEnd;

		// Real time of next event, relative to the origin
	int32			nextEventTime;

		// Clock variables
	TimeState		real,					// Real-time clock vars
					metered;					// Metered-time clock vars

	uint16			flags;					// various clock flags
	uint16			pbOptions;				// playback options

		// Sync variables
	enum ESyncType	syncType;				// what type of external sync
	enum ELocateTarget locateType;			// what type of locate
	CTrack			*mainTracks[ 2 ];		// the source tracks from which
	thread_id		locatorThread;			// Seperate thread for locating

	CTempoMapEntry	tempo;					// current tempo state

/*	long				nextMidiClock,			// time of next midi clock out
					nextMetronomeTick,		// time of next metronome out
					metronomeTickSize;		// size of a metronome tick
*/
		// Velocity contours
// VelocityState	vstates[ MAX_VCONTOURS ];// velocity contour tables

// external synchronization variables
// long				lastExternalTimeVal,		// time value of last external tick
// 				lastExternalTimeTick,		// timer value at time of external tick
// 				externalTickRate,		// timer ticks between external tick
// 				externalOrigin;			// origin of external time

private:
		// Note that nonoe of the private functions have any locking,
		// since it is assumed that the caller will locks them.

		// Tempo management
	void ChangeTempo( long newRate, long start, long duration, long clockType );

		// Update the local time of the playback context
	void Update( long internalTicks );
	void FlushThreads();
	void FlushNotes( CEventStack &stack );
	void KillChildThreads( CPlaybackThread *parent );
	void LocateNextChunk( TimeState &tState );
	void Locate();
	void Restart();
	static int32 LocatorThreadFunc( void *data );

		// Takes a MeV event sends it out to the MIDI stream
	void ExecuteEvent( Event &ev, TimeState & );

		// Note: Private destructor; only friends can delete.
	~CPlaybackThreadTeam();

public:
	CPlaybackThreadTeam(	CMeVDoc *inDocument = NULL );

		// Start playing
	void Start(	CTrack				*inTrack1,
				CTrack				*inTrack2,
				int32				inLocTime,
				enum ELocateTarget	inLocTarget = LocateTarget_Real,
				int32				inDuration = -1,
				enum ESyncType		inSyncType = SyncType_SongInternal,
				int32				inOptionFlags = 0 );

		// Halt playing and flush all pending note-offs.
	void Stop();

		// Pause the sequence
	void Pause( bool inPause )
	{
		if (inPause) flags |= Clock_Paused;
		else flags &= ~Clock_Paused;
	}

	void FlushEvents();
	void FlushNotes();
	
	bool ClockRunning() { return (flags & Clock_Halted) == 0; }
	
	int32 CurrentTempoPeriod();
};

#endif /* __C_PlaybackThreadTeam_H__ */