/* ===================================================================== *
 * Observable.h (MeV/Framework)
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
 * History:
 *	1997		Talin
 *		Original implementation
 *	04/08/2000	cell
 *		General cleanup in preparation for initial SourceForge checkin
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __C_Observable_H__
#define __C_Observable_H__

#include "Lockable.h"
#include "Undo.h"

// Application Kit
#include <Message.h>
// Support Kit
#include <List.h>

class CObserver;
class CUpdateHint;
class UndoHistory;

/**
 *	CObservable is a base class for observable objects
 *	@author		Talin, Christopher Lenz
 */
class CObservable
	:	public CLockable
{
	friend class		CObserver;

public:							// Constants

	enum
	{
								UPDATED = 'obsA',

								RELEASED
	};

public:							// Constructor/Destructor

	/**	Constructor. */
								CObservable(
									const char *name = NULL);

	/**	Destructor. */
	virtual						~CObservable();

public:							// Operations

	/** Add an observer to this subject. */
	bool						AddObserver	(
									CObserver *observer);

	/**	Return the number of observers. */
	long						CountObservers() const
								{ return m_observers.CountItems(); }

	/** Remove an observer from this subject. */
	bool						RemoveObserver(
									CObserver *observer);

	/**	Request to delete. This means asking all observers to
		release the subject, so that the subject's reference count
		will go to zero and the subject will be deleted.
		
		This is virtual so that any observables which contain
		subcomponents which are also observable, we can recursively
		call RequestDelete on the subcomponents.
	*/
	virtual void				RequestDelete();

	/** Send an update hint to all observers except the excluded one.
		Deletes the hint object when completed.
	 */
	void						PostUpdate(
									CUpdateHint *hint,
									CObserver *excludeObserver = NULL);

	/**	Return true if there is an undoable action. */
	bool						CanUndo()
								{ return m_undoHistory.CanUndo(); }

	/**	Return true if there is a redoable action. */
	bool						CanRedo()
								{ return m_undoHistory.CanRedo(); }

	/**	Undo the undoable action, if any. */
	virtual bool				Undo()
								{ return m_undoHistory.Undo(); }

	/**	Redo the redoable action, if any. */
	virtual bool				Redo()
								{ return m_undoHistory.Redo(); }

	/**	returns TRUE if the undo action is the most recent one. */
	bool						IsMostRecentUndoAction(
									UndoAction *action)
								{ return m_undoHistory.IsMostRecent(action); }

	/**	Return the description of the current undo action. Returns NULL
		if current undo action has no description, or there is no current
		undo action.
	*/
	const char *				UndoDescription() const
								{ return m_undoHistory.UndoDescription(); }

	/**	Return the description of the current redo action. Returns NULL
		if current redo action has no description, or there is no current
		redo action.
	*/
	const char *				RedoDescription() const
								{ return m_undoHistory.RedoDescription(); }

	/**	Add a new action to be undone to the list. This discards any
		pending re-do actions.
	*/
	void						AddUndoAction(
									UndoAction *action)
								{ m_undoHistory.Add(action); }

	/**	Set how much undo information we wish to keep. */
	void						SetMaxUndoSize(
									int32 maxUndo)
								{ m_undoHistory.SetMaxUndoSize(maxUndo); }

private:						// Instance Data

	BList						m_observers;

	/** Undo history for this subject. */
	UndoHistory					m_undoHistory;
};

typedef enum
{
	//	A shared lock that waits if there is an exclusive lock waiting.
	Lock_Shared,

	//	An exclusive lock.
	Lock_Exclusive,
} TLockType;

/**
 *	Stack-based locker for subjects.
 *	@author		Talin, Christopher Lenz
 */
class StSubjectLock
{

public:							// Constructor/Destructor

								StSubjectLock(
									CObservable &subject,
									TLockType lockType = Lock_Exclusive)
									:	m_subject(&subject),
										m_type(lockType),
										m_locked(false)
								{ Acquire(); }

								~StSubjectLock()
								{ Release(); }

public:							// Operations

	void						Acquire()
								{
									if (!m_locked)
									{
										if (m_type == Lock_Exclusive)
											m_locked = m_subject->WriteLock();
										else
											m_locked = m_subject->ReadLock();
									}
								}

	void						Release()
								{
									if (m_locked)
									{
										if (m_type == Lock_Exclusive)
											m_locked = !m_subject->WriteUnlock();
										else
											m_locked = !m_subject->ReadUnlock();
									}
								}

	bool						LockValid()
								{ return m_locked; }

private:						// Instance Data

	CObservable *				m_subject;

	TLockType					m_type;

	bool						m_locked;
};

/** 
 *	CUpdateHint is an abstract base class for observer notification hints
 *	@author		Talin, Christopher Lenz
 */
class CUpdateHint
	:	public BMessage
{

public:							// Constructor/Destructor

	/**	Subclasses will have member functions to add additional constraints. */
								CUpdateHint()
									:	BMessage(CObservable::UPDATED)
								{ }

								CUpdateHint(
									int32 what)
									:	BMessage(what)
								{ }
};

#endif /* __C_Observable_H__ */
