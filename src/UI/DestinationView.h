/* ===================================================================== *
 * DestinationView.h (MeV/UI)
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
 *		Dan Walton (dwalton)
 *		Christopher Lenz (cell)
 *
 * ---------------------------------------------------------------------
 * Purpose:
 * A class that manages the m_destinations.  
 * ---------------------------------------------------------------------
 * History:
 *	6/21/2000		dwalton
 *		Original implementation.
 *	8/01/2000		dwalton
 *      Name change, many improvements.
 * ---------------------------------------------------------------------
 * To Do:
 * 
 * ===================================================================== */

#ifndef __C_DestinationView_H__
#define __C_DestinationView_H__

#include "ConsoleView.h"

// Support Kit
#include <String.h>

class CColorWell;
class CDestination;

class BBitmap;
class BCheckBox;
class BPopUpMenu;
class BStringView;
class BTextControl;

class CDestinationView
	:	public CConsoleView
{

public:							// Constants

	enum messages
	{
								NAME_CHANGED = 'dmoA',

								RENAME,

								MUTED,

								SOLO,

								LATENCY_CHANGED,

								CHANGE_COLOR,

								COLOR_CHANGED
	};

public:							// Constructor/Destructor

								CDestinationView(
									BRect frame,
									CDestination *destination);

	virtual 					~CDestinationView();

public:							// Accessors

	CDestination *				Destination() const
								{ return m_destination; }

public:							// CConsoleView Implementation

	virtual void				AttachedToWindow();

	virtual void				Draw(
									BRect updateRect);

	virtual void				GetPreferredSize(
									float *width,
									float *height);

	virtual void				MessageReceived(
									BMessage *message);

	virtual void				MouseDown(
									BPoint point);

	virtual bool				SubjectReleased(
									CObservable *subject);

	virtual void				SubjectUpdated(
									BMessage *message);

private:						// Internal Operations

	void						_showContextMenu(
									BPoint point);

	void						_updateIcon();

	void						_updateLatency();

	void						_updateName();

private:						// Instance Data

	CDestination *				m_destination;

	BString						m_truncatedName;

	BRect						m_nameFrame;

	bool						m_editingName;

	BBitmap *					m_icon;

	BPoint						m_iconOffset;

	CColorWell *				m_colorWell;

	BCheckBox *					m_mutedCheckBox;

	BCheckBox *					m_soloCheckBox;

	BTextControl *				m_latencyControl;
	BStringView *				m_msLabel;

	CConsoleView *				m_configView;

	CConsoleView *				m_monitorView;

	BPopUpMenu *				m_contextMenu;
};

#endif /* __C_DestinationView_H__ */
