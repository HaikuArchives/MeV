/* ===================================================================== *
 * Splitter.h (MeV/User Interface)
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
 *	04/21/2000	cell
 *		Initial version
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __C_Splitter_H__
#define __C_Splitter_H__

// Interface Kit
#include <View.h>

#define SPLITPANE_STATE 'spst'

class CSplitter :
	public BView
{

public:							// Constants

	static const float			V_SPLITTER_WIDTH;

	static const float			H_SPLITTER_HEIGHT;

public:							// Constructor/Destructor

								CSplitter(
									BRect frame,
									BView *primaryTarget,
									BView *secondaryTarget,
									orientation posture = B_VERTICAL,
									uint32 resizingMode = B_FOLLOW_ALL_SIDES);

public:							// BView Implementation

	virtual void				AllAttached();

	virtual void				Draw(
									BRect updateRect);

	virtual void				MouseDown(
									BPoint point);

	virtual void				MouseMoved(
									BPoint point,
									uint32 transit,
									const BMessage *message);

	virtual void				MouseUp(
									BPoint point);

private:						// Instance Data

	BView *						m_primaryTarget;
	BView *						m_secondaryTarget;

	orientation					m_posture;

	bool						m_dragging;

private:						// Class Data

	static const rgb_color		WHITE_COLOR;
	static const rgb_color		GRAY_COLOR;
	static const rgb_color		MEDIUM_GRAY_COLOR;
	static const rgb_color		DARK_GRAY_COLOR;
};

#endif /* __C_Splitter_H__ */
