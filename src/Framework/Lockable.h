/* ===================================================================== *
 * Lockable.h (MeV/Framework)
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
 *	11/08/2000	cell
 *		Renamed CSharedLock to CLockable.
 *		Implemented stack based locks for read and write access.
 * ===================================================================== */

#ifndef __C_Lockable_H__
#define __C_Lockable_H__

// Kernel Kit
#include <OS.h>
// Support Kit
#include <Debug.h>

/**
 *  Implements single writer, multiple readers locking.
 *	@author		Christopher Lenz
 */
class CLockable
{

public:							// Constructor/Destructor

								CLockable(
									const char *name = NULL);

								~CLockable();

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

public:							// Operations

	/**	Locks the object for read access. Many readers can hold a read 
	 *	simultaneously.
	 *	@return		true if the object could be locked.
	 */
	bool						ReadLock(
									bigtime_t timeout = B_INFINITE_TIMEOUT);

	/** Unlocks the object. */
	bool						ReadUnlock();

	/**	Locks the object for write access. Can only be locked by one thread
	 *	at a time.
	 *	@return		true if the object could be locked.
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

/**
 *  Stack-based locker for read-locking a CLockable.
 *	@author		Christopher Lenz
 */
class CReadLock
{

public:							// Constructor/Destructor

								CReadLock(
									CLockable *object,
									bigtime_t timeout = B_INFINITE_TIMEOUT)
									:	m_lockable(object)
								{ m_lockable->ReadLock(timeout); }

								~CReadLock()
								{ m_lockable->ReadUnlock(); }

public:							// Accessors

	status_t					InitCheck() const
								{ return m_lockable->InitCheck(); }

private:						// Instance Data

	CLockable *					m_lockable;
};

/**
 *  Stack-based locker for write-locking a CLockable.
 *	@author		Christopher Lenz
 */
class CWriteLock
{

public:							// Constructor/Destructor

								CWriteLock(
									CLockable *object,
									bigtime_t timeout = B_INFINITE_TIMEOUT)
									:	m_lockable(object)
								{ m_lockable->WriteLock(timeout); }

								~CWriteLock()
								{ m_lockable->WriteUnlock(); }

public:							// Accessors

	status_t					InitCheck() const
								{ return m_lockable->InitCheck(); }

private:						// Instance Data

	CLockable *					m_lockable;
};

#endif /* __C_Lockable_H__ */
