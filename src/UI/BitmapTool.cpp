/* ===================================================================== *
 * BitmapTool.cpp (MeV/User Interface)
 * ===================================================================== */

#include "BitmapTool.h"

#include "ToolBar.h"
#include "StdBevels.h"

// Interface Kit
#include <Bitmap.h>
#include <Screen.h>
#include <View.h>
// Support Kit
#include <Debug.h>

// Debugging Macros
#define D_ALLOC(x) //PRINT (x)		// Constructor/Destructor
#define D_HOOK(x) //PRINT (x)		// CTool Implementation

// ---------------------------------------------------------------------------
// Constructor/Destructor

CBitmapTool::CBitmapTool(
	const char *name,
	BBitmap *bitmap,
	BMessage *message,
	int32 mode,
	uint32 flags)
	:	CTool(name, message, mode, flags),
		m_bitmap(bitmap),
		m_disabledBitmap(NULL)
{
	D_ALLOC(("CBitmapTool::CBitmapTool()\n"));

	// calculate disabled bitmap
	if (bitmap)
	{
		BScreen screen;
		uint8 light = screen.IndexForColor(tint_color(ui_color(B_PANEL_BACKGROUND_COLOR),
													  B_LIGHTEN_1_TINT));
		uint8 dark  = screen.IndexForColor(tint_color(ui_color(B_PANEL_BACKGROUND_COLOR),
													  B_DARKEN_1_TINT));
		m_disabledBitmap = new BBitmap(bitmap);
		uint8 *start = (uint8 *)m_disabledBitmap->Bits();
		uint8 *end = start + m_disabledBitmap->BitsLength();
		for (uint8 *pos = start; pos < end; pos++)
		{
			rgb_color color = screen.ColorForIndex(*pos);
			if ((color.alpha < 255) || (color.red + color.green + color.blue >= 765))
			{
				continue;
			}
			if (color.red + color.green + color.blue > 384)
			{
				*pos = light;
			}
			else
			{
				*pos = dark;
			}
		}
	}
}

CBitmapTool::~CBitmapTool()
{
	D_ALLOC(("CBitmapTool::~CBitmapTool()\n"));

	if (m_bitmap)
	{
		delete m_bitmap;
		m_bitmap = NULL;
	}

	if (m_disabledBitmap)
	{
		delete m_disabledBitmap;
		m_disabledBitmap = NULL;
	}
}

// ---------------------------------------------------------------------------
// CTool Implementation

void
CBitmapTool::DrawTool(
	BView *owner,
	BRect toolRect)
{
	D_HOOK(("CBitmapTool::DrawTool()\n"));

	if (IsEnabled() && (IsSelected() || Value()))
	{
		if (Value() == B_CONTROL_ON)
		{
			StdBevels::DrawBorderBevel(owner, toolRect, StdBevels::DEPRESSED_BEVEL);
		}
		else
		{
			StdBevels::DrawBorderBevel(owner, toolRect, StdBevels::NORMAL_BEVEL);
		}
	}
	else
	{
		owner->SetDrawingMode(B_OP_COPY);
		owner->SetLowColor(ToolBar()->ViewColor());
		owner->FillRect(toolRect, B_SOLID_LOW);
	}

	if (IsEnabled())
	{
		owner->SetDrawingMode(B_OP_OVER);
		owner->DrawBitmapAsync(m_bitmap, toolRect.LeftTop()
										 + BPoint(BORDER_WIDTH,
										 		  BORDER_HEIGHT));
	}
	else
	{
		owner->SetDrawingMode(B_OP_OVER);
		owner->DrawBitmapAsync(m_disabledBitmap, toolRect.LeftTop()
												 + BPoint(BORDER_WIDTH,
										 				  BORDER_HEIGHT));
	}
}

void
CBitmapTool::GetContentSize(
	float *width,
	float *height) const
{
	*width = m_bitmap->Bounds().Width() + 2 * BORDER_WIDTH;
	*height = m_bitmap->Bounds().Height() + 2 * BORDER_HEIGHT;
}

void
CBitmapTool::Clicked(
	BPoint point,
	uint32 buttons)
{
	SetValue(!Value());
}

void
CBitmapTool::ValueChanged()
{
	ToolBar()->Invalidate(Frame());
}

// END - BitmapTool.cpp
