/* ===================================================================== *
 * ColorWell.cpp (MeV/UI)
 * ===================================================================== */

#include "ColorWell.h"

#include "ColorDialogWindow.h"

// Interface Kit
#include <Window.h>
// Support Kot
#include <Debug.h>

#define D_HOOK(x) //PRINT(x) // BControl Implementation

// ---------------------------------------------------------------------------
// Constructor/Destructor

CColorWell::CColorWell(
	BRect frame,
	BMessage *message,
	uint32 resizingMode)
	:	BControl(frame, "ColorWell", "", message, resizingMode,
				 B_WILL_DRAW | B_NAVIGABLE)
{
	SetViewColor(B_TRANSPARENT_COLOR);
}

// ---------------------------------------------------------------------------
// BView Implementation

void
CColorWell::Draw(
	BRect updateRect)
{
	BRect r(Bounds());

	SetDrawingMode(B_OP_COPY);
	rgb_color light = tint_color(Parent()->ViewColor(), B_LIGHTEN_2_TINT);
	rgb_color dark = tint_color(Parent()->ViewColor(), B_DARKEN_2_TINT);
	rgb_color blue = ui_color(B_KEYBOARD_NAVIGATION_COLOR);
	BeginLineArray(8);
	{
		AddLine(r.RightTop(), r.LeftTop(), dark);
		AddLine(r.LeftTop(), r.LeftBottom(), dark);
		AddLine(r.LeftBottom(), r.RightBottom(), light);
		AddLine(r.RightBottom(), r.RightTop(), light);
		r.InsetBy(1.0, 1.0);
		AddLine(r.RightTop(), r.LeftTop(), IsFocus() ? blue : dark);
		AddLine(r.LeftTop(), r.LeftBottom(), IsFocus() ? blue : dark);
		AddLine(r.LeftBottom(), r.RightBottom(), IsFocus() ? blue : light);
		AddLine(r.RightBottom(), r.RightTop(), IsFocus() ? blue : light);
		r.InsetBy(1.0, 1.0);
	}
	EndLineArray();

	SetHighColor(m_color);
	FillRect(r, B_SOLID_HIGH);
}

void
CColorWell::KeyDown(
	const char *bytes,
	int32 numBytes)
{
	if (bytes[0] == B_SPACE)
	{
		BRect rect(100, 100, 300, 200);
		CColorDialogWindow *window;
		window = new CColorDialogWindow(rect, m_color,
										new BMessage(*Message()),
										Parent(), Window());
		window->Show();
	}
	else
	{
		BControl::KeyDown(bytes, numBytes);
	}
}

void
CColorWell::MessageReceived(
	BMessage *message)
{
	switch (message->what)
	{
		case B_PASTE:
		{
			ssize_t size;
			const rgb_color *color;
			if (message->FindData("RGBColor", B_RGB_COLOR_TYPE,
								  (const void **)&color, &size) != B_OK)
				return;
			SetColor(*color);
			MakeFocus(false);
			BMessage *message = Message();
			if (message->ReplaceData("color", B_RGB_COLOR_TYPE, color,
									 sizeof(color)) != B_OK)
				message->AddData("color", B_RGB_COLOR_TYPE, color,
								 sizeof(color));
			Invoke(message);
			break;
		}
		default:
		{
			BView::MessageReceived(message);
		}
	}
}

void
CColorWell::MouseDown(
	BPoint point)
{
	int32 clicks = 0;
	BMessage *message = Window()->CurrentMessage();
	message->FindInt32("clicks", &clicks);

	if (clicks >= 2)
	{
		BRect rect(100, 100, 300, 200);
		CColorDialogWindow *window;
		window = new CColorDialogWindow(rect, m_color,
										new BMessage(*Message()),
										Parent(), Window());
		window->Show();
	}
	else
	{
		MakeFocus(true);
	}
}

void
CColorWell::MouseMoved(
	BPoint point,
	uint32 transit,
	const BMessage *message)
{
	if (message && (message->what == B_PASTE)
	 && (message->HasData("RGBColor", B_RGB_COLOR_TYPE)))
	{
		if ((transit == B_OUTSIDE_VIEW) || (transit == B_EXITED_VIEW))
		{
			MakeFocus(false);
			Invalidate();
		}
		else
		{
			MakeFocus(true);
			Invalidate();
		}
	}
}

// END - ColorWell.cpp
