/* ===================================================================== *
 * BitmapMenu.h (MeV/User Interface)
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
 *  Menus with pictures for labels.
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


#ifndef __C_BitmapMenu_H__
#define __C_BitmapMenu_H__

// Application Kit
#include <Looper.h>
// Interface Kit
#include <Bitmap.h>
#include <Menu.h>
#include <MenuItem.h>

/**	There is one and only one reason for the existance of this class:
	BPopUpMenu doesn't allow matrix-style layout!
*/

class CPopUpMatrixMenu : public BMenu {
	BPoint			screenPos;

public:
	CPopUpMatrixMenu( const char *name, float width, float height )
		: BMenu( name, width, height ) {}
		
	BPoint ScreenLocation() { return screenPos; }

	void SetPosition( BRect inScreenRect );
	
	BMenuItem *Go(	BPoint screenPoint,
					bool deliversMessage = false,
					bool openAnyway = false,
					BRect *clickToOpenRect = NULL );
};

/**	A button which displays a popup menu full of bitmaps, and which can
	display the selected menu's bitmap for it's label.
*/

class CBitmapMenuButton : public BView {
	BBitmap			*bitmap;
	CPopUpMatrixMenu	*menu;

	void Draw( BRect updateRect );
	void MouseDown( BPoint point );
	void AttachedToWindow()
	{
		if (Parent()) SetViewColor( Parent()->ViewColor() );
	}

public:

		/**	Constructor for popup menu button. Args are the same as for a BView */
	CBitmapMenuButton( BRect frame, const char *name, 
		uint32 resizingMode = B_FOLLOW_LEFT | B_FOLLOW_TOP,
		uint32 flags = B_WILL_DRAW | B_NAVIGABLE )
		: BView( frame, name, resizingMode, flags )
	{
		bitmap = NULL;
		menu = NULL;
	}
	
		/**	Attach the menu to the button. */
	void SetMenu( CPopUpMatrixMenu *inMenu ) { menu = inMenu; }

		/**	Set the bitmap of the menu to be the same as the one on the menu item. */
	void SelectItem( BMenuItem *inItem );
};

/**	A menu item which displays a bitmap instead of a text string.
*/

class CBitmapMenuItem : public BMenuItem {
	BBitmap			*bm;

	void Draw();

	void GetContentSize( float *width, float *height )
	{
		*width = bm->Bounds().right + 1;
		*height = bm->Bounds().bottom + 1;
	}

public:
	CBitmapMenuItem( BBitmap *inBM, BMessage *message, char shortcut = NULL,
		uint32 modifiers = NULL )
		: BMenuItem( "", message, shortcut, modifiers )
	{
		bm = inBM;
	}

	BBitmap *Bitmap() { return bm; }
	
	void SelectItem( BMenuItem *inItem );
};

#endif /* __C_BitmapMenu_H__ */
