/* ===================================================================== *
 * BitmapTool.h (MeV/User Interface)
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
 *  Extends the CTool class to provide 'buttcons', ie buttons displaying
 *  icons in a tool bar
 * ---------------------------------------------------------------------
 * History:
 *	1997		Talin
 *		Original implementation
 *	04/08/2000	cell
 *		General cleanup in preparation for initial SourceForge checkin
 *	04/21/2000	cell
 *		Separated functionality from CToolBar class
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __C_BitmapTool_H__
#define __C_BitmapTool_H__

#include "Tool.h"

class CBitmapTool :
	public CTool {

public:									// Constructor/Destructor

										CBitmapTool(
											const char *name,
											BBitmap *bitmap,
											BMessage *message,
											int32 mode = TOGGLE_MODE,
											uint32 flags = 0);

										~CBitmapTool();

public:									// CTool Implementation

	virtual void						DrawTool(
											BView *owner,
											BRect toolRect);

	virtual void						GetContentSize(
											float *width,
											float *height) const;

	virtual void						Clicked(
											BPoint point,
											uint32 buttons);

	virtual void						ValueChanged();

private:								// Instance Data

	BBitmap *							m_bitmap;

	BBitmap *							m_disabledBitmap;
};

#endif /* __C_BitmapTool_H__ */
