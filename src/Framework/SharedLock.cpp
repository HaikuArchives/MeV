/* ===================================================================== *
 * SharedLock.cpp (MeV/Framework)
 * ===================================================================== */

#include "SharedLock.h"

// Support Kit
#include <Debug.h>

// ---------------------------------------------------------------------------
// Constants

const int32			Max_Shared_Locks = 10000;

// ---------------------------------------------------------------------------
// Constructor/Destructor

CSharedLock::CSharedLock(
	char *name = NULL)
	:	m_eOwner(-1),
		m_eCount(0),
		m_sCount(0)
{
	m_sem = create_sem(Max_Shared_Locks, name);
}

CSharedLock::~CSharedLock()
{
	delete_sem(m_sem);
}

// ---------------------------------------------------------------------------
// Operations

bool
CSharedLock::Acquire(
	TLockType inLockType)
{
	status_t status;
	thread_id current = find_thread(NULL);

	// Note: We shouldn't need to lock here, because the only way this test can
	// succeed is if we're already locked.
	// If we're already the exclusive owner
	if (current == m_eOwner)
	{
		// Add to lock counts and return
		if (inLockType == Lock_Shared)
			m_sCount++;
		else
			m_eCount++;
		return true;
	}
	
	if (inLockType == Lock_Exclusive)
	{
		// Lock exclusive by doing exceeding max readlock count
		status = acquire_sem_etc(m_sem, Max_Shared_Locks, B_TIMEOUT, 5000000);
		if (status != B_NO_ERROR)
		{
			IsLocked();
			m_eOwner = current;
			ASSERT(false);
		}
		if (status != B_NO_ERROR)
			return false;
		m_eOwner = current;
		m_eCount = 1;
		m_sCount = 0;
	}
	else
	{
		status = acquire_sem_etc(m_sem, 1, B_TIMEOUT, 5000000);
		if (status != B_NO_ERROR)
		{
			IsLocked();
			m_eOwner = current;
			ASSERT(false);
		}
		if (status != B_NO_ERROR)
			return false;
	}
	
	return true;
}

bool
CSharedLock::Acquire(
	TLockType inLockType,
	bigtime_t inTimeout)
{
	status_t status;
	thread_id current = find_thread(NULL);

	// Note: We shouldn't need to lock here, because the only way this test can
	// succeed is if we're already locked.
	// If we're already the exclusive owner
	if (current == m_eOwner)
	{
		// Add to lock counts and return
		if (inLockType == Lock_Shared)
			m_sCount++;
		else
			m_eCount++;
		return true;
	}
	
	if (inLockType == Lock_Exclusive)
	{
		// Lock exclusive by doing exceeding max readlock count
		status = acquire_sem_etc(m_sem, Max_Shared_Locks, B_TIMEOUT, inTimeout);
		if (status != B_NO_ERROR)
			return false;
		m_eOwner = current;
		m_eCount = 1;
		m_sCount = 0;
	}
	else
	{
		status = acquire_sem_etc(m_sem, 1, B_TIMEOUT, inTimeout);
		if (status != B_NO_ERROR)
			return false;
	}
	
	return true;
}

bool
CSharedLock::Release(
	TLockType inLockType)
{
	thread_id current = find_thread(NULL);

	// Note: We shouldn't need to lock here, since the only way that this comparison
	// can ever be true is if we're already locked.
	if (m_eOwner == current)
	{
		if (inLockType == Lock_Exclusive)
		{
			if (--m_eCount == 0)
			{
				m_eOwner = -1;
				ASSERT(m_sCount >= 0);

				// Release the exclusive lock, minus the number of shared
				// locks that we acquired while we were locking exclusively.
				return release_sem_etc(m_sem, Max_Shared_Locks - m_sCount,
									   0) == B_NO_ERROR;
			}
			return true;
		}
		else
		{
			m_sCount--;		// Reduce count of shared locks.
			return true;
		}
	}
	else if (inLockType == Lock_Shared)
	{
		// Release shared lock normally...
		return release_sem_etc(m_sem, 1, B_DO_NOT_RESCHEDULE) == B_NO_ERROR;
	}
	else
	{
		ASSERT(false);
	}

	return false;
}

bool
CSharedLock::IsLocked() const
{
	status_t status;
	long count;

	status = get_sem_count(m_sem, &count);
	return ((status == 0) && (count < Max_Shared_Locks));
}

// END - SharedLock.cpp
