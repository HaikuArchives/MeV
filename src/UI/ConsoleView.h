/* ===================================================================== *
 * ConsoleView.h (MeV/UI)
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
 *	10/13/2000	cell
 *		Original implementation
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __C_ConsoleView_H__
#define __C_ConsoleView_H__

#include "Observer.h"

// Interface Kit
#include <View.h>
// Support Kit
#include <String.h>

class CConsoleContainerView;

/**
 *		@author	Christoper Lenz.  
 */
 
class CConsoleView :
	public BView,
	public CObserver
{

public:							// Constructor/Destructor

								CConsoleView(
									BRect frame,
									const char *name,
									bool expanded = true,
									uint32 resizingMode = B_FOLLOW_LEFT | B_FOLLOW_RIGHT,
									uint32 flags = B_WILL_DRAW | B_FRAME_EVENTS);

	virtual						~CConsoleView();

public:							// Hook Functions

	virtual void				Expanded(
									bool expanded)
								{ }

	virtual void				Selected(
									bool expanded)
								{ }

	virtual bool				SubjectReleased(
									CObservable *subject)
								{ return false; }

	virtual void				SubjectUpdated(
									BMessage *message)
								{ }

public:							// Accessors

	CConsoleContainerView *		Container() const;

	bool						IsExpanded() const
								{ return m_expanded; }
	void						MakeExpandable(
									bool expandable = true)
								{ m_expandable = expandable; }
	void						SetExpanded(
									bool expanded = true);

	bool						IsSelected() const
								{ return m_selected; }
	void						MakeSelectable(
									bool selectable = true)
								{ m_selectable = selectable; }
	void						SetSelected(
									bool selected = true);

public:							// BView Implementation

	virtual void				AttachedToWindow();

	virtual void				Draw(
									BRect updateRect);

	virtual void				MessageReceived(
									BMessage *message);

	virtual void				MouseDown(
									BPoint point);

public:							// CObserver Implementation

	bool						Released(
									CObservable *subject);

	void						Updated(
									BMessage *message);

private:						// Instance Data

	bool						m_expandable;
	bool						m_expanded;

	bool						m_selectable;
	bool						m_selected;
};

#endif /* __C_ConsoleView_H__ */
