/* ===================================================================== *
 * DialogWindow.h (MeV/Framework)
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
 *  History:
 *  11/13/2000	cell
 *		Original implementation
 * ===================================================================== */

#ifndef __C_DialogWindow_H__
#define __C_DialogWindow_H__

// Interface Kit
#include <Window.h>

class CDialogBackgroundView;

/**	A special, small window that is associated with a parent window. */
class CDialogWindow :
	public BWindow
{

public:							// Constructor/Destructor

								CDialogWindow(
									BRect frame,
									BWindow *parent);

	virtual						~CDialogWindow();

public:							// Accessors

	BView *						BackgroundView() const;

	BWindow *					Parent() const;

	BRect						ContentFrame() const;

public:							// Operations

	BButton *					AddButton(
									const char *label,
									BMessage *message);

public:							// BWindow Implementation

	virtual void				Show();

private:						// Instance Data

	BWindow *					m_parent;

	CDialogBackgroundView *		m_backgroundView;

	float						m_buttonOffset;
};

#endif /* __C_DialogWindow_H__ */
