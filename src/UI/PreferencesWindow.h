/* ===================================================================== *
 * PreferencesWindow.h (MeV/UI)
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
 *  
 * ---------------------------------------------------------------------
 * History:
 *	1997		Talin
 *		Original implementation
 *	04/08/2000	cell
 *		General cleanup in preparation for initial SourceForge checkin
 *	08/06/2000	cell
 *		Separated from Junk
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __PreferencesWindow_H__
#define __PreferencesWindow_H__

#include "MeVApp.h"
#include "AppWindow.h"

class CTextSlider;

#define MAX_PREFERENCES_PANELS 32

class CPreferencesWindow
	:	public CAppWindow
{

public:							// Constants

	enum messages
	{
								DEFAULTS_REQUESTED = 'pwdA',

								REVERT_REQUESTED,

								PANEL_SELECTED,

								VALUE_CHANGED
	};

	enum panels
	{
								FEEDBACK_PANEL = 0,

								EDITING_PANEL
	};

public:							// Constructor/Destructor

								CPreferencesWindow(
									CWindowState &state);

								~CPreferencesWindow();

public:							// CAppWindow Implementation

	virtual void				MessageReceived(
									BMessage *message);

protected:						// Internal Operations

	BView *						AddPanel(
									const char *name);

	int32						CurrentPanel() const
								{ return m_currentPanel; }

	void						SetCurrentPanel(
									int index);

	void						ReadPrefs();
	void						WritePrefs();

private:

	void						AddFeedbackPanel();

	void						AddEditingPanel();

private:						// Instance Data

	BView *						m_bgView;

	BListView *					m_listView;

	BBox *						m_panels[MAX_PREFERENCES_PANELS];

	int32						m_panelCount;
	int32						m_currentPanel;

	CGlobalPrefs				m_prefs;
	BCheckBox *					m_cb[4][2];
	BCheckBox *					m_chan_cb;
	BCheckBox *					m_rect_cb;
	CTextSlider *				m_fbDelay;
};

#endif /* __PreferencesWindow_H__ */
