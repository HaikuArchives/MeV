/* ===================================================================== *
 * EventStack.h (MeV/Engine)
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
 *  a prioritized stack of events, sorted by time
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

#ifndef __C_EventStack_H__
#define __C_EventStack_H__

#include "Event.h"

class CEventStack {
	friend class CEventStackIterator;

private:
	Event				*current,					// top item of stack
						*max;						// max item of stack
	Event				stack[ 256 ];				// the stack of items

public:
		// ---------- Consructor
	CEventStack()
	{
		current = stack;
		max = &stack[sizeof(stack)/sizeof(stack[0])];
	}

		// ---------- 	Stack access functions
	bool Push( const Event &ev );					// add event to stack
	bool Push( const Event &ev, long t );			// add event to stack at time t
	bool Pop( Event &ev, long time );				// pop event to stack if time reached
	bool Pop( Event &ev );						// pop event from stack always

	bool NextTime( long &next );					// return time of top event

		// Test if stack empty
	bool Empty() const { return (current <= stack); }

		// push a list of events (all or none)
	bool PushList( Event *eventList, int16 count, long startTime );
};

	/** 	A class used in selectively filtering events from the event stack. */
class CEventStackIterator {
	Event				*read,
						*write;
	CEventStack			&stack;

public:

		/**	Constructor -- takes a stack as an argument. */
	CEventStackIterator( CEventStack &inStack ) : stack( inStack )
	{
		read = write = stack.stack;
	}
	
		/**	Destructor -- cleans up all pending deletes. */
	~CEventStackIterator()
	{
		if (read > stack.current) read = stack.current;
		if (read > write && stack.current > read)
		{
			Event::Relocate( write, read, stack.current - read );
			stack.current = read;
		}
	}

		/**	Returns pointer to current event, if any. */
	Event *Current()
	{
		return read < stack.current ? read : NULL;
	}

		/**	Skips to the next event. */
	bool Next()
	{
		if (read >= stack.current) return false;
	
		if (read > write)
		{
			Event::Relocate( write, read, 1 );
		}
		read++;
		write++;
		return true;
	}

		/**	Removes the current event and skips to the next one. */
	void Remove()
	{
		if (read < stack.current)
		{
			Event::Destruct( read, 1 );
			read++;
		}
	}
};

// ---------------------------------------------------------------------------
// returns TRUE if time b is creater than time a
// correctly handles the case of wraparound time

inline bool IsTimeGreater( int32 a, int32 b )
{
	return ((int32)(b - a) > 0);
}

#endif /* __C_EventStack_H__ */
