/* ===================================================================== *
 * WindowState.cpp (MeV/Application Framework)
 * ===================================================================== */

#include "WindowState.h"

// Application Kit
#include <AppDefs.h>
// Support Kit
#include <Debug.h>

// Debugging Macros
#define D_ALLOC(x) //PRINT (x)		// Constructor/Destructor
#define D_HOOK(x) //PRINT (x)		// BControl Implementation

// ---------------------------------------------------------------------------
// Constructor/Destructor

CAppWindow::CAppWindow(
	BRect frame,
	const char *title,
	window_type type,
	uint32 flags,
	uint32 workspaces = B_CURRENT_WORKSPACE)
	:	BWindow(frame, title, type, flags, workspaces),
		m_state(NULL)
//		m_cursor(B_CURSOR_SYSTEM_DEFAULT),
//		m_cursorHidden(false)
{
	D_ALLOC(("CAppWindow::CAppWindow()\n"));
}

CAppWindow::CAppWindow(
	CWindowState &state,
	BRect frame,
	const char *title,
	window_type type,
	uint32 flags,
	uint32 workspaces = B_CURRENT_WORKSPACE)
	:	BWindow(frame, title, type, flags, workspaces),
		m_state(NULL)
//		m_cursor(B_CURSOR_SYSTEM_DEFAULT),
//		m_cursorHidden(false)
{
	D_ALLOC(("CAppWindow::CAppWindow()\n"));

	RememberState(state);
}

CAppWindow::~CAppWindow()
{
	D_ALLOC(("CAppWindow::~CAppWindow()\n"));

	if (m_state)
	{
		m_state->OnWindowClosing();
	}
}

// ---------------------------------------------------------------------------
// BWindow Implementation

bool
CAppWindow::QuitRequested()
{
	if (m_state)
	{
		m_state->OnWindowClosing();
	}
	return true;
}

void
CAppWindow::WindowActivated(
	bool active)
{
//	RestoreCursor();
}

// ---------------------------------------------------------------------------
// Operations

void
CAppWindow::RememberState(
	CWindowState &state)
{
	m_state = &state;
	m_state->OnWindowOpen(this);
}

void
CWindowState::OnWindowOpen(
	CAppWindow *window)
{
	lock.Lock();
	w = window;
	lock.Unlock();
}
	
void CWindowState::OnWindowClosing()
{
	if (w != NULL)
	{
		lock.Lock();
		if (w != NULL)
		{
			wRect = w->Frame();
			w = NULL;
		}
		lock.Unlock();
	}
}

bool CWindowState::Activate()
{
	lock.Lock();
	if (w != NULL)
	{
		w->Activate();
		lock.Unlock();
		return true;
	}
	lock.Unlock();
	return false;
}

BRect CWindowState::Rect()
{
	lock.Lock();
	if (w != NULL) wRect = w->Frame();
	lock.Unlock();
	return wRect;
}

BPoint CWindowState::Pos()
{
	Rect();
	return BPoint( wRect.left, wRect.top );
}

void CWindowState::Close()
{
	lock.Lock();
	if (w != NULL)
	{
		BWindow		*wnd = w;
	
		wRect = wnd->Frame();
		w = NULL;
		wnd->Quit();
	}
	lock.Unlock();
}
