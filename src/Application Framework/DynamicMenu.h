/* ===================================================================== *
 * DynamicMenu.h (MeV)
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
 *	A dynamic menu definition. Dynamic menus are used for menus who's
 *	contents are not known at compile time, such as a list of open windows
 *	or a list of available plug-ins or fonts.
 *
 *	Each dynamic menu represents a list of menus. The menus must be of
 *	the standard BMenuItem class -- custom menus are not supported.
 *
 *	Dynamic menu definitions can be shared between multiple windows.
 *	Each window must have it's own CDynamicMenuInstance. When an item is
 *	added, removed, or changed, all CDynamicMenuInstance objects associated
 *	with this menu definition are marked for update. The update does not
 *	actually occur until the instance's CheckMenusChanged member is called,
 *	which would normally be called by the window's MenusBeginning() member.
 *	This avoids needless updating of the menu structures. In addition,
 *	the menu instance tries to do as little work as possible when updating
 *	the menu structure to avoid flickering or other cosmetic problems.
 * ---------------------------------------------------------------------
 * History:
 *	1997		Talin
 *		Original implementation
 *	04/08/2000	cell
 *		General cleanup in preparation for initial SourceForge checkin
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __C_DynamicMenu_H__
#define __C_DynamicMenu_H__

// Gnu C Library
#include <string.h>
// Interface Kit
#include <MenuItem.h>
// Support Kit
#include <Locker.h>

class CDynamicMenuDef {
	friend class CDynamicMenuInstance;

	BList	instanceList;		//	List of DynamicMenu objects observing this
	BList	itemList;			//	List of dynamic menu items
	BLocker	lock;
	bool		itemChanged,			//	TRUE if some items have changed
			itemListChanged;		//	TRUE if some items added / deleted

	struct CItemDef {
		char		*name;
		BMessage	*msg;
		BHandler	*handler;
		char		shortcut;
		uint32	modifiers;
		
		CItemDef(	char		*inName,
					BMessage	*inMsg,
					BHandler	*inHandler,
					char		inShortcut,
					uint32	inMod )
		{
			name = strdup( inName );
			msg = inMsg;
			handler = inHandler;
			shortcut = inShortcut;
			modifiers = inMod;
		}
		
		~CItemDef() { delete name; delete msg; }
	};
public:
		/**	Constructor. */
	CDynamicMenuDef();

		/**	Destructor. */
	~CDynamicMenuDef();

		/**	Adds an item to the list of menu items. Returns the index of the
			item added.
		*/
	int32 AddItem(	char			*label,
					BMessage		*msg,
					BHandler		*handler = NULL,
					char			shortcut = NULL,
					uint32		modifiers = NULL );

		/**	Adds an item to the list of menu items at a specific index.
		*/
	int32 AddItem(	int32		index,
					char			*label,
					BMessage		*msg,
					BHandler		*handler = NULL,
					char			shortcut = NULL,
					uint32		modifiers = NULL );

		/**	Replaces the Nth menu item of the dynamic menu. */
	bool ReplaceItem(	int32		index,
						char			*label,
						BMessage		*msg,
						BHandler		*handler = NULL,
						char			shortcut = NULL,
						uint32		modifiers = NULL );

		/**	Deletes the Nth menu item of the dynamic menu. */
	bool DeleteItem( int32 inIndex );
	
		/**	Returns the number of items in this list. */
	int32 CountItems() { return itemList.CountItems(); }

		/**	Returns the name of the Nth item. */
	char *NameAt( int32 n );

		/**	Returns the BMessage of the Nth item. */
	BMessage *MessageAt( int32 n );

		/**	Returns the handler of the Nth item. */
	BHandler *HandlerAt( int32 n );
	
		/**	Locks the dynamic menu list. This list must be locked before any
			changes can occur.
		*/
	void Lock() { lock.Lock(); }

		/**	Unlocks the dynamic menu list. */
	void Unlock();
};

	/**	An instance of a dynamic menu (see CDynamicMenuDef). */
class CDynamicMenuInstance {
	friend class CDynamicMenuDef;

	CDynamicMenuDef &def;
	int32		startIndex;				//	Start index of items
	BList		itemList;				//	List of created items
	bool			itemChanged,				//	TRUE if some items have changed
				itemListChanged;			//	TRUE if some items added / deleted
	BMenu		*baseMenu;				//	Menu to insert dynamic menus into
	BMessage		*modelMessage;

public:
		/**	Indicates that the dynamic menu items should be inserted into the menu
			starting at the Nth item. An index of -1 means to insert the menu
			items at the end of the list.
			
			<p>Because of the fact that the starting index of the dynamic menu is
			a fixed integer, it is recommended that you not use more than one
			set of dynamic menu items within a single menu.
		*/
	CDynamicMenuInstance(	 CDynamicMenuDef	&inDef );
	
		/**	Set the menu that the list of dynamic menus is supposed to be
			added to, and optionally the index of where to add them.
		*/
	void SetBaseMenu( BMenu *inMenu, int32 inIndex = -1 );

		/**	Set additional attributes specific to this instance. */
	void SetMessageAttributes( BMessage *inModelMessage );

		/**	Destructor -- removes this from the def's list of instances */
	~CDynamicMenuInstance();
	
		/**	Check if the menu items need to be updated, and update them if so. */
	void CheckMenusChanged();
};

#endif /* __C_DynamicWindow_H__ */
