/* ===================================================================== *
 * WindowState.h (MeV)
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
 *	A window class that keeps track of it's state (position, open-ness)
 *	in a seperate structure which can be used to manipulate the window,
 *	even if the window is not open.
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

#ifndef __C_WindowState_H__
#define __C_WindowState_H__

//#include "AppHelp.h"

// Interface Kit
#include <Window.h>
// Support Kit
#include <Locker.h>

class BCursor;
class CWindowState;

class CAppWindow :
	public BWindow
{

protected:
	bool QuitRequested();

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

public:							// BWindow Implementation

	virtual void				WindowActivated(
									bool active );

public:							// Operations

	void						RememberState(
									CWindowState &state);

private:							// Instance Data

	CWindowState *					m_state;

	// Current cursor image
//	BCursor	*						m_cursor;

	// true if cursor is hidden
//	bool							m_cursorHidden;
};

class CWindowState {
	friend class CAppWindow;

	BLocker		lock;
	CAppWindow	*w;
	BRect		wRect;

	void OnWindowOpen( CAppWindow *inWindow );
	void OnWindowClosing();

public:

	CWindowState( BRect inRect ) { wRect = inRect; w = NULL; }

		/**	Call this to activate the window. Returns false if not open. */
	bool Activate();
	bool IsOpen()
	{
		bool		result;
		
		lock.Lock();
		result = (w != NULL);
		lock.Unlock();
		
		return result;
	}
	
		/**	Call this to request that the window be closed. */
	void Close();
	
		/**	Call this to get the rectangle of the window when it was last open. */
	BRect Rect();
	BPoint Pos();

	/**	Returns a pointer to the window. You should lock before calling this. */
	CAppWindow *				Window() const
								{ return w; }
	
		/**	Lock the window state. */
	void Lock() { lock.Lock(); }
	
		/**	Unlock the window state. */
	void Unlock() { lock.Unlock(); }
	
		/**	Set position of window */
	void SetPos( const BPoint &inPos )
	{
		wRect.OffsetTo( inPos );
	}

		/**	Set position of window */
	void SetPos( const BRect &inPos )
	{
		wRect = inPos;
	}
};

#endif /* __C_WindowState_H__ */
