/* ===================================================================== *
 * AppWindow.cpp (MeV/Framework)
 * ===================================================================== */

#include "AppWindow.h"

#include "WindowState.h"

// Application Kit
#include <AppDefs.h>
// Support Kit
#include <Debug.h>

// Debugging Macros
#define D_ALLOC(x) //PRINT(x)		// Constructor/Destructor
#define D_HOOK(x) //PRINT(x)		// BWindow/CObserver Implementation
#define D_MESSAGE(x) //PRINT(x)		// MessageReceived()

// ---------------------------------------------------------------------------
// Constructor/Destructor

CAppWindow::CAppWindow(
	BRect frame,
	const char *title,
	window_type type,
	uint32 flags,
	uint32 workspaces)
	:	BWindow(frame, title, type, flags, workspaces),
		CObserver(),
		m_state(NULL)
{
	D_ALLOC(("CAppWindow::CAppWindow()\n"));
}

CAppWindow::CAppWindow(
	CWindowState &state,
	BRect frame,
	const char *title,
	window_type type,
	uint32 flags,
	uint32 workspaces)
	:	BWindow(frame, title, type, flags, workspaces),
		CObserver(),
		m_state(NULL)
{
	D_ALLOC(("CAppWindow::CAppWindow()\n"));

	RememberState(state);
}

CAppWindow::~CAppWindow()
{
	D_ALLOC(("CAppWindow::~CAppWindow()\n"));

	if (m_state)
		m_state->WindowClosed();
}

// ---------------------------------------------------------------------------
// BWindow Implementation

void
CAppWindow::MessageReceived(
	BMessage *message)
{
	D_MESSAGE(("CAppWindow::MessageReceived()\n"));

	switch (message->what)
	{
		case CObservable::UPDATED:
		{
			D_MESSAGE((" -> CObservable::UPDATED\n"));

			SubjectUpdated(message);
			break;
		}
		default:
		{
			BWindow::MessageReceived(message);
		}
	}
}

bool
CAppWindow::QuitRequested()
{
	if (m_state)
		m_state->WindowClosed();

	return true;
}

// ---------------------------------------------------------------------------
// CObservable Implementation

void
CAppWindow::Released(
	CObservable *subject)
{
	D_OBSERVE(("CAppWindow<%p>::Released()\n", this));

	if (Lock())
	{
		SubjectReleased(subject);
		Unlock();
	}
}

// ---------------------------------------------------------------------------
// Operations

void
CAppWindow::RememberState(
	CWindowState &state)
{
	m_state = &state;
	m_state->WindowOpened(this);
}

// END - AppWindow.cpp
