/* ===================================================================== *
 * AppWindow.h (MeV/Framework)
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
 *  04/08/2000	cell
 *		General cleanup in preparation for initial SourceForge checkin
 *	09/30/2000	cell
 *		Separated from WindowState.h/cpp
 * ---------------------------------------------------------------------
 *  To Do:
 *
 * ===================================================================== */

#ifndef __C_AppWindow_H__
#define __C_AppWindow_H__

#include "Observable.h"
#include "Observer.h"

// Interface Kit
#include <Window.h>

class BCursor;
class CWindowState;

class CAppWindow :
	public BWindow,
	public CObserver
{

public:							// Constructor/Destructor

								CAppWindow(
									BRect frame,
									const char *title,
									window_type type,
									uint32 flags,
									uint32 workspace = B_CURRENT_WORKSPACE);

								CAppWindow(
									CWindowState &state,
									BRect frame,
									const char *title, 
									window_type type,
									uint32 flags,
									uint32 workspace = B_CURRENT_WORKSPACE);
	
	virtual						~CAppWindow();

public:							// Hook Functions

	virtual void				SubjectReleased(
									CObservable *subject) = 0;

	virtual void				SubjectUpdated(
									BMessage *message)
								{ }

public:							// BWindow Implementation

	virtual void				MessageReceived(
									BMessage *message);

	virtual bool				QuitRequested();

public:							// CObserver Implementation

	virtual void				Released(
									CObservable *subject);

	virtual void				Updated(
									BMessage *message)
								{ PostMessage(message, this); }

public:							// Operations

	void						RememberState(
									CWindowState &state);

private:							// Instance Data

	CWindowState *				m_state;
};

#endif /* __C_AppWindow_H__ */
