/* ===================================================================== *
 * StringEditView.h (MeV/UI)
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
 *  Displays a bordered, self-resizing, editable BTextView for editing
 *	a single line of text
 * ---------------------------------------------------------------------
 * History:
 *	06/02/2000	cell
 *		Original implementation
 *	07/10/200	cell
 *		added ctor for alternative font & colors
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __C_StringEditView_H__
#define __C_StringEditView_H__

// Application Kit
#include <Invoker.h>
// Interfacem Kit
#include <View.h>
// Support Kit
#include <String.h>

class BTextView;

class CStringEditView :
	public BView,
	public BInvoker
{

public:							// Constructor/Destructor

								CStringEditView(
									BRect frame,
									BString text,
									BMessage *message,
									BMessenger messenger);

								CStringEditView(
									BRect frame,
									BString text,
									BFont *font,
									rgb_color textColor,
									rgb_color bgColor,
									BMessage *message,
									BMessenger messenger);

	virtual 					~CStringEditView();

public:							// Accessors

	BTextView *					TextView() const
								{ return m_textView; }

public:							// BMenuItem Implementation

	virtual void				AllAttached();

	virtual void				AttachedToWindow();

	virtual void				DetachedFromWindow();

	virtual void				Draw(
									BRect updateRect);

	virtual void				KeyDown(
									const char *bytes,
									int32 numBytes);

	virtual void				MouseDown(
									BPoint point);

private:						// Instance Data

	BString						m_text;

	BTextView *					m_textView;

	bool						m_restoreAvoidFocus;
};

#endif /* __C_IconMenuItem_H__ */
