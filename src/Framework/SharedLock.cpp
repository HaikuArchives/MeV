/* ===================================================================== *
 * SharedLock.cpp (MeV/Application Framework)
 * ===================================================================== */

#include "SharedLock.h"

// Support Kit
#include <Debug.h>

int rlockcount = 0;

bool CSharedLock::Acquire( TLockType inLockType )
{
	status_t			status;
	thread_id			current = find_thread( NULL );

		// Note: We shouldn't need to lock here, because the only way this test can
		// succeed is if we're already locked.
		// If we're already the exclusive owner
	if (current == eOwner)
	{
			// Add to lock counts and return
		if (inLockType == Lock_Shared) sCount++;
		else eCount++;
		return true;
	}
	
	if (inLockType == Lock_Exclusive)
	{
			// Lock exclusive by doing exceeding max readlock count
		status = acquire_sem_etc( sem, Max_Shared_Locks, B_TIMEOUT, 5000000 );
		if (status != B_NO_ERROR)
		{
			IsLocked();
			eOwner = current;
			ASSERT( false );
		}
		if (status != B_NO_ERROR) return false;
		eOwner = current;
		eCount = 1;
		sCount = 0;
	}
	else
	{
		status = acquire_sem_etc( sem, 1, B_TIMEOUT, 5000000 );
		if (status != B_NO_ERROR)
		{
			IsLocked();
			eOwner = current;
			ASSERT( false );
		}
		if (status != B_NO_ERROR) return false;
		rlockcount++;
	}
	
	return true;
}

bool CSharedLock::Acquire( TLockType inLockType, bigtime_t inTimeout )
{
	status_t			status;
	thread_id			current = find_thread( NULL );

		// Note: We shouldn't need to lock here, because the only way this test can
		// succeed is if we're already locked.
		// If we're already the exclusive owner
	if (current == eOwner)
	{
			// Add to lock counts and return
		if (inLockType == Lock_Shared) sCount++;
		else eCount++;
		return true;
	}
	
	if (inLockType == Lock_Exclusive)
	{
			// Lock exclusive by doing exceeding max readlock count
		status = acquire_sem_etc( sem, Max_Shared_Locks, B_TIMEOUT, inTimeout );
		if (status != B_NO_ERROR) return false;
		eOwner = current;
		eCount = 1;
		sCount = 0;
	}
	else
	{
		status = acquire_sem_etc( sem, 1, B_TIMEOUT, inTimeout );
		if (status != B_NO_ERROR) return false;
		rlockcount++;
	}
	
	return true;
}

bool CSharedLock::Release( TLockType inLockType )
{
	thread_id			current = find_thread( NULL );
	
		// Note: We shouldn't need to lock here, since the only way that this comparison
		// can ever be true is if we're already locked.
	if (eOwner == current)
	{
		if (inLockType == Lock_Exclusive)
		{
			if (--eCount == 0)
			{
				eOwner = -1;
				ASSERT( sCount >= 0 );
				
					// Release the exclusive lock, minus the number of shared
					// locks that we acquired while we were locking exclusively.
				return release_sem_etc( sem, Max_Shared_Locks - sCount, 0 ) == B_NO_ERROR;
			}
			return true;
		}
		else
		{
			sCount--;		// Reduce count of shared locks.
			return true;
		}
	}
	else if (inLockType == Lock_Shared)
	{
		rlockcount--;
			// Release shared lock normally...
		return release_sem_etc( sem, 1, B_DO_NOT_RESCHEDULE ) == B_NO_ERROR;
	}
	else { ASSERT( false ); }
	
	return false;
}

bool CSharedLock::IsLocked()
{
	status_t			status;
	long				count;
	
	status = get_sem_count( sem, &count );
	return (status == 0) && count < Max_Shared_Locks;
}
