/* ===================================================================== *
 * TimeEditControl.cpp (MeV/TransportWindow)
 * ===================================================================== */

#include "TimeEditControl.h"

#include "Event.h"
#include "PlayerControl.h"

// Gnu C Library
#include <stdio.h>
// Application Kit
#include <MessageRunner.h>
// Interface Kit
#include <Bitmap.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
// Support Kit
#include <Debug.h>

// ---------------------------------------------------------------------------
// Constants

const bigtime_t
CTimeEditControl::UPDATE_TIME_PERIOD = 32 * 1000;

const rgb_color
CTimeEditControl::LIGHT_GRAY_COLOR =
tint_color(ui_color(B_PANEL_BACKGROUND_COLOR), B_LIGHTEN_2_TINT);

const rgb_color
CTimeEditControl::DARK_GRAY_COLOR =
tint_color(ui_color(B_PANEL_BACKGROUND_COLOR), B_DARKEN_2_TINT);

// ---------------------------------------------------------------------------
// Utility Functions

inline
int32 udiv(
	int32 dividend,
	int32 divisor)
{
	if (dividend >= 0)
		return dividend / divisor;
	else
		return (dividend - divisor + 1) / divisor;
}

// ---------------------------------------------------------------------------
// Constructor/Destructor

CTimeEditControl::CTimeEditControl(
	BRect frame,
	BMessage *message,
	uint32 resizingMode,
	uint32 flags)
	:	BControl(frame, "TimeEditControl", NULL, message,
				 resizingMode, flags),
		m_document(NULL),
		m_realTime(0L),
		m_meteredTime(0),
		m_clockType(ClockType_Metered),
		m_measureBase(0),
		m_measureSize(Ticks_Per_QtrNote * 4),
		m_beatSize(Ticks_Per_QtrNote),
		m_digitFont(be_fixed_font),
		m_labelFont(be_plain_font),
		m_messageRunner(NULL),
		m_backBitmap(NULL),
		m_editable(true),
		m_contextMenu(NULL),
		m_draggingColumn(0)
{
	float size;

	size = m_labelFont.Size();
	size = ceil(0.9 * size);
	m_labelFont.SetSize(size);
	m_labelFont.GetHeight(&m_labelFontHeight);

	size = m_digitFont.Size();
	size = ceil(1.2 * size);
	m_digitFont.SetSize(size);
	m_digitFont.SetFace(B_BOLD_FACE);
	m_digitFont.GetHeight(&m_digitFontHeight);
}

CTimeEditControl::~CTimeEditControl()
{
	if (m_contextMenu)
	{
		delete m_contextMenu;
		m_contextMenu = NULL;
	}
	
	if (m_backBitmap)
	{
		delete m_backBitmap;
		m_backBitmap = NULL;
		m_backView = NULL;
	}
}

// ---------------------------------------------------------------------------
// Accessors

void
CTimeEditControl::SetClockType(
	TClockType type)
{
	m_clockType = type;
	m_dirty = true;
	Invalidate();
}

void
CTimeEditControl::SetDocument(
	CMeVDoc *doc)
{
	m_document = doc;
	if (m_document)
		SetEnabled(true);
	else
		SetEnabled(false);
}

void
CTimeEditControl::SetTime(
	bigtime_t realTime,
	int32 meteredTime)
{
	m_realTime = realTime;
	m_meteredTime = meteredTime;
	m_dirty = true;
	Invalidate();
}

// ---------------------------------------------------------------------------
// Operations

void
CTimeEditControl::Started()
{
	if (!m_messageRunner)
	{
		BMessenger messenger(this, Window());
		m_messageRunner = new BMessageRunner(messenger,
											 new BMessage(UPDATE_TIME),
											 UPDATE_TIME_PERIOD);
	}
	if (m_document)
	{
		PlaybackState pbState;
		if (CPlayerControl::GetPlaybackState(m_document, pbState))
		{
			if (pbState.running)
				SetTime(pbState.realTime * 1000, pbState.meteredTime);
			else
				SetTime(0L, 0);
		}
	}
}

void
CTimeEditControl::Stopped()
{
	if (m_messageRunner)
	{
		delete m_messageRunner;
		m_messageRunner = 0;
	}

	SetTime(0L, 0);
}

// ---------------------------------------------------------------------------
// BControl Implementation

void
CTimeEditControl::AttachedToWindow()
{
	BControl::AttachedToWindow();
	SetViewColor(B_TRANSPARENT_32_BIT);

	m_backBitmap = new BBitmap(Bounds(), B_CMAP8, true);
	m_backView = new BView(Bounds(), "", B_FOLLOW_NONE, B_WILL_DRAW);
	m_backBitmap->AddChild(m_backView);
	m_dirty = true;
}

void
CTimeEditControl::Draw(
	BRect updateRect)
{
	if (!m_backBitmap)
	{
		DrawInto(this, updateRect);
	}
	else
	{
		if (m_dirty && m_backBitmap->Lock())
		{
			DrawInto(m_backView, updateRect);
			m_backView->Sync();
			m_backBitmap->Unlock();
			m_dirty = false;
		}
		SetDrawingMode(B_OP_OVER);
		DrawBitmap(m_backBitmap);
	}
}

void
CTimeEditControl::GetPreferredSize(
	float *width,
	float *height)
{
	float sw[3];
	sw[0] = m_digitFont.StringWidth("00:00:00:00");
	sw[1] = m_labelFont.StringWidth("hr min sec frm");
	sw[2] = m_labelFont.StringWidth("bar beat tick");
	if (sw[0] > sw[1])
	{
		if (sw[0] > sw[2])
			*width = sw[0];
		else
			*width = sw[2];
	}
	else
	{
		if (sw[1] > sw[2])
			*width = sw[1];
		else
			*width = sw[2];
	}
	
	*width += 6.0;

	*height = m_digitFontHeight.ascent + m_labelFontHeight.ascent;
	*height += 5.0;
}

void
CTimeEditControl::MessageReceived(
	BMessage *message)
{
	switch (message->what)
	{
		case UPDATE_TIME:
		{
			if (m_document)
			{
				PlaybackState pbState;
				if (CPlayerControl::GetPlaybackState(m_document, pbState))
				{
					if (pbState.running)
						SetTime(pbState.realTime * 1000, pbState.meteredTime);
					else
						SetTime(0L, 0);
				}
			}
			break;
		}
		case CLOCK_TYPE_CHANGED:
		{
			int32 clockType = m_clockType;
			if (message->FindInt32("clock_type", &clockType) == B_OK) {
				SetClockType(static_cast<E_ClockTypes>(clockType));
			}
			break;
		}
		default:
		{
			BControl::MessageReceived(message);
		}
	}
}

void
CTimeEditControl::MouseDown(
	BPoint point)
{
	if (IsEnabled())
	{
		int32 buttons = B_PRIMARY_MOUSE_BUTTON;
		Window()->CurrentMessage()->FindInt32("buttons", &buttons);

		if ((buttons == B_PRIMARY_MOUSE_BUTTON)
		 &&  IsEditable())
		{
			PlaybackState pbState;
			if (m_document
			 && CPlayerControl::GetPlaybackState(m_document, pbState)
			 && pbState.running)
			{
				return;
			}

			m_draggingColumn = ColumnAt(point);
			m_dragOffset = point;
			SetMouseEventMask(B_POINTER_EVENTS,
							  B_NO_POINTER_HISTORY | B_LOCK_WINDOW_FOCUS);
		}
		else if (buttons == B_SECONDARY_MOUSE_BUTTON)
		{
			ShowContextMenu(point);
			return;
		}
	}
}

void
CTimeEditControl::MouseMoved(
	BPoint point,
	uint32 transit,
	const BMessage *message)
{
	if (m_draggingColumn > 0)
	{
		int32 change = static_cast<int>(m_dragOffset.y - point.y);
		if (modifiers() & B_SHIFT_KEY)
			change *= 10;

		switch (m_clockType)
		{
			case ClockType_Metered:
			{
				int32 bars, beats, ticks;
				GetMeteredTime(&bars, &beats, &ticks);
				if (m_draggingColumn == 1)
					SetMeteredTime(bars + change, beats, ticks);
				else if (m_draggingColumn == 2)
					SetMeteredTime(bars, beats + change, ticks);
				else if (m_draggingColumn == 3)
					SetMeteredTime(bars, beats, ticks + change);
				break;
			}
			case ClockType_Real:
			{
				int32 hours, minutes, seconds, frames;
				GetRealTime(&hours, &minutes, &seconds, &frames);
				if (m_draggingColumn == 1)
					SetRealTime(hours + change, minutes, seconds, frames);
				else if (m_draggingColumn == 2)
					SetRealTime(hours, minutes + change, seconds, frames);
				else if (m_draggingColumn == 3)
					SetRealTime(hours, minutes, seconds + change, frames);
				else if (m_draggingColumn == 4)
					SetRealTime(hours, minutes, seconds, frames + change);
				break;
			}
			default:
			{
				/* do nothing */
			}
		}
		SetTime(m_realTime.Microseconds(), m_meteredTime);
		m_dragOffset = point;
	}
}

void
CTimeEditControl::MouseUp(
	BPoint point)
{
	m_draggingColumn = 0;
}

// ---------------------------------------------------------------------------
// Internal Operations

uint32
CTimeEditControl::ColumnAt(
	BPoint point)
{
	switch (m_clockType)
	{
		case ClockType_Metered:
		{
			BRect rect = Bounds();
			rect.right = floor(rect.Width() / 3.0);
			for (int i = 1; i < 4; i++)
			{
				if (rect.Contains(point))
					return i;
				rect.OffsetBy(rect.Width() + 1.0, 0.0);
			}
			return 0;
		}
		case ClockType_Real:
		{
			BRect rect = Bounds();
			rect.right = floor(rect.Width() / 4.0);
			for (int i = 1; i < 5; i++)
			{
				if (rect.Contains(point))
					return i;
				rect.OffsetBy(rect.Width() + 1.0, 0.0);
			}
			return 0;
		}
		default:
		{
			return 0;
		}
	}
}

void
CTimeEditControl::DrawInto(
	BView *view,
	BRect updateRect)
{
	BRect rect(Bounds());

	view->BeginLineArray(4);
	view->AddLine(rect.LeftBottom(), rect.RightBottom(), LIGHT_GRAY_COLOR);
	view->AddLine(rect.RightBottom(), rect.RightTop(), LIGHT_GRAY_COLOR);
	view->AddLine(rect.RightTop(), rect.LeftTop(), DARK_GRAY_COLOR);
	view->AddLine(rect.LeftTop(), rect.LeftBottom(), DARK_GRAY_COLOR);
	view->EndLineArray();

	rect.InsetBy(1.0, 1.0);
	if (IsEnabled())
		view->SetLowColor(0, 0, 0, 255);
	else
		view->SetLowColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	view->SetDrawingMode(B_OP_COPY);
	view->FillRect(rect, B_SOLID_LOW);

	rect.InsetBy(1.0, 1.0);

	switch (m_clockType)
	{
		case ClockType_Metered:
		{
			// draw label
			const char *label[] = { "bar", "beat", "tick" };
			view->SetFont(&m_labelFont);
			BRect cellRect(rect);
			cellRect.top = cellRect.bottom - m_labelFontHeight.ascent;
			cellRect.right = floor(cellRect.left + rect.Width() / 3.0);
			if (IsEnabled())
				view->SetHighColor(160, 160, 160, 255);
			else
				view->SetHighColor(203, 203, 203, 203);
			view->SetDrawingMode(B_OP_OVER);
			for (int i = 0; i < 3; i++)
			{
				BPoint offset(cellRect.LeftBottom());
				offset.x += cellRect.Width() / 2.0
							- m_labelFont.StringWidth(label[i]) / 2.0;
				view->DrawString(label[i], offset);
				cellRect.OffsetBy(cellRect.Width() + 1.0, 0.0);
			}

			// draw digits
			view->SetFont(&m_digitFont);
			cellRect.left = rect.left;
			cellRect.bottom = cellRect.top - 1.0;
			cellRect.top = rect.top;
			cellRect.right = floor(rect.left + rect.Width() / 3.0);
			if (IsEnabled())
				view->SetHighColor(0, 255, 0, 255);
			else
				view->SetHighColor(128, 192, 128, 255);
			int32 val[3];
			GetMeteredTime(&val[0], &val[1], &val[2]);
			for (int i = 0; i < 3; i++)
			{
				char digits[4];
				sprintf(digits, "%03ld", val[i] + 1);
				BPoint offset(cellRect.LeftBottom());
				offset.x += cellRect.Width() / 2.0
							- m_digitFont.StringWidth(digits) / 2.0;
				view->DrawString(digits, offset);
				cellRect.OffsetBy(cellRect.Width() + 1.0, 0.0);
			}
			break;
		}
		case ClockType_Real:
		{
			// draw labels
			const char *label[] = { "hr", "min", "sec", "frm" };
			view->SetFont(&m_labelFont);
			BRect cellRect(rect);
			cellRect.top = cellRect.bottom - m_labelFontHeight.ascent;
			cellRect.right = floor(rect.left + rect.Width() / 4.0);
			if (IsEnabled())
				view->SetHighColor(160, 160, 160, 255);
			else
				view->SetHighColor(203, 203, 203, 203);
			view->SetDrawingMode(B_OP_OVER);
			for (int i = 0; i < 4; i++)
			{
				BPoint offset(cellRect.LeftBottom());
				offset.x += cellRect.Width() / 2.0
							- m_labelFont.StringWidth(label[i]) / 2.0;
				view->DrawString(label[i], offset);
				cellRect.OffsetBy(cellRect.Width() + 1.0, 0.0);
			}

			// draw digits
			view->SetFont(&m_digitFont);
			cellRect.left = rect.left;
			cellRect.bottom = cellRect.top - 1.0;
			cellRect.top = rect.top;
			cellRect.right = floor(rect.left + rect.Width() / 4.0);
			int32 val[4];
			GetRealTime(&val[0], &val[1], &val[2], &val[3]);
			if (IsEnabled())
				view->SetHighColor(0, 255, 0, 255);
			else
				view->SetHighColor(128, 192, 128, 255);
			for (int i = 0; i < 4; i++)
			{
				char digits[4];
				sprintf(digits, "%02ld", val[i]); 
				BPoint offset(cellRect.LeftBottom());
				offset.x += cellRect.Width() / 2.0
							- m_digitFont.StringWidth(digits) / 2.0;
				view->DrawString(digits, offset);
				cellRect.OffsetBy(cellRect.Width() + 1.0, 0.0);
			}
			break;
		}
		default:
		{
			/* do nothing */
		}
	}
}

void
CTimeEditControl::GetMeteredTime(
	int32 *bars,
	int32 *beats,
	int32 *ticks)
{
	*ticks = m_meteredTime - m_measureBase;
	*bars = udiv(*ticks, m_measureSize);
	*ticks -= *bars * m_measureSize;
	*beats = *ticks / m_beatSize;
	*ticks -= *beats * m_beatSize;
}

void
CTimeEditControl::SetMeteredTime(
	int32 bars,
	int32 beats,
	int32 ticks)
{
	m_meteredTime = m_measureBase;
	m_meteredTime += bars * m_measureSize;
	m_meteredTime += beats * m_beatSize;
	m_meteredTime += ticks;
	if (m_meteredTime < 0)
		m_meteredTime = 0;
}

void
CTimeEditControl::GetRealTime(
	int32 *hours,
	int32 *minutes,
	int32 *seconds,
	int32 *frames)
{
	*hours = m_realTime.Hours();
	*minutes = m_realTime.Minutes();
	*seconds = m_realTime.Seconds();
	*frames = m_realTime.Frames();
}

void
CTimeEditControl::SetRealTime(
	int32 hours,
	int32 minutes,
	int32 seconds,
	int32 frames)
{
	m_realTime.SetData(hours, minutes, seconds, frames);
	if (m_realTime < 0L)
		m_realTime = 0L;
}

void
CTimeEditControl::ShowContextMenu(
	BPoint point)
{
	BMenuItem *item;

	if (!m_contextMenu)
	{
		m_contextMenu = new BPopUpMenu("", false, false, B_ITEMS_IN_COLUMN);
		m_contextMenu->SetFont(be_plain_font);
		BMessage *message;

		message = new BMessage(CLOCK_TYPE_CHANGED);
		message->AddInt32("clock_type", ClockType_Metered);
		m_contextMenu->AddItem(new BMenuItem("Metered", message));
		message = new BMessage(CLOCK_TYPE_CHANGED);
		message->AddInt32("clock_type", ClockType_Real);
		m_contextMenu->AddItem(new BMenuItem("Real", message));
	}

	item = m_contextMenu->FindItem("Metered");
	if (item)
		item->SetMarked((m_clockType == ClockType_Metered));
	item = m_contextMenu->FindItem("Real");
	if (item)
		item->SetMarked((m_clockType == ClockType_Real));

	m_contextMenu->SetTargetForItems(this);
	ConvertToScreen(&point);
	point -= BPoint(1.0, 1.0);
	m_contextMenu->Go(point, true, true, true);
}

// END - TimeEditControl.cpp
