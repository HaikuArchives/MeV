/* ===================================================================== *
 * RefCount.h (MeV/Application Framework)
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
 *  Framework for reference-counted objects
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

#ifndef __C_RefCountObject_H__
#define __C_RefCountObject_H__

#include "AppFrameSpec.h"

// Kernel Kit
#include <OS.h>

#ifdef __POWERPC__
	#pragma export on
#endif

class AppFrameSpec CRefCountObject {

	int32			count;

public:

		/**	Constructor. Initializes reference count to 1 */
	CRefCountObject() : count( 1 ) {}

		/**	Virtual constructor (does nothing, serves as place-holder). */
	virtual ~CRefCountObject() {}

		/**	Increment the object's reference count. */
	CRefCountObject *Acquire();

		/**	Static version to decrement reference count, safe if inObj == NULL. */
	static void Release( CRefCountObject *inObj );
};

#ifdef __POWERPC__
#pragma export off
#endif

#endif /* __C_RefCountObject_H__ */
