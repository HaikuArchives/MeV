/* ===================================================================== *
 * StatusBar.cpp (MeV/UI)
 * ===================================================================== */

#include "StatusBar.h"

// Application Kit
#include <Message.h>
// Interface Kit
#include <Bitmap.h>
#include <ScrollBar.h>
#include <Window.h>
// Support Kit
#include <Debug.h>

#define D_ALLOC(x) //PRINT(x)
#define D_HOOK(x) //PRINT(x)
#define D_OPERATION(x) //PRINT(x)

// -------------------------------------------------------- //
// Constants

const rgb_color BLACK_COLOR			= {0, 0, 0, 255};
const rgb_color WHITE_COLOR			= {255, 255, 255, 255}; 
const rgb_color GRAY_COLOR			= ui_color(B_PANEL_BACKGROUND_COLOR);
const rgb_color LIGHT_GRAY_COLOR	= tint_color(GRAY_COLOR, B_LIGHTEN_2_TINT); 
const rgb_color MED_GRAY_COLOR		= tint_color(GRAY_COLOR, B_DARKEN_2_TINT); 
const rgb_color DARK_GRAY_COLOR		= tint_color(MED_GRAY_COLOR, B_DARKEN_2_TINT); 

// -------------------------------------------------------- //
// Constructor/Destructor

CStatusBar::CStatusBar(
	BRect frame,
	BScrollBar *scrollBar,
	bool dimOnDeactivate,
	uint32 resizingMode)
	:	BView(frame, "StatusBar", resizingMode,
					B_FRAME_EVENTS | B_WILL_DRAW),
		m_scrollBar(scrollBar),
		m_dragging(false),
		m_backBitmap(NULL),
		m_backView(NULL),
		m_dirty(true),
		m_minWidth(10.0),
		m_dimOnDeactivate(dimOnDeactivate) {
	D_ALLOC(("CStatusBar::CStatusBar()\n"));

	SetViewColor(B_TRANSPARENT_COLOR);

	if (m_dimOnDeactivate)
		SetFlags(Flags() | B_DRAW_ON_CHILDREN);
}

CStatusBar::~CStatusBar() {
	D_ALLOC(("CStatusBar::~CStatusBar()\n"));

	delete m_backBitmap;
}

// -------------------------------------------------------- //
// Hook Functions

void 
CStatusBar::DrawInto(
	BView *v,
	BRect updateRect)
{
	BRect r(Bounds());

	// draw border (minus right edge, which the scrollbar draws)
	v->SetDrawingMode(B_OP_COPY);
	v->BeginLineArray(8);
	v->AddLine(r.LeftTop(), r.RightTop(), MED_GRAY_COLOR);
	BPoint rtop = r.RightTop();
	rtop.y++;
	v->AddLine(rtop, r.RightBottom(), tint_color(MED_GRAY_COLOR, B_LIGHTEN_1_TINT));
	v->AddLine(r.RightBottom(), r.LeftBottom(), MED_GRAY_COLOR);
	v->AddLine(r.LeftBottom(), r.LeftTop(), MED_GRAY_COLOR);
	r.InsetBy(1.0, 1.0);
	v->AddLine(r.LeftTop(), r.RightTop(), LIGHT_GRAY_COLOR);
	rtop.y++;
	rtop.x--;
	v->AddLine(rtop, r.RightBottom(), GRAY_COLOR);
	v->AddLine(r.RightBottom(), r.LeftBottom(), tint_color(MED_GRAY_COLOR, B_LIGHTEN_1_TINT));
	v->AddLine(r.LeftBottom(), r.LeftTop(), LIGHT_GRAY_COLOR);
	v->EndLineArray();
	r.InsetBy(1.0, 1.0);
	v->SetLowColor(GRAY_COLOR);
	v->FillRect(r, B_SOLID_LOW);

	// draw resize dragger
	if (m_scrollBar) {
		v->SetDrawingMode(B_OP_OVER);
		r = Bounds();
		r.right -= 2.0;
		r.left = r.right - 2.0;
		r.InsetBy(0.0, 3.0);
		r.top += 1.0;
		for (int32 i = 0; i < r.IntegerHeight(); i += 3) {
			BPoint p = r.LeftTop() + BPoint(0.0, i);
			v->SetHighColor(MED_GRAY_COLOR);
			v->StrokeLine(p, p, B_SOLID_HIGH);
			p += BPoint(1.0, 1.0);
			v->SetHighColor(WHITE_COLOR);
			v->StrokeLine(p, p, B_SOLID_HIGH);
		}
	}
}

// -------------------------------------------------------- //
// Operations

void
CStatusBar::SetMinimumWidth(
	float width)
{
	m_minWidth = width;
	if (Bounds().Width() < width)
		ResizeBy(width - Bounds().Width(), 0.0);
}

void
CStatusBar::Update()
{
	D_OPERATION(("CStatusBar::Update()\n"));

	m_dirty = true;
	Invalidate();
}

// -------------------------------------------------------- //
// BView Implementation

void
CStatusBar::AttachedToWindow()
{
	D_HOOK(("CStatusBar::AttachedToWindow()\n"));

	FrameResized(Bounds().Width(), Bounds().Height());
}

void
CStatusBar::Draw(
	BRect updateRect)
{
	D_HOOK(("CStatusBar::Draw()\n"));

	if (!m_backView)
	{
		DrawInto(this, updateRect);
	}
	else
	{
		if (m_dirty)
		{
			m_backBitmap->Lock();
			DrawInto(m_backView, updateRect);
			m_backView->Sync();
			m_backBitmap->Unlock();
			m_dirty = false;
		}

		SetDrawingMode(B_OP_COPY);
		DrawBitmap(m_backBitmap, updateRect, updateRect);
	}
}

void
CStatusBar::DrawAfterChildren(
	BRect updateRect)
{
	if (m_dimOnDeactivate && !Window()->IsActive())
	{
		SetHighColor(255, 255, 255, 255);
		SetDrawingMode(B_OP_BLEND);
		BRect rect(Bounds());
		rect.InsetBy(1.0, 1.0);
		FillRect(rect & updateRect);
	}
}

void
CStatusBar::FrameResized(
	float width,
	float height)
{
	D_HOOK(("CStatusBar::FrameResized()\n"));

	AllocBackBitmap(width, height);

	if (m_scrollBar)
	{
		float minWidth, maxWidth, minHeight, maxHeight;
		Window()->GetSizeLimits(&minWidth, &maxWidth, &minHeight, &maxHeight);
		minWidth = width + 6 * B_V_SCROLL_BAR_WIDTH + 2.0;
		Window()->SetSizeLimits(minWidth, maxWidth, minHeight, maxHeight);
	}
}

void
CStatusBar::MouseDown(
	BPoint point)
{
	D_HOOK(("CStatusBar::MouseDown()\n"));

	if ((m_scrollBar == NULL) || !Window()->IsActive())
		return;

	int32 buttons;
	if (Window()->CurrentMessage()->FindInt32("buttons", &buttons) != B_OK)
		buttons = B_PRIMARY_MOUSE_BUTTON;

	if (buttons == B_PRIMARY_MOUSE_BUTTON)
	{
		// drag rect
		BRect dragRect(Bounds());
		dragRect.left = dragRect.right - 10.0;
		if (dragRect.Contains(point))
		{
			// resize
			m_dragging = true;
			SetMouseEventMask(B_POINTER_EVENTS,
							  B_LOCK_WINDOW_FOCUS | B_NO_POINTER_HISTORY);
		}
	}
}

void
CStatusBar::MouseMoved(
	BPoint point,
	uint32 transit,
	const BMessage *message) {
	D_HOOK(("CStatusBar::MouseMoved()\n"));

	if (!m_scrollBar)
		return;

	if (m_dragging)
	{
		float x = point.x - (Bounds().right - 5.0);
		if ((Bounds().Width() + x) <= 16.0)
			return;
		if (m_scrollBar
		 && ((m_scrollBar->Bounds().Width() - x) < (3 * B_V_SCROLL_BAR_WIDTH + 3)))
			x = m_scrollBar->Bounds().Width() - (3 * B_V_SCROLL_BAR_WIDTH + 3);
		if ((Bounds().Width() + x) < m_minWidth)
			x = m_minWidth - Bounds().Width();
		ResizeBy(x, 0.0);
		BRect rect(Bounds());
		rect.left = rect.right - 10.0;
		m_dirty = true;
		Invalidate(rect);
		if (m_scrollBar)
		{
			m_scrollBar->ResizeBy(-x, 0.0);
			m_scrollBar->MoveBy(x, 0.0);
		}
	}
}

void CStatusBar::MouseUp(
	BPoint point) {
	D_HOOK(("CStatusBar::MouseUp()\n"));

	if (m_scrollBar == NULL)
		return;

	m_dragging = false;
}

void
CStatusBar::WindowActivated(
	bool active)
{
	if (m_dimOnDeactivate)
		Invalidate();
}

// -------------------------------------------------------- //
// Internal Operations

void
CStatusBar::AllocBackBitmap(
	float width,
	float height) {
	D_OPERATION(("CStatusBar::AllocBackBitmap(%.1f, %.1f)\n", width, height));

	// sanity check
	if (width <= 0.0 || height <= 0.0)
		return;

	if (m_backBitmap != NULL)
	{
		// see if the bitmap needs to be expanded
		BRect rect = m_backBitmap->Bounds();
		if ((rect.Width() >= width) && (rect.Height() >= height))
			return;

		// it does; clean up:
		FreeBackBitmap();
	}

	BRect rect(0.0, 0.0, width, height);
	m_backBitmap = new BBitmap(rect, B_RGB32, true);

	m_backView = new BView(rect, 0, B_FOLLOW_NONE, B_WILL_DRAW);
	m_backBitmap->AddChild(m_backView);
	m_dirty = true;
}

void
CStatusBar::FreeBackBitmap() {
	D_OPERATION(("CStatusBar::FreeBackBitmap()\n"));

	if (m_backBitmap) {
		delete m_backBitmap;
		m_backBitmap = 0;
		m_backView = 0;
	}
}

// END -- StatusBar.cpp --
