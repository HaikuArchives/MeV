/* ===================================================================== *
 * EventStack.cpp (MeV/Engine)
 * ===================================================================== */

#include "EventStack.h"

#include "Player.h"

// Support Kit
#include <Debug.h>

// Debugging Macros
#define D_ALLOC(x) //PRINT(x)		// Constructor/Destructor
#define D_ACCESS(x) //PRINT(x)		// Accessors
#define D_OPERATION(x) //PRINT(x)	// Operations

// ---------------------------------------------------------------------------
// Constructor/Destructor

CEventStack::CEventStack()
{
	D_ALLOC(("CEventStack::CEventStack()\n"));

	m_current = m_stack;
	m_max = &m_stack[sizeof(m_stack) / sizeof(Event)];
}

// ---------------------------------------------------------------------------
// Accessors

bool
CEventStack::Empty() const
{
	D_ACCESS(("CEventStack::Empty()\n"));

	return m_current <= m_stack;
}

bool
CEventStack::NextTime(
	long *outTime) const
{
	D_ACCESS(("CEventStack::NextTime()\n"));

	thePlayer.CheckLock();

	// If stack has no items, then nothing to pop
	if (m_current <= m_stack)
		return false;

	// return the time of the next item
	*outTime = m_current[-1].stack.start;
	return true;
}

// ---------------------------------------------------------------------------
// Operations

bool
CEventStack::Push(
	const Event &ev)
{
	D_OPERATION(("CEventStack::Push()\n"));

	thePlayer.CheckLock();

	// if stack is full then bail
	if (m_current >= m_max)
		return false;

	// Insert event onto stack in time order

	// While search pointer is greater than bottom of stack and the
	// element being displaced is earlier, then bubble old event up
	// and continue searching.
	Event *search = m_current;
	while ((search > m_stack) && IsTimeGreater(search[-1].stack.start, ev.stack.start))
		search--;

	if (search < m_current)
		Event::Relocate(search + 1, search, m_current - search);
	Event::Construct(search, &ev, 1);
	m_current++;

	return true;
}

bool
CEventStack::Push(
	const Event &ev,
	long time)
{
	D_OPERATION(("CEventStack::Push(time)\n"));

	thePlayer.CheckLock();

	// if stack is full then bail
	if (m_current >= m_max)
		return false;

	// Insert event onto stack in time order

	// While search pointer is greater than bottom of stack and the
	// element being displaced is earlier, then bubble old event up
	// and continue searching.
	Event *search = m_current;
	while ((search > m_stack) && IsTimeGreater(search[-1].stack.start, time))
		search--;

	if (search < m_current)
		Event::Relocate(search + 1, search, m_current - search);
	Event::Construct(search, &ev, 1);
	search->stack.start = time;
	m_current++;

	return true;
}

bool
CEventStack::PushList(
	Event *list,
	int16 count,
	long startTime)
{
	D_OPERATION(("CEventStack::PushList()\n"));

	thePlayer.CheckLock();

	if ((m_current + count) > m_max)
		return false;

	list += count;
	while (count--)
	{
		--list;
		list->stack.start += startTime;
		Push(*list);
	}
	return true;
}

bool
CEventStack::Pop(
	Event &ev)
{
	D_OPERATION(("CEventStack::Pop()\n"));

	thePlayer.CheckLock();

	// If stack has no items, then nothing to pop
	if (m_current <= m_stack)
		return false;

	// pop one item and return it.
	Event::Destruct(&ev, 1);
	Event::Relocate(&ev, --m_current, 1);

	return true;
}

bool
CEventStack::Pop(
	Event &ev,
	long time)
{
	D_OPERATION(("CEventStack::Pop(time)\n"));

	thePlayer.CheckLock();

	// If stack has no items, or the top item is greater than the
	// current time, then return nothing.
	if ((m_current <= m_stack) || IsTimeGreater(time, m_current[-1].stack.start))
		return false;

	// pop one item and return it.
	Event::Destruct(&ev, 1);
	Event::Relocate(&ev, --m_current, 1);

	return true;
}

// ---------------------------------------------------------------------------
// CEventStackIterator Implementation

CEventStackIterator::CEventStackIterator(
	CEventStack &stack)
	:	m_stack(stack)
{
	m_read = m_write = m_stack.m_stack;
}

CEventStackIterator::~CEventStackIterator()
{
	if (m_read > m_stack.m_current)
		m_read = m_stack.m_current;
	if ((m_read > m_write) && (m_stack.m_current > m_read))
	{
		Event::Relocate(m_write, m_read, m_stack.m_current - m_read);
		m_stack.m_current = m_read;
	}
}

Event *
CEventStackIterator::Current() const
{
	return (m_read < m_stack.m_current) ? m_read : NULL;
}

bool
CEventStackIterator::Next()
{
	if (m_read >= m_stack.m_current)
		return false;

	if (m_read > m_write)
		Event::Relocate(m_write, m_read, 1);

	m_read++;
	m_write++;
	return true;
}

void
CEventStackIterator::Remove()
{
	if (m_read < m_stack.m_current)
	{
		Event::Destruct(m_read, 1);
		m_read++;
	}
}

// END - EventStack.cpp
