/* ===================================================================== *
 * StripLabelView.cpp (MeV/StripView)
 * ===================================================================== */

#include "StripLabelView.h"

#include "StripFrameView.h"
#include "StripView.h"

// Application Kit
#include <Message.h>
// Interface Kit
#include <Bitmap.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <Region.h>
#include <Window.h>
// Support Kit
#include <Debug.h>

// Debugging Macros
#define D_ALLOC(x) //PRINT(x)			// Constructor/Destructor
#define D_HOOK(x) //PRINT(x)			// CBorderView Implementation
#define D_INTERNAL(x) //PRINT(x)		// Internal Operations
#define D_OPERATION(x) //PRINT(x)		// Operations

// ---------------------------------------------------------------------------
// Constructor/Destructor

CStripLabelView::CStripLabelView(
	BRect frame,
	const char *name,
	uint32 resizingMode,
	ulong flags)
	:	BView(frame, name, resizingMode, flags),
		m_stripView(NULL),
		m_bitmap(NULL),
		m_contextMenu(NULL),
		m_dragging(false)
{
	SetViewColor(B_TRANSPARENT_COLOR);
	SetLowColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	SetHighColor(128, 128, 128, 255);

	InitContextMenu();
}

CStripLabelView::~CStripLabelView()
{
	if (m_bitmap)
	{
		delete m_bitmap;
		m_bitmap = NULL;
	}

	if (m_contextMenu)
	{
		delete m_contextMenu;
		m_contextMenu = NULL;
	}
}

// ---------------------------------------------------------------------------
// Hook Functions

void
CStripLabelView::DrawInto(
	BView *view,
	BRect updateRect)
{
	BRect rect(Bounds());
	BRegion region(updateRect);

	view->SetLowColor(LowColor());
	view->SetHighColor(HighColor());

	if (m_bitmap)
	{
		BRect bitmapRect(m_bitmap->Bounds());
		bitmapRect.OffsetTo(rect.Width() / 2.0 - bitmapRect.Width() / 2.0,
							rect.Height() / 2.0 - bitmapRect.Height() / 2.0);
		view->DrawBitmapAsync(m_bitmap, bitmapRect.LeftTop());
		bitmapRect.InsetBy(1.0, 1.0);
		region.Exclude(bitmapRect);
	}

	if (updateRect.right >= rect.right)
	{
		view->StrokeLine(updateRect.RightTop(), updateRect.RightBottom(),
						 B_SOLID_HIGH);
		region.Exclude(BRect(updateRect.RightTop(), updateRect.RightBottom()));
	}

	view->FillRegion(&region, B_SOLID_LOW);
}

// ---------------------------------------------------------------------------
// Accessors

CStripFrameView *
CStripLabelView::FrameView() const
{
	return m_stripView->FrameView();
}

// ---------------------------------------------------------------------------
// BView Implementation

void
CStripLabelView::AttachedToWindow()
{
	if (m_bitmap == NULL)
	{
		BFont font(be_bold_font);
		font.SetRotation(-90.0);
		font_height fh;
		font.GetHeight(&fh);

		BRect rect(0.0, 0.0, 0.0, 0.0);
		rect.right = fh.ascent + fh.descent;
		rect.bottom = font.StringWidth(Name());

		BBitmap *tempBitmap = new BBitmap(rect, B_CMAP8, true);
		BView *tempView = new BView(rect, "", B_FOLLOW_NONE, B_WILL_DRAW);
		if (tempBitmap->Lock())
		{
			tempBitmap->AddChild(tempView);
			tempView->SetDrawingMode(B_OP_COPY);
			tempView->SetLowColor(LowColor());
			tempView->FillRect(rect, B_SOLID_LOW);
			tempView->SetDrawingMode(B_OP_OVER);
			tempView->SetFont(&font);
			tempView->SetHighColor(HighColor());
			tempView->DrawString(Name(),
								 rect.LeftTop() + BPoint(fh.descent, 0.0));
			tempView->Sync();
			tempBitmap->Unlock();
		}

		m_bitmap = new BBitmap(tempBitmap);
		delete tempBitmap;
	}
}

void
CStripLabelView::Draw(
	BRect updateRect)
{
	DrawInto(this, updateRect);
}

void
CStripLabelView::MouseDown(
	BPoint point)
{
	int32 buttons = B_PRIMARY_MOUSE_BUTTON;
	Window()->CurrentMessage()->FindInt32("buttons", &buttons);

	if ((buttons == B_SECONDARY_MOUSE_BUTTON)
	 || ((buttons == B_PRIMARY_MOUSE_BUTTON) && (modifiers() & B_CONTROL_KEY)))
	{
		ShowContextMenu(point);
	}
	else if (buttons == B_PRIMARY_MOUSE_BUTTON)
	{
		SetMouseEventMask(B_POINTER_EVENTS,
						  B_NO_POINTER_HISTORY | B_LOCK_WINDOW_FOCUS);
		BMessage message(CStripView::REARRANGE_STRIPS);
		message.AddPointer("strip", reinterpret_cast<void *>(m_stripView));

		// make the drag bitmap
		BBitmap *dragBitmap = make_drag_bitmap();
		DragMessage(&message, dragBitmap, B_OP_ALPHA,
					point - Bounds().LeftTop(), m_stripView);
		m_dragging = true;
	}
}

void
CStripLabelView::MouseMoved(
	BPoint point,
	uint32 transit,
	const BMessage *message)
{
	if (!m_dragging && message)
	{
		if (message->what == CStripView::REARRANGE_STRIPS)
		{
			CStripView *view;
			if (message->FindPointer("strip", reinterpret_cast<void **>(&view)) == B_OK)
			{
				if (view != Parent())
				{
					float center = Bounds().Height() / 2;
					float offset = point.y - Bounds().top;
					int32 thisIndex = FrameView()->IndexOf(m_stripView);
					int32 dragIndex = FrameView()->IndexOf(view);
					if (((dragIndex < thisIndex) && (offset > center))
					 || ((dragIndex > thisIndex) && (offset < center)))
					{
						FrameView()->SwapStrips(view, m_stripView);
					}
				}
			}
		}
	}
}

void
CStripLabelView::MouseUp(
	BPoint point)
{
	if (m_dragging)
	{
		m_dragging = false;
	}
}

// ---------------------------------------------------------------------------
// Internal Methods

void
CStripLabelView::InitContextMenu()
{
	if (m_contextMenu)
	{
		delete m_contextMenu;
		m_contextMenu = NULL;
	}

	m_contextMenu = new BPopUpMenu("", false, false, B_ITEMS_IN_COLUMN);
	m_contextMenu->SetFont(be_plain_font);

	m_contextMenu->AddItem(new BMenuItem("Remove Strip",
										 new BMessage(CStripView::REMOVE_STRIP)));
}

void
CStripLabelView::ShowContextMenu(
	BPoint point)
{
	if (!m_contextMenu)
		return;

	BMenuItem *item = m_contextMenu->FindItem(CStripView::REMOVE_STRIP);
	if (item)
	{
		if (!m_stripView->IsRemovable() || (FrameView()->CountStrips() < 2))
			item->SetEnabled(false);
	}

	BMessenger messenger(m_stripView, Window());
	m_contextMenu->SetTargetForItems(messenger);
	ConvertToScreen(&point);
	point -= BPoint(1.0, 1.0);
	m_contextMenu->Go(point, true, true, true);
}

// ---------------------------------------------------------------------------
// Internal Operations

void
CStripLabelView::attach(
	CStripView *stripView)
{
	m_stripView = stripView;
}

BBitmap *
CStripLabelView::make_drag_bitmap()
{
	BRect rect(Bounds());
	rect.OffsetTo(B_ORIGIN);

	// draw into a temporary bitmap
	BBitmap *drawBitmap = new BBitmap(rect, B_CMAP8, true);
	if (drawBitmap->Lock())
	{
		BView *drawView = new BView(rect, "", B_FOLLOW_NONE,
									B_WILL_DRAW);
		drawBitmap->AddChild(drawView);
		DrawInto(drawView, rect);
		drawView->Sync();
		drawBitmap->Unlock();
	}

	// make the drag bitmap using alpha mode
	BBitmap *dragBitmap = new BBitmap(Bounds(), B_RGBA32, true);
	if (dragBitmap->Lock())
	{
		BView *dragView = new BView(rect, "", B_FOLLOW_NONE, 0);
		dragBitmap->AddChild(dragView);
		dragView->SetOrigin(0.0, 0.0);
		dragView->SetHighColor(0, 0, 0, 0);
		dragView->FillRect(dragView->Bounds(), B_SOLID_HIGH); 
		dragView->SetDrawingMode(B_OP_ALPHA);
		dragView->SetHighColor(0, 0, 0, 128);
	    dragView->SetBlendingMode(B_CONSTANT_ALPHA, B_ALPHA_COMPOSITE); 
		dragView->DrawBitmap(drawBitmap, B_ORIGIN);
		dragView->SetHighColor(0, 0, 255, 128);
		dragView->StrokeRect(dragView->Bounds(), B_SOLID_HIGH);
		dragView->Sync();
		dragBitmap->Unlock();
	}

	delete drawBitmap;
	return dragBitmap;
}

// END - StripLabelView.cpp
