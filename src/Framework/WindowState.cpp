/* ===================================================================== *
 * WindowState.cpp (MeV/Framework)
 * ===================================================================== */

#include "WindowState.h"

#include "AppWindow.h"

// Application Kit
#include <AppDefs.h>
// Support Kit
#include <Debug.h>

// ---------------------------------------------------------------------------
// Operations

bool
CWindowState::IsOpen()
{
	bool result = false;
	if (Lock())
	{
		result = (m_window != NULL);
		Unlock();
	}
	
	return result;
}

bool
CWindowState::Activate()
{
	if (Lock())
	{
		if (m_window != NULL)
		{
			m_window->Activate();
			Unlock();
			return true;
		}
		Unlock();
	}

	return false;
}

BRect
CWindowState::Rect()
{
	if (Lock())
	{
		if (m_window != NULL)
			m_frame = m_window->Frame();
		Unlock();
	}

	return m_frame;
}

BPoint
CWindowState::Pos()
{
	Rect();
	return BPoint(m_frame.left, m_frame.top);
}

void
CWindowState::Close()
{
	if (Lock())
	{
		if (m_window != NULL)
		{
			BWindow *window = m_window;
		
			m_frame = window->Frame();
			m_window = NULL;
			window->Quit();
		}
		Unlock();
	}
}

// ---------------------------------------------------------------------------
// Internal Operations

void
CWindowState::WindowOpened(
	CAppWindow *window)
{
	if (Lock())
	{
		m_window = window;
		Unlock();
	}
}

void
CWindowState::WindowClosed()
{
	if (m_window != NULL)
	{
		if (Lock())
		{
			m_frame = m_window->Frame();
			m_window = NULL;
			Unlock();
		}
	}
}

// END - WindowState.cpp
