/* ===================================================================== *
 * RecentDocumentsMenu.h (MeV/UI)
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
 * History:
 *	10/08/2000	cell
 *		Initial Implementation.
 * ===================================================================== */

#ifndef __C_RecentDocumentsMenu_H__
#define __C_RecentDocumentsMenu_H__

#include <Menu.h>

/**
 *  A BMenu derived class displaying a list of the most recent documents.
 *	This class is basically merges BMenu with the BRoster method 
 *	GetRecentDocuments()
 *	@author		Christopher Lenz
 */
 
class CRecentDocumentsMenu
	:	public BMenu
{

public:							// Constructor/Destructor

/**	Constructor for a single document MIME types.
 *	@param name					Name (and potentially label) of the BMenu.
 *	@param message				A model message that should be sent when the
 *											user invokes an item in this menu.
 *	@param maxCount			The maximum number of recent documents to put
 *											in the menu.
 *	@param ofType				The MIME type of the documents. Specifying NULL 
 *				 							will fetch all files of all types.
 *	@param openedByApp		If you're only interested in files that want to 
 *											be opened by a specific application, specify that 
 *											application's signature. Otherwise specify NULL.
 */
								CRecentDocumentsMenu(
									const char *name,
									BMessage *message,
									int32 maxCount,
									const char *ofType = NULL,
									const char *openedByApp = NULL);

/**	Constructor for multiple document MIME types.
 *	@param name				Name (and potentially label) of the BMenu.
 *	@param message			A model message that should be sent when the
 *										user invokes an item in this menu.
 *	@param maxCount		The maximum number of recent documents to put
 *										in the menu.
 *	@param ofTypeList		Specify a pointer to an array of strings.
 *	@param ofTypeListCount	The number of types in ofTypeList.
 *	@param openedByApp		If you're only interested in files that want 
 *											to be opened by a specific application, 
 *											specify that application's signature. 
 *											Otherwise specify NULL.
 */
								CRecentDocumentsMenu(
									const char *name,
									BMessage *message,
									int32 maxCount,
									const char *ofTypeList[],
									int32 ofTypeListCount,
									const char *openedByApp = NULL);

	/**	Destructor. Does nothing.	*/
	virtual						~CRecentDocumentsMenu();

private:						// Internal Operations

	/**	Populate the menu with BMenuItems pointing to the recent
	 *		documents.
	 *	@param refList		The list of refs of recent documents, as returned 
	 *								by BRoster::GetRecentDocuments()
	 *	@param itemMessage	Model message for the menu items.
	 */
	 
	void						_populateMenu(
									BMessage *refList,
									BMessage *itemMessage);
};

#endif /* __C_RecentDocumentsMenu_H__ */
