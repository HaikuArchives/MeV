/* ===================================================================== *
 * StdButton.h (MeV/UI)
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

#ifndef __StdButton_H__
#define __StdButton_H__

#include <Control.h>

/**	
 *		A button class which pushes on but never off.
 *		@author	Talin, Christoper Lenz.   
 */
 
class CPushOnButton
	:	public BControl
{

public:							// Constructor/Destructor

								CPushOnButton(
									BRect frame,
									const char *name,
									BBitmap	*bitmap,
									BMessage *message,
									uint32 resizingMode = B_FOLLOW_LEFT | B_FOLLOW_TOP,
									uint32 flags = B_WILL_DRAW);

								~CPushOnButton();

public:							// BControl Implementation

	virtual void				AttachedToWindow();

	virtual void				Draw(
									BRect updateRect);

	virtual void				MouseDown(
									BPoint point);

	virtual void				MouseUp(
									BPoint point);

private:						// Instance Data

	BBitmap	*					m_glyphs[2];
};

/**	A button which toggles it's state.
*/
class CToggleButton
	:	public CPushOnButton
{

public:							// Constructor/Destructor

	CToggleButton(	BRect frame,
					const char *name,
					BBitmap	*bitmap,
					BMessage *message,
					uint32 resizingMode = B_FOLLOW_LEFT | B_FOLLOW_TOP,
					uint32 flags = B_WILL_DRAW);

public:							// CPushOnButton Implementation

	virtual void				MouseDown(
									BPoint point);
};

#endif /* __StdButton_H__*/
