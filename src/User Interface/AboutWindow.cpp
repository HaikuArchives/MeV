/* ===================================================================== *
 * AboutWindow.cpp (MeV/User Interface)
 * ===================================================================== */

#include "AboutWindow.h"

// ---------------------------------------------------------------------------
// Constructor/Destructor

CAboutWindow::CAboutWindow(
	CWindowState &state)
	:	CAppWindow(
			state, state.Rect(), "About MeV",
			B_TITLED_WINDOW, B_NOT_RESIZABLE | B_NOT_ZOOMABLE )
{
//	BRect rect(Bounds());

	Show();
}

// END - AboutWindow.cpp