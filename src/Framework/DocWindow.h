/* ===================================================================== *
 * DocWindow.h (MeV/Framework)
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

#include "WindowState.h"

const uint32 Select_ID		= '#SEL';		//	We lost selection
const uint32 Activate_ID	= '#ACT';		//	Activate this window

class CDocument;
class CToolBar;

class CDocWindow :
	public CAppWindow
{
	friend class CDocument;

public:							// Constructor/Destructor

								CDocWindow(
									BRect frame,
									CDocument *document,
									const char *inWinTypeName = NULL,
									window_type = B_DOCUMENT_WINDOW,
									uint32 flags = 0);

								CDocWindow(
									CWindowState &state,
									CDocument *document,
									const char	*inWinTypeName = NULL,
									window_type = B_DOCUMENT_WINDOW,
									uint32 flags = 0);

	virtual						~CDocWindow();

public:							// Accessors

	// Return the document window which has the active token
	static CDocWindow *			ActiveDocWindow()
								{ return s_activeDocWin; }

	// Returns a pointer to the document for this DocWindow
	virtual CDocument *			Document();
	
	// Return true if this window has the active selection token
	bool						HasSelectToken()
								{ return (this == s_activeDocWin); }
	
	// set and get the current toolbar
	CToolBar *					ToolBar() const
								{ return m_toolBar; }
	void						SetToolBar(
									CToolBar *toolBar);

	BMenu *						WindowMenu() const
								{ return m_windowMenu; }
	void						SetWindowMenu(
									BMenu *menu)
								{ m_windowMenu = menu; }

	int32						WindowNumber() const
								{ return m_windowNumber; }

public:							// Operations

	// Acquires the active selection token
	void						AcquireSelectToken();
	
public:							// CAppWindow Implementation

	virtual void				MessageReceived(
									BMessage *message);

	virtual void				MenusBeginning();
	
	virtual bool				QuitRequested();

	virtual void				WindowActivated(
									bool active);

protected:						// Internal Operations

	void						CalcWindowTitle(
									const char *inTypeName);

	void						RecalcWindowTitle();
	
	void						SetWindowNumber(
									int32 number)
								{ m_windowNumber = number; }

	void						UpdateWindowMenu();

private:						// Instance Data

	CDocument *					m_document;

	CToolBar *					m_toolBar;

	// Menu of opened/openable windows
	BMenu *						m_windowMenu;

	//	Which # view of document is this
	int16						m_windowNumber;

	//	Start item of window menu
	int16						m_windowMenuStart;

private:						// Class Data

	static CDocWindow *			s_activeDocWin;
};

#endif /* __C_DocWindow_H__ */
