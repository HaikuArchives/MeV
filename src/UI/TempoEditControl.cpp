/* ===================================================================== *
 * TempoEditControl.cpp (MeV/TransportWindow)
 * ===================================================================== */

#include "TempoEditControl.h"

#include "Event.h"
#include "StringEditView.h"

// Gnu C Library
#include <stdio.h>
// Interface Kit
#include <Bitmap.h>
#include <TextView.h>
#include <Window.h>
// Support Kit
#include <Debug.h>

// ---------------------------------------------------------------------------
// Constants

const rgb_color
CTempoEditControl::LIGHT_GRAY_COLOR =
tint_color(ui_color(B_PANEL_BACKGROUND_COLOR), B_LIGHTEN_2_TINT);

const rgb_color
CTempoEditControl::DARK_GRAY_COLOR =
tint_color(ui_color(B_PANEL_BACKGROUND_COLOR), B_DARKEN_2_TINT);

// ---------------------------------------------------------------------------
// Constructor/Destructor

CTempoEditControl::CTempoEditControl(
	BRect frame,
	BMessage *message,
	uint32 resizingMode,
	uint32 flags )
	:	BControl(frame, "TempoEditControl", NULL, message,
				 resizingMode, flags),
		m_document(NULL),
		m_bpm(0.0),
		m_digitFont(be_fixed_font),
		m_backBitmap(NULL),
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

	SetEnabled(false);
}

CTempoEditControl::~CTempoEditControl()
{
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
CTempoEditControl::SetDocument(
	CMeVDoc *doc)
{
	if (doc != m_document)
	{
		m_document = doc;
		if (m_document)
			SetEnabled(true);
		else
			SetEnabled(false);
		m_dirty = true;
		Invalidate();
	}
}

void
CTempoEditControl::SetTempo(
	double bpm)
{
	if ((bpm != m_bpm) && (bpm >= 1.0) && (bpm < 1000.0))
	{
		m_bpm = bpm;
		m_dirty = true;
		Invalidate();
	}
}

// ---------------------------------------------------------------------------
// Operations

void
CTempoEditControl::StartEdit()
{
	char digits[13];
	sprintf(digits, "%3.3f", m_bpm);

	BRect rect(Bounds());
	rect.InsetBy(3.0, 1.0);
	rect.bottom -= m_labelFontHeight.ascent;
	rect.left = rect.right - m_digitFont.StringWidth(digits);

	rgb_color bgColor = {255, 255, 255, 255};
	rgb_color textColor = {0, 0, 0, 255};

	Window()->Activate(true);

	BMessage *message = new BMessage(TEMPO_CHANGED);
	BMessenger messenger(this, Window());
	CStringEditView *view = new CStringEditView(rect, digits,
												&m_digitFont,
												textColor, bgColor,
												message, messenger);
	AddChild(view);
	view->TextView()->SetAlignment(B_ALIGN_RIGHT);

	m_editing = true;
}

void
CTempoEditControl::StopEdit()
{
	m_editing = false;
}

// ---------------------------------------------------------------------------
// BControl Implementation

void
CTempoEditControl::AttachedToWindow()
{
	BControl::AttachedToWindow();
	SetViewColor(B_TRANSPARENT_32_BIT);

	m_backBitmap = new BBitmap(Bounds(), B_CMAP8, true);
	m_backView = new BView(Bounds(), "", B_FOLLOW_NONE, B_WILL_DRAW);
	m_backBitmap->AddChild(m_backView);
	m_dirty = true;
}

void
CTempoEditControl::Draw(
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
CTempoEditControl::GetPreferredSize(
	float *width,
	float *height)
{
	float sw[2];
	sw[0] = m_digitFont.StringWidth("000.000");
	sw[1] = m_labelFont.StringWidth("bpm");
	if (sw[0] > sw[1])
		*width = sw[0];
	else
		*width = sw[1];
	
	*width += 6.0;

	*height = m_digitFontHeight.ascent + m_labelFontHeight.ascent;
	*height += 5.0;
}

void
CTempoEditControl::MessageReceived(
	BMessage *message)
{
	switch (message->what)
	{
		case TEMPO_CHANGED:
		{
			BString text;
			if (message->FindString("text", &text) == B_OK)
			{
				double tempo = Tempo();
				if (sscanf(text.String(), "%lf", &tempo) >= 1) 
				{
					BMessage invokeMsg(*Message());
					invokeMsg.AddFloat("delta", tempo - Tempo());
					SetTempo(tempo);
					Invoke(&invokeMsg);
				}
			}
			StopEdit();
			break;
		}
		default:
		{
			BControl::MessageReceived(message);
		}
	}
}

void
CTempoEditControl::MouseDown(
	BPoint point)
{
	if (IsEnabled())
	{
		int32 buttons = B_PRIMARY_MOUSE_BUTTON;
		Window()->CurrentMessage()->FindInt32("buttons", &buttons);

		if (buttons == B_PRIMARY_MOUSE_BUTTON)
		{
			m_draggingColumn = ColumnAt(point);
			m_dragOffset = point;
			SetMouseEventMask(B_POINTER_EVENTS,
							  B_NO_POINTER_HISTORY | B_LOCK_WINDOW_FOCUS);
		}
		else if (buttons == B_SECONDARY_MOUSE_BUTTON)
		{
			StartEdit();
		}
	}
}

void
CTempoEditControl::MouseMoved(
	BPoint point,
	uint32 transit,
	const BMessage *message)
{
	if (m_draggingColumn > 0)
	{
		double change = m_dragOffset.y - point.y;

		if (m_draggingColumn == 1)
		{
			if (modifiers() & B_SHIFT_KEY)
				change *= 10.0;
		}
		else
		{
			if (modifiers() & B_SHIFT_KEY)
				change *= 5.0;
			int32 decimal = m_draggingColumn - 1;
			change /= pow(10, decimal);
		}

		m_dragOffset = point;
		BMessage invokeMsg(*Message());
		invokeMsg.AddFloat("delta", change);
		SetTempo(m_bpm + change);
		Invoke(&invokeMsg);
	}
}

void
CTempoEditControl::MouseUp(
	BPoint point)
{
	m_draggingColumn = 0;
}

// ---------------------------------------------------------------------------
// Internal Operations

uint32
CTempoEditControl::ColumnAt(
	BPoint point)
{
	BRect rect = Bounds();
	rect.right -= 2.0;
	rect.left = rect.right - m_digitFont.StringWidth("0");
	float delta = m_digitFont.StringWidth("00")
				  - m_digitFont.StringWidth("0") * 2.0;
	for (int i = 4; i > 1; i--)
	{
		if (rect.Contains(point))
			return i;
		rect.OffsetBy(- rect.Width() - delta, 0.0);
	}

	return 1;
}

void
CTempoEditControl::DrawInto(
	BView *view,
	BRect updateRect)
{
	BRect rect(Bounds());
	BPoint offset;

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

	// draw label
	view->SetFont(&m_labelFont);
	BRect cellRect(rect);
	cellRect.top = cellRect.bottom - m_labelFontHeight.ascent;
	if (IsEnabled())
		view->SetHighColor(160, 160, 160, 255);
	else
		view->SetHighColor(203, 203, 203, 203);
	view->SetDrawingMode(B_OP_OVER);
	offset = cellRect.LeftBottom();
	offset.x += cellRect.Width() / 2.0
				- m_labelFont.StringWidth("bpm") / 2.0;
	view->DrawString("bpm", offset);

	// draw digits
	view->SetFont(&m_digitFont);
	cellRect.left = rect.left;
	cellRect.bottom = cellRect.top - 1.0;
	cellRect.top = rect.top;
	if (IsEnabled())
		view->SetHighColor(0, 255, 0, 255);
	else
		view->SetHighColor(128, 192, 128, 255);
	char digits[13];
	sprintf(digits, "%3.3f", m_bpm);
	BPoint offset2 = cellRect.LeftBottom();
	offset2.x += cellRect.right - m_digitFont.StringWidth(digits) - 2.0;
	view->DrawString(digits, offset2);
}

// END - TempoEditControl.cpp
