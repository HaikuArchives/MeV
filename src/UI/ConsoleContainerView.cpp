/* ===================================================================== *
 * ConsoleContainerView.cpp (MeV/UI)
 * ===================================================================== */

#include "ConsoleContainerView.h"

#include "ConsoleView.h"

// Interface Kit
#include <ScrollBar.h>
#include <Window.h>
// Support Kit
#include <Debug.h>

// Debugging Macros
#define D_ALLOC(x) //PRINT(x)		// Constructor/Destructor
#define D_HOOK(x) //PRINT(x)		// BView Implementation

// ---------------------------------------------------------------------------
// Constructor/Destructor

CConsoleContainerView::CConsoleContainerView(
	BRect frame,
	const char *name)
	:	BView(frame, name, B_FOLLOW_ALL_SIDES, B_WILL_DRAW)
{
	D_ALLOC(("CConsoleContainerView::CConsoleContainerView(%s)\n", name));

	SetViewColor(tint_color(ui_color(B_PANEL_BACKGROUND_COLOR),
							B_DARKEN_1_TINT));
}

// ---------------------------------------------------------------------------
// Operations

void
CConsoleContainerView::AddSlot(
	CConsoleView *view,
	int32 atIndex)
{
	CConsoleView *beforeSlot = NULL;
	if (atIndex > 0)
		beforeSlot = SlotAt(atIndex);
	AddChild(view, beforeSlot);
}

CConsoleView *
CConsoleContainerView::FindSlot(
	const char *name)
{
	return dynamic_cast<CConsoleView *>(FindView(name));
}

CConsoleView *
CConsoleContainerView::SlotAt(
	int32 index)
{
	return dynamic_cast<CConsoleView *>(ChildAt(index));
}

void
CConsoleContainerView::RemoveSlot(
	CConsoleView *view)
{
	RemoveChild(view);
}

CConsoleView *
CConsoleContainerView::RemoveSlot(
	int32 index)
{
	CConsoleView *view = SlotAt(index);
	if (view)
		RemoveSlot(view);
	return view;
}

void
CConsoleContainerView::Pack()
{
	float horizontalOffset = 0.0;
	for (int32 i = 0; i < CountSlots(); i++)
	{
		CConsoleView *view = SlotAt(i);
		view->ResizeToPreferred();
		view->MoveTo(horizontalOffset, 0.0);
		horizontalOffset = view->Frame().right + 1.0;
	}
}

// ---------------------------------------------------------------------------
// AttachedToWindow

void
CConsoleContainerView::AttachedToWindow()
{
	// add horizontal scrollbar
	BRect hsFrame = Frame();
	hsFrame.InsetBy(-1.0, -1.0);
	hsFrame.top = hsFrame.bottom - B_H_SCROLL_BAR_HEIGHT;;
	hsFrame.right -= B_V_SCROLL_BAR_WIDTH;
	m_hScrollBar = new BScrollBar(hsFrame, "", this, 0, 0, B_HORIZONTAL);
	Window()->AddChild(m_hScrollBar);

	// add vertical scrollbar
	BRect vsFrame = Frame();
	vsFrame.InsetBy(-1.0, -1.0);
	vsFrame.left = vsFrame.right - B_V_SCROLL_BAR_WIDTH;
	vsFrame.bottom -= B_H_SCROLL_BAR_HEIGHT;
	m_vScrollBar = new BScrollBar(vsFrame, "", this, 0, 0, B_VERTICAL);
	Window()->AddChild(m_vScrollBar);

	ResizeBy(- vsFrame.Width(), - hsFrame.Height());
}

void
CConsoleContainerView::GetPreferredSize(
	float *width,
	float *height)
{
	*width = 0.0;
	*height = 0.0;

	for (int32 i = 0; i < CountSlots(); i++)
	{
		BRect rect = SlotAt(i)->Bounds();
		if (rect.Width() > *width)
			*width = rect.Width();
		if (rect.Height() > *height)
			*height = rect.Height();
	}

	if (ScrollBar(B_VERTICAL))
		*width += B_V_SCROLL_BAR_WIDTH;
	if (ScrollBar(B_HORIZONTAL))
		*height += B_H_SCROLL_BAR_HEIGHT;
}

// END - ConsoleContainerView.cpp
