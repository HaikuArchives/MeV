/* ===================================================================== *
 * Event.h (MeV/Engine)
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
 *  Defines basic musical event
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

#ifndef _EVENT_H
#define _EVENT_H

#include "MeVSpec.h"

// Gnu C Library
#include <string.h>
// Support Kit
#include <OS.h>
#include <MidiProducer.h>
// ---------------------------------------------------------------------
//	Forward declarations

class MeVSpec Event;

/* ============================================================================ *
	Time Signature structure
 * ============================================================================ */

class TimeSig {									// time signature structure
public:
	uint8			beats,						// number of beats in measure
					beatsize;					// the "denominator" of time sig
};

/* ============================================================================ *
	Event structure. An Event is basically an atom of musical data.

	Note on overloading of fields: This structure is used slightly differently
	when found in a track and when it is on the playback stack.

	First off, when on the task stack, the upper 8 bits of the start time are
	used to store the task number, so that when that task is altered,
	all pending items which were started from that task can be found.

	Secondly, in a track, the vChannel field is an index into the
	virtual channel table, but in a stacked item it is divided into two
	nybbles, the first of which is the output port and the second is the
	real MIDI channel. This allows the virtual channel table to be
	altered while a track is playing without causing stuck notes.
 * ============================================================================ */

typedef enum E_EventType {

		// MIDI channel events
	EvtType_End = 0,							// end of track marker
	EvtType_Note,								// note event
	EvtType_NoteOff,							// note-off event 
	EvtType_ChannelATouch,					// channel aftertouch
	EvtType_PolyATouch,						// polyphonic aftertouch
	EvtType_Controller,							// controller change
	EvtType_ProgramChange,					// program change
	EvtType_PitchBend,							// pitch bend

		// MIDI system events
	EvtType_SysEx,							// system exclusive

		// Cosmetic events
	EvtType_Text,								// text message
	EvtType_Reserved1,

		// Programmable event types for plug-ins
	EvtType_UserEvent,							// has type and data fields
	EvtType_Reserved2,

		// Track control events
	EvtType_Repeat,								// repeat a section
	EvtType_Sequence,							// play another track
	EvtType_Branch,								// conditional branch to track
	EvtType_Reserved3,
	EvtType_Reserved4,
	
		// VChannel property events
	EvtType_ChannelTranspose,					// transposition for vChannel
	EvtType_ChannelMute,						// mute a vChannel
	EvtType_ChannelVolume,						// VChannel velocity contour
	EvtType_Reserved5,
	
		// Track property events
	EvtType_MuteTrack,							// mute a track
	EvtType_Reserved6,

		// Clock control events
	EvtType_Tempo,								// change tempo event
	EvtType_TimeSig,							// change time signature event

		// Only used on playback stack -- never saved in a file
	EvtType_TaskMarker,						// indicates task is ready
	EvtType_StartInterpolate,					// Start an interpolation
	EvtType_Interpolate,						// Continue an interpolation

//	EvtType_EndSequence,						// end of a play track
//	EvtType_EndErase,							// end of erase event
//	EvtType_EndPunch,							// end of a "punch" event

/*		some future event ideas... (user events?)

		// Global control events
	EvtType_Splice,							// a "splice" event for overdub
	EvtType_SpliceOut,							// a "splice" event for overdub
	EvtType_Stop,								// stop the sequencer
	EvtType_Go,								// start the sequencer
	EvtType_Locate,							// locate to "duration" time
	EvtType_Cue,								// trigger a cue point
	EvtType_MTCCue,							// trigger an MTC cue point
	EvtType_Erase,							// erase notes on channel
	EvtType_Punch,							// punch in over track

	EvtType_KeySig,		-- key signature
	EvtType_BarLine,		-- for std midi files?
	EvtType_Map,			-- change output map or key map.
	EvtType_Switch,		-- set switches like MTC on
	EvtType_Config,		-- set program configuration
	EvtType_LaunchScript,-- send ARexx msg
	EvtType_Dump,		-- send patch dump (pause music...)
	??					-- trigger a cue point in the program (via MTC?)
*/

	EvtType_Count,							// number of events

		// used by filter functions */

	EvtType_All = ~0							// select all event types
} TEventType;

typedef uint8 event_type;

	// A number of general functions can operate on more than one attribute
	// of an event. Here are some enum codes which are used to tell these
	// functions which attribute to operate on
typedef enum E_EventAttribute {
	EvAttr_None = 0,								// no attribute
	EvAttr_StartTime,								// start time
	EvAttr_Duration,								// event duration
	EvAttr_StopTime,								// event stop time
	EvAttr_Type,									// event type
	EvAttr_Selected,								// Whether it's selected or not
	EvAttr_Channel,								// virtual channel number
	EvAttr_Pitch,									// pitch attribute
	EvAttr_AttackVelocity,							// attack velocity
	EvAttr_ReleaseVelocity,							// release velocity
	EvAttr_AfterTouch,								// aftertouch value
	EvAttr_Program,								// program number attribute
	EvAttr_ProgramBank,							// program bank number
	EvAttr_VPos,									// arbitrary vertical position
	EvAttr_ControllerNumber,						// controller number
	EvAttr_ControllerValue8,							// 8-bit controller value
	EvAttr_ControllerValue16,						// 16-bit controller value
	EvAttr_BendValue,								// 16-bit pitch-bend value
	EvAttr_InitialBend,								// 16-bit initial bend value
	EvAttr_UpdatePeriod,							// Update period for interpolated controls
	EvAttr_TempoValue,							// tempo value
	EvAttr_RepeatCount,							// repeat count
	EvAttr_SequenceNumber,						// sequence number
	EvAttr_Transposition,							// transposition
	EvAttr_CountourLevel,							// level for countour events
	EvAttr_DataSize,								// Size of extended data
	EvAttr_TSigBeatCount,							// Timesig numerator
	EvAttr_TSigBeatSize,							// Timesig denominator
	
		// Attributes for specific controller types
	EvAttr_BankSelect,								// Bank select controller value
	EvAttr_Modulation,								// Modulation value
	EvAttr_BreathController,							// Breath controller
	EvAttr_FootController,							// Foot Controller
	EvAttr_PortamentoTime,							// Portamento Time
	EvAttr_DataEntry,								// Data Entry
	EvAttr_ChannelVolume,							// Channel Volume
	EvAttr_Balance,								// Balance
	EvAttr_Pan,									// Pan
	EvAttr_ExpressionCtl,							// Expression Ctl.

	EvAttr_DamperPedal,							// Damper Pedal
	EvAttr_PortamentoOnOff,						// Portamento On/Off
	EvAttr_Sostenuto,								// Sostenuto
	EvAttr_SoftPedal,								// Soft Pedal

	EvAttr_LegatoFootswitch,						// Legato Footswitch
	EvAttr_Hold2,									// Hold 2

	EvAttr_PortamentoControl,						// Portamento Control

	EvAttr_EffectsDepth,							// Effects Depth

	EvAttr_DataIncrement,							// Data Increment
	EvAttr_DataDecrement,							// Data Decrement
	EvAttr_ParamNumber,							// Param

	EvAttr_LocalControl,							// Local Control
	EvAttr_MonoNumChannels,						// Number of mono channels

	EvAttr_Count
} TEventAttribute;

enum textTypes {
	TextType_Comment,							// just text
	TextType_Lyric,								// lyric
	TextType_Marker,								// "First Verse", etc.
	TextType_Cue,									// Cue point description
												// (can convert to anchor...)
};

struct ControllerInfo {

		// If this controller is an MSB, then this is the number of it's LSB.
		// If it's an LSB, then this will be 0xff;
		// If it is a single byte controller, it will be the number of the controller itself.
	uint8			LSBNumber;
	uint8			attrID;					// Event attribute ID for this controller
	char			*controllerName;		// string name of the controller
};

	// A table of information about controllers
extern ControllerInfo	controllerInfoTable[ 128 ];

	/*	When used in a field which would normally be filled in by a MIDI
		message, this value indicates that the message was never recieved.
		For example, the release velocity of a note that was never matched
		up with a note-off.
	*/

const int			MIDIValueUnset = 0xff,		// indicates value was never set
					MIDIReleaseSpecial = 128;	// note-on was released by note-on
					
	// Interpolation types 0-127 are for controllers
const int			Interpolation_PitchBend = 128,
					Interpolation_AfterTouch = 129;

class				CPlaybackTask;

	// An Event is a single musical data item. It is a low-level structure
	// designed to be handled in bulk. Programmatic access to events are probably
	// better handled by Event.
class Event {

		// Static tables for calculating event properties
	static unsigned char		propTable[];
	static char				*nameTable[];

		// Some events require a variable amount of data. The extra data
		// is store in this structure.
	class ExtendedEventData {
	public:
	
		int32				length;				// length of extra data
		int32				useCount;			// # of references to this
		// The actual extended data follows this
	};
	
	class ExtDataPtr {
	private:
		ExtendedEventData	*data;

	public:
		inline int32 Length() const { return data ? data->length : 0; }
		void SetLength( size_t inNewLength );

		inline void Init() { data = NULL; }
		inline void *Data() const { return data ? (void *)(data + 1) : NULL; }
		
		inline void Use() { if (data) data->useCount++; }
		void Release()
		{
			if (data)
			{
				if (--data->useCount == 0) delete[] (char *)data;
				data = NULL;
			}
		}
	};

	static void DefineEvent( enum E_EventType type, char *name, int32 properties );

public:

		// These constants define the "selected" flag, and the command
		// mask for retrieving it.
	enum {
		selected = (1<<7),						// selected event bit
		commandMask = 0x7f					// mask out selected bit
	};

		// These constants define "event properties" which can be used to
		// query things about the type of event.

	enum E_EventProp {
		Prop_Channel	= (1<<0),				// it's a channel message
		Prop_Duration	= (1<<1),				// it has a duration
		Prop_MIDI		= (1<<2),				// corresponds to MIDI message
		Prop_VertPos	= (1<<3),				// vertical pos in last byte
		Prop_ExtraData	= (1<<4)				// has EventExtra data
	};
	
	enum E_SequenceFlags {
		Seq_Interruptable	= (1<<0),			// sequence is interruptable
	};

	union {
	
			// New version takes 16 bytes

		struct {								// data common to most events
			int32		start;					// start time of event
			uint32		duration;				// duration of event
			uint8		command,				// event type
						vChannel,				// virtual channel
						data1,					// 1st data byte
						data2,					// 2nd data byte
						data3,					// 3rd data byte
						data4,					// 4th data byte
						data5,					// 5th data byte
						data6;					// 6th data byte
						BMidiLocalProducer		*actualPort;	
		
		} common;

			// In events which are placed on the playback stack, the
			// duration field is not used (since events on the playback
			// stack can't have duration). Instead, the space occupied
			// by the duration field is used for stack-related information.
			
		struct {
			int32		start;					// start time of event
			uint8			actualChannel;			// actual channel number
												// playback thread of this evt.
			
			uint8		hack;
			uint8		task,					// playback task of this evt.
						pad;
			uint8		command,				// command byte + control bits
						vChannel,				// virtual channel
						data1,					// 1st data byte
						data2,					// 2nd data byte (usually vPos)
						data3,					// 3rd data byte
						data4,					// 4th data byte
						data5,					// 5th data byte
						data6;					// 6th data byte
			BMidiLocalProducer		*actualPort;	
		} stack;

		struct {
			int32		start;					// start time of event
			uint32		duration;				// duration of event
			uint8		command,				// event type
						vChannel,				// virtual channel
						pitch,					// note pitch
						attackVelocity,			// attack velocity
						releaseVelocity,		// release velocity (or unused)
						data4,					// 4th data byte (not used)
						data5,					// 5th data byte (not used)
						data6;					// 6th data byte (not used)
						BMidiLocalProducer		*actualPort;	
		
		} note;

		struct {
			int32		start;					// start time of event
			uint32		duration;				// duration of event
			uint8		command,				// event type
						vChannel,				// virtual channel
						pitch,					// note pitch (poly atouch only)
						value,			// attack velocity
						data3,				// 3rd data bute (not used)
						data4;					// 4th data byte (not used)
			uint16		updatePeriod;			// # of clock cycles per increment
			BMidiLocalProducer		*actualPort;	
		
		} aTouch;

		struct {
			int32		start;					// start time of event
			uint32		duration;				// duration of event
			uint8		command,				// event type
						vChannel,				// virtual channel
						controller,				// controller number
						MSB,					// controller MSB
						LSB,					// controller LSB (or unused)
						data4;					// 4th data byte (not used)
			uint16		updatePeriod;			// # of interpolation steps
			BMidiLocalProducer		*actualPort;	
		
		} controlChange;

		struct {
			int32		start;					// start time of event
			uint32		duration;				// duration of event
			uint8		command,				// event type
						vChannel,				// virtual channel
						program,				// program number
						vPos,					// vertical position
						bankMSB,				// program bank MSB (or unused)
						bankLSB,				// program bank LSB (or unused)
						data5,					// 5th data byte (not used)
						data6;					// 6th data byte (not used)
						BMidiLocalProducer		*actualPort;	
		
		} programChange;

		struct {
			int32		start;				// start time of event
			uint32		duration;				// duration of event
			uint8		command,			// event type
						vChannel;			// virtual channel
			uint16		targetBend,			// target bend value
						startBend,			// initial bend value
						updatePeriod;			// # of clock cycles per increment
						BMidiLocalProducer		*actualPort;	
		
		} pitchBend;

			// Rem: Could short sysex messages be stored IN the data 2-6?

		struct {
			int32		start;					// program change time
			ExtDataPtr	extData;				// pointer to sysex buffer
			uint8		command,				// sysex cmd
						vChannel,				// virtual channel to send to
						data1,					// 1st data byte (not used)
						vPos,					// vertical position
						data3,					// 3rd data byte (not used)
						data4,					// 4th data byte (not used)
						data5,					// 5th data byte (not used)
						data6;					// 6th data byte (not used)
						BMidiLocalProducer		*actualPort;	
		
		} sysEx;

		struct {
			int32		start;					// program change time
			ExtDataPtr	extData;				// pointer to text buffer
			uint8		command,				// sysex cmd
						vChannel,				// virtual channel to send to
						textType,				// type of text (lyric, cue, etc)
						vPos,					// vertical position
						data3,					// 3rd data byte (not used)
						data4,					// 4th data byte (not used)
						data5,					// 5th data byte (not used)
						data6;					// 6th data byte (not used)
						BMidiLocalProducer		*actualPort;	
		
		} text;

		struct {
			int32		start;					// tempo time
			uint32		duration;				// duration
			uint8		command,				// command = Tempo
						vChannel,				// (unused)	
						data1,					// 1st data byte (not used)
						vPos;					// 2nd data byte (not used)
			uint32		newTempo;				// new tempo (packed 7)
												// tempo is in 1000ths of a beat
												// per minute
			BMidiLocalProducer		*actualPort;	
		
		} tempo;

		struct {
			int32		start;					// tempo time
			uint32		duration;				// duration
			uint8		command,				// command = Tempo
						vChannel,				// (unused)	
						data1,					// 1st data byte (not used)
						vPos;					// vertical position
			uint32		period;					// microseconds per qtr note
			BMidiLocalProducer		*actualPort;	
		
		} exactTempo;

		struct {
			int32		start;					// start time of event
			uint32		duration;				// duration of event
			uint8		command,				// event type
						vChannel,				// virtual channel
						data1,					// 1st data byte (not used)
						vPos;					// vertical position
			uint8		numerator;				// numerator of timesig
			uint8		denominator;			// denominator of timesig
			uint8		data5,					// 5th data byte (not used)
						data6;					// 6th data byte (not used)
						BMidiLocalProducer		*actualPort;	
		
		} sigChange;

		struct {
			int32		start;					// start time of event
			uint32		duration;				// duration of event
			uint8		command,				// event type
						vChannel,				// virtual channel
						data1,					// 1st data byte (not used)
						vPos;					// vertical position
			uint16		repeatCount;			// # of times to repeat (0xffff == infinity)
			uint8		data5,					// 5th data byte (not used)
						data6;					// 6th data byte (not used)
						BMidiLocalProducer		*actualPort;	
		
		} repeat;

		struct {
			int32		start;					// start time of event
			uint32		duration;				// duration of event
			uint8		command,				// event type
						vChannel,				// virtual channel (not used)
						data1,					// (not used)
						vPos;					// vertical position
			int8		transposition;			// sequence transposition
			uint8		flags;					// flags, none defined
			uint16		sequence;				// which sequence ID to play
		BMidiLocalProducer		*actualPort;	
		
		} sequence;

			// REM: I'm not sure this is right, looks like a copy of the sequence
			// struct...

		struct {
			int32		start;					// time of branch
			int32		offset;					// time offset to branch to.
			uint8		command,				// command = ETYPE_SEQUENCE
						vChannel,				// virtual channel (not used)
						data1,
						vPos;					// vertical position
			uint16		condition,				// condition bits to match
						mask;					// mask of bits
				BMidiLocalProducer		*actualPort;	
		
		} branch;

		struct {								// contour control vertex
			int32		start;					// contour change start time
			uint32		duration;				// duration of interpolation
			uint8		command,				// command = ETYPE_VELOCITY
						vChannel,				// virtual channel (not used)
						scale;					// new velocity scale factor
			uint8		vPos; 					// vertical position
			int8			base;					// new velocity base
			uint8		contour;				// contour number
			uint8		data5,					// 5th data byte (not used)
						data6;					// 6th data byte (not used)
			BMidiLocalProducer		*actualPort;	
		
		} contour;

		struct {
			int32		start;					// sequence start time
			CPlaybackTask *taskPtr;				// pointer to playback task
			uint8		command,				// command = EvtType_TaskMarker
						pad[ 7 ];				// unused
			BMidiLocalProducer		*actualPort;	
		
		} task;

		struct {
			int32		start;				// start time of event
			uint32		pad;					// duration of event
			uint8		command,			// event type
						interpolationType;		// what type of interpolation
			uint16		startValue,			// start interpolation value
						targetValue,			// final interpolation value
						pad2;
			BMidiLocalProducer		*actualPort;	
		
		} startInterpolate;

		struct {
			int32		start;				// start time of event
			uint32		pad;					// duration of event
			uint8		command,			// event type
						interpolationType;		// what type of interpolation
			uint16		timeStep;				// time step between interpolations
			uint32		duration;				// total duration of interpolation
			BMidiLocalProducer		*actualPort;	
		
		} interpolate;
	};

private:
		// Bump the use count on the extended data (used when making a copy of the
		// event.
	void UseExtendedData()
	{
		if (HasProperty( Event::Prop_ExtraData ))	sysEx.extData.Use();
	}

		// Release the extended data (used when destructing an event)
	void ReleaseExtendedData()
	{
		if (HasProperty( Event::Prop_ExtraData ))	sysEx.extData.Release();
	}

public:

		// Constructor
	Event()
	{
			// Initialize to a type which has no data.
		common.command = EvtType_End;
// 	sysEx.extData.Init();
	}

	Event( const Event &inEvent );
#if 0
	Event( const Event &inEvent )
	{
		memcpy( this, &inEvent, sizeof *this );
		UseExtendedData();
	}
#endif

		// Destructor
	~Event()
	{
		ReleaseExtendedData();
	}

		// assignment '=' operator
	Event &operator=( Event &inEvent );
#if 0
	Event &operator=( Event &inEvent )
	{
		ReleaseExtendedData();
		memcpy( this, &inEvent, sizeof *this );
		UseExtendedData();
		return *this;
	}
#endif

		// Access functions for start time
	int32 Start() const { return common.start; }
	void SetStart( const int32 inStart ) { common.start = inStart; }

		// Access functions for duration
	int32 Duration() const { return common.duration; }
	void SetDuration( const int32 inDuration ) { common.duration = inDuration; }
	
		// Access functions for stop time (start + duration)
	int32 Stop() const { return common.start + Duration(); }

		// Access functions for selected bit
	bool IsSelected() const { return common.command & selected; }
	void SetSelected( bool inSelected )
	{
		if (inSelected)
			common.command |= selected;
		else common.command &= ~selected;
	}

		// Access functions for command field
	uint8 Command() const { return common.command & commandMask; }
	void SetCommand( uint8 inCommand )
	{
			// Delete extra data whenever changing command types.
		if (inCommand != Command())
		{
			ReleaseExtendedData();
			common.command = common.command & 0x80 | inCommand;
			if (HasProperty( Event::Prop_ExtraData )) sysEx.extData.Init();
		}
	}

		// Access functions for VChannel field
	uint8 GetVChannel() const { return common.vChannel; }
	void SetVChannel( uint8 inChannel ) { common.vChannel = inChannel; }

		// Properties query functions
	static void InitTables();
	int Properties() const { return propTable[ Command() ]; }
	static int Properties( int inCommand ) { return propTable[ inCommand ]; }
	bool HasProperty( int inPropertyMask ) const
		{ return (propTable[ Command() ] & inPropertyMask) != 0; }

		// Event name query function
	char *NameText() const { return nameTable[ Command() ]; }
	static char *NameText( int inCommand ) { return nameTable[ inCommand ]; }

		// Functions dealing with extra data
	size_t ExtendedDataSize() const
	{
		return HasProperty( Event::Prop_ExtraData ) ? sysEx.extData.Length() : 0;
	}
	bool SetExtendedDataSize( int inLength )
	{
		if (HasProperty( Event::Prop_ExtraData ))
		{
			sysEx.extData.SetLength( inLength );
			return true;
		}
		return false;
	}
	void *ExtendedData() const
	{
		return HasProperty( Event::Prop_ExtraData ) ? sysEx.extData.Data() : NULL;
	}

		/**	Ask this event about its Nth event-specific attribute. */
	enum E_EventAttribute QueryAttribute( int32 inIndex ) const;
	
		/**	returns true if the event possesses the attribute.
		*/
	bool HasAttribute( enum E_EventAttribute inAttr ) const;

		/**	Get the value of an attribute.
			(Note: some attributes are aliases of others, in which
			case only the "main" attribute is reported by this function.)
		*/
	int32 GetAttribute( enum E_EventAttribute inAttr ) const ;

		/**	Set the value of an attribute.
			(returns false if the event has no such attribute.)
		*/
	bool SetAttribute( enum E_EventAttribute inAttr, int32 inValue );

	// void shareExtendedData( Event *ev );
	
		// Static functions for event management
	static void Construct( Event *inEventArray, int32 count )
	{
		while (count--)
		{
				// Initialize to blank event
			inEventArray->common.command = EvtType_End;
			inEventArray++;
		}
	}

	static void Construct( Event *inDst, const Event *inSrc, int32 count )
	{
		memmove( inDst, inSrc, count * sizeof (Event) );
		while (count--)
		{
				// Copy event into blank memory
			inDst->UseExtendedData();
			inDst++;
		}
	}

	static void Destruct( Event *inEventArray, int32 count )
	{
		while (count--)
		{
				// Release extended data pointer if needed
			inEventArray->ReleaseExtendedData();
			inEventArray++;
		}
	}

	static void Relocate( Event *inDst, const Event *inSrc, int32 count )
	{
		memmove( inDst, inSrc, count * sizeof (Event) );
	}
};

inline Event::Event( const Event &inEvent )
{
	memcpy( this, &inEvent, sizeof *this );
	UseExtendedData();
}

		// assignment '=' operator
inline Event &Event::operator=( Event &inEvent )
{
	ReleaseExtendedData();
	memcpy( this, &inEvent, sizeof *this );
	UseExtendedData();
	return *this;
}

typedef Event		*EventPtr;
typedef const Event	*ConstEventPtr;

	/**	A class which returns information about an event attribute. */
class UEventAttributeTable {

	struct EvAttr {
		char			*name;				// name of attribute
		int32		offset;				// 0-based, 1-based, etc.
		int32		minVal,				// minimum legal value (w/o offset)
					maxVal;				// maximum legal value (w/o offset)
	};

	static EvAttr	attrTable[ EvAttr_Count ];

public:
		/**	Returns the ascii name of the attribute */
	static char *Name( enum E_EventAttribute inAttr )
	{
		return attrTable[ (int)inAttr ].name;
	}

		/**	Returns the inclusive (min,max) range of the attribute */
	static int32 Offset( enum E_EventAttribute inAttr )
	{
		return attrTable[ (int)inAttr ].offset;
	}

		/**	Returns the inclusive (min,max) range of the attribute */
	static void Range( enum E_EventAttribute inAttr, int32 &minVal, int32 &maxVal )
	{
		minVal = attrTable[ (int)inAttr ].minVal;
		maxVal = attrTable[ (int)inAttr ].maxVal;
	}
};

#endif // _EVENT_H