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
#define D_ACCESS(x) //PRINT(x)		// Accessors
#define D_HOOK(x) //PRINT(x)		// BView Implementation
#define D_INTERNAL(x) //PRINT(x)	// Internal Operations

// ---------------------------------------------------------------------------
// Constructor/Destructor

CConsoleContainerView::CConsoleContainerView(
	BRect frame,
	const char *name)
	:	BView(frame, name, B_FOLLOW_ALL_SIDES,
			  B_WILL_DRAW | B_FRAME_EVENTS)
{
	D_ALLOC(("CConsoleContainerView::CConsoleContainerView(%s)\n", name));

	SetViewColor(tint_color(ui_color(B_PANEL_BACKGROUND_COLOR),
							B_DARKEN_1_TINT));
}

// ---------------------------------------------------------------------------
// Accessors

CConsoleView *
CConsoleContainerView::GetNextSelected(
	long *index) const
{
	D_ACCESS(("CConsoleContainerView::GetNextSelected()\n"));

	// iterate through the list of console-views, and return the nth
	// selected console, if available
	long count = 0;
	for (long i = 0; i < CountSlots(); i++)
	{
		if (SlotAt(i)->IsSelected())
		{
			if (count == *index)
			{
				(*index)++;
				return SlotAt(i);
			}
			count++;
		}
	}

	return NULL;
}

// ---------------------------------------------------------------------------
// Operations

void
CConsoleContainerView::AddSlot(
	CConsoleView *view,
	long atIndex)
{
	CConsoleView *beforeSlot = NULL;
	if (atIndex >= 0)
		beforeSlot = SlotAt(atIndex);
	AddChild(view, beforeSlot);
}

void
CConsoleContainerView::DeselectAll()
{
	for (int32 i = 0; i < CountSlots(); i++)
		SlotAt(i)->SetSelected(false);
}

CConsoleView *
CConsoleContainerView::FindSlot(
	const char *name) const
{
	return dynamic_cast<CConsoleView *>(FindView(name));
}

CConsoleView *
CConsoleContainerView::SlotAt(
	long index) const
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
	long index)
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
		view->MoveTo(horizontalOffset - Bounds().left,
					 0.0 - Bounds().top);
		horizontalOffset += view->Frame().Width() + 1.0;
	}

	_updateScrollBar(B_HORIZONTAL);

	// tell the window that we might be no longer zoomed, 
	// in case we were before
	Window()->FrameResized(Window()->Frame().Width(),
						   Window()->Frame().Height());
}

void
CConsoleContainerView::SelectAll()
{
	for (long i = 0; i < CountSlots(); i++)
		SlotAt(i)->SetSelected(true);
}

// ---------------------------------------------------------------------------
// BView Implementation

void
CConsoleContainerView::AttachedToWindow()
{
	BScrollBar *scrollBar;

	// add horizontal scrollbar
	BRect hsFrame(Frame());
	hsFrame.InsetBy(-1.0, -1.0);
	hsFrame.top = hsFrame.bottom - B_H_SCROLL_BAR_HEIGHT;;
	hsFrame.right -= B_V_SCROLL_BAR_WIDTH;
	scrollBar = new BScrollBar(hsFrame, "", this, 0.0, 0.0, B_HORIZONTAL);
	Window()->AddChild(scrollBar);
	scrollBar->SetSteps(15.0, 150.0);

	// add vertical scrollbar
	BRect vsFrame(Frame());
	vsFrame.InsetBy(-1.0, -1.0);
	vsFrame.left = vsFrame.right - B_V_SCROLL_BAR_WIDTH;
	vsFrame.bottom -= B_H_SCROLL_BAR_HEIGHT;
	scrollBar = new BScrollBar(vsFrame, "", this, 0.0, 0.0, B_VERTICAL);
	Window()->AddChild(scrollBar);
	scrollBar->SetSteps(10.0, 100.0);

	ResizeBy(- vsFrame.Width(), - hsFrame.Height());
}

void
CConsoleContainerView::FrameResized(
	float width,
	float height)
{
	_updateScrollBar(B_HORIZONTAL);
	_updateScrollBar(B_VERTICAL);
}

void
CConsoleContainerView::GetPreferredSize(
	float *width,
	float *height)
{
	*width = -1.0;
	*height = 0.0;

	for (int32 i = 0; i < CountSlots(); i++)
	{
		BRect rect = SlotAt(i)->Bounds();
		*width += rect.Width() + 1.0;
		if (rect.Height() > *height)
			*height = rect.Height();
	}
}

void
CConsoleContainerView::MouseDown(
	BPoint point)
{
	D_HOOK(("CConsoleContainerView::MouseDown()\n"));

	if (!(modifiers() & B_SHIFT_KEY))
		DeselectAll();
}

// ---------------------------------------------------------------------------
// Internal Operations

void
CConsoleContainerView::_updateScrollBar(
	orientation posture)
{
	D_INTERNAL(("CConsoleView::_updateScrollBar()\n"));

	float preferredWidth, preferredHeight;
	GetPreferredSize(&preferredWidth, &preferredHeight);
	BScrollBar *scrollBar = ScrollBar(posture);

	switch (posture)
	{
		case B_HORIZONTAL:
		{
			float currentWidth = Bounds().Width();
			if (preferredWidth > currentWidth)
			{
				scrollBar->SetRange(0.0, preferredWidth - currentWidth);
				scrollBar->SetProportion(currentWidth / preferredWidth);
			}
			else
			{
				scrollBar->SetRange(0.0, 0.0);
				scrollBar->SetProportion(1.0);
			}
			break;
		}
		case B_VERTICAL:
		{
			float currentHeight = Bounds().Height();
			if (preferredHeight > currentHeight)
			{
				scrollBar->SetRange(0.0, preferredHeight - currentHeight);
				scrollBar->SetProportion(currentHeight / preferredHeight);
			}
			else
			{
				scrollBar->SetRange(0.0, 0.0);
				scrollBar->SetProportion(1.0);
			}
			break;
		}
	}
}

// END - ConsoleContainerView.cpp
