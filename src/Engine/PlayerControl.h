/* ===================================================================== *
 * PlayerControl.h (MeV/Engine)
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
 *  Allows control of the player without including tons of other files.
 * ---------------------------------------------------------------------
 * History:
 *	1997		Talin
 *		Original implementation
 *	04/08/2000	cell
 *		General cleanup in preparation for initial SourceForge checkin
 *  5/13/2000   dwalton
 *		Added procedures to control the midi2 kit.
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __C_PlayerControl_H__
#define __C_PlayerControl_H__

#include "MeV.h"

class CMeVDoc;
class CTrack;

// ---------------------------------------------------------------------------
// Commands sent to player

enum EMidiCommands {
	Command_Start	= 0,		// Args = document,track,time,locTarget,SyncType,options
	Command_Stop,			// Args = document
	Command_Pause,			// Args = document
	Command_Continue,		// Args = document
	Command_Quit,			// Args = <none>
	Command_Attention,		// Args = <none>
};

// ---------------------------------------------------------------------------
// Enumeration values used as parameters when launching a sequence

enum ELocateTarget {
	LocateTarget_Continue = 0,		// no locate, just play from pos
	LocateTarget_Real,				// locate to real pos
	LocateTarget_Metered,				// locate to metered pos
};

enum ESyncType {
	SyncType_FreeRunning = 0,			// not synced with anything
	SyncType_SongInternal,			// sync with internal song time
	SyncType_SongMTC,				// sync song with MIDI Time Code
	SyncType_SongMidiClocks,			// sync song with MIDI clocks
};

enum EPlaybackOption {
	PB_Paused		= (1<<0),		// set up track initially paused
	PB_Record		= (1<<1),		// record while playing
	PB_Loop			= (1<<2),		// loop over and over
	PB_Folded		= (1<<3),		// song start time not including repeats
};

// ---------------------------------------------------------------------------
// Command arguments sent to player

class CMeVDoc;

struct CommandArgs {
	CMeVDoc			*document;			// Pointer to document (validate)
	int32			trackID;				// ID of track to play
	long				locTime;				// time to play at
	int8				locateTarget;		// Locator target type
	long				duration;			// Duration of playback
	int8				syncType;			// Synchronization type
	int16			options;				// Various options flags
};

// ---------------------------------------------------------------------------
// State of player...

struct PlaybackState {
	bool				running;
	long				realTime;
	long				meteredTime;
};

// ---------------------------------------------------------------------------
// PlayerControl utility class

class CPlayerControl {
public:
	static void InitPlayer();

		/**	Play some audible feedback. */
	static void DoAudioFeedback(
		CMeVDoc					*doc,				// doc (for VChannel)
		enum E_EventAttribute		feedbackAttribute,	// Which attr is changing
		uint8					attributeValue,		// New attribute value
		const Event				*demoEvent );			// Prototype event
		
		/**	Start playing a song. */
	static void PlaySong(
		CMeVDoc				*document,
		int32				trackID,
		long					locTime,
		enum ELocateTarget	locateTarget,
		long					duration,
		enum ESyncType		syncType,
		int16				options );
		
		/**	Determine if this document or any tracks in it are playing. */
	static bool IsPlaying( CMeVDoc *document );
	
		/**	If this document is playing (or paused), return the current playback state */
	static bool GetPlaybackState( CMeVDoc *document, PlaybackState &outState );

		/**	Determine the pause state for this document. */
	static bool PauseState( CMeVDoc *document );

		/** Set the state of the pause flag for this document, if it's playing. */
	static void SetPauseState( CMeVDoc *document, bool inPauseState );

		/** Toggle the state of the pause flag for this document, if it's playing. */
	static void TogglePauseState( CMeVDoc *document );

		/**	Stop playing a song. */
	static void StopSong( CMeVDoc *document );
	
		/**	Return the current tempo. */
	static double Tempo( CMeVDoc *document );
	
		/**	Set the current tempo */
	static void SetTempo( CMeVDoc *document, double newTempo );

		/**	Given a track, find all of the playback threads that are currently
			playing that track, and return the playback time of each thread
			relative to the start of the track. This is used to provide visual
			feedback of the playback.
		*/
	static int32 GetPlaybackMarkerTimes(
		CTrack		*track,					// Track to search for
		int32		*resultBuf,				// Buffer to place results in
		int32		bufSize );				// size of buffer
		
		/**	Get the port name of the Nth port. */
	static char *PortName(uint32 inPortIndex );
	
		/**	Set the port name of the Nth port. */
	static void SetPortName(uint32 inPortIndex, char *inPortName );
	
		/**	Get the port device string of the Nth port. */
	
	static const char *PortDevice(uint32 inPortIndex );
		/**	Set the port device string of the Nth port. */
	static bool SetPortDevice(uint32 inPortIndex, char *inPortName );
	//in general we don't seem to like working with pointers here, but for now...
	static bool SetPortConnect(uint32 inPortIndex, BMidiConsumer *sink );
	
	//returns the midiroster proviced endpoint ID.
	static int32 PortDeviceID(uint32 inPortIndex);
	static bool DeleteDevice(int inPortIndex);
	
	static int CountDefinedPorts();
	static bool IsDefined(uint32 inPortIndex);

private:
};

#endif /* __C_PlayerControl_H__ */
