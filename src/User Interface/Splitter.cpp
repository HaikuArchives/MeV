/* ===================================================================== *
 * Splitter.cpp (MeV/User Interface)
 * ===================================================================== */

#include "Splitter.h"
#include "ResourceUtils.h"

// Application Kit
#include <Application.h>
#include <Cursor.h>
#include <Message.h>
// Interface Kit
#include <Window.h>

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
		m_dragging(false)
{
	SetViewColor(B_TRANSPARENT_COLOR);
	
	switch (m_posture)
	{
		case B_VERTICAL:
		{
			if (frame.Width() != V_SPLITTER_WIDTH)
			{
				ResizeTo(V_SPLITTER_WIDTH, frame.Height());
			}
			break;
		}
		case B_HORIZONTAL:
		{
			if (frame.Height() != H_SPLITTER_HEIGHT)
			{
				ResizeTo(H_SPLITTER_HEIGHT, frame.Width());
			}
			break;
		}
	}
}

// ---------------------------------------------------------------------------
// BControl Implementation

void
CSplitter::AllAttached()
{
}

void
CSplitter::Draw(
	BRect updateRect)
{
	BRect rect(Bounds());

	switch (m_posture)
	{
		case B_VERTICAL:
		{
			BeginLineArray(7);
			AddLine(rect.LeftTop(), rect.LeftBottom(), DARK_GRAY_COLOR);
			AddLine(rect.RightTop(), rect.RightBottom(), DARK_GRAY_COLOR);
			AddLine(rect.LeftTop(), rect.RightTop(), DARK_GRAY_COLOR);
			AddLine(rect.LeftBottom(), rect.RightBottom(), DARK_GRAY_COLOR);
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
			BeginLineArray(7);
			AddLine(rect.LeftTop(), rect.RightTop(), DARK_GRAY_COLOR);
			AddLine(rect.LeftBottom(), rect.RightBottom(), DARK_GRAY_COLOR);
			AddLine(rect.LeftTop(), rect.LeftBottom(), DARK_GRAY_COLOR);
			AddLine(rect.RightTop(), rect.RightBottom(), DARK_GRAY_COLOR);
			rect.InsetBy(1.0, 1.0);
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
		SetMouseEventMask(B_POINTER_EVENTS, B_NO_POINTER_HISTORY | B_LOCK_WINDOW_FOCUS);
		m_dragging = true;
	}
}

void CSplitter::MouseMoved(
	BPoint point,
	uint32 transit,
	const BMessage *message)
{
	if (m_dragging)
	{
		switch (m_posture)
		{
			case B_VERTICAL:
			{
				if ((transit == B_EXITED_VIEW)
				 || (transit == B_OUTSIDE_VIEW))
				{
					float diff = point.x - V_SPLITTER_WIDTH / 2.0;
					m_primaryTarget->ResizeBy(diff, 0.0);
					m_secondaryTarget->MoveBy(diff, 0.0);
					m_secondaryTarget->ResizeBy(-diff, 0.0);
					MoveBy(diff, 0.0);
				}
				break;
			}
			case B_HORIZONTAL:
			{
				if ((transit == B_EXITED_VIEW)
				 || (transit == B_OUTSIDE_VIEW))
				{
					float diff = point.y - H_SPLITTER_HEIGHT / 2.0;
					m_primaryTarget->ResizeBy(0.0, diff);
					m_secondaryTarget->MoveBy(0.0, diff);
					m_secondaryTarget->ResizeBy(0.0, -diff);
					MoveBy(0.0, diff);
				}
				break;
			}
		}
	}
	else
	{
		if (transit == B_ENTERED_VIEW)
		{
			switch (m_posture)
			{
				case B_VERTICAL:
				{
					be_app->SetCursor(ResourceUtils::LoadCursor(4));
					break;
				}
				case B_HORIZONTAL:
				{
					be_app->SetCursor(ResourceUtils::LoadCursor(3));
				}
			}
		}
		else if (transit == B_EXITED_VIEW)
		{
			be_app->SetCursor(B_CURSOR_SYSTEM_DEFAULT);
		}
	}
}

void
CSplitter::MouseUp(
	BPoint point)
{
	m_dragging = false;
}

// END - Splitter.cpp