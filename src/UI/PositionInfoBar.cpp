/* ===================================================================== *
 * PositionInfoBar.cpp (MeV/UI)
 * ===================================================================== */

#include "PositionInfoBar.h"

#include "ResourceUtils.h"

// Interface Kit
#include <Bitmap.h>
// Support Kit
#include <Debug.h>

#define D_ALLOC(x) //PRINT(x)
#define D_HOOK(x) //PRINT(x)
#define D_OPERATION(x) //PRINT(x)

// -------------------------------------------------------- //
// Constructor/Destructor

CPositionInfoBar::CPositionInfoBar(
	BRect frame,
	BScrollBar *scrollBar)
	:	CStatusBar(frame, scrollBar, true, B_FOLLOW_LEFT | B_FOLLOW_BOTTOM),
		m_verticalIcon(NULL),
		m_horizontalIcon(NULL) {
	D_ALLOC(("CPositionInfoBar::CPositionInfoBar()\n"));

	m_verticalIcon = ResourceUtils::LoadImage("VerticalPositionInfo");
	m_horizontalIcon = ResourceUtils::LoadImage("HorizontalPositionInfo");
}

CPositionInfoBar::~CPositionInfoBar() {
	D_ALLOC(("CPositionInfoBar::~CPositionInfoBar()\n"));

	delete m_verticalIcon;
	delete m_horizontalIcon;
}

// -------------------------------------------------------- //
// Operations

void
CPositionInfoBar::SetText(
	orientation which,
	BString text)
{
	D_OPERATION(("CPositionInfoBar::SetText()\n"));

	switch (which)
	{
		case B_VERTICAL:
		{
			m_verticalText = text;
			Update();
			break;
		}
		case B_HORIZONTAL:
		{
			m_horizontalText = text;
			Update();
			break;
		}
	}
}

// -------------------------------------------------------- //
// CStatusBar Implementation

void
CPositionInfoBar::FrameResized(
	float width,
	float height)
{
	D_HOOK(("CPositionInfoBar::FrameResized()\n"));

	CStatusBar::FrameResized(width, height);
	Update();
}

void 
CPositionInfoBar::DrawInto(
	BView *v,
	BRect updateRect)
{
	D_HOOK(("CPositionInfoBar::DrawInto()\n"));

	CStatusBar::DrawInto(v, updateRect);

	BRect rect(Bounds());
	BPoint iconOffset;
	BPoint textOffset;
	BString text;
	
	v->SetFont(be_fixed_font);
	v->SetFontSize(10.0);
	v->SetHighColor(64, 64, 64, 255);

	font_height fh;
	v->GetFontHeight(&fh);

	iconOffset = rect.LeftTop() + BPoint(2.0, 2.0);
	if (m_verticalText == "")
		v->SetDrawingMode(B_OP_BLEND);
	v->DrawBitmapAsync(m_verticalIcon, iconOffset);
	if (m_verticalText == "")
		v->SetDrawingMode(B_OP_OVER);
	textOffset = rect.LeftBottom();
	textOffset.x += 6.0;
	textOffset.y = (rect.Height() / 2.0) + (fh.ascent / 2.0);
	textOffset.x += m_verticalIcon->Bounds().Width();
	text = m_verticalText;
	v->TruncateString(&text, B_TRUNCATE_END, rect.Width() / 2.0 - textOffset.x - 6.0);
	v->DrawString(text.String(), textOffset);

	iconOffset.x += rect.Width() / 2.0;
	textOffset.x += rect.Width() / 2.0;
	if (m_horizontalText == "")
		v->SetDrawingMode(B_OP_BLEND);
	v->DrawBitmapAsync(m_horizontalIcon, iconOffset);
	if (m_horizontalText == "")
		v->SetDrawingMode(B_OP_OVER);
	text = m_horizontalText;
	v->TruncateString(&text, B_TRUNCATE_END, rect.Width() - textOffset.x - 6.0);
	v->DrawString(text.String(), textOffset);
}

// END -- PositionInfoBar.cpp --
