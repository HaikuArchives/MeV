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

typedef enum {
	Lock_Shared,				// A shared lock that waits if there is an exclusive lock waiting
	Lock_Exclusive,			// An exclusive lock
} TLockType;

const int32			Max_Shared_Locks = 10000;

	/** 	A class which (I believe) implements shared and exclusive locking. */
class CSharedLock {
	sem_id			sem;
	int				eOwner,			// Exclusive owner, or -1
					eCount,			// Exclusion lock recursion count
					sCount;			// Number of shared locks for exclusive lock owner
public:

		/** Constructor. */
	CSharedLock( char *inName = NULL )
	{
		sem = create_sem( Max_Shared_Locks, inName );
		eOwner = -1;
		eCount = sCount = 0;
	}

		/** Destructor */
	~CSharedLock()
	{
		delete_sem( sem );
	}

		/** Acquire the lock. */
	bool Acquire( TLockType inLockType );

		/** Acquire the lock with a timeout. */
	bool Acquire( TLockType inLockType, bigtime_t inTimeout );

		/** Release the lock. */
	bool Release( TLockType inLockType );

		/** Returns true if the lock is exclusively locked. */
	bool IsExclusiveLocked() { return eCount > 0; }

		/** Returns true if the lock is read-locked. */
	bool IsLocked();
};

	/** Stack based class to acquire a shared lock. */
class StLocker {

	CSharedLock		&lock;
	TLockType		type;
	bool				locked;

public:

	StLocker( CSharedLock &inLock, TLockType inType ) : lock( inLock ), type( inType )
	{
		locked = lock.Acquire( type );
	}

	~StLocker()
	{
		if (locked) { lock.Release( type ); locked = false; }
	}

	void Acquire()
	{
		if (!locked) locked = lock.Acquire( type );
	}

	void Release()
	{
		if (locked) { lock.Release( type ); locked = false; }
	}
};

#endif /* __C_SharedLock_H__ */