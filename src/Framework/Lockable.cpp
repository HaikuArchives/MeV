/* ===================================================================== *
 * Lockable.cpp (MeV/Framework)
 * ===================================================================== */

#include "Lockable.h"

// Support Kit
#include <Debug.h>

#define D_ALLOC(x) //PRINT(x)		// Constructor/Destructor
#define D_ACCESS(x) //PRINT(x)		// Accessors
#define D_OPERATION(x) //PRINT(x)	// Operations

// ---------------------------------------------------------------------------
// Constants

const int32			MAX_READER_COUNT = 10000;

// ---------------------------------------------------------------------------
// Constructor/Destructor

CLockable::CLockable(
	const char *name = NULL)
	:	m_sem(-1),
		m_writerStackBase(0),
		m_writerThread(-1),
		m_writerNest(0),
		m_readerCount(0)
{
	D_ALLOC(("CLockable::CLockable(%s)\n", name ? name : "NULL"));

	m_sem = create_sem(MAX_READER_COUNT, name);
}

CLockable::~CLockable()
{
	D_ALLOC(("CLockable::~CLockable()\n"));

	//become the writer
	if (!IsWriteLocked())
		WriteLock();

	delete_sem(m_sem);
	m_sem = -1;
}

// ---------------------------------------------------------------------------
// Accessors

status_t
CLockable::InitCheck() const
{
	if (m_sem > 0)
		return B_OK;

	return B_ERROR;
}

bool 
CLockable::IsReadLocked() const
{
	D_ACCESS(("CLockable::IsReadLocked()\n"));

	return ((m_readerCount > 0) || IsWriteLocked());
}

bool 
CLockable::IsWriteLocked(
	uint32 *outStackBase,
	thread_id *outThread) const
{
	D_ACCESS(("CLockable::IsWriteLocked()\n"));

	bool locked = false;
	uint32 stackBase;
	thread_id thread = 0;	
		
	// determine which page in memory this stack represents
	// this is managed by taking the address of the item on the
	// stack and dividing it by the size of the memory pages
	// if it is the same as the cached stack_page, there is a match 
	stackBase = (uint32)&locked / B_PAGE_SIZE;
	if (stackBase == m_writerStackBase)
	{
		locked = true;
	}
	else
	{
		// as there was no stack_page match we resort to the
		// tried and true methods
		thread = find_thread(NULL);
		if (m_writerThread == thread)
			locked = true;
	}
	
	// if someone wants this information, give it to them
	if (outStackBase != NULL)
		*outStackBase = stackBase;
	if (outThread != NULL)
		*outThread = thread;

	return locked;
}

// ---------------------------------------------------------------------------
// Operations

bool
CLockable::ReadLock(
	bigtime_t timeout)
{
	D_OPERATION(("CLockable::ReadLock(%Ld)\n", timeout));

	bool locked = false;

	if (IsWriteLocked())
	{
		// the writer simply increments the nesting
		m_writerNest++;
		locked = true;
	}
	else
	{
		// acquire sem and increment reader count
		locked = (acquire_sem_etc(m_sem, 1, B_DO_NOT_RESCHEDULE,
								  timeout) == B_OK);
		if (locked)
			m_readerCount++;
	}

	return locked;
}

bool
CLockable::ReadUnlock()
{
	D_OPERATION(("CLockable::ReadUnlock()\n"));

	bool unlocked = false;

	if (IsWriteLocked())
	{
		// writers simply decrement the nesting count
		m_writerNest--;
		unlocked = true;	
	}
	else
	{
		// release sem and decrement reader count
		unlocked = (release_sem_etc(m_sem, 1,
									B_DO_NOT_RESCHEDULE) == B_OK);
		if (unlocked)
			m_readerCount--;
	}
	
	return unlocked;	
}

bool 
CLockable::WriteLock(
	bigtime_t timeout)
{
	D_OPERATION(("CLockable::WriteLock(%Ld)\n", timeout));

	bool locked = false;
	uint32 stackBase = 0;
	thread_id thread = -1;

	if (IsWriteLocked(&stackBase, &thread))
	{
		// already the writer - increment the nesting count
		m_writerNest++;
		locked = true;
	}
	else
	{
		// another writer in the lock - acquire the semaphore
		locked = (acquire_sem_etc(m_sem, MAX_READER_COUNT,
								  B_DO_NOT_RESCHEDULE, timeout) == B_OK);
		if (locked)
		{
			ASSERT(m_writerThread == -1);
			// record thread information
			m_writerThread = thread;
			m_writerStackBase = stackBase;
		}
	}

	return locked;
}

bool 
CLockable::WriteUnlock()
{
	D_OPERATION(("CLockable::WriteUnlock()\n", m_sem));

	bool unlocked = false;

	if (IsWriteLocked())
	{
		// if this is a nested lock simply decrement the nest count
		if (m_writerNest > 0)
		{
			m_writerNest--;
			unlocked = true;
		}
		else
		{
			unlocked = (release_sem_etc(m_sem, MAX_READER_COUNT,
										B_DO_NOT_RESCHEDULE) == B_OK);
			if (unlocked)
			{
				//clear the information
				m_writerThread = -1;
				m_writerStackBase = 0;
			}
		}
		
	}
	else
	{
		debugger("Non-writer attempting to WriteUnlock()\n");
	}

	return unlocked;
}

// END - Lockable.cpp
