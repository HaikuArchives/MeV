/* ===================================================================== *
 * QuickKeyMenuItem.h (MeV/User Interface)
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
 
#ifndef __C_QuickKeyMenuItem_H__
#define __C_QuickKeyMenuItem_H__

// Interface Kit
#include <MenuItem.h>
// Support Kit
#include <String.h>

/**
 *		CQuickKeyMenuItem is a BMenuItem subclass, inspired by my all-time
 *		favorite application, Deluxe Paint by Electronic Arts.
 *	
 *		This class implements a menu item which can display a shortcut key
 *		without any modifier keys. It can also display shortcut keys which
 *		have long names like "Enter" or "Space".
 *		
 *		The primary reason for this class is for apps which want to have
 *		functions attached to unmodified keys. A typical example is a paint
 *		program, which might associate "d" for free (d)raw and "r" for
 *		(r)ectangle. A typical paint program "power user" keeps one hand on
 *		the keyboard, and the other on the mouse. Requiring painting functions
 *		to be activated with two-key combinations makes the product inherently
 *		more difficult to use by the expert user.
 *		
 *		Unfortunately, this class does not have any way of displaying
 *		modifier key symbols along with the names (for example, if you wanted
 *		"Shift-space" [i.e. shift + space with no command key held down] to
 *		mean something, this class cannot display shift modifier symbol).
 *
 *		The messages emitted by this class don't have all of the attributes
 *		that messages emitted by real menus have. That's because the
 *		BMenu::Invoke() function is private and not available to subclasses,
 *		so the underlying BInvoker function must be called.
 *		@author	Talin, Christoper Lenz.  
 */

class CQuickKeyMenuItem
	:	public BMenuItem
{
								
public:							// Constructor/Destructor

								CQuickKeyMenuItem(
									const char *label,
									BMessage *message,
									char shortcutKey,
									const char *shortcutLabel);

public:							// BMenuItem Implementation

	virtual void				DrawContent();

	virtual void				GetContentSize(
									float *width,
									float *height);
	
	/**	Given an unmodified shortcut key, search a menu for a
	 menu item which has a matching shortcut key, and invoke it.
	 Returns true if such an item was found.
	 You should call this in your app when you recieve a
	 keydown event which does not have a command modifier
	 (unless, of course, the current focus view is a BTextView
	 of other view which could logically accept the key).	*/
	 
	static bool					TriggerShortcutMenu(
									BMenu *menu,
									char key);

private:						// Instance Data

	char						m_shortcutKey;

	BString						m_shortcutLabel;
};

#endif /* __C_QuickKeyMenuItem_H__ */
