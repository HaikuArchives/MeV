/* ===================================================================== *
 * AppHelp.h (MeV/Application Framework)
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
 *  
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

#ifndef __U_AppHelp_H__
#define __U_AppHelp_H__

//#include "sylvan/except.h"

// Application Kit
#include <Application.h>
// Interface Kit
#include <ListView.h>
#include <Window.h>

inline void CheckMem( void *inPtr )
{
//	if (inPtr == NULL) throw CMemoryAllocException();
}

inline void CheckSuccess(bool inSuccess )
{
//	if (inSuccess == false) throw CGenericException( "Internal Error" );
}

	/**	Load resource from application resource file */
void *LoadResource(type_code type, long id, size_t *data_size);

	/**	Load resource from application resource file */
void *LoadResource(type_code type, const char *name, size_t *data_size);

	/**	Load Bitmap from application resource file. */
BBitmap *LoadBitmap( type_code type, long id );

	/**	Load Bitmap from application resource file. */
BBitmap *LoadBitmap( type_code type, const char *name );

inline void DeleteListItems( BListView *inView )
{
	BListItem		*listItem;
	
	while ((listItem = inView->RemoveItem((int32)0))) {
		delete listItem;
	}
}

char *LookupErrorText( status_t );

#endif /* __C_AppHelp_H__ */
