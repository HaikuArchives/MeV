/* ===================================================================== *
 * ConsoleView.cpp (MeV/UI)
 * ===================================================================== */

#include "ConsoleView.h"

#include "Observable.h"

// Application Kit
#include <Message.h>
// Interface Kit
#include <Window.h>
// Support Kot
#include <Debug.h>

// Debugging Macros
#define D_ALLOC(x) //PRINT(x)		// Constructor/Destructor
#define D_HOOK(x) //PRINT(x)		// BView Implementation
#define D_MESSAGE(x) //PRINT(x)		// MessageReceived()

// ---------------------------------------------------------------------------
// Constructor/Destructor

CConsoleView::CConsoleView(
	BRect frame,
	const char *name,
	bool expanded,
	uint32 resizingMode,
	uint32 flags)
	:	BView(frame, name, resizingMode, flags),
		m_expandable(true),
		m_expanded(expanded),
		m_selectable(true),
		m_selected(false)
{
	D_ALLOC(("CConsoleView::CConsoleView(%s)\n", name));
}

CConsoleView::~CConsoleView()
{
	D_ALLOC(("CConsoleView::~CConsoleView()\n"));
}

// ---------------------------------------------------------------------------
// Accessors

void
CConsoleView::SetExpanded(
	bool expanded)
{
	if (m_expandable && (expanded != m_expanded))
	{
		m_expanded = expanded;
		Expanded(m_expanded);
		Invalidate();
	}
}

void
CConsoleView::SetSelected(
	bool selected)
{
	if (m_selectable && (selected != m_selected))
	{
		m_selected = selected;
		Selected(m_selected);
		Invalidate();
	}
}

// ---------------------------------------------------------------------------
// BView Implementation

void
CConsoleView::AttachedToWindow()
{
	D_HOOK(("CConsoleView::AttachedToWindow()\n"));

	if (Parent())
		SetViewColor(tint_color(Parent()->ViewColor(), B_LIGHTEN_1_TINT));
	else
		SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	SetFont(be_plain_font);
}

void
CConsoleView::Draw(
	BRect updateRect)
{
	D_HOOK(("CConsoleView::Draw()\n"));

	BRect rect(Bounds());
	rgb_color color;
	
	if (m_selectable && m_selected)
		color = ui_color(B_KEYBOARD_NAVIGATION_COLOR);
	else
		color = ViewColor();

	// draw box
	BeginLineArray(4);
	AddLine(rect.LeftBottom(), rect.LeftTop(),
			tint_color(color, B_LIGHTEN_1_TINT));
	AddLine(rect.LeftTop(), rect.RightTop(),
			tint_color(color, B_LIGHTEN_1_TINT));
	AddLine(rect.RightTop(), rect.RightBottom(),
			tint_color(color, B_DARKEN_1_TINT));
	AddLine(rect.LeftBottom(), rect.RightBottom(),
			tint_color(color, B_DARKEN_1_TINT));
	EndLineArray();
}

void
CConsoleView::MessageReceived(
	BMessage *message)
{
	D_MESSAGE(("CConsoleView::MessageReceived()\n"));

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
			BView::MessageReceived(message);
		}
	}
}

void
CConsoleView::MouseDown(
	BPoint point)
{
	D_HOOK(("CConsoleView::MouseDown()\n"));

	int32 buttons = B_PRIMARY_MOUSE_BUTTON;
	Window()->CurrentMessage()->FindInt32("buttons", &buttons);

	if (m_selectable && (buttons == B_PRIMARY_MOUSE_BUTTON))
	{
		if (!m_selected)
			SetSelected(true);
		else
			SetSelected(false);
	}
}

// ---------------------------------------------------------------------------
// CObserver Implementation

bool
CConsoleView::Released(
	CObservable *subject)
{
	bool released = false;

	if (LockLooper())
	{
		released = SubjectReleased(subject);
		UnlockLooper();
	}

	return released;
}

void
CConsoleView::Updated(
	BMessage *message)
{
	if (Window())
		Window()->PostMessage(message, this);
}

// END - ConsoleView.cpp
