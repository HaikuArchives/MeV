/* ===================================================================== *
 * OperatorWindow.h (MeV/User Interface)
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
 *  Shows the list of operators
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

#ifndef __C_OperatorWindow_H__
#define __C_OperatorWindow_H__

#include "MeVDoc.h"
#include "DocWindow.h"
#include "WindowState.h"
#include "DynamicMenu.h"
#include "AppHelp.h"

class CMultiColumnListView;
class CEventTrack;

	/**	Real-time operator editor window. */

class COperatorWindow : public CDocWindow, public CObserver {
	CMultiColumnListView	*operList;
	CEventTrack			*watchTrack;
	BButton				*editButton,
						*applyButton;
	CDynamicMenuInstance	plugInMenuInstance;
//	BMenu				*plugInMenu;
	
	enum EControlIDs {
		Select_ID 		= 'selc',
		ClickCheckbox_ID	= 'clck',
		Edit_ID			= 'edit',
	};
	
	void MessageReceived( BMessage *msg );
	void MenusBeginning();
	void OnUpdate( BMessage *inMsg );

public:
	COperatorWindow( CWindowState &inState, CMeVDoc &inDocument );
	~COperatorWindow();
	
		// Set which track we are currently viewing
	void SetTrack( CEventTrack *inViewTrack );
};

#endif /* __C_OperatorWindow_H__ */
