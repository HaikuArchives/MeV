/* ===================================================================== *
 * BorderView.cpp (MeV/UI)
 * ===================================================================== */

#include "BorderView.h"

// Interface Kit
#include <Window.h>
// Support Kot
#include <Debug.h>

// ---------------------------------------------------------------------------
// Constructor/Destructor

CBorderView::CBorderView(
	BRect frame,
	const char *name,
	uint32 resizingMode,
	uint32 flags,
	bool dimOnDeactivate,
	const rgb_color *color,
	border_style style)
	:	BView(frame, name, resizingMode,
			  flags | B_FULL_UPDATE_ON_RESIZE),
		m_style(style),
		m_dimOnDeactivate(dimOnDeactivate)
{
	if (color == NULL)
		m_color = ui_color(B_PANEL_BACKGROUND_COLOR);
	else
		m_color = *color;

	SetViewColor(m_color);

	if (m_dimOnDeactivate)
		SetFlags(Flags() | B_DRAW_ON_CHILDREN);
}

// ---------------------------------------------------------------------------
// BView Implementation

void
CBorderView::Draw(
	BRect updateRect)
{
	BRect r(Bounds());

	switch (m_style)
	{
		case NORMAL_BORDER:
		{
			SetDrawingMode(B_OP_COPY);
			rgb_color dark = tint_color(m_color, B_DARKEN_2_TINT);
			SetHighColor(dark);
			StrokeRect(r);
			break;
		}
		case BEVEL_BORDER:
		{
			SetDrawingMode(B_OP_COPY);
			rgb_color light = tint_color(m_color, B_LIGHTEN_2_TINT);
			rgb_color dark = tint_color(m_color, B_DARKEN_2_TINT);
			BeginLineArray(4);
			{
				AddLine(r.LeftBottom(), r.RightBottom(), dark);
				AddLine(r.RightBottom(), r.RightTop(), dark);
				AddLine(r.RightTop(), r.LeftTop(), light);
				AddLine(r.LeftTop(), r.LeftBottom(), light);
			}
			EndLineArray();
			break;
		}
	}
}

void
CBorderView::DrawAfterChildren(
	BRect updateRect)
{
	if (m_dimOnDeactivate && !Window()->IsActive())
	{
		SetHighColor(255, 255, 255, 255);
		SetDrawingMode(B_OP_BLEND);
		BRect r(Bounds());
		r.InsetBy(1.0, 1.0);
		FillRect(r & updateRect);
	}
}

void
CBorderView::WindowActivated(
	bool active)
{
	if (m_dimOnDeactivate)
		Invalidate();
}

// END - BorderView.cpp
