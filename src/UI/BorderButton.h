/* ===================================================================== *
 * BorderButton.h (MeV/UI)
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
 *	04/18/2000	cell
 *		Updated mouse handling to use the new Interface Kit API
 *	09/12/2000	cell
 *		Added optional dimming of the button when window is not
 *		activated (similar to BBorderView).
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __C_BorderButton_H__
#define __C_BorderButton_H__

// Interface Kit
#include <Control.h>

/**   
 *		Buttons which can look good in the border.
 *		@author	Talin, Christoper Lenz.   
 */
 
class CBorderButton :
	public BControl
{

public:							//Constructor/Destructor

								CBorderButton(
									BRect frame,
									const char *name,
									BBitmap	*bitmap,
									BMessage *message,
									bool dimOnDeactivate = true,
									uint32 resizingMode = B_FOLLOW_LEFT | B_FOLLOW_TOP,
									uint32 flags = B_WILL_DRAW);

public:							// BControl Implementation

	virtual void				Draw(
									BRect updateRect);

	virtual void				DrawAfterChildren(
									BRect updateRect);

	virtual void				MouseDown(
									BPoint point);

	virtual void				MouseMoved(
									BPoint point,
									uint32 transit,
									const BMessage *message);

	virtual void				MouseUp(
									BPoint point);

	virtual void				WindowActivated(
									bool active);

private:						// Instance Data
	
	BBitmap	*					m_glyphs[2];

	bool						m_pressed;

	bool						m_tracking;

	bool						m_dimOnDeactivate;
};

#endif /* __C_BorderButton_H__ */
