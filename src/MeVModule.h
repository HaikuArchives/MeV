/* ===================================================================== *
 * MeVModule.h (MeV)
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
 * History:
 *	6/21/2000		dwalton
 *		Original implementation
 *		added to the repository...
 *	8/13/2000		dwalton
 *		modified to support one producer per CDestination.
 * ===================================================================== */

#ifndef __C_MeVModule_H__
#define __C_MeVModule_H__

#include "Observer.h"

// Application Kit
#include <Looper.h>
// Storage Kit
#include <Mime.h>

class CDestination;
class CMeVDoc;

class CMeVModule
	:	public BLooper,
		public CObserver
{

public:							// Constructor/Destructor

								CMeVModule(
									unsigned long type,
									const char *name);

	virtual						~CMeVModule();

public:							// Hook Functions

	/** Called by the document when a new destination is requested
		from this module. The default implementation returns NULL.
	*/
	virtual CDestination *		CreateDestination(
									CMeVDoc *document,
									int32 *id = NULL,
									const char *name = NULL);

	/** Called by the app when a document has been created or loaded.
		We should start observing the document at this point. The 
		default implementation does nothing.
	 */
	virtual void				DocumentOpened(
									CMeVDoc *document);

	/** Returns the generic icon for this module. */
	virtual status_t			GetIcon(
									icon_size which,
									BBitmap *outBitmap) = 0;

	virtual bool				SubjectReleased(
									CObservable *subject);

	virtual void				SubjectUpdated(
									BMessage *message);

public:							// BLooper Implementation

	virtual void				MessageReceived(
									BMessage *message);

public:							// Accessors

	unsigned long				Type() const;

public:							// CObserver Implementation

	bool						Released(
									CObservable *subject);

	void						Updated(
									BMessage *message);

private:						// Instance Data

	unsigned long				m_type;
};

#endif /* __C_MeVModule_H__ */
