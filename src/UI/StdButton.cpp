/* ===================================================================== *
 * StdButton.cpp (MeV/UI)
 * ===================================================================== */

#include "StdButton.h"
#include "StdBevels.h"

// Interface Kit
#include <Bitmap.h>

// ---------------------------------------------------------------------------
// Constructor/Destructor

CPushOnButton::CPushOnButton(
	BRect frame,
	const char *name,
	BBitmap	*bitmap,
	BMessage *message,
	uint32 resizingMode,
	uint32 flags)
	:	BControl(frame, name, NULL, message, resizingMode, flags)
{
	m_glyphs[0] = bitmap;
	m_glyphs[1] = NULL;
}

CPushOnButton::~CPushOnButton()
{
	if (m_glyphs[0])
	{
		delete m_glyphs[0];
		m_glyphs[0] = NULL;
	}

	if (m_glyphs[1])
	{
		delete m_glyphs[1];
		m_glyphs[1] = NULL;
	}
}

// ---------------------------------------------------------------------------
// BControl Implemetation

void
CPushOnButton::AttachedToWindow()
{
	BControl::AttachedToWindow();
	SetViewColor(B_TRANSPARENT_COLOR);
}

void
CPushOnButton::Draw(
	BRect updateRect)
{
	BRect r(Bounds());

	if (IsEnabled())
	{
		StdBevels::DrawBorderBevel(this, r, 
								   Value() ? StdBevels::DEPRESSED_BEVEL
								   		   : StdBevels::NORMAL_BEVEL);
	}
	else
	{
		StdBevels::DrawBorderBevel(this, r, StdBevels::DIMMED_BEVEL);
	}

	BBitmap *image = (Value() && m_glyphs[1]) ? m_glyphs[1] : m_glyphs[0];
	if (image != NULL)
	{
		BRect br(image->Bounds());

		SetDrawingMode(IsEnabled() ? B_OP_OVER : B_OP_BLEND);
		DrawBitmapAsync(image, BPoint((r.left + r.right - br.Width()) / 2,
									  (r.top + r.bottom - br.Height()) / 2));
		SetDrawingMode(B_OP_COPY);
	}
}

void
CPushOnButton::MouseDown(
	BPoint point)
{
	if (!Value()) 
	{
		SetValue(true);
		Invoke();
	}
}

void
CPushOnButton::MouseUp(
	BPoint point)
{
}

// ---------------------------------------------------------------------------
// Constructor/Destructor

CToggleButton::CToggleButton(
	BRect frame,
	const char *name,
	BBitmap	*bitmap,
	BMessage *message,
	uint32 resizingMode,
	uint32 flags)
	:	CPushOnButton(frame, name, bitmap, message, resizingMode, flags)
{
}

// ---------------------------------------------------------------------------
// CPushOnButton Implementation

void
CToggleButton::MouseDown(
	BPoint point)
{
	SetValue(!Value());
	Invoke();
}

// END - StdButton.cpp
