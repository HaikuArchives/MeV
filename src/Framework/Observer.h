/* ===================================================================== *
 * Observer.h (MeV/Application Framework)
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
 *  implements the observer pattern
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

#ifndef __C_Observer_H__
#define __C_Observer_H__

#define USE_SHARED_LOCKS	0

#include "Undo.h"
#include "RefCount.h"

// Application Kit
#include <Message.h>
#include <Handler.h>
// Support Kit
#include <List.h>
#include <Locker.h>

#if USE_SHARED_LOCKS
#include "SharedLock.h"
#else
typedef enum {
	Lock_Shared,				// A shared lock that waits if there is an exclusive lock waiting
	Lock_Exclusive,			// An exclusive lock
} TLockType;
#endif

class CObserver;
class CObservableSubject;

/** --------------------------------------------------------------------
	CUpdateHint is an abstract base class for observer notification hints
*/

const long			Update_ID = '#UPD',		//	Data has changed
					Delete_ID = '#DEL';		//	Observerable going away

class CUpdateHint : public BMessage {
public:

		/**	Subclasses will have member functions to add additional constraints. */
	CUpdateHint() : BMessage( Update_ID ) {}
	CUpdateHint( long id ) : BMessage( id ) {}
};

/** --------------------------------------------------------------------
	CObserver is a class which "watches" a subject for changes. Other observers
	can post updates indicating that changes have occured.
*/

class CObserver : public BHandler {
protected:

	CObservableSubject	*subject;
	
		//	BMessageHandler overrides
	void MessageReceived( BMessage *inMessage );
	
		/**	Update message from another observer */
	virtual void OnUpdate( BMessage * ) = 0;

		/**	We want to delete the subject, please stop observing...
			Note that observers are free to ignore this message, in which
			case the object will not be deleted.
		*/
	virtual void OnDeleteRequested( BMessage * ) {}
	
public:

		//	Send an update hint to all other observers. Deletes the hint
		//	object when completed
	void PostUpdate( CUpdateHint *inHint, bool inExcludeOriginal = true );
	
	CObserver( BLooper &inLooper, CObservableSubject *inSubject );
	virtual ~CObserver();
	
		/**	Change the observable that this observer is looking at. */
	void SetSubject( CObservableSubject *subject );
};

/** --------------------------------------------------------------------
	CObservableSubject is a base class for observable objects
*/
class CObservableSubject : public CRefCountObject {
	friend class		CObserver;
	friend class		StSubjectLock;

private:
	BList				observers;
#if USE_SHARED_LOCKS
	CSharedLock		lock;
#else
	BLocker			lock;
#endif
	UndoHistory		undoHistory;				//	Undo history for this track
	
protected:
		//	Observer-list management
	void AddObserver	( CObserver &inObserver );
	void RemoveObserver	( CObserver &inObserver );

		//	Send an update hint to all observers except the excluded one.
		//	Deletes the hint object when completed
	void PostUpdate( CUpdateHint *inHint, CObserver *inExcludeObserver );

public:

		/**	Return the number of observers. */
	long CountObservers() { return observers.CountItems(); }
	
#if USE_SHARED_LOCKS
		/**	Lock this observable subject in order to make changes to it */
	bool Lock( TLockType inLockType ) { return lock.Acquire( inLockType ); }

		/**	Lock this observable subject with a timeout in microseconds */
	bool Lock( TLockType inLockType, bigtime_t timeout ) { return lock.Acquire( inLockType, timeout ); }

		/**	Unlock this observable object after making changes */
	void Unlock( TLockType inLockType ) { lock.Release( inLockType ); }

		/**	Returns true if the onservable is currently locked. */
	bool IsLocked() { return lock.IsLocked(); }

#else
		/**	Lock this observable subject in order to make changes to it */
	void Lock() { lock.Lock(); }

		/**	Lock this observable subject with a timeout in microseconds */
	bool Lock( bigtime_t timeout ) { return lock.LockWithTimeout( timeout ) == B_OK; }

		/**	Unlock this observable object after making changes */
	void Unlock() { lock.Unlock(); }

		/**	Returns true if the onservable is currently locked. */
	bool IsLocked() { return lock.IsLocked(); }

#endif

		/**	Constructor. */
	CObservableSubject();

		/**	Destructor. */
	virtual ~CObservableSubject();

		/**	Return true if there is an undoable action. */
	bool CanUndo() { return undoHistory.CanUndo(); }
	
		/**	Return true if there is a redoable action. */
	bool CanRedo() { return undoHistory.CanRedo(); }

		/**	Undo the undoable action, if any. */
	virtual bool Undo()
	{
		return undoHistory.Undo();
	}

		/**	Redo the redoable action, if any. */
	virtual bool Redo()
	{
		return undoHistory.Redo();
	}
	
		/**	returns TRUE if the undo action is the most recent one. */
	bool IsMostRecentUndoAction( UndoAction *inAction )
	{
		return undoHistory.IsMostRecent( inAction );
	}
	
		/**	Return the description of the current undo action. Returns NULL
			if current undo action has no description, or there is no current
			undo action.
		*/
	const char *UndoDescription() const { return undoHistory.UndoDescription(); }
	
		/**	Return the description of the current redo action. Returns NULL
			if current redo action has no description, or there is no current
			redo action.
		*/
	const char *RedoDescription() const { return undoHistory.RedoDescription(); }
	
		/**	Add a new action to be undone to the list. This discards any
			pending re-do actions.
		*/
	void AddUndoAction( UndoAction *inAction ) { undoHistory.Add( inAction ); }

		/**	Set how much undo information we wish to keep. */
	void SetMaxUndoSize( int32 inMaxUndo ) { undoHistory.SetMaxUndoSize( inMaxUndo ); }

		/**	Request to delete. This means asking all observers to
			release the subject, so that the subject's reference count
			will go to zero and the subject will be deleted.
			
			<p>This is virtual so that any observables which contain
			subcomponents which are also observable, we can recursively
			call RequestDelete on the subcomponents.
		*/
	virtual void RequestDelete();

		/**	Post a general update from a non-observer. */
	void PostUpdate( CUpdateHint *inHint ) { PostUpdate( inHint, NULL ); }
};

	//	Stack-based locker for subjects

#if USE_SHARED_LOCKS
class StSubjectLock {
	CSharedLock			&lock;
	TLockType			type;
	bool					locked;

public:
	StSubjectLock( CObservableSubject &inSubject, TLockType inLockType = Lock_Exclusive )
		: lock( inSubject.lock ), type( inLockType )
	{
		locked = false;
		Acquire();
	}

	~StSubjectLock()	{ Release(); }
	void Acquire()		{ if (!locked) locked = lock.Acquire( type ); }
	void Release()		{ if (locked) { lock.Release( type ); locked = false; } }
	bool LockValid()	{ return locked; }
};
#else
class StSubjectLock {
	BLocker				&lock;

public:
	StSubjectLock( CObservableSubject &inSubject, TLockType inLockType = Lock_Exclusive )
		: lock( inSubject.lock )
	{
		Acquire();
	}

	~StSubjectLock()	{ Release(); }
	void Acquire()		{ lock.Lock(); }
	void Release()		{ lock.Unlock(); }
};
#endif

#endif /* __C_Observer_H__ */