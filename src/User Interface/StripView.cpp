/* ===================================================================== *
 * StripView.cpp (MeV/User Interface)
 * ===================================================================== */

#include "StripView.h"
// General
#include "Idents.h"
// User Interface
#include "BorderView.h"
#include "BorderButton.h"
// Support
#include "ResourceUtils.h"

// Support Kit
#include <Debug.h>

// Debugging Macros
#define D_ALLOC(x) //PRINT (x)			// Constructor/Destructor
#define D_HOOK(x) //PRINT (x)			// CScrollTarget Implementation
#define D_OPERATION(x) //PRINT (x)		// Operations

// ---------------------------------------------------------------------------
// Constructor/Destructor

CStripView::CStripView(
	CStripFrameView &inFrame,
	BRect frame,
	const char *name,
	bool makeScroller,
	bool makeMagButtons )
	:	CScrollerTarget(frame.OffsetToCopy(B_ORIGIN),
	  					name, B_FOLLOW_ALL,
	  					B_WILL_DRAW | B_FRAME_EVENTS),
		frame(inFrame),
		rightScroller(NULL),
		rightSpacer(NULL),
		magIncButton(NULL),
		magDecButton(NULL)
{
	BRect rect(Bounds());

	m_container = new CScrollerTarget(rect, NULL,
									  B_FOLLOW_LEFT_RIGHT, B_FRAME_EVENTS );
	m_container->AddChild(this);

	if (makeScroller)
	{
		ResizeBy(-14.0, 0.0);
		float bottom = Bounds().bottom + 1;

		if (makeMagButtons)
		{
			magIncButton = new CBorderButton(BRect(rect.right - 13,
												   rect.bottom - 27,
												   rect.right + 1,
												   rect.bottom - 13),
											 NULL, 
											 ResourceUtils::LoadImage("SmallPlus"),
											 new BMessage(ZoomOut_ID),
											 B_FOLLOW_BOTTOM | B_FOLLOW_RIGHT,
											 B_WILL_DRAW);
			m_container->AddChild(magIncButton);

			magDecButton = new CBorderButton(BRect(rect.right - 13,
												   rect.bottom - 13,
												   rect.right + 1,
												   rect.bottom + 1),
											 NULL,
											 ResourceUtils::LoadImage("SmallMinus"),
											 new BMessage(ZoomIn_ID),
											 B_FOLLOW_BOTTOM | B_FOLLOW_RIGHT,
											 B_WILL_DRAW);
			m_container->AddChild(magDecButton);
			
			bottom = Bounds().bottom - 27;
		}

		rightScroller = new CScroller(BRect(rect.right - 13,
											rect.top - 1,
											rect.right + 1,
											bottom),
									  NULL, this, 0.0, 50.0, B_VERTICAL);
		m_container->AddChild(rightScroller);
	}
	else
	{
		ResizeBy(-14.0, 0.0);
		rightSpacer = new CBorderView(BRect(rect.right - 13,
											rect.top - 1,
											rect.right + 1,
											rect.bottom + 1), NULL,
									  B_FOLLOW_RIGHT | B_FOLLOW_TOP_BOTTOM,
									  B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE );
		m_container->AddChild(rightSpacer);
	}

	m_container->SetTarget(this);
	SetZoomTarget(this);
}

// ---------------------------------------------------------------------------
// Operations

void
CStripView::SetScrollValue(
	float value,
	orientation posture)
{
	CScrollerTarget::SetScrollValue(value, posture);
	ScrollTo(scrollValue.x, scrollValue.y);
}

void
CStripView::SetSelectionVisible(
	bool visible)
{
	if (visible != selectionVisible)
	{
		selectionVisible = visible;
		if (visible)
		{
			OnGainSelection();
		}
		else
		{
			OnLoseSelection();
		}
	}
}

void
CStripView::SetZoomTarget(
	BHandler *handler)
{
	if (magIncButton)
	{
		magIncButton->SetTarget(handler);
	}
	if (magDecButton)
	{
		magDecButton->SetTarget(handler);
	}
}

// ---------------------------------------------------------------------------
// CScrollTarget Implementation

void
CStripView::AttachedToWindow()
{
	bounds = Bounds();
	CScrollerTarget::AttachedToWindow();
	AdjustScrollers();
}

void
CStripView::Draw(
	BRect updateRect)
{
	StrokeLine(BPoint(0.0, 0.0), BPoint(300.0, 100.0));
	StrokeLine(BPoint(300.0, 0.0), BPoint(0.0, 100.0));
}

void
CStripView::FrameResized(
	float width,
	float height)
{
	AdjustScrollers();
}

// END - StripView.cpp
