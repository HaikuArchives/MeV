/* ===================================================================== *
 * EventOp.cpp (MeV/Engine)
 * ===================================================================== */

#include "EventOp.h"

#include "MeVPlugin.h"
#include "StdEventOps.h"
#include "MathUtils.h"
#include <stdio.h>
#pragma export on
EventOp::EventOp( MeVPlugIn *inCreator )
{
	creator = inCreator;
}

const char *EventOp::CreatorName() const
{
	return creator ? creator->Name() : "MeV";
}
#pragma export off

// ---------------------------------------------------------------------------
// A null operation

void NullOp::operator()( Event &, TClockType ) {}

// ---------------------------------------------------------------------------
// An operator which modifies pitch in a relative fashion

void PitchOffsetOp::operator()( Event &ev, TClockType )
{
	if (ev.Command() == EvtType_Note)
	{
		ev.note.pitch =
			(uint8)CLAMP(0, ((int16)ev.note.pitch) + deltaPitch, 127);
	}
}

// ---------------------------------------------------------------------------
// An operator which modifies attack velocity

class AttackVelOffsetOp : public EventOp {
	void operator()( Event &, TClockType );

public:
	int16			delta;

	AttackVelOffsetOp( int16 dp ) { delta = dp; }
	const char *UndoDescription() const { return "Change Attack Velocity"; }
};

void AttackVelOffsetOp::operator()( Event &ev, TClockType )
{
	if (ev.Command() == EvtType_Note)
	{
		ev.note.attackVelocity =
			(uint8)CLAMP(0, ((int16)ev.note.attackVelocity) + delta, 127);
	}
}

// ---------------------------------------------------------------------------
// An operator which modifies release velocity

class ReleaseVelOffsetOp : public EventOp {
	void operator()( Event &, TClockType );

public:
	int16			delta;

	ReleaseVelOffsetOp( int16 dp ) { delta = dp; }
	const char *UndoDescription() const { return "Change Release Velocity"; }
};

void ReleaseVelOffsetOp::operator()( Event &ev, TClockType )
{
	if (ev.Command() == EvtType_Note)
	{
		ev.note.releaseVelocity =
			(uint8)CLAMP(0, ((int16)ev.note.releaseVelocity) + delta, 127);
	}
}

// ---------------------------------------------------------------------------
// An operator which modifies target pitch bend

void BendOffsetOp::operator()( Event &ev, TClockType )
{
	if (ev.Command() == EvtType_PitchBend)
	{
		ev.pitchBend.targetBend =
			(uint16)CLAMP(0, ((int16)ev.pitchBend.targetBend) + delta, 0x3fff);
	}
}

// ---------------------------------------------------------------------------
// An operator which modifies initial pitch bend

void IBendOffsetOp::operator()( Event &ev, TClockType )
{
	if (ev.Command() == EvtType_PitchBend)
	{
		ev.pitchBend.startBend =
			(uint16)CLAMP(0, ((int16)ev.pitchBend.startBend) + delta, 0x3fff);
	}
}

// ---------------------------------------------------------------------------
// An operator which modifies pitch bend

class SetUpdateRateOp : public EventOp {
	void operator()( Event &, TClockType );

public:
	int32			v;

	SetUpdateRateOp( int16 inV ) { v = (uint16)CLAMP( (uint16)0, inV, (uint16)0x3fff ); }
	const char *UndoDescription() const { return "Change Update Rate"; }
};

void SetUpdateRateOp::operator()( Event &ev, TClockType )
{
	if (	ev.Command() == EvtType_PitchBend
		|| ev.Command() == EvtType_Controller
		|| ev.Command() == EvtType_ChannelATouch)
	{
		ev.pitchBend.updatePeriod = v;
	}
}

// ---------------------------------------------------------------------------
// An operator which sets the repeat count of a repeat event

class SetRepeatOp : public EventOp {
	void operator()( Event &, TClockType );

public:
	uint16			v;

	SetRepeatOp( uint16 inV ) { v = (uint16)CLAMP( (uint16)0, inV, (uint16)0xffff ); }
	const char *UndoDescription() const { return "Set Repeat Count"; }
};

void SetRepeatOp::operator()( Event &ev, TClockType )
{
	if (ev.Command() == EvtType_Repeat) ev.repeat.repeatCount = v;
}

// ---------------------------------------------------------------------------
// An operator which sets the program value of a program change

class SetProgramOp : public EventOp {
	void operator()( Event &, TClockType );

public:
	uint16			v;

	SetProgramOp( uint16 inV ) { v = (uint16)CLAMP( (uint16)0, inV, (uint16)0x7f ); }
	const char *UndoDescription() const { return "Set Program"; }
};

void SetProgramOp::operator()( Event &ev, TClockType )
{
	if (ev.Command() == EvtType_ProgramChange) ev.programChange.program = v;
}

// ---------------------------------------------------------------------------
// An operator which sets the program bank of a program change

class SetBankOp : public EventOp {
	void operator()( Event &, TClockType );

public:
	uint16			v;

	SetBankOp( uint16 inV ) { v = inV; }
	const char *UndoDescription() const { return "Set Program Bank"; }
};

void SetBankOp::operator()( Event &ev, TClockType )
{
	if (ev.Command() == EvtType_ProgramChange)
	{
		ev.programChange.bankMSB = v >> 7;
		ev.programChange.bankLSB = v & 0x7f;
	}
}

// ---------------------------------------------------------------------------
// An operator which sets the beat count of a timesig event

class SetBeatCountOp : public EventOp {
	void operator()( Event &, TClockType );

public:
	uint16			v;

	SetBeatCountOp( uint16 inV ) { v = (uint16)CLAMP( (uint16)1, inV, (uint16)64 ); }
	const char *UndoDescription() const { return "Set Time Signature"; }
};

void SetBeatCountOp::operator()( Event &ev, TClockType )
{
	if (ev.Command() == EvtType_TimeSig) ev.sigChange.numerator = v;
}

// ---------------------------------------------------------------------------
// An operator which sets the beat count of a timesig event

class SetScaledTempoOp : public EventOp {
	void operator()( Event &, TClockType );

public:
	uint32			v;

	SetScaledTempoOp( uint32 inV ) { v = (uint32)CLAMP( 1L, inV, 1000*500 ); }
	const char *UndoDescription() const { return "Set Tempo"; }
};

void SetScaledTempoOp::operator()( Event &ev, TClockType )
{
	if (ev.Command() == EvtType_Tempo) ev.tempo.newTempo = v;
}

// ---------------------------------------------------------------------------
// An operator which sets the beat size of a timesig event

class SetBeatSizeOp : public EventOp {
	void operator()( Event &, TClockType );

public:
	uint16			v;

	SetBeatSizeOp( uint16 inV ) { v = (uint16)CLAMP( (uint16)0, inV, (uint16)6 ); }
	const char *UndoDescription() const { return "Set Time Signature"; }
};

void SetBeatSizeOp::operator()( Event &ev, TClockType )
{
	if (ev.Command() == EvtType_TimeSig) ev.sigChange.denominator = v;
}

// ---------------------------------------------------------------------------
// An operator which sets the sequence number

class SetSequenceNumOp : public EventOp {
	void operator()( Event &, TClockType );

public:
	uint16			v;

	SetSequenceNumOp( uint16 inV ) { v = inV; }
	const char *UndoDescription() const { return "Set Part"; }
};

void SetSequenceNumOp::operator()( Event &ev, TClockType )
{
	if (ev.Command() == EvtType_Sequence)
	{
		ev.sequence.sequence = v;
	}
}

// ---------------------------------------------------------------------------
// An operator which modifies transposition

class TransposeOffsetOp : public EventOp {
	void operator()( Event &, TClockType );

public:
	int16			delta;

	TransposeOffsetOp( int16 dp ) { delta = dp; }
	const char *UndoDescription() const { return "Change Transpose"; }
};

void TransposeOffsetOp::operator()( Event &ev, TClockType )
{
	if (ev.Command() == EvtType_Sequence)
	{
		ev.sequence.transposition =
			(int8)CLAMP( -128, ((int16)ev.sequence.transposition) + delta, 127 );
	}
}

// ---------------------------------------------------------------------------
// An operator which modifies explicit vertical position

void VPosOffsetOp::operator()( Event &ev, TClockType )
{
	switch (ev.Command()) {
	case EvtType_Repeat:
	case EvtType_Sequence:
	case EvtType_TimeSig:
	case EvtType_ProgramChange:
	case EvtType_Tempo:
		ev.repeat.vPos =
			(uint8)CLAMP( 0, ((int16)ev.repeat.vPos) + deltaPos, 63 );
		break;
	}
}

// ---------------------------------------------------------------------------
// A function which created an event operation given an attribute type

EventOp *CreateOffsetOp( enum E_EventAttribute inAttr, int32 inDelta, int32 inValue )
{
	switch (inAttr) {
	case EvAttr_StartTime:								// start time
	case EvAttr_Duration:								// event duration
	case EvAttr_StopTime:								// event stop time
	case EvAttr_Channel:								// virtual channel number
		break;

	case EvAttr_Pitch:			return new PitchOffsetOp( inDelta );
	case EvAttr_AttackVelocity:	return new AttackVelOffsetOp( inDelta );
	case EvAttr_ReleaseVelocity:	return new ReleaseVelOffsetOp( inDelta );

	case EvAttr_AfterTouch:								// aftertouch value
		break;
	case EvAttr_Program:		return new SetProgramOp( inValue ); // program number attribute
	case EvAttr_ProgramBank:	return new SetBankOp( inValue );	// program bank number
	case EvAttr_VPos: 			return new VPosOffsetOp( inDelta );
		
	case EvAttr_ControllerNumber:						// controller number
	case EvAttr_ControllerValue8:						// 8-bit controller value
	case EvAttr_ControllerValue16:						// 16-bit controller value
		break;

	case EvAttr_TempoValue:	return new SetScaledTempoOp( inValue );	// tempo value

	case EvAttr_BendValue:		return new BendOffsetOp( inDelta );		// 16-bit pitch-bend value
	case EvAttr_InitialBend:		return new IBendOffsetOp( inDelta );		// 16-bit initial pitch-bend value
	case EvAttr_UpdatePeriod:	return new SetUpdateRateOp( inValue );	// 16-bit update period in ticks
		
	case EvAttr_RepeatCount:		return new SetRepeatOp( inValue );

	case EvAttr_SequenceNumber:	return new SetSequenceNumOp( inValue );
	case EvAttr_Transposition:	return new TransposeOffsetOp( inDelta );
	case EvAttr_CountourLevel:						// level for countour events
// case EvAttr_DataSize:								// Size of extended data
		break;

	case EvAttr_TSigBeatCount:	return new SetBeatCountOp( inValue );
	case EvAttr_TSigBeatSize:		return new SetBeatSizeOp( inValue );
	default:					return NULL;
	}
}
