/* ===================================================================== *
 * WindowState.cpp (MeV/Application Framework)
 * ===================================================================== */

#include "WindowState.h"

void CAppWindow::RememberState( CWindowState &inState )
{
	state = &inState;
	inState.OnWindowOpen( this );
}

CAppWindow::~CAppWindow()
{
	if (state) state->OnWindowClosing();
}

bool CAppWindow::QuitRequested()
{
	if (state) state->OnWindowClosing();
	return true;
}

void CAppWindow::SetCursor( const uint8 *inCursor )
{
	if (inCursor == NULL) inCursor = B_HAND_CURSOR;
	if (inCursor != cursorImage)
	{
		cursorImage = inCursor;
		be_app->SetCursor( cursorImage );
	}
}

void CAppWindow::HideCursor()
{
	if (be_app->IsCursorHidden() == false)
	{
		cursorHidden = true;
		be_app->HideCursor();
	}
}

void CAppWindow::ShowCursor()
{
	if (cursorHidden)
	{
		cursorHidden = false;
		be_app->ShowCursor();
	}
}

void CAppWindow::RestoreCursor()
{
	SetCursor( NULL );
	ShowCursor();
}

void CAppWindow::WindowActivated( bool active )
{
	RestoreCursor();
}

void CWindowState::OnWindowOpen( CAppWindow *inWindow )
{
	lock.Lock();
	w = inWindow;
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
