/* ===================================================================== *
 * Splitter.cpp (MeV/UI)
 * ===================================================================== */

#include "Splitter.h"

#include "CursorCache.h"

// Application Kit
#include <Application.h>
#include <Cursor.h>
#include <Message.h>
#include <MessageFilter.h>
// Interface Kit
#include <Window.h>
// Support Kit
#include <Debug.h>

// ---------------------------------------------------------------------------
// BMessageFilter subclass for handling of B_MOUSE_MOVED while dragging

class CSplitterMessageFilter
	:	public BMessageFilter
{

public:							// Constructor/Destructor

								CSplitterMessageFilter(
									CSplitter *splitter);

public:							// BMessageFilter Implementaton

	virtual filter_result		Filter(
									BMessage *message,
									BHandler **target);

private:						// Instance Data

	CSplitter *					m_splitter;
};

// ---------------------------------------------------------------------------
// Class Data Initialization

const float
CSplitter::V_SPLITTER_WIDTH = 4.0;

const float
CSplitter::H_SPLITTER_HEIGHT = 4.0;

const rgb_color
CSplitter::WHITE_COLOR = {255, 255, 255, 255};

const rgb_color
CSplitter::GRAY_COLOR = ui_color(B_PANEL_BACKGROUND_COLOR);

const rgb_color
CSplitter::MEDIUM_GRAY_COLOR = tint_color(ui_color(B_PANEL_BACKGROUND_COLOR),
										  B_DARKEN_1_TINT);

const rgb_color
CSplitter::DARK_GRAY_COLOR = tint_color(ui_color(B_PANEL_BACKGROUND_COLOR),
										B_DARKEN_2_TINT);

const bigtime_t
CSplitter::DRAG_LAG_TIME = 25 * 1000;

// ---------------------------------------------------------------------------
// Constructor/Destructor

CSplitter::CSplitter(
	BRect frame,
	BView *primaryTarget,
	BView *secondaryTarget,
	orientation posture,
	uint32 resizingMode)
	:	BView(frame, "", resizingMode, B_WILL_DRAW | B_FRAME_EVENTS),
		m_primaryTarget(primaryTarget),
		m_secondaryTarget(secondaryTarget),
		m_posture(posture),
		m_dragging(false),
		m_messageFilter(NULL)
{
	SetViewColor(B_TRANSPARENT_COLOR);
	
	switch (m_posture)
	{
		case B_VERTICAL:
		{
			if (frame.Width() != V_SPLITTER_WIDTH)
				ResizeTo(V_SPLITTER_WIDTH, frame.Height());
			break;
		}
		case B_HORIZONTAL:
		{
			if (frame.Height() != H_SPLITTER_HEIGHT)
				ResizeTo(frame.Width(), H_SPLITTER_HEIGHT);
			break;
		}
	}
}

CSplitter::~CSplitter()
{
}

// ---------------------------------------------------------------------------
// Hook Functions

void
CSplitter::MoveRequested (
	float diff)
{
	switch (m_posture)
	{
		case B_HORIZONTAL:
		{
			if (m_primaryTarget->Bounds().Height() + diff < 0.0)
				diff = - m_primaryTarget->Bounds().Height();
			else if (m_secondaryTarget->Bounds().Height() - diff < 0.0)
				diff = m_secondaryTarget->Bounds().Height();
			m_primaryTarget->ResizeBy(0.0, diff);
			m_secondaryTarget->MoveBy(0.0, diff);
			m_secondaryTarget->ResizeBy(0.0, -diff);
			MoveBy(0.0, diff);
			break;
		}
		case B_VERTICAL:
		{
			if (m_primaryTarget->Bounds().Width() + diff < 0.0)
				diff = - m_primaryTarget->Bounds().Width();
			else if (m_secondaryTarget->Bounds().Width() - diff < 0.0)
				diff = m_secondaryTarget->Bounds().Width();
			m_primaryTarget->ResizeBy(diff, 0.0);
			m_secondaryTarget->MoveBy(diff, 0.0);
			m_secondaryTarget->ResizeBy(-diff, 0.0);
			MoveBy(diff, 0.0);
			break;
		}
	}
}

// ---------------------------------------------------------------------------
// Operations

void
CSplitter::Dragged(
	BPoint point)
{
	bigtime_t when;
	Window()->CurrentMessage()->FindInt64("when", &when);
	if (when < (m_lastDragTime + DRAG_LAG_TIME))
		return;
	switch (m_posture)
	{
		case B_HORIZONTAL:
		{
			float diff = point.y - m_offset;
			MoveRequested(diff);
			m_offset = point.y;
			break;
		}
		case B_VERTICAL:
		{
			float diff = point.x - m_offset;
			MoveRequested(diff);
			m_offset = point.x;
			break;
		}
	}
	m_lastDragTime = when;
}

// ---------------------------------------------------------------------------
// BView Implementation

void
CSplitter::Draw(
	BRect updateRect)
{
	BRect rect(Bounds());

	switch (m_posture)
	{
		case B_VERTICAL:
		{
			BeginLineArray(5);
			AddLine(rect.LeftTop(), rect.LeftBottom(), DARK_GRAY_COLOR);
			AddLine(rect.RightTop(), rect.RightBottom(), DARK_GRAY_COLOR);
			rect.InsetBy(1.0, 1.0);
			AddLine(rect.LeftTop(), rect.LeftBottom(), WHITE_COLOR);
			AddLine(rect.RightTop(), rect.RightBottom(), MEDIUM_GRAY_COLOR);
			rect.InsetBy(1.0, 0.0);
			AddLine(rect.LeftTop(), rect.LeftBottom(), GRAY_COLOR);
			EndLineArray();
			break;
		}
		case B_HORIZONTAL:
		{
			BeginLineArray(5);
			AddLine(rect.LeftTop(), rect.RightTop(), DARK_GRAY_COLOR);
			AddLine(rect.LeftBottom(), rect.RightBottom(), DARK_GRAY_COLOR);
			rect.InsetBy(0.0, 1.0);
			AddLine(rect.LeftTop(), rect.RightTop(), WHITE_COLOR);
			AddLine(rect.LeftBottom(), rect.RightBottom(), MEDIUM_GRAY_COLOR);
			rect.InsetBy(0.0, 1.0);
			AddLine(rect.LeftTop(), rect.RightTop(), GRAY_COLOR);
			EndLineArray();
			break;
		}
	}
}

void
CSplitter::MouseDown(
	BPoint point)
{
	int32 buttons;
	Window()->CurrentMessage()->FindInt32("buttons", &buttons);

	if (buttons == B_PRIMARY_MOUSE_BUTTON)
	{
		SetMouseEventMask(B_POINTER_EVENTS,
						  B_NO_POINTER_HISTORY | B_LOCK_WINDOW_FOCUS);
		m_dragging = true;

		// get the point in window coordinates
		BView *view = this;
		do
		{
			view->ConvertToParent(&point);
		} while ((view = view->Parent()) != NULL);
		m_offset = (m_posture == B_VERTICAL) ? point.x : point.y;
		Window()->CurrentMessage()->FindInt64("when", &m_lastDragTime);

		if (m_messageFilter == NULL)
		{
			m_messageFilter = new CSplitterMessageFilter(this);
			Window()->AddCommonFilter(m_messageFilter);
		}
		be_app->SetCursor(CCursorCache::GetCursor(m_posture == B_HORIZONTAL ?
												  CCursorCache::VERTICAL_MOVE :
												  CCursorCache::HORIZONTAL_MOVE));
	}
}

void CSplitter::MouseMoved(
	BPoint point,
	uint32 transit,
	const BMessage *message)
{
	if ((message == NULL) && Window()->IsActive())
	{
		if (transit == B_ENTERED_VIEW)
		{
			be_app->SetCursor(CCursorCache::GetCursor(m_posture == B_HORIZONTAL ?
													  CCursorCache::VERTICAL_MOVE :
													  CCursorCache::HORIZONTAL_MOVE));
		}
		else if (transit == B_EXITED_VIEW)
		{
			be_app->SetCursor(CCursorCache::GetCursor(CCursorCache::DEFAULT));
		}
	}
}

void
CSplitter::MouseUp(
	BPoint point)
{
	if (m_dragging)
	{
		if (m_messageFilter != NULL)
		{
			Window()->RemoveCommonFilter(m_messageFilter);
			delete m_messageFilter;
			m_messageFilter = NULL;
		}
		m_dragging = false;
	}

	if (!Bounds().Contains(point))
	{
		be_app->SetCursor(CCursorCache::GetCursor(CCursorCache::DEFAULT));
	}
}

// ---------------------------------------------------------------------------
// CSplitterMessageFilter Implementation

CSplitterMessageFilter::CSplitterMessageFilter(
	CSplitter *splitter)
	:	BMessageFilter(B_ANY_DELIVERY, B_ANY_SOURCE, B_MOUSE_MOVED),
		m_splitter(splitter)
{
}

filter_result
CSplitterMessageFilter::Filter(
	BMessage *message,
	BHandler **target)
{
	BPoint point;
	if (message->FindPoint("where", &point) == B_OK)
		m_splitter->Dragged(point);

	return B_SKIP_MESSAGE;
}

// END - Splitter.cpp
