/* ===================================================================== *
 * StripView.cpp (MeV/UI)
 * ===================================================================== */

#include "StripView.h"

#include "BorderView.h"
#include "BorderButton.h"
#include "Idents.h"
#include "IFFReader.h"
#include "IFFWriter.h"
#include "ResourceUtils.h"
#include "StripFrameView.h"

// Support Kit
#include <Debug.h>

// Debugging Macros
#define D_ALLOC(x) //PRINT(x)			// Constructor/Destructor
#define D_HOOK(x) //PRINT(x)			// CScrollTarget Implementation
#define D_MESSAGE(x) //PRINT(x)			// MessageReceived()
#define D_OPERATION(x) //PRINT(x)		// Operations

// ---------------------------------------------------------------------------
// Constructor/Destructor

CStripView::CStripView(
	CStripFrameView &inFrame,
	BRect frame,
	const char *name,
	bool makeScroller,
	bool makeMagButtons)
	:	CScrollerTarget(frame.OffsetToCopy(B_ORIGIN),
	  					name, B_FOLLOW_ALL_SIDES,
	  					B_WILL_DRAW | B_FRAME_EVENTS),
		frame(inFrame),
		m_labelView(NULL),
		m_rulerView(NULL),
		rightScroller(NULL),
		rightSpacer(NULL),
		magIncButton(NULL),
		magDecButton(NULL),
		m_removable(true),
		m_verticalZoom(0)
{
	D_ALLOC(("CStripView::CStripView('%s')\n", name));
	
	BRect rect(Bounds());

	m_container = new CScrollerTarget(rect, NULL,
									  B_FOLLOW_LEFT_RIGHT,
									  B_FRAME_EVENTS);
	m_container->AddChild(this);

	BRect scrollerRect(rect);
	scrollerRect.InsetBy(-1.0, -1.0);
	scrollerRect.left = scrollerRect.right - B_V_SCROLL_BAR_WIDTH;

	if (makeScroller)
	{
		ResizeBy(- B_V_SCROLL_BAR_WIDTH, 0.0);

		if (makeMagButtons)
		{
			BRect magRect(scrollerRect);
			magRect.top = magRect.bottom - B_H_SCROLL_BAR_HEIGHT;
			magDecButton = new CBorderButton(magRect, NULL,
											 ResourceUtils::LoadImage("SmallMinus"),
											 new BMessage(ZoomOut_ID), true,
											 B_FOLLOW_BOTTOM | B_FOLLOW_RIGHT,
											 B_WILL_DRAW);
			m_container->AddChild(magDecButton);

			magRect.OffsetBy(0.0, - B_H_SCROLL_BAR_HEIGHT);
			magIncButton = new CBorderButton(magRect, NULL, 
											 ResourceUtils::LoadImage("SmallPlus"),
											 new BMessage(ZoomIn_ID), true,
											 B_FOLLOW_BOTTOM | B_FOLLOW_RIGHT,
											 B_WILL_DRAW);
			m_container->AddChild(magIncButton);
			
			scrollerRect.bottom = magRect.top;
		}

		rightScroller = new CScroller(scrollerRect, NULL, this,
									  0.0, 50.0, B_VERTICAL);
		m_container->AddChild(rightScroller);
	}
	else
	{
		ResizeBy(- B_V_SCROLL_BAR_WIDTH, 0.0);
		rightSpacer = new CBorderView(scrollerRect, NULL,
									  B_FOLLOW_RIGHT | B_FOLLOW_TOP_BOTTOM,
									  B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE);
		m_container->AddChild(rightSpacer);
	}

	m_container->SetTarget(this);
}

// ---------------------------------------------------------------------------
// Hook Functions

float
CStripView::MinimumHeight() const
{
	float height = 0.0;
	if (rightScroller)
		height += 3 * B_H_SCROLL_BAR_HEIGHT + 1.0;
	if (magIncButton)
		height += B_H_SCROLL_BAR_HEIGHT;
	if (magDecButton)
		height += B_H_SCROLL_BAR_HEIGHT;
	if (magIncButton && magDecButton)
		height += 1.0;
	if (RulerView())
		height += RulerView()->Bounds().Height() + 1.0;

	return height;
}

// ---------------------------------------------------------------------------
// Accessors

void
CStripView::SetLabelView(
	CStripLabelView *labelView) {	

	if (m_labelView) {
		TopView()->RemoveChild(m_labelView);
		ResizeBy(m_labelView->Bounds().Width(), 0.0);
		MoveBy(- m_labelView->Bounds().Width(), 0.0);
		delete m_labelView;
		m_labelView = NULL;
	}

	if (labelView) {
		ResizeBy(- labelView->Bounds().Width(), 0.0);
		MoveBy(labelView->Bounds().Width(), 0.0);
		TopView()->AddChild(labelView);
		m_labelView = labelView;
		m_labelView->attach(this);
	}
}

void
CStripView::SetRulerView(
	CScrollerTarget *rulerView)
{
	if (m_rulerView) {
		TopView()->RemoveChild(m_rulerView);
		if (rightScroller)
		{
			rightScroller->MoveBy(0.0, - m_rulerView->Bounds().Height() - 1.0);
			rightScroller->ResizeBy(0.0, m_rulerView->Bounds().Height() + 1.0);
		}
		MoveBy(0.0, - m_rulerView->Bounds().Height());
		ResizeBy(0.0, m_rulerView->Bounds().Height());
		delete m_rulerView;
		m_rulerView = NULL;
	}

	if (rulerView)
	{
		MoveBy(0.0, rulerView->Bounds().Height());
		ResizeBy(0.0, - rulerView->Bounds().Height());
		TopView()->AddChild(rulerView);
		m_rulerView = rulerView;
		if (LabelView())
		{
			m_rulerView->MoveBy(LabelView()->Bounds().Width(), 0.0);
		}
		if (rightScroller)
		{
			rightScroller->MoveBy(0.0, m_rulerView->Bounds().Height() + 1.0);
			rightScroller->ResizeBy(0.0, - m_rulerView->Bounds().Height() - 1.0);
		}
	}
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
	if (visible != m_selectionVisible)
	{
		m_selectionVisible = visible;
		if (visible)
			OnGainSelection();
		else
			OnLoseSelection();
	}
}

void
CStripView::ZoomBy(
	int32 diff)
{
	if (diff != 0)
	{
		m_verticalZoom += diff;
		ZoomChanged(diff);
	}
}

// ---------------------------------------------------------------------------
// Serialization

void
CStripView::ExportSettings(
	BMessage *settings) const
{
	settings->AddBool("removable", m_removable);
	settings->AddFloat("scroll_value", ScrollValue(B_VERTICAL));
	settings->AddInt32("zoom_value", ZoomValue());
}

void
CStripView::ImportSettings(
	const BMessage *settings)
{
	settings->FindBool("removable", &m_removable);
	settings->FindInt32("zoom_value", &m_verticalZoom);
	ZoomChanged(m_verticalZoom);
	float scroll = 0.0;
	settings->FindFloat("scroll_value", &scroll);
	SetScrollValue(scroll, B_VERTICAL);
}

void
CStripView::ReadState(
	CIFFReader &reader,
	BMessage *settings)
{
	int8 removable;
	reader >> removable;
	settings->AddBool("removable", (bool)removable);
	float scroll;
	reader >> scroll;
	settings->AddFloat("scroll_value", scroll);
	int32 zoom;
	reader >> zoom;
	settings->AddInt32("zoom_value", zoom);
}

void
CStripView::WriteState(
	CIFFWriter &writer,
	const BMessage *settings)
{
	int8 removable;
	settings->FindBool("removable", (bool *)&removable);
	writer << removable;
	float scroll;
	settings->FindFloat("scroll_value", &scroll);
	writer << scroll;
	int32 zoom;
	settings->FindInt32("zoom_value", &zoom);
	writer << zoom;
}

// ---------------------------------------------------------------------------
// CScrollTarget Implementation

void
CStripView::AllAttached()
{
	if (magDecButton)
		magDecButton->SetTarget(this);
	if (magIncButton)
		magIncButton->SetTarget(this);
}

void
CStripView::AttachedToWindow()
{
	bounds = Bounds();
	CScrollerTarget::AttachedToWindow();
	AdjustScrollers();
}

void
CStripView::FrameResized(
	float width,
	float height)
{
	D_HOOK(("CStripView<%s>::FrameResized()\n", Name()));

	AdjustScrollers();
}

void
CStripView::MessageReceived(
	BMessage *message)
{
	D_MESSAGE(("CStripView::MessageReceived()\n"));

	switch (message->what)
	{
		case REMOVE_STRIP:
		{
			D_MESSAGE((" -> REMOVE_STRIP\n"));
			if (FrameView()->RemoveStrip(this))
				delete m_container;
			FrameView()->PackStrips();
			break;
		}
		case ZoomIn_ID:
		{
			D_MESSAGE((" -> ZoomIn_ID\n"));

			ZoomBy(1);
			break;
		}
		case ZoomOut_ID:
		{
			D_MESSAGE((" -> ZoomOut_ID\n"));

			ZoomBy(-1);
			break;
		}
		default:
		{
			CScrollerTarget::MessageReceived(message);
		}
	}
}

// END - StripView.cpp
