/* ===================================================================== *
 * DocWindow.h (MeV)
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
 *  Abstract document window class
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

#ifndef __C_DocWindow_H__
#define __C_DocWindow_H__

#include "Document.h"
#include "WindowState.h"

const uint32			Select_ID = '#SEL';		//	We lost selection
const uint32			Activate_ID = '#ACT';		//	Activate this window

class CDocWindow :
	public CAppWindow
{

	static CDocWindow	*activeDocWin;
	friend class			CDocument;

protected:
	void CalcWindowTitle(const char *inTypeName);
	void RecalcWindowTitle();
	
protected:

	CDocument		&document;
	BMenuBar			*menus;					//	Pointer to menu bar
	BMenu			*windowMenu;				//	Menu of opened/openable windows.
	bool				updateMenus;				//	True means refigure windows menu
	int16			windowNumber;			//	Which # view of document is this
	int16			windowMenuStart;			//	Start item of window menu

	bool QuitRequested();
	void WindowActivated( bool active );
	void MessageReceived( BMessage *msg );
	void MenusBeginning();
	
	void BuildWindowMenu( BMenu *inMenu );
	
public:

		/**	Constructor. */
	CDocWindow(	BRect		frame,
				CDocument	&inDocument,
				const char	*inWinTypeName = NULL,
				window_type = B_DOCUMENT_WINDOW, uint32 flags = 0 );
	CDocWindow(	CWindowState	&inState,
				CDocument	&inDocument,
				const char	*inWinTypeName = NULL,
				window_type = B_DOCUMENT_WINDOW, uint32 flags = 0 );
	virtual ~CDocWindow();

		/**	Returns a pointer to the document for this DocWindow. */
	CDocument *Document() { document.Acquire(); return &document; }
	
		/**	Acquires the active selection token. */
	void AcquireSelectToken();
	
		/**	Return true if this window has the active selection token */
	bool HasSelectToken() { return this == activeDocWin; }
	
		/**	Return the document window which has the active token. */
	static CDocWindow *ActiveDocWindow() { return activeDocWin; }
};

#endif /* __C_DocWindow_H__ */