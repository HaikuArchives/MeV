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
 *  History:
 *  1997		Talin
 *		Original implementation
 *  04/08/2000		cell
 *		General cleanup in preparation for initial SourceForge checkin
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __C_DocWindow_H__
#define __C_DocWindow_H__

#include "AppWindow.h"

// Support Kit
#include <String.h>

class CDocument;
class CToolBar;

/**	Abstract document window class.
 *		@author		Talin, Christopher Lenz
 */

class CDocWindow :
	public CAppWindow
{
	friend class CDocument;

public:							// Constants

	enum messages
	{
								//	Activate this window.
								ACTIVATE = 'dwdA',

								//	Hide all windows for this document.
								HIDE_ALL,

								//	Show all windows for this document.
								SHOW_ALL,

								/** Show all windows for this document.
								 *	@param workspace_id	Which workspace.
								 */
								MOVE_TO_WORKSPACE,

								//	We lost selection.
								SELECTED,

								DONE_SAVING
	};

public:							// Constructor/Destructor

								CDocWindow(
									BRect frame,
									CDocument *document,
									bool isMaster = true,
									const char *inWinTypeName = NULL,
									window_type = B_DOCUMENT_WINDOW,
									uint32 flags = 0);

								CDocWindow(
									CWindowState &state,
									CDocument *document,
									bool isMaster = true,
									const char	*inWinTypeName = NULL,
									window_type = B_DOCUMENT_WINDOW,
									uint32 flags = 0);

	virtual						~CDocWindow();

public:							// Hook Functions

	/**	Returns a pointer to the document of this window. 
	 *	You may override this method to return your CDocument 
	 *	subclass.
	 */
	virtual CDocument *			Document();
	
	/**	Implement this method to return the size of the content that
	 *	is being displayed by this window. The default implementation
	 *	returns the current window bounds.
	 */
	virtual void				GetContentSize(
									float *width,
									float *height) const;

public:							// Accessors

	/**	Return the document window which has the active token.	*/
	static CDocWindow *			ActiveDocWindow()
								{ return s_activeDocWin; }

	/**	Return true if this window has the active selection token.	*/
	bool						HasSelectToken()
								{ return (this == s_activeDocWin); }
	
	bool						IsMasterWindow() const
								{ return m_isMaster; }

	/**	Set and get the current toolbar.	*/
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

	virtual void				FrameResized(
									float width,
									float height);

	virtual void				MessageReceived(
									BMessage *message);

	virtual void				MenusBeginning();
	
	virtual bool				QuitRequested();

	virtual bool				SubjectReleased(
									CObservable *subject);

	virtual void				WindowActivated(
									bool active);

	virtual void				Zoom(
									BPoint origin,
									float width,
									float height);

protected:						// Internal Operations

	void						CalcWindowTitle(
									const char *documentName = NULL,
									const char *windowName = NULL);

	void						SetWindowNumber(
									int32 number)
								{ m_windowNumber = number; }

	void						UpdateWindowMenu();

private:						// Instance Data

	CDocument *					m_document;

	bool						m_isMaster;

	CToolBar *					m_toolBar;

	/**	Menu of opened/openable windows.	*/
	BMenu *						m_windowMenu;

	/**	Which # view of document is this.	*/
	int16						m_windowNumber;

	/**	Start item of window menu.	*/
	int16						m_windowMenuStart;

	/**	The 'sub-name' of the window, i.e. the part after the colon.	*/
	BString						m_name;

	bool						m_waitingToQuit;

	bool						m_zoomed;

	bool						m_zooming;

	BRect						m_manualSize;

private:						// Class Data

	static CDocWindow *			s_activeDocWin;
};

#endif /* __C_DocWindow_H__ */
