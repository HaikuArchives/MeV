/* ===================================================================== *
 * BorderView.cpp (MeV/User Interface)
 * ===================================================================== */

#include "BorderView.h"

// ---------------------------------------------------------------------------
// Constructor/Destructor

CBorderView::CBorderView(
	BRect frame,
	const char *name,
	uint32 resizingMode,
	uint32 flags,
	const rgb_color *color,
	border_style style)
	:	BView(frame, name, resizingMode, flags | B_FULL_UPDATE_ON_RESIZE),
		m_style(style)
{
	if (!color)
	{
		m_color = ui_color(B_PANEL_BACKGROUND_COLOR);
	}
	else
	{
		m_color = *color;
	}

	SetViewColor(m_color);
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
			rgb_color dark = tint_color(m_color, B_DARKEN_2_TINT);
			SetHighColor(dark);
			StrokeRect(r);
			break;
		}
		case BEVEL_BORDER:
		{
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

// END - BorderView.cpp
