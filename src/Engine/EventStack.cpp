/* ===================================================================== *
 * EventStack.cpp (MeV/Engine)
 * ===================================================================== */

#include "Player.h"
#include "EventStack.h"

	// Stack access functions
bool CEventStack::Push( const Event &ev )
{
	Event		*search;
	
	thePlayer.CheckLock();

	if (current >= max) return false;			// if stack is full then bail

		// Insert event onto stack in time order

		// While search pointer is greater than bottom of stack and the
		// element being displaced is earlier, then bubble old event up
		// and continue searching.
	search = current;
	while (		search > stack
			&&	IsTimeGreater( search[ -1 ].stack.start, ev.stack.start ))
	{
		search--;
	}

	if (search < current)
		Event::Relocate( search + 1, search, current - search );
	Event::Construct( search, &ev, 1 );
	current++;								// bump stack size

	return true;
}

	// Stack access functions
bool CEventStack::Push( const Event &ev, long time )
{
	Event		*search;

	thePlayer.CheckLock();

	if (current >= max) return false;			// if stack is full then bail

		// Insert event onto stack in time order

		// While search pointer is greater than bottom of stack and the
		// element being displaced is earlier, then bubble old event up
		// and continue searching.
	search = current;
	while (		search > stack
			&&	IsTimeGreater( search[ -1 ].stack.start, time ))
	{
		search--;
	}

	if (search < current)
		Event::Relocate( search + 1, search, current - search );
	Event::Construct( search, &ev, 1 );
	search->stack.start = time;
	current++;								// bump stack size

	return true;
}

	// pop event to stack if time reached
bool CEventStack::Pop( Event &ev, long time )
{
	thePlayer.CheckLock();

		// If stack has no items, or the top item is greater than the
		// current time, then return nothing.
	if (current <= stack || IsTimeGreater( time, current[ -1 ].stack.start) )
	{
		return false;
	}

		// pop one item and return it.
	Event::Destruct( &ev, 1 );
	Event::Relocate( &ev, --current, 1 );
	return true;
}

	// pop event from stack always
bool CEventStack::Pop( Event &ev )
{
	thePlayer.CheckLock();

		// If stack has no items, then nothing to pop
	if (current <= stack) return false;

		// pop one item and return it.
	Event::Destruct( &ev, 1 );
	Event::Relocate( &ev, --current, 1 );
	return true;
}

	// Retrieve time of next event, if any
bool CEventStack::NextTime( long &next )
{
	thePlayer.CheckLock();

		// If stack has no items, then nothing to pop
	if (current <= stack) return false;

		// return the time of the next item
	next = current[ -1 ].stack.start;
	return true;
}

// ---------------------------------------------------------------------------
// Push a list of events

bool CEventStack::PushList( Event *eventList, int16 count, long startTime )
{
	thePlayer.CheckLock();
	if (current + count > max) return false;

	eventList += count;
	while (count--)
	{
		--eventList;
		eventList->stack.start += startTime;
		Push( *eventList );
	}
	return true;
}
