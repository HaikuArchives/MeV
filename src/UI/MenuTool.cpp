/* ===================================================================== *
 * MenuTool.cpp (MeV/User Interface)
 * ===================================================================== */

#include "MenuTool.h"
// User Interface
#include "ToolBar.h"
#include "StdBevels.h"

// Interface Kit
#include <Bitmap.h>
#include <PopUpMenu.h>
#include <Screen.h>
#include <View.h>
// Support Kit
#include <Debug.h>

// Debugging Macros
#define D_ALLOC(x) //PRINT (x)		// Constructor/Destructor
#define D_HOOK(x) //PRINT (x)		// CTool Implementation
#define D_OPERATION(x) //PRINT (x)	// Operations

// ---------------------------------------------------------------------------
// Constructor/Destructor

CMenuTool::CMenuTool(
	const char *name,
	BBitmap *bitmap,
	BPopUpMenu *menu,
	BMessage *message,
	int32 mode,
	uint32 flags)
	:	CTool(name, message, mode, flags),
		m_bitmap(NULL),
		m_disabledBitmap(NULL),
		m_menu(menu)
{
	D_ALLOC(("CMenuTool::CMenuTool()\n"));

	SetBitmap(bitmap);

	if (!m_menu)
	{
		// create an empty menu
		m_menu = new BPopUpMenu("", false, false);
	}
}

CMenuTool::~CMenuTool()
{
	D_ALLOC(("CMenuTool::~CMenuTool()\n"));

	delete m_bitmap;
	delete m_disabledBitmap;
	delete m_menu;
}

// ---------------------------------------------------------------------------
// Operations

void
CMenuTool::SetBitmap(
	BBitmap *bitmap)
{
	D_OPERATION(("CMenuTool::SetBitmap()\n"));

	if (bitmap && (bitmap != m_bitmap))
	{
		delete m_bitmap;
		delete m_disabledBitmap;

		m_bitmap = bitmap;

		// calculate disabled bitmap
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
				continue;
			if (color.red + color.green + color.blue > 384)
				*pos = light;
			else
				*pos = dark;
		}
	}

	if (ToolBar())
		ToolBar()->Invalidate();
}

// ---------------------------------------------------------------------------
// CTool Implementation

void
CMenuTool::DrawTool(
	BView *owner,
	BRect toolRect)
{
	D_HOOK(("CMenuTool::DrawTool()\n"));

	BRect bitmapRect = toolRect;
	bitmapRect.right -= 8;
	BRect menuRect = toolRect;
	menuRect.left = bitmapRect.right;

	m_popUpOffset = menuRect.LeftTop();
		// remember offset of the popup menu

	if (IsEnabled() && (IsSelected() || Value()))
	{
		if (Value() == B_CONTROL_ON)
		{
			StdBevels::DrawBorderBevel(owner, bitmapRect, StdBevels::DEPRESSED_BEVEL);
			if (IsSelected())
			{
				StdBevels::DrawBorderBevel(owner, menuRect, StdBevels::DEPRESSED_BEVEL);
			}
		}
		else
		{
			StdBevels::DrawBorderBevel(owner, bitmapRect, StdBevels::NORMAL_BEVEL);
			StdBevels::DrawBorderBevel(owner, menuRect, StdBevels::NORMAL_BEVEL);
		}
		if (IsSelected())
		{
			BRect arrowRect = menuRect;
			arrowRect.InsetBy(1.0, arrowRect.Height() / 2.0 - 1.0);
			rgb_color gray = tint_color(ui_color(B_PANEL_BACKGROUND_COLOR),
										B_DARKEN_2_TINT);
			owner->BeginLineArray(3);
			owner->AddLine(arrowRect.LeftTop(), arrowRect.RightTop(), gray);
			arrowRect.InsetBy(1.0, 0.0);
			arrowRect.top += 1.0;
			owner->AddLine(arrowRect.LeftTop(), arrowRect.RightTop(), gray);
			arrowRect.InsetBy(1.0, 0.0);
			arrowRect.top += 1.0;
			owner->AddLine(arrowRect.LeftTop(), arrowRect.RightTop(), gray);
			owner->EndLineArray();
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
CMenuTool::GetContentSize(
	float *width,
	float *height) const
{
	*width = m_bitmap->Bounds().Width() + 2 * BORDER_WIDTH + 8.0;
	*height = m_bitmap->Bounds().Height() + 2 * BORDER_HEIGHT;
}

void
CMenuTool::Clicked(
	BPoint point,
	uint32 buttons)
{
	if (point.x < m_popUpOffset.x)
	{
		SetValue(!Value());
	}
	else if (m_menu)
	{
		BPoint screenPoint = ToolBar()->ConvertToScreen(m_popUpOffset);
		m_menu->Go(screenPoint, true, true, true);
		Select(false);
	}
}

void
CMenuTool::ValueChanged()
{
	ToolBar()->Invalidate(Frame());
}

// END - BitmapTool.cpp
