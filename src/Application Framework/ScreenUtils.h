/* ===================================================================== *
 * ScreenUtils.h (MeV)
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
 *  Various utility functions related to the screen.
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


#ifndef __U_ScreenUtils_H__
#define __U_ScreenUtils_H__

#include "AppFrameSpec.h"

// Interface Kit
#include <Point.h>
#include <Rect.h>
#include <Window.h>

class BWindow;

#ifdef __POWERPC__
#pragma export on
#endif

class AppFrameSpec UScreenUtils {
public:
	static BPoint		windowPos;

	static BRect CenterOnScreen( int32 inWidth, int32 inHeight, screen_id id = B_MAIN_SCREEN_ID );
	static BRect StackOnScreen( int32 inWidth, int32 inHeight, screen_id id = B_MAIN_SCREEN_ID );
	static BRect ConstrainToScreen( BRect inRect, screen_id id = B_MAIN_SCREEN_ID );
	static BRect CenterOnWindow( int32 inWidth, int32 inHeight, BWindow *parent );
};

#ifdef __POWERPC__
#pragma export on
#endif

#endif /* __U_Screen_Utils_H__ */