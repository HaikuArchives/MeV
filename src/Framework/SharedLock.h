/* ===================================================================== *
 * SharedLock.h (MeV/Application Framework)
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
 *  Implements shared / exclusive locks, I hope...
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

#ifndef __C_SharedLock_H__
#define __C_SharedLock_H__

#include <OS.h>

typedef enum
{
	// A shared lock that waits if there is an exclusive lock waiting
	Lock_Shared,

	// An exclusive lock
	Lock_Exclusive,
} TLockType;

class CSharedLock
{

public:							// Constructor/Destructor

								CSharedLock(
									char *name = NULL);

								~CSharedLock();

public:							// Operations

	/** Acquire the lock. */
	bool						Acquire(
									TLockType inLockType);

	/** Acquire the lock with a timeout. */
	bool						Acquire(
									TLockType inLockType,
									bigtime_t inTimeout);

	/** Release the lock. */
	bool						Release(
									TLockType inLockType);

	/** Returns true if the lock is exclusively locked. */
	bool						IsExclusiveLocked() const
								{ return m_eCount > 0; }

	/** Returns true if the lock is read-locked. */
	bool						IsLocked() const;

private:						// Instance Data

	sem_id						m_sem;

	// Exclusive owner, or -1
	int							m_eOwner;

	// Exclusion lock recursion count
	int							m_eCount;

	// Number of shared locks for exclusive lock owner
	int							m_sCount;
};

/** Stack based class to acquire a shared lock. */
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
