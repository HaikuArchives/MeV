/* ===================================================================== *
 * Event.cpp (MeV/Engine)
 * ===================================================================== */

#include "MeVSpec.h"
#include "Event.h"
#include "Destination.h"
#include "MathUtils.h"
#include <stdio.h>
/** ======================================================================= **
	Event Members
 ** ======================================================================= **/

// ---------------------------------------------------------------------------
// Event Properties Table ~~~EVENTLIST

unsigned char Event::propTable[ EvtType_Count ];

// ---------------------------------------------------------------------------
// Event Names Table ~~~EVENTLIST

char *Event::nameTable[ EvtType_Count ];

// ---------------------------------------------------------------------------
// Define table of events

inline void Event::DefineEvent( enum E_EventType type, char *name, int32 properties )
{
	propTable[ type ] = properties;
	nameTable[ type ] = name;
}

#define MIDI_FLAGS (Event::Prop_Channel | Event::Prop_MIDI)
#define MIDI_DURATION_FLAGS (Event::Prop_Channel | Event::Prop_Duration | Event::Prop_MIDI)

void Event::InitTables()
{
	DefineEvent( EvtType_Note, "Note", MIDI_DURATION_FLAGS );
	DefineEvent( EvtType_NoteOff, "Note Off", MIDI_FLAGS );
	DefineEvent( EvtType_ChannelATouch, "Channel Aftertouch", MIDI_DURATION_FLAGS );
	DefineEvent( EvtType_PolyATouch, "Polyphonic Aftertouch", MIDI_FLAGS );
	DefineEvent( EvtType_Controller,	 "Control Change", MIDI_DURATION_FLAGS );
	DefineEvent( EvtType_ProgramChange, "Program Change", MIDI_FLAGS | Event::Prop_VertPos );
	DefineEvent( EvtType_PitchBend,	 "Pitch Bend", MIDI_DURATION_FLAGS );
	DefineEvent( EvtType_SysEx, "System Exclusive", 
		Event::Prop_ExtraData	| Event::Prop_MIDI	| Event::Prop_VertPos );
	DefineEvent( EvtType_End, "End", 0 );
	DefineEvent( EvtType_Text, "Text", Event::Prop_ExtraData | Event::Prop_VertPos );
	DefineEvent( EvtType_UserEvent, "User Event", Event::Prop_ExtraData );
	DefineEvent( EvtType_Repeat, "Repeat", Event::Prop_Duration );
	DefineEvent( EvtType_Sequence, "Part", Event::Prop_Duration | Event::Prop_VertPos );
	DefineEvent( EvtType_Branch, "Conditional Branch", Event::Prop_VertPos );
	DefineEvent( EvtType_ChannelMute, "Channel Mute", Event::Prop_Duration );
	DefineEvent( EvtType_ChannelTranspose, "Channel Tranpose", 0 );
	DefineEvent( EvtType_ChannelVolume, 	"Channel Volume", Event::Prop_VertPos );
	DefineEvent( EvtType_MuteTrack, 	"Mute Part", Event::Prop_Duration );
	DefineEvent( EvtType_Tempo, "Tempo", Event::Prop_Duration | Event::Prop_VertPos );
	DefineEvent( EvtType_TimeSig, "Time Signature", 0 );
};

// ---------------------------------------------------------------------------
// Table of event attributes

UEventAttributeTable::EvAttr UEventAttributeTable::attrTable[ EvAttr_Count ] = {
	{	"", 				0, 0, 	0			},	// EvAttr_None
	{	"Start Time",		0, LONG_MIN, LONG_MAX	}, 	// EvAttr_StartTime
	{	"Stop Time",		0, LONG_MIN, LONG_MAX	}, 	// EvAttr_Duration
	{	"Duration", 		0, 0, 	LONG_MAX		},	// EvAttr_StopTime
	{	"Type", 			0, 0,		EvtType_Count},	// EvAttr_Type
	{	"Selected",		0, 0,		1			},	// EvAttr_Selected
	{	"Channel",		1, 0,		63			},	// EvAttr_Channel
	{	"Pitch",			0, 0,		127			}, 	// EvAttr_Pitch
	{	"Attack",			0, 1,		127			},	// EvAttr_AttackVelocity
	{	"Release",		0, 0,		127			},	// EvAttr_ReleaseVelocity
	{	"Pressure",		0, 0,		127			},	// EvAttr_AfterTouch
	{	"Program",		0, 0,		127			}, 	// EvAttr_Program
	{	"Bank",			0, 0,		0x3fff		},	// EvAttr_ProgramBank
	{	"Position",		0, 0,		63			},	//?	EvAttr_VPos
	{	"Controller",		0, 0,		127			},	// EvAttr_ControllerNumber
	{	"Value",			0, 0,		127			},	// EvAttr_ControllerValue8
	{	"Value",			0, 0,		0x3fff		}, 	// EvAttr_ControllerValue16
	{	"End Pitch",		-0x2000, 0,0x3fff		}, 	// EvAttr_BendValue
	{	"Start Pitch",		-0x2000, 0,0x3fff		}, 	// EvAttr_InitialBend
	{	"Update Rate",		0, 0,		0x3fff		}, 	// EvAttr_UpdatePeriod
	{	"Tempo",			0, 1000*10,1000*500	}, 	//?	EvAttr_TempoValue
	{	"Repeats",		0, 0,		0x0ffff		},	//?	EvAttr_RepeatCount
	{	"Part",			0, 2,		0x0ffff		}, 	//?	EvAttr_SequenceNumber
	{	"Transpose",		0, -128,	127			}, 	// EvAttr_Transposition
	{	"Level",			0, 0,		255			}, 	// EvAttr_CountourLevel
	{	"Bytes",			0, 0,		LONG_MAX	}, 	// EvAttr_DataSize
	{	"# Beats",		0, 1,		64			}, 	// EvAttr_TSigBeatCount
	{	"Beat Size",		0, 0,		6			}, 	// EvAttr_TSigBeatSize

			// Attributes for specific controller types
	{	"Bank", 			0, 0,		0x3fff		},	// EvAttr_BankSelect
	{	"Modulation", 		0, 0,		0x3fff		},	// EvAttr_Modulation
	{	"Breath", 			0, 0,		0x3fff		},	// EvAttr_BreathController
	{	"Foot", 			0, 0,		0x3fff		},	// EvAttr_FootController
	{	"Portamento",		0, 0,		0x3fff		},	// EvAttr_PortamentoTime
	{	"Data",			0, 0,		0x3fff		},	// EvAttr_DataEntry
	{	"Volume",			0, 0,		0x3fff		},	// EvAttr_ChannelVolume
	{	"Balance",		0, 0,		0x3fff		},	// EvAttr_Balance
	{	"Pan",			0, 0,		0x3fff		},	// EvAttr_Pan
	{	"Expression",		0, 0,		0x3fff		},	// EvAttr_ExpressionCtl

	{	"Damper", 		0, 0,		127			},	// EvAttr_DamperPedal
	{	"On/Off",			0, 0,		127			},	// EvAttr_PortamentoOnOff
	{	"Sostenuto",		0, 0,		127			},	// EvAttr_Sostenuto
	{	"Soft Pedal",		0, 0,		127			},	// EvAttr_SoftPedal
	{	"Legato",			0, 0,		127			},	// EvAttr_LegatoFootswitch
	{	"Hold 2",			0, 0,		127			},	// EvAttr_Hold2

	{	"Port. Note", 		0, 0,		127			},	// EvAttr_PortamentoControl

	{	"Depth", 			0, 0,		127			},	// EvAttr_Effects1Depth

	{	"Increment",	 	0, 0,		127			},	// EvAttr_DataIncrement
	{	"Decrement",		0, 0,		127			},	// EvAttr_DataDecrement
	{	"Parameter",		0, 0,		0x3fff		},	// EvAttr_Param

	{	"On/Off",			0, 0,		127			},	// EvAttr_LocalControl

	{	"Channel Count", 	0, 0,		1			},	// EvAttr_MonoNumChannels
};

// ---------------------------------------------------------------------------
// A function to iterate through the event-specific attributes

enum E_EventAttribute Event::QueryAttribute( int32 index ) const
{
	switch (Command()) {
	case EvtType_Note:
		switch (index) {
		case 0: return EvAttr_Pitch;
		case 1: return EvAttr_AttackVelocity;
		case 2: return EvAttr_ReleaseVelocity;
		}
		break;

	case EvtType_NoteOff:
		switch (index) {
		case 0: return EvAttr_Pitch;
		case 1: return EvAttr_ReleaseVelocity;
		}
		break;

	case EvtType_ChannelATouch:
		switch (index) {
		case 0: return EvAttr_AfterTouch;
		case 1: return EvAttr_UpdatePeriod;
		}

	case EvtType_PolyATouch:
		switch (index) {
		case 0: return EvAttr_Pitch;
		case 1: return EvAttr_AfterTouch;
		}
		break;

	case EvtType_Controller:
		switch (index) {
		case 0: return EvAttr_ControllerNumber;
		case 1:
			uint8	ctl;
			
			ctl = controlChange.controller;
		
				// REM: An experiment.
			if (ctl == 1) return EvAttr_Modulation;
			
			if (		ctl <= 31
				||	ctl >= 98 && ctl <= 101)
			{
				return EvAttr_ControllerValue16;
			}
			
				// Most channel mode messages have NO editable parameter...
			if (ctl == 121 || ctl >= 123) return EvAttr_None;
			
				// Otherwise, we have an 8-bit controller.
			return EvAttr_ControllerValue8;

		case 2: return EvAttr_UpdatePeriod;
		}
		break;

	case EvtType_ProgramChange:
		switch (index) {
		case 0: return EvAttr_Program;
		case 1: return EvAttr_ProgramBank;
		}
		break;

	case EvtType_PitchBend:
		switch (index) {
		case 0: return EvAttr_InitialBend;
		case 1: return EvAttr_BendValue;
		case 2: return EvAttr_UpdatePeriod;
		}
		break;

		// MIDI system events
	case EvtType_SysEx:
	case EvtType_Text:
	case EvtType_UserEvent:
		return (index == 0) ? EvAttr_DataSize : EvAttr_None;

		// Global control events
// EvtType_Stop,								// stop the sequencer
// EvtType_Go,									// start the sequencer
// EvtType_Locate,								// locate to "duration" time
// EvtType_Cue,								// trigger a cue point
// EvtType_MTCCue,								// trigger an MTC cue point
// EvtType_MuteVChannel,						// mute a vChannel
// EvtType_MuteTrack,							// mute a track
// EvtType_SpliceIn,							// a "splice" event for overdub
// EvtType_SpliceOut,							// a "splice" event for overdub
	
		// Programmable event types for plug-ins
// EvtType_UserEvent,							// has type and data fields

		// Track control events
	case EvtType_Repeat:							// repeat a section
		return (index == 0) ? EvAttr_RepeatCount : EvAttr_None;
		break;

	case EvtType_Sequence:						// play another track
		switch (index) {
		case 0: return EvAttr_SequenceNumber;
		case 1: return EvAttr_Transposition;
		}
		break;

// EvtType_Branch,								// conditional branch to track
// EvtType_Erase,								// erase notes on channel
// EvtType_Punch,								// punch in over track

		// Clock control events
	case EvtType_Tempo:							// change time signature event
		return (index == 0) ? EvAttr_TempoValue : EvAttr_None;
		break;

	case EvtType_TimeSig:							// change time signature event
		switch (index) {
		case 0: return EvAttr_TSigBeatCount;
		case 1: return EvAttr_TSigBeatSize;
		}
		break;

// EvtType_VelocityContour,					// velocity contour event
// EvtType_Transpose,							// transposition for vChannel

		// Only used on playback stack
// EvtType_EndSequence,						// end of a play track
// EvtType_EndErase,							// end of erase event
// EvtType_EndPunch,							// end of a "punch" event
// EvtType_TaskMarker,						// indicates task is ready

/*		some future event ideas...

	EvtType_KeySig,		-- key signature
	EvtType_BarLine,		-- for std midi files?
.	EvtType_Locate,		-- "jump to" a particular clock time (not in track?)
	EvtType_Map,			-- change output map or key map.
	EvtType_Switch,		-- set switches like MTC on
	EvtType_Config,		-- set program configuration
	EvtType_LaunchScript,-- send ARexx msg
	EvtType_Dump,		-- send patch dump (pause music...)
	??					-- trigger a cue point in the program (via MTC?)
*/
	}
	
	return EvAttr_None;
}

// ---------------------------------------------------------------------------
// Returns true if the event possesses the attribute.

bool Event::HasAttribute( enum E_EventAttribute inAttr ) const
{
	uint8		cmd = Command();

	switch (inAttr) {
	case EvAttr_StartTime:					// start time
	case EvAttr_Type:						// event type
	case EvAttr_Selected:						// Whether it's selected or not
		return true;

	case EvAttr_Duration:						// event duration
	case EvAttr_StopTime:						// event stop time
		return HasProperty( Prop_Duration );
	
	case EvAttr_Channel:						// virtual channel number
		return HasProperty( Prop_Channel );

	case EvAttr_Pitch:						// pitch attribute
		return (		cmd == EvtType_Note
				||	cmd == EvtType_NoteOff
				||	cmd == EvtType_PolyATouch) ;

	case EvAttr_AttackVelocity:				// attack velocity
	case EvAttr_ReleaseVelocity:				// release velocity
		return (		cmd == EvtType_Note
				||	cmd == EvtType_NoteOff) ;

	case EvAttr_AfterTouch:					// aftertouch value
		return (		cmd == EvtType_ChannelATouch
				||	cmd == EvtType_PolyATouch) ;

	case EvAttr_Program:						// program number attribute
	case EvAttr_ProgramBank:					// program bank number
		return (cmd == EvtType_ProgramChange) ;

	case EvAttr_ControllerNumber:				// controller number
		return (cmd == EvtType_Controller) ;

	case EvAttr_ControllerValue8:				// 8-bit controller value
		if (cmd == EvtType_Controller)
		{
// 		uint8	ctl = controlChange.controller;

				// Questionable...for now I assume that all 16-bit controllers
				// can in fact be set with just the MSB only
			return true;
		}
		return false;
	
	case EvAttr_ControllerValue16:			// 16-bit controller value
		if (cmd == EvtType_Controller)
		{
			uint8	ctl = controlChange.controller;
			return (	ctl <= 31 || ctl >= 98 && ctl <= 101);
		}
		return false;

	case EvAttr_Modulation:					// Modulation value
		return (		cmd == EvtType_Controller
				&&	controlChange.controller == 1 );

	case EvAttr_BendValue:					// 16-bit pitch-bend value
	case EvAttr_InitialBend:					// 16-bit pitch-bend initial value
		return (cmd == EvtType_PitchBend) ;

	case EvAttr_UpdatePeriod:				// 16-bit update period
		return (		cmd == EvtType_PitchBend
				||	cmd == EvtType_Controller
				||	cmd == EvtType_ChannelATouch) ;

	case EvAttr_TempoValue:					// tempo value
		return (cmd == EvtType_Tempo) ;

	case EvAttr_RepeatCount:					// repeat count
		return (cmd == EvtType_Repeat) ;

	case EvAttr_DataSize:						// Size of extended data
		return (cmd == EvtType_SysEx || cmd == EvtType_Text || EvtType_UserEvent) ;

	case EvAttr_SequenceNumber:				// sequence number
		return (cmd == EvtType_Sequence);
		
	case EvAttr_Transposition:				// transposition
		return (cmd == EvtType_Sequence);

	case EvAttr_TSigBeatCount:
	case EvAttr_TSigBeatSize:
		return (cmd == EvtType_TimeSig);
		
	case EvAttr_VPos:
		return HasProperty( Prop_VertPos );

	case EvAttr_CountourLevel:				// level for countour events
	default:
		return false;
	}
}

// ---------------------------------------------------------------------------
// Get the value of an attribute.

int32 Event::GetAttribute( enum E_EventAttribute inAttr ) const
{
	uint8		cmd = Command();

	switch (inAttr) {
	case EvAttr_StartTime:					// start time
		return Start();

	case EvAttr_Duration:						// event duration
		if (HasProperty( Prop_Duration )) return Duration();
		return -1;
	
	case EvAttr_StopTime:						// event stop time
		if (HasProperty( Prop_Duration )) return Stop();
		return -1;

	case EvAttr_Type:						// event type
		return Command();

	case EvAttr_Selected:						// Whether it's selected or not
		return IsSelected();
	
	case EvAttr_Channel:						// virtual channel number
		if (HasProperty( Prop_Channel )) return GetVChannel();
		return -1;
	
	case EvAttr_Pitch:						// pitch attribute
		if (	cmd == EvtType_Note || cmd == EvtType_NoteOff)
			return note.pitch;
		else if (cmd == EvtType_PolyATouch)
			return aTouch.pitch;
		else return -1;

	case EvAttr_AttackVelocity:				// attack velocity
		if (	cmd == EvtType_Note || cmd == EvtType_NoteOff)
			return note.attackVelocity;
		else return -1;

	case EvAttr_ReleaseVelocity:				// release velocity
		if (	cmd == EvtType_Note || cmd == EvtType_NoteOff)
			return note.releaseVelocity;
		else return -1;

	case EvAttr_AfterTouch:					// aftertouch value
		if (	cmd == EvtType_PolyATouch) return aTouch.value;
		else return -1;

	case EvAttr_Program:						// program number attribute
		if (	cmd == EvtType_ProgramChange) return programChange.program;
		else return -1;

	case EvAttr_ProgramBank:					// program bank number
		if (	cmd == EvtType_ProgramChange)
			return (programChange.bankMSB << 7) | programChange.bankLSB;
		else return -1;

	case EvAttr_VPos:						// arbitrary vertical position
		if (HasProperty( Prop_VertPos )) return repeat.vPos;
		return -1;

	case EvAttr_ControllerNumber:				// controller number
		if (cmd == EvtType_Controller) return controlChange.controller;
		return -1;

	case EvAttr_ControllerValue8:				// 8-bit controller value
		if (cmd == EvtType_Controller) return controlChange.MSB;
		return -1;

	case EvAttr_ControllerValue16:			// 16-bit controller value
		if (cmd == EvtType_Controller)
			return (controlChange.MSB << 7) | controlChange.LSB;
		return -1;

	case EvAttr_BendValue:					// 16-bit pitch-bend value
		if (cmd == EvtType_PitchBend) return pitchBend.targetBend - 0x2000;
		return -1;

	case EvAttr_InitialBend:						// 16-bit pitch-bend value
		if (cmd == EvtType_PitchBend) return pitchBend.startBend - 0x2000;
		return -1;

	case EvAttr_UpdatePeriod:				// update period in ticks
		if (cmd == EvtType_PitchBend || cmd == EvtType_Controller || cmd == EvtType_ChannelATouch)
			return pitchBend.updatePeriod;
		return -1;

	case EvAttr_TempoValue:					// tempo value
		if (cmd == EvtType_Tempo) return tempo.newTempo;
		return -1;

	case EvAttr_RepeatCount:					// repeat count
		if (cmd == EvtType_Repeat) return repeat.repeatCount;
		return -1;

	case EvAttr_SequenceNumber:				// sequence number
		if (cmd == EvtType_Sequence) return sequence.sequence;
		return -1;

	case EvAttr_Transposition:				// transposition
		if (cmd == EvtType_Sequence) return sequence.transposition;
		return -1;

	case EvAttr_Modulation:					// Modulation value
		if (		cmd == EvtType_Controller
			&&	controlChange.controller == 1)
		{
			return (controlChange.MSB << 7) | controlChange.LSB;
		}
		return -1;

	case EvAttr_TSigBeatCount:
		if (cmd == EvtType_TimeSig) return sigChange.numerator;
		return -1;

	case EvAttr_TSigBeatSize:
		if (cmd == EvtType_TimeSig) return sigChange.denominator;
		return -1;

	case EvAttr_CountourLevel:				// level for countour events
	case EvAttr_DataSize:						// Size of extended data
	default:
		return -1;
	}
}

// ---------------------------------------------------------------------------
// Returns true if the event possesses the attribute.

bool Event::SetAttribute( enum E_EventAttribute inAttr, int32 inValue )
{
	uint8		cmd = Command();

	switch (inAttr) {
	case EvAttr_StartTime:					// start time
		SetStart( inValue );
		return true;

	case EvAttr_Duration:						// event duration
		if (HasProperty( Prop_Duration ))
		{
			SetDuration(1 > inValue ? 1 : inValue);
			return true;
		}
		return false;
	
	case EvAttr_StopTime:						// event stop time
		return false;						// Can't set stop -- must set duration

	case EvAttr_Type:						// event type
		SetCommand( CLAMP( EvtType_End, inValue, EvtType_Count - 1 ) );
		return true;

	case EvAttr_Selected:						// Whether it's selected or not
		SetSelected( inValue != 0 );
		return true;
	
	case EvAttr_Channel:						// virtual channel number
		if (HasProperty( Prop_Channel ))
			SetVChannel( CLAMP( 0L, inValue, Max_Destinations ) );
		return true;
	
	case EvAttr_Pitch:						// pitch attribute
		if (	cmd == EvtType_Note || cmd == EvtType_NoteOff)
		{
			note.pitch = CLAMP( 0L, inValue, 127 );
			return true;
		}
		else if (cmd == EvtType_PolyATouch)
		{
			aTouch.pitch = CLAMP( 0L, inValue, 127 );
			return true;
		}
		else return false;

	case EvAttr_AttackVelocity:				// attack velocity
		if (	cmd == EvtType_Note || cmd == EvtType_NoteOff)
		{
			note.attackVelocity = CLAMP( 1L, inValue, 127 );
	
			return true;
		}
		else return false;

	case EvAttr_ReleaseVelocity:				// release velocity
		if (	cmd == EvtType_Note || cmd == EvtType_NoteOff)
		{
			note.releaseVelocity = CLAMP( 0L, inValue, 127 );
			return true;
		}
		else return false;

	case EvAttr_AfterTouch:					// aftertouch value
		if (	cmd == EvtType_PolyATouch)
		{
			aTouch.value = CLAMP( 0L, inValue, 127 );
			return true;
		}
		else return false;

	case EvAttr_Program:						// program number attribute
		if (	cmd == EvtType_ProgramChange)
		{
			programChange.program = CLAMP( 0L, inValue, 127 );
			return true;
		}
		return false;

	case EvAttr_ProgramBank:					// program bank number
		if (	cmd == EvtType_ProgramChange)
		{
			inValue = CLAMP( 0L, inValue, 0x3fff );
			programChange.bankMSB = inValue >> 7;
			programChange.bankLSB = inValue & 0x7f;
			return true;
		}
		return false;

	case EvAttr_VPos:						// arbitrary vertical position
		if (HasProperty( Prop_VertPos )) repeat.vPos = CLAMP( 0L, inValue, 63 );
		return false;

	case EvAttr_ControllerNumber:				// controller number
		if (cmd == EvtType_Controller)
		{
			controlChange.controller = CLAMP( 0L, inValue, 127 );
		}
		return false;

	case EvAttr_ControllerValue8:				// 8-bit controller value
		if (cmd == EvtType_Controller)
		{
			controlChange.MSB = CLAMP( 0L, inValue, 127 );
			controlChange.LSB = 0;
		}
		return false;

	case EvAttr_ControllerValue16:			// 16-bit controller value
		if (	cmd == EvtType_Controller)
		{
			inValue = CLAMP( 0L, inValue, 0x3fff );
			controlChange.MSB = inValue >> 7;
			controlChange.LSB = inValue & 0x7f;
			return true;
		}
		return false;

	case EvAttr_BendValue:					// 16-bit pitch-bend value
		if (	cmd == EvtType_PitchBend)
		{
			pitchBend.targetBend = CLAMP( 0L, inValue + 0x2000, 0x3fff );
			if (Duration() == 0 || pitchBend.updatePeriod == 0)
				pitchBend.startBend = pitchBend.targetBend;
			return true;
		}
		return false;

	case EvAttr_InitialBend:					// 16-bit pitch-bend value
		if (	cmd == EvtType_PitchBend)
		{
			pitchBend.startBend = CLAMP( 0L, inValue + 0x2000, 0x3fff );
			return true;
		}
		return false;

	case EvAttr_UpdatePeriod:					// 16-bit pitch-bend value
		if (cmd == EvtType_PitchBend || cmd == EvtType_Controller || cmd == EvtType_ChannelATouch)
		{
			pitchBend.updatePeriod = CLAMP( 0L, inValue, 0x3fff );
			return true;
		}
		return false;

	case EvAttr_TempoValue:					// tempo value
		if (cmd == EvtType_Tempo)
		{
			tempo.newTempo = CLAMP( 1L, inValue, 1000*500 );
			return true;
		}
		return false;

	case EvAttr_RepeatCount:					// repeat count
		if (cmd == EvtType_Repeat)
		{
			repeat.repeatCount = CLAMP( 0L, inValue, 0x0ffffL );
			return true;
		}
		return false;

	case EvAttr_SequenceNumber:				// sequence number
		if (cmd == EvtType_Sequence)
		{
			sequence.sequence = CLAMP( 0L, inValue, 0x0ffffL );
			return true;
		}
		return false;

	case EvAttr_Transposition:				// transposition
		if (cmd == EvtType_Sequence)
		{
			sequence.transposition = CLAMP( -128L, inValue, 127L );
			return true;
		}
		return false;

	case EvAttr_Modulation:					// Modulation value
		if (		cmd == EvtType_Controller
			&&	controlChange.controller == 1)
		{
			inValue = CLAMP( 0L, inValue, 0x3fff );
			controlChange.MSB = inValue >> 7;
			controlChange.LSB = inValue & 0x7f;
			return true;
		}
		return false;

	case EvAttr_TSigBeatCount:
		if (cmd == EvtType_TimeSig)
		{
			sigChange.numerator = CLAMP( 1, inValue, 64 );
			return true;
		}
		return -1;

	case EvAttr_TSigBeatSize:
		if (cmd == EvtType_TimeSig)
		{
			sigChange.denominator = CLAMP( 0, inValue, 6 );
			return true;
		}
		return -1;

	case EvAttr_CountourLevel:				// level for countour events
	case EvAttr_DataSize:						// Size of extended data
	default:
		return false;
	}
}

// ---------------------------------------------------------------------------
// Function to quantize size of extended data to avoid frequent reallocation

inline int Quantize16( int length ) { return (length + 15) & ~15; }

// ---------------------------------------------------------------------------
// Function to allocate and deallocate EventExtra data.
// Does not preserve the data in the event.

void Event::ExtDataPtr::SetLength( size_t length )
{
		// If we're deleting the extra data
	if (length == 0)
	{
		Release();
	}
	else if (length > 0)
	{
		ExtendedEventData	*newData;
		int16				q = Quantize16( length );

			// If the old size and the new are quantized to the same size, then no need
			// to do anything except adjust the non-quantized length.
		if (data && Quantize16( data->length ) == q)
		{
			data->length = length;
			return;
		}

			// Allocate the new data.
		newData = (ExtendedEventData *)( new char[ q + sizeof(ExtendedEventData) ]);

			// Decrement use count of old data, and delete if it reaches 0
		if (data && data->useCount <= 0)
			delete (char *)data;

			// Fill in extended data record.
		data = newData;
		data->length = length;
		data->useCount = 1;
	}
}

// ---------------------------------------------------------------------------
// A table of information about controllers

ControllerInfo	controllerInfoTable[ 128 ] = {
	32, EvAttr_BankSelect,			"Bank Select",			// 0
	33, EvAttr_Modulation,			"Modulation wheel",		// 1
	34, EvAttr_BreathController,		"Breath Controller",		// 2
	35, EvAttr_ControllerValue16,		"Controller 3",			// 3

	36, EvAttr_FootController,		"Foot Controller",		// 4
	37, EvAttr_PortamentoTime,		"Portamento Time",		// 5
	38, EvAttr_DataEntry,			"Data Entry",			// 6
	39, EvAttr_ChannelVolume,		"Channel Volume",		// 7

	40, EvAttr_Balance,				"Balance",			// 8
	41, EvAttr_ControllerValue16,		"Controller 9",			// 9
	42, EvAttr_Pan,				"Pan",				// 10
	43, EvAttr_ExpressionCtl,		"Expression Ctl.",		// 11

	44, EvAttr_ControllerValue16,		"Effect Control 1",		// 12
	45, EvAttr_ControllerValue16,		"Effect Control 2",		// 13
	46, EvAttr_ControllerValue16,		"Controller 14",		// 14
	47, EvAttr_ControllerValue16,		"Controller 15",		// 15

	48, EvAttr_ControllerValue16,		"General Purpose #1",	// 16
	49, EvAttr_ControllerValue16,		"General Purpose #2",	// 17
	50, EvAttr_ControllerValue16,		"General Purpose #3",	// 18
	51, EvAttr_ControllerValue16,		"General Purpose #4",	// 19

	52, EvAttr_ControllerValue16,		"Controller 20",		// 20
	53, EvAttr_ControllerValue16,		"Controller 21",		// 21
	54, EvAttr_ControllerValue16,		"Controller 22",		// 22
	55, EvAttr_ControllerValue16,		"Controller 23",		// 23

	56, EvAttr_ControllerValue16,		"Controller 24",		// 24
	57, EvAttr_ControllerValue16,		"Controller 25",		// 25
	58, EvAttr_ControllerValue16,		"Controller 26",		// 26
	59, EvAttr_ControllerValue16,		"Controller 27",		// 27

	60, EvAttr_ControllerValue16,		"Controller 28",		// 28
	61, EvAttr_ControllerValue16,		"Controller 29",		// 29
	62, EvAttr_ControllerValue16,		"Controller 30",		// 30
	63, EvAttr_ControllerValue16,		"Controller 31",		// 31
	
	255,EvAttr_BankSelect,			"Bank Select LSB",		// 32
	255,EvAttr_Modulation,			"Modulation wheel LSB",	// 33
	255,EvAttr_BreathController,		"Breath Controller LSB",	// 34
	255,EvAttr_ControllerValue16,	"Controller 3 LSB",		// 35

	255,EvAttr_FootController,		"Foot Controller LSB",	// 36
	255,EvAttr_PortamentoTime,		"Portamento Time LSB",	// 37
	255,EvAttr_DataEntry,			"Data Entry LSB",		// 38
	255,EvAttr_ChannelVolume,		"Channel Volume LSB",	// 39

	255,EvAttr_Balance,			"Balance LSB",			// 40
	255,EvAttr_ControllerValue16,	"Controller 9 LSB",		// 41
	255,EvAttr_Pan,				"Pan LSB",			// 42
	255,EvAttr_ExpressionCtl,		"Expression Ctl. LSB",	// 43

	255,EvAttr_ControllerValue16,	"Effect Control 1 LSB",	// 44
	255,EvAttr_ControllerValue16,	"Effect Control 2 LSB",	// 45
	255,EvAttr_ControllerValue16,	"Controller 14 LSB",		// 46
	255,EvAttr_ControllerValue16,	"Controller 15 LSB",		// 47

	255,EvAttr_ControllerValue16,	"General Purpose #1 LSB",// 48
	255,EvAttr_ControllerValue16,	"General Purpose #2 LSB",// 49
	255,EvAttr_ControllerValue16,	"General Purpose #3 LSB",// 50
	255,EvAttr_ControllerValue16,	"General Purpose #4 LSB",// 51

	255,EvAttr_ControllerValue16,	"Controller 20 LSB",		// 52
	255,EvAttr_ControllerValue16,	"Controller 21 LSB",		// 53
	255,EvAttr_ControllerValue16,	"Controller 22 LSB",		// 54
	255,EvAttr_ControllerValue16,	"Controller 23 LSB",		// 55

	255,EvAttr_ControllerValue16,	"Controller 24 LSB",		// 56
	255,EvAttr_ControllerValue16,	"Controller 25 LSB",		// 57
	255,EvAttr_ControllerValue16,	"Controller 26 LSB",		// 58
	255,EvAttr_ControllerValue16,	"Controller 27 LSB",		// 59

	255,EvAttr_ControllerValue16,	"Controller 28 LSB",		// 60
	255,EvAttr_ControllerValue16,	"Controller 29 LSB",		// 61
	255,EvAttr_ControllerValue16,	"Controller 30 LSB",		// 62
	255,EvAttr_ControllerValue16,	"Controller 31 LSB",		// 63
	
	64, EvAttr_DamperPedal,		"Damper Pedal",		// 64
	65, EvAttr_PortamentoOnOff,		"Portamento On/Off",	// 65
	66, EvAttr_Sostenuto,			"Sostenuto",			// 66
	67, EvAttr_SoftPedal,			"Soft Pedal",			// 67

	68, EvAttr_LegatoFootswitch,		"Legato Footswitch", 	// 68
	69, EvAttr_Hold2,				"Hold 2",				// 69
	70, EvAttr_ControllerValue8,		"Sound Controller #1",	// 70
	71, EvAttr_ControllerValue8,		"Sound Controller #2",	// 71

	72, EvAttr_ControllerValue8,		"Sound Controller #3",	// 72
	73, EvAttr_ControllerValue8,		"Sound Controller #4",	// 73
	74, EvAttr_ControllerValue8,		"Sound Controller #5",	// 74
	75, EvAttr_ControllerValue8,		"Sound Controller #6",	// 75

	76, EvAttr_ControllerValue8,		"Sound Controller #7",	// 76
	77, EvAttr_ControllerValue8,		"Sound Controller #8",	// 77
	78, EvAttr_ControllerValue8,		"Sound Controller #9",	// 78
	79, EvAttr_ControllerValue8,		"Sound Controller #10",	// 79
	
	80, EvAttr_ControllerValue8,		"General Purpose #5",	// 80
	81, EvAttr_ControllerValue8,		"General Purpose #6",	// 81
	82, EvAttr_ControllerValue8,		"General Purpose #7",	// 82
	83, EvAttr_ControllerValue8,		"General Purpose #8",	// 83
	
	84, EvAttr_PortamentoControl,	"Portamento Control",	// 84
	85, EvAttr_ControllerValue8,		"Controller 85",		// 85
	86, EvAttr_ControllerValue8,		"Controller 86",		// 86
	87, EvAttr_ControllerValue8,		"Controller 87",		// 87

	88, EvAttr_ControllerValue8,		"Controller 88",		// 88
	89, EvAttr_ControllerValue8,		"Controller 89",		// 89
	90, EvAttr_ControllerValue8,		"Controller 90",		// 90
	91, EvAttr_EffectsDepth,			"Effects 1 Depth",		// 91

	92, EvAttr_EffectsDepth,			"Effects 2 Depth",		// 92
	93, EvAttr_EffectsDepth,			"Effects 3 Depth",		// 93
	94, EvAttr_EffectsDepth,			"Effects 4 Depth",		// 94
	95, EvAttr_EffectsDepth,			"Effects 5 Depth",		// 95

	96, EvAttr_DataIncrement,		"Data Increment",		// 96
	97, EvAttr_DataDecrement,		"Data Decrement",		// 97
	255,EvAttr_ParamNumber,		"Non Reg. Param LSB",	// 98
	98, EvAttr_ParamNumber,		"Non Registered Param",	// 99

	255,EvAttr_ParamNumber,		"Registered Param LSB",	// 100
	100,EvAttr_ParamNumber,		"Registered Param",	// 101
	102, EvAttr_ControllerValue8,		"Controller 102",		// 102
	103, EvAttr_ControllerValue8,		"Controller 103",		// 103

	104, EvAttr_ControllerValue8,		"Controller 104",		// 104
	105, EvAttr_ControllerValue8,		"Controller 105",		// 105
	106, EvAttr_ControllerValue8,		"Controller 106",		// 106
	107, EvAttr_ControllerValue8,		"Controller 107",		// 107

	108, EvAttr_ControllerValue8,		"Controller 108",		// 108
	109, EvAttr_ControllerValue8,		"Controller 109",		// 109
	110, EvAttr_ControllerValue8,		"Controller 110",		// 110
	111, EvAttr_ControllerValue8,		"Controller 111",		// 111

	112, EvAttr_ControllerValue8,		"Controller 112",		// 112
	113, EvAttr_ControllerValue8,		"Controller 113",		// 113
	114, EvAttr_ControllerValue8,		"Controller 114",		// 114
	115, EvAttr_ControllerValue8,		"Controller 115",		// 115

	116, EvAttr_ControllerValue8,		"Controller 116",		// 116
	117, EvAttr_ControllerValue8,		"Controller 117",		// 117
	118, EvAttr_ControllerValue8,		"Controller 118",		// 118
	119, EvAttr_ControllerValue8,		"Controller 119",		// 119

	120, EvAttr_None,				"All Sound Off",		// 120
	121, EvAttr_None,				"Reset All Controllers",	// 121
	122, EvAttr_LocalControl,		"Local Control",		// 122
	123, EvAttr_None,				"All Notes Off",			// 123

	124, EvAttr_None,				"Omni Mode Off",		// 124
	125, EvAttr_None,				"Omni Mode On",		// 125
	126, EvAttr_MonoNumChannels,	"Mono Mode On",		// 126
	127, EvAttr_None,				"Poly Mode On",		// 127
};

