/* ===================================================================== *
 * SharedLock.h (MeV/Framework)
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
 *  History:
 *  1997		Talin
 *		Original implementation
 *  04/08/2000	cell
 *		General cleanup in preparation for initial SourceForge checkin
 *	11/04/2000	cell
 *		Separated shared and exclusive locking into separate methods.
 *		Inspired by some of the MultiLock sample-code by Be, Inc.
 * ===================================================================== */

#ifndef __C_SharedLock_H__
#define __C_SharedLock_H__

// Kernel Kit
#include <OS.h>
// Support Kit
#include <Debug.h>

#define D_LOCK(x) PRINT(x)		// Locking messages for many classes

typedef enum
{
	//	A shared lock that waits if there is an exclusive lock waiting.
	Lock_Shared,

	//	An exclusive lock.
	Lock_Exclusive,
} TLockType;

/**
 *  Implements single writer, multiple readers locking.
 *	@author		Talin, Christopher Lenz
 *	@package	Framework
 */
class CSharedLock
{

public:							// Constructor/Destructor

								CSharedLock(
									const char *name = NULL);

								~CSharedLock();

public:							// Operations

	/**	Acquire the lock.	*/
	bool						Acquire(
									TLockType type)
								{ return type == Lock_Exclusive ?
										 WriteLock() :
										 ReadLock(); }

	/**	Acquire the lock with a timeout.	*/
	bool						Acquire(
									TLockType type,
									bigtime_t timeout)
								{ return type == Lock_Exclusive ?
										 WriteLock(timeout) :
										 ReadLock(timeout); }
										 

	/**	Release the lock.	*/
	bool						Release(
									TLockType type)
								{ return type == Lock_Exclusive ?
										 WriteUnlock() :
										 ReadUnlock(); }

	/** Returns true if the lock is exclusively locked. */
	bool						IsExclusiveLocked() const
								{ return IsWriteLocked(); }

	/** Returns true if the lock is read-locked. */
	bool						IsLocked() const
								{ return (IsReadLocked() || IsWriteLocked()); }

public:							// Accessors

	/**	Returns B_OK if the semaphore initialized correctly. */
	status_t					InitCheck() const;

	/**	Determines whether the object is locked for read access.
	 *	Also returns true if the object has been locked for writing 
	 *	by the same thread calling this function. If some other thread
	 *	holds a write lock, or the object is not locked at all,
	 *	returns false.
	 */
	bool						IsReadLocked() const;

	/**	Returns whether the object is locked for write access by
	 *	the thread calling this.
	 */
	bool						IsWriteLocked(
									uint32 *stack_base = NULL,
									thread_id *thread = NULL) const;

	/**	Returns the name of the lock (and thus the semaphore). */
	const char *				Name() const;

public:							// Operations

	/**	Locks the object for read access. Many readers can hold a read 
	 *	simultaneously.
	 *	@ return	true if the object could be locked.
	 */
	bool						ReadLock(
									bigtime_t timeout = B_INFINITE_TIMEOUT);

	/** Unlocks the object. */
	bool						ReadUnlock();

	/**	Locks the object for write access. Can only be locked by one thread
	 *	at a time.
	 *	@ return	true if the object could be locked.
	 */
	bool						WriteLock(
									bigtime_t timeout = B_INFINITE_TIMEOUT);

	/** Unlocks the object. */
	bool						WriteUnlock();

private:						// Instance Data

	sem_id						m_sem;

	uint32						m_writerStackBase;
	thread_id					m_writerThread;
	uint32						m_writerNest;

	int32						m_readerCount;
};

/**	Stack based class to acquire a shared lock.	*/
class StLocker
{

private:						// Instance Data

	CSharedLock &				m_lock;

	TLockType					m_type;

	bool						m_locked;

public:							// Constructor/Destructor

								StLocker(
									CSharedLock &lock,
									TLockType type)
									:	m_lock(lock),
										m_type(type)
								{ m_locked = m_lock.Acquire(m_type); }

								~StLocker();

public:							// Operations

	void						Acquire();

	void						Release();
};

inline
StLocker::~StLocker()
{
	if (m_locked)
	{
		m_lock.Release(m_type);
		m_locked = false;
	}
}

inline void
StLocker::Acquire()
{
	if (!m_locked)
	{
		m_locked = m_lock.Acquire(m_type);
	}
}

inline void
StLocker::Release()
{
	if (m_locked)
	{
		m_lock.Release(m_type);
		m_locked = false;
	}
}

#endif /* __C_SharedLock_H__ */
