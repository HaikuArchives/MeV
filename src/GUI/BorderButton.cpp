/* ===================================================================== *
 * BorderButton.cpp (MeV/User Interface)
 * ===================================================================== */

#include "BorderButton.h"
#include "StdBevels.h"

// Interface Kit
#include <Bitmap.h>
#include <Window.h>
// Support Kit
#include <Debug.h>

// Debugging Macros
#define D_ALLOC(x) //PRINT (x)		// Constructor/Destructor
#define D_HOOK(x) //PRINT (x)		// BControl Implementation

// ---------------------------------------------------------------------------
// Constructor/Destructor

CBorderButton::CBorderButton(
	BRect frame,
	const char *name,
	BBitmap *bitmap,
	BMessage *message,
	uint32 resizingMode,
	uint32 flags)
	:	BControl(frame, name, NULL, message, resizingMode, flags),
		m_pressed(false),
		m_tracking(false)
{
	m_glyphs[0] = bitmap;
	m_glyphs[1] = NULL;

	SetViewColor(B_TRANSPARENT_COLOR);
}

// ---------------------------------------------------------------------------
// BControl Implementation

void
CBorderButton::Draw(
	BRect updateRect)
{
	D_HOOK(("CBorderButton::Draw()\n"));

	StdBevels::DrawBorderBevel(this, Bounds(),
							   m_pressed ? StdBevels::DEPRESSED_BEVEL
										 : StdBevels::NORMAL_BEVEL);
 	BBitmap	*bitmap = (m_pressed && m_glyphs[1]) ? m_glyphs[1] : m_glyphs[0];
	if (bitmap)
	{
		SetDrawingMode(B_OP_OVER);
		BPoint offset((Bounds().Width() - bitmap->Bounds().Width()) / 2,
					  (Bounds().Height() - bitmap->Bounds().Height()) / 2);
		DrawBitmapAsync(bitmap, offset);
	}
}

void
CBorderButton::MouseDown(
	BPoint point)
{
	D_HOOK(("CBorderButton::MouseDown()\n"));

	int32 buttons = 0;
	Looper()->CurrentMessage()->FindInt32("buttons", &buttons);
	if (buttons == B_PRIMARY_MOUSE_BUTTON)
	{
		// only accept left button clicks
		m_tracking = true;
		m_pressed = true;
		Invalidate();
		SetMouseEventMask(B_POINTER_EVENTS | B_LOCK_WINDOW_FOCUS);
	}
}

void
CBorderButton::MouseMoved(
	BPoint point,
	uint32 transit,
	const BMessage *message)
{
	D_HOOK(("CBorderButton::MouseMoved()\n"));

	if (!m_tracking)
	{
		return;
	}

	switch (transit)
	{
		case B_ENTERED_VIEW:
		{
			if (!m_pressed) {
				m_pressed = true;
				Invalidate();
			}
			break;
		}
		case B_EXITED_VIEW:
		{
			if (m_pressed) {
				m_pressed = false;
				Invalidate();
			}
			break;
		}
	}	
}

void
CBorderButton::MouseUp(
	BPoint point)
{
	D_HOOK(("CBorderButton::MouseUp()\n"));

	if (Bounds().Contains(point))
	{
		SetValue((Value() == B_CONTROL_OFF) ? B_CONTROL_ON : B_CONTROL_OFF);
		Invoke(Message());
	}

	m_pressed = false;
	m_tracking = false;
	Invalidate();
}
