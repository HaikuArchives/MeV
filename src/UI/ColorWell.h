/* ===================================================================== *
 * ColorWell.h (MeV/UI)
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
 *		Merged CColorWell and CBevelColorWell into a single class,
 *		only separated by the border_style argument
 *	09/12/2000	cell
 *		Added optional dimming of the button when window is not
 *		activated (similar to BColorWell).
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __C_ColorWell_H__
#define __C_ColorWell_H__

// Interface Kit
#include <Control.h>

class CColorWell :
	public BControl
{

public:						// Constructor/Destructor


							CColorWell(
								BRect frame,
								BMessage *message,
								uint32 resizingMode = B_FOLLOW_LEFT | B_FOLLOW_TOP);

public:						// Accessors

	rgb_color				Color() const
							{ return m_color; }

	void					SetColor(
								rgb_color color)
							{ m_color = color; }

public:						// BView Implementation

	virtual void			Draw(
								BRect updateRect);

	virtual void			KeyDown(
								const char *bytes,
								int32 numBytes);

	virtual void			MessageReceived(
								BMessage *message);

	virtual void			MouseDown(
								BPoint point);

	virtual void			MouseMoved(
								BPoint point,
								uint32 transit,
								const BMessage *message);

private:					// Instance Data

	rgb_color				m_color;
};

#endif /* __C_ColorWell_H__ */
