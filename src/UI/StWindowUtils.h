/* ===================================================================== *
 * StWindowUtils.h (MeV/User Interface)
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
 *	Original implementation
 *	04/08/2000	cell
 *	General cleanup in preparation for initial SourceForge checkin
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __StWindowUtils_H__
#define __StWindowUtils_H__

// Interface Kit
#include <Window.h>

class StWindowLocker {
	BWindow		*window;
public:
	StWindowLocker( BWindow *inWin ): window( inWin )
	{
		if (window) window->Lock();
	}
	
	~StWindowLocker()
	{
		if (window) window->Unlock();
	}
};
	
class StWindowUpdateDisabler {
	BWindow		*window;
public:
	StWindowUpdateDisabler( BWindow *inWin ) : window( inWin )
	{
		if (window) window->DisableUpdates();
	}
	
	~StWindowUpdateDisabler()
	{
		if (window) window->EnableUpdates();
	}
};

#endif /* __StWindowUtils_H__ */
