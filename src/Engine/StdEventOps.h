/* ===================================================================== *
 * StdEventOps.h (MeV/Engine)
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
 *  Defines a few "standardized" event operations
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

#ifndef __StdEventOps_H__
#define __StdEventOps_H__

#include "EventOp.h"
//#include "sylvan/apptypes.h"

/** ---------------------------------------------------------------------------
	A null operation
*/

class NullOp : public EventOp {
	void operator()( Event &, TClockType );
	const char *UndoDescription() const { return "Nothing"; }
};

/** ---------------------------------------------------------------------------
	An operator which modifies pitch in a relative fashion
*/

class PitchOffsetOp : public EventOp {
	void operator()( Event &, TClockType );

public:
	int16			deltaPitch;

	PitchOffsetOp( int16 dp ) { deltaPitch = dp; }
	const char *UndoDescription() const { return "Change Pitch"; }
};

/** ---------------------------------------------------------------------------
	An operator which modifies start time
*/

class TimeOffsetOp : public EventOp {
private:

	int32			deltaTime;
	void operator()( Event &ev, TClockType ) { ev.common.start += deltaTime; }
public:

	virtual bool CanModifyOrder() const { return deltaTime != 0; }
	TimeOffsetOp( int32 inDeltaTime ) { deltaTime = inDeltaTime; }
	const char *UndoDescription() const { return "Change Start Time"; }
};

/** ---------------------------------------------------------------------------
	An operator which modifies duration
*/

class DurationOffsetOp : public EventOp {
private:

	int32			deltaTime;

	void operator()( Event &ev, TClockType )
	{
		if (ev.HasProperty( Event::Prop_Duration ))
		{
			ev.SetDuration((ev.Duration() + deltaTime) > 1 ? (ev.Duration() + deltaTime) : 1);
		}
	}

public:
	DurationOffsetOp( int32 inDeltaTime ) { deltaTime = inDeltaTime; }
	const char *UndoDescription() const { return "Change Duration"; }
};

/** ---------------------------------------------------------------------------
	An operator which modifies channel
*/

class ChannelOp : public EventOp {
private:

	uint8			channel;

	void operator()( Event &ev, TClockType )
	{
		if (ev.HasProperty( Event::Prop_Channel )) ev.SetVChannel( channel );
	}

public:
	ChannelOp( uint8 inChannel ) { channel = inChannel; }
	const char *UndoDescription() const { return "Channel Change"; }
};

/** ---------------------------------------------------------------------------
	An operator which modifies the vertical position of an event which has
	explicit vertical position.
*/

class VPosOffsetOp : public EventOp {
	void operator()( Event &, TClockType );

public:
	int16			deltaPos;

	VPosOffsetOp( int16 dp ) { deltaPos = dp; }
	const char *UndoDescription() const { return "Change Position"; }
};

/** ---------------------------------------------------------------------------
	A composite of two operations
*/

class PairOp : public EventOp {
	EventOp			*op1,
					*op2;
					
public:
	void operator()( Event &ev, TClockType cl ) { if (op1) (*op1)( ev, cl ); if (op2) (*op2)( ev, cl ); }
	PairOp( EventOp *a, EventOp *b ) : op1( a ), op2( b )
	{
		if (op1) op1->Acquire();
		if (op2) op2->Acquire();
	}
	
	~PairOp()
	{
		CRefCountObject::Release( op1 );
		CRefCountObject::Release( op2 );
	}

	virtual bool CanModifyOrder() const
	{
		return (op1 && op1->CanModifyOrder()) || (op2 && op2->CanModifyOrder());
	}
	const char *UndoDescription() const { return "Combined Operation"; }
};

// ---------------------------------------------------------------------------
// An operator which modifies target pitch bend

class BendOffsetOp : public EventOp {
	void operator()( Event &, TClockType );

public:
	int32			delta;

	BendOffsetOp( int16 dp ) { delta = dp; }
	const char *UndoDescription() const { return "Change Pitch Bend"; }
};

// ---------------------------------------------------------------------------
// An operator which modifies initial pitch bend

class IBendOffsetOp : public EventOp {
	void operator()( Event &, TClockType );

public:
	int32			delta;

	IBendOffsetOp( int16 dp ) { delta = dp; }
	const char *UndoDescription() const { return "Change Pitch Bend"; }
};

/** ---------------------------------------------------------------------------
	Allocates a new EventOp based on an attribute code.
*/

//EventOp *NewAttrOp( enum E_EventAttribute attr, int16 newVal );
EventOp *CreateOffsetOp( enum E_EventAttribute inAttr, int32 inDelta, int32 inValue );

#endif /* __StdEventOps_H__ */
