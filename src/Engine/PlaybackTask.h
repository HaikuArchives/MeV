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

// ---------------------------------------------------------------------------
// CPlaybackTask -- an instance of a track being played

const int				cMaxNormalTask = 250,
						cFeedbackTask = 251;

class CPlaybackTask : public DNode {

	friend class		CMIDIPlayer;
	friend class		CPlaybackTaskGroup;
	friend class		CPlayerControl;

	friend int32 GetPlaybackMarkerTimes( CTrack *track, int32 *resultBuf, int32 bufSize );

public:
	enum taskFlags {
		Task_Muted		= (1<<0),			// turns off sound on track
		Task_Solo		= (1<<1),			// mute all other tracks
		Task_Wild		= (1<<2),			// use internal tick, not master time
		Task_Finished	= (1<<3),			// no more playback for this task
		Task_ImplicitLoop = (1<<4),			// task has implicit looping
	};

protected:
	uint8				taskID;				// unique ID number of task
	uint8				flags;				// task flags (see above)
	uint16				pad;					// longword-align

	CPlaybackTaskGroup	&group;				// which song or context is this in.
	CTrack				*track;				// pointer to track being played
	CPlaybackTask		*parent;				// pointer to parent track

		// The start of this sequence, relative to the start of the song.
		// The origin may be different due to repeat events and other non-
		// linear playback features.
	int32				startTime,			// clock time that task started
						originTime;			// base time that clock is relative to

		// Current time, relative to beginning of track.
	int32				currentTime;			// clock time relative to start

public:
		// constructor
	CPlaybackTask(	CPlaybackTaskGroup &group,
					CTrack *track,
					CPlaybackTask *parent,
					long start );

		// copy constructor -- can copy to a different group
	CPlaybackTask(	CPlaybackTaskGroup &group,
					CPlaybackTask &th );

		// destructor
	virtual ~CPlaybackTask();

		// Play next event if event is earlier than time 't'; either way,
		// return the time that the task wants to be woken up next.
	virtual void Play() = 0;

		// Re-queue this task on the list of tasks to do.
	void ReQueue( CEventStack &stack, long time );
	
		/**	Returns the current time of this track. */
	virtual int32 CurrentTime() = 0;

		/**	Returns pointer to parent track, if any. */
	CPlaybackTask *Parent() { return parent; }

		/**	Returns pointer to track. */
	CTrack *Track() { return track; }
	
		/**	This instructs the track that the implicit loop feature is going to be used.
			If the "inAtStart" is true, it means the loop point is at the start of the
			sequence; If it is false, then it means that the loop point is at the
			current point to which the sequence is located.
		*/
//	virtual void RecalcImplicitLoop( bool inAtStart, int32 inRepeatEnd ) {}
};

#if 0
// ---------------------------------------------------------------------------
// CPlayerState -- describes the state of the music at a given point in time

class CPlayer {

	CPlayerState		current,				// state of music right now
					recordLoopState;		// state of music at start of record loop
};

/* ============================================================================ *
                                  VelocityState

	There are 4 of these, one for each velocity envelope.
 * ============================================================================ */

typedef struct _VelocityState {
	UBYTE				currentScale,			/* current value				*/
						orgScale,				/* start value of ramp			*/
						targetScale;			/* end value of ramp			*/

	BYTE				currentBase,			/* current value				*/
						orgBase,				/* start value of ramp			*/
						targetBase;				/* end value of ramp			*/

	ULONG				orgTime,				/* start of ramp (real)			*/
						targetTime;				/* end of ramp (real)			*/
} VelocityState;

/* ============================================================================ *
                                    PBState

	PBState describes the current state of the music
 * ============================================================================ */

class PlayBackState {

		/*	Velocity Contours */

	VelocityState		vContours[ MAX_VCONTOURS ],	/* velocity contour tables	*/
						saveContours[ MAX_VCONTOURS ];	/* for record loop		*/

		/*	MIDI input / output state */

	ControllerCache		controllerState;		/* state of controllers sent	*/

		/*	Input-related variables */

	LONG				inputStamp,				/* timestamp for input events	*/
						inputStampOffset;		/* offset for above timestamp	*/

	UBYTE				lastNotePitch,			/* most recent note pitch		*/
						lastProgramChange,		/* most recent program change	*/
						lastSystemRealTime;		/* most recent midi realtime	*/
	UBYTE				MidiEventFlags;			/* what happened, midi-wise		*/

		/*	Misc variables */

	UBYTE				recordClockType,		/* clock type for recording		*/
						metronomeType;

};

	/*	These flags are used to inform the user-interface task that a MIDI
		or other real time event has occured which might be of interest to the
		main tasks.
	*/

#define MEVENT_NOTE		(1<<0)					/* a note received				*/
#define MEVENT_NOTEOFF	(1<<1)					/* a note-off received			*/
#define MEVENT_PROGRAM	(1<<2)					/* a program change received	*/
#define MEVENT_REALTIME	(1<<3)					/* a midi start/stop received	*/
#define MEVENT_RECORDED	(1<<4)					/* an event was added			*/

#define METROTYPE_NONE	0						/* no metronome					*/
#define METROTYPE_AUDIO	1						/* audio metronome				*/
#define METROTYPE_MIDI	2						/* MIDI metronome				*/

#define PBF_LOOP_ON		(1<<0)					/* loop mode active				*/
#define PBF_OVERLAY		(1<<1)					/* overlay recording			*/
#define PBF_RECORD		(1<<2)					/* currently recording			*/
#define PBF_SOLO		(1<<3)					/* playing just a single track	*/

#endif

#endif /* __C_PlaybackTask_H__ */
