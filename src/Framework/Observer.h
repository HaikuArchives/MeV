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

// Support Kit
#include <SupportDefs.h>

class CObservable;
class CUpdateHint;

#define D_OBSERVE(x) //PRINT(x)	// Observer messages from many classes

/** --------------------------------------------------------------------
 *	CObserver is a class which "watches" subjects for changes. Other 
 *	observers can post updates indicating that changes have occured.
 *  Implements the observer pattern
 *	@author		Talin, Christopher Lenz
 *	@package	Framework
*/
class CObserver
{

public:							// Constructor/Destructor

	/**	Constructor. If a subject is given, we start observing it
	 *	immediately.
	*/
								CObserver(
									CObservable *subject = NULL);

	virtual						~CObserver();

public:							// Hook Functions

	/**	We want to delete the subject, please stop observing...
		You should have stopped observing the subject when this 
		call returns. Otherwise you must return false.
		@see CObservable::RemoveObserver()
		@return	true if the observer stopped reserving the object,
				false otherwise.
	*/
	virtual bool				Released(
									CObservable *subject)
								{ return false; }

	/**	A subject has been updated. */
	virtual void				Updated(
									BMessage *message) = 0;
};

#endif /* __C_Observer_H__ */
