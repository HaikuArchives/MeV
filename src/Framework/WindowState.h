/* ===================================================================== *
 * WindowState.h (MeV/Framework)
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
 *		Moved CAppWindow into AppWindow.h/cpp
 * ---------------------------------------------------------------------
 *  To Do:
 *
 * ===================================================================== */

#ifndef __C_WindowState_H__
#define __C_WindowState_H__

// Interface Kit
#include <Rect.h>
// Support Kit
#include <Locker.h>

class BCursor;
class CWindowState;

/**
 *	A window class that keeps track of it's state (position, open-ness)
 *	in a seperate structure which can be used to manipulate the window,
 *	even if the window is not open.
 *	@author		Talin, Christopher Lenz
 *	@package	Framework
 */
class CWindowState
{
	friend class CAppWindow;

public:							// Constructor/Destructor

								CWindowState(
									BRect frame)
									:	m_window(NULL),
										m_frame(frame)
								{ }

								CWindowState()
									:	m_window(NULL),
										m_frame(BRect(0.0, 0.0, -1.0, -1.0))
								{ }

public:							// Operations

	/**	Call this to activate the window. Returns false if not open. */
	bool						Activate();

	bool						IsOpen();

	/**	Call this to request that the window be closed.	*/
	void						Close();

	/**	Call this to get the rectangle of the window when it was last open.	*/
	BRect						Rect();

	BPoint						Pos();

	/**	Returns a pointer to the window. You should lock before calling this.	*/
	CAppWindow *				Window() const
								{ return m_window; }

	/**	Lock the window state.	*/
	bool						Lock()
								{ return m_lock.Lock(); }

	/**	Unlock the window state. */
	void						Unlock()
								{ m_lock.Unlock(); }

	/**	Set position of window.	*/
	void						SetPos(
									const BPoint &pos)
								{ m_frame.OffsetTo(pos); }

	/**	Set the position of window.	*/
	void						SetPos(
									const BRect &rect)
								{ m_frame = rect; }

private:						// Internal Operations

	void						WindowOpened(
									CAppWindow *window);

	void						WindowClosed();

private:						// Instance Data

	BLocker						m_lock;

	CAppWindow *				m_window;

	BRect						m_frame;
};

#endif /* __C_WindowState_H__ */
