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
 * History:
 *	1997		Talin
 *		Original implementation
 *	04/08/2000	cell
 *		General cleanup in preparation for initial SourceForge checkin
 * ===================================================================== */

#ifndef __C_EventStack_H__
#define __C_EventStack_H__

#include "Event.h"
#include "Time.h"

/**
 *	A prioritized stack of events, sorted by time.
 *	@author Talin, Christopher Lenz
 */
class CEventStack
{
	friend class CEventStackIterator;

public:							// Constructor/Destructor

	/** Constructor.
	 *	@param	capacity	Determines how many events the stack is able
	 * 						to hold at a time. Default is 256.
	 */
								CEventStack(
									long capacity = 256);

	/** Destructor. */
								~CEventStack();

public:							// Accessors

	/** Test if stack empty. */
	bool						Empty() const;

	/** Return time of top event. */
	bool						NextTime(
									long *outTime) const;

public:							// Operations

	/** Add event to stack. */
	bool						Push(
									const CEvent &ev);

	/** Add event to stack at a specfic time. */
	bool						Push(
									const CEvent &ev,
									CTime time);

	/** push a list of events (all or none). */
	bool						PushList(
									CEvent *eventList,
									int16 count,
									CTime offset);

	/** Pop event from stack. */
	bool						Pop(
									CEvent &ev);

	/** Pop event to stack if time reached. */
	bool						Pop(
									CEvent &ev,
									CTime time);

private:						// Instance Data

	/** The stack of items. */
	CEvent *					m_stack;

	/** Top item of stack. */
	CEvent *					m_current;

	/** Max item of stack. */
	CEvent *					m_max;
};

/**	A class used in selectively filtering events from the event stack. */
class CEventStackIterator
{

public:							// Constructor/Destructor

	/**	Constructor -- takes a stack as an argument. */
								CEventStackIterator(
									CEventStack &stack);

	/**	Destructor -- cleans up all pending deletes. */
								~CEventStackIterator();

public:							// Accessors

	/**	Returns pointer to current event, if any. */
	CEvent *					Current() const;

public:							// Operations

	/**	Skips to the next event. */
	bool						Next();

	/**	Removes the current event and skips to the next one. */
	void						Remove();

private:						// Instance Data

	CEvent *					m_read;

	CEvent *					m_write;

	CEventStack &				m_stack;
};

// ---------------------------------------------------------------------------
// returns TRUE if time b is creater than time a
// correctly handles the case of wraparound time

inline bool IsTimeGreater( int32 a, int32 b )
{
	return ((int32)(b - a) > 0);
}

#endif /* __C_EventStack_H__ */
