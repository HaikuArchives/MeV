/* ===================================================================== *
 * TextDisplay.h (MeV/User Interface)
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
 *  Similar to BorderView, but draws a shadow / shine border
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

#ifndef __C_TextDisplay_H__
#define __C_TextDisplay_H__
 
// Gnu C Library
#include <malloc.h>
#include <string.h>
// Interface Kit
#include <Control.h>
#include <Window.h>

	/**	Similar to BorderView, but draws a shadow / shine border */
class CTextDisplay : public BView {
	char			*text;
	alignment	align;
	rgb_color	backColor;
	bool			borders;

	void Draw( BRect updateRect );
	void AttachedToWindow()
	{
		if (Parent()) backColor = Parent()->ViewColor();
		SetViewColor( B_TRANSPARENT_32_BIT );
	}
	
public:
		/**	Constructor */
	CTextDisplay(	BRect		rect,
					const char	*name,
					bool			drawBorders = true,
					ulong		resizeFlags = B_FOLLOW_LEFT | B_FOLLOW_TOP,
					ulong		flags = B_WILL_DRAW )
		: BView( rect, name, resizeFlags, flags )
	{
		borders = drawBorders;
		text = NULL;
		align = B_ALIGN_CENTER;
	}
	
	~CTextDisplay() { if (text) free( text ); }

		/*	Change the text being displayed. */
	void SetText( char *inText, bool inRefresh = true )
	{
		if (text) free( text );
		text = strdup( inText );
		if (inRefresh && Window())
		{
			Window()->Lock();
			BRect	r( Bounds() );
			r.InsetBy( -1, -1 );
			Invalidate( r );
			Window()->Unlock();
		}
	}

		/*	Set the alignment of the text being displayed. */
	void SetAlignment( alignment inAlign ) { align = inAlign; }
};

#endif /* __C_TextDisplay_H__ */