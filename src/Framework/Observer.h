/* ===================================================================== *
 * Observer.h (MeV/Framework)
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

#ifndef __C_Observer_H__
#define __C_Observer_H__

#include "Undo.h"
#include "RefCount.h"
#include "SharedLock.h"

// Application Kit
#include <Message.h>
#include <Handler.h>
// Support Kit
#include <List.h>
#include <Locker.h>

class CObservableSubject;
class CUpdateHint;

/** --------------------------------------------------------------------
 *	CObserver is a class which "watches" a subject for changes. Other 
 *	observers can post updates indicating that changes have occured.
 *  Implements the observer pattern
 *	@author		Talin, Christopher Lenz
 *	@package	Framework
*/
class CObserver
	:	public BHandler
{

public:							// Constructor/Destructor

								CObserver(
									BLooper &looper,
									CObservableSubject *subject);

	virtual						~CObserver();

public:							// Hook Functions

	/**	Update message from another observer */
	virtual void				OnUpdate(
									BMessage *message) = 0;

	/**	We want to delete the subject, please stop observing...
		Note that observers are free to ignore this message, in which
		case the object will not be deleted.
	*/
	virtual void				OnDeleteRequested(
									BMessage *message)
								{ }
	
public:							// Operations

	/** Send an update hint to all other observers. Deletes the hint
		object when completed.
	 */
	void						PostUpdate(
									CUpdateHint *hint,
									bool excludeOriginal = true);
	
	/**	Change the observable that this observer is looking at. */
	void						SetSubject(
									CObservableSubject *subject);

public:							// BHandler Implementation

	virtual void				MessageReceived(
									BMessage *message);
	
private:						// Instance Data

	CObservableSubject *		m_subject;
};

#endif /* __C_Observer_H__ */
