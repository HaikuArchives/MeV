/* ===================================================================== *
 * ColorDialogWindow.h (MeV/UI)
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

#ifndef __C_ColorDialogWindow_H__
#define __C_ColorDialogWindow_H__

#include "DialogWindow.h"

// Application Kit
#include <Message.h>
#include <Messenger.h>

class CColorDialogWindow :
	public CDialogWindow
{

public:							// Constants

	enum messages
	{
								REVERT_REQUESTED = 'revt',

								VALUE_CHANGED
	};

public:							// Constructor/Destructor

								CColorDialogWindow(
									BRect frame,
									rgb_color color,
									BMessage *message,
									const BMessenger &target,
									BWindow *parent);

	virtual						~CColorDialogWindow();

public:							// BWindow Implementation

	virtual void				MessageReceived(
									BMessage *message);

private:						// Instance Data

	BColorControl *				m_colorControl;

	rgb_color					m_initialColor;

	BMessage *					m_message;

	BMessenger					m_target;
};

#endif /* __C_ColorDialogWindow_H__ */
