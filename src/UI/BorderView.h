/* ===================================================================== *
 * BorderView.h (MeV/UI)
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
 *  Class which draws a decorative border and fill
 * ---------------------------------------------------------------------
 * History:
 *	1997		Talin
 *		Original implementation
 *	04/08/2000	cell
 *		General cleanup in preparation for initial SourceForge checkin
 *  04/15/2000	cell
 *		Merged CBorderView and CBevelBorderView into a single class,
 *		only separated by the border_style argument
 *	09/12/2000	cell
 *		Added optional dimming of the button when window is not
 *		activated (similar to BBorderView).
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __C_BorderView_H__
#define __C_BorderView_H__

// Interface Kit
#include <View.h>

class CBorderView :
	public BView
{

public:						// Constants

	enum border_style
	{
							NORMAL_BORDER,

							BEVEL_BORDER
	};

public:						// Constructor/Destructor


							CBorderView(
								BRect frame,
								const char *name,
								uint32 resizingMode,
								uint32 flags,
								bool dimOnDeactivate = true,
								const rgb_color *color = NULL,
								border_style style = NORMAL_BORDER);

public:						// BView Implementation

	virtual void			Draw(
								BRect updateRect);

	virtual void			DrawAfterChildren(
								BRect updateRect);

	virtual void			WindowActivated(
								bool active);

private:					// Instance Data

	rgb_color				m_color;

	border_style			m_style;

	bool					m_dimOnDeactivate;
};

#endif /* __C_BorderView_H__ */
