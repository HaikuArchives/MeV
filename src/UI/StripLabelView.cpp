/* ===================================================================== *
 * StripLabelView.cpp (MeV/StripView)
 * ===================================================================== */

#include "StripLabelView.h"

// StripView
#include "StripView.h"

// Application Kit
#include "Message.h"
// Interface Kit
#include "MenuItem.h"
#include "PopUpMenu.h"
#include "Window.h"
// Support Kit
#include "Debug.h"

// Debugging Macros
#define D_ALLOC(x) //PRINT(x)			// Constructor/Destructor
#define D_HOOK(x) //PRINT(x)			// CBorderView Implementation
#define D_INTERNAL(x) PRINT(x)		// Internal Operations
#define D_OPERATION(x) //PRINT(x)		// Operations

// ---------------------------------------------------------------------------
// Constructor/Destructor

CStripLabelView::CStripLabelView(
	BRect frame,
	const char *name,
	const rgb_color	&fillColor,
	uint32 resizingMode,
	uint32 flags)
	:	CBorderView(frame, name, resizingMode, flags, &fillColor),
		m_stripView(NULL),
		m_contextMenu(NULL)
{
	BFont font(be_bold_font);
	font.SetRotation(-90.0);
	SetFont(&font);

	InitContextMenu();
}

CStripLabelView::CStripLabelView(
	BRect frame,
	const char *name,
	uint32 resizingMode,
	ulong flags)
	:	CBorderView(frame, name, resizingMode, flags),
		m_stripView(NULL),
		m_contextMenu(NULL)
{
	BFont font(be_bold_font);
	font.SetRotation(-90.0);
	SetFont(&font);

	InitContextMenu();
}

CStripLabelView::~CStripLabelView()
{
	if (m_contextMenu)
	{
		delete m_contextMenu;
		m_contextMenu = NULL;
	}
}

// ---------------------------------------------------------------------------
// CBorderView Implementation

void
CStripLabelView::Draw(
	BRect updateRect)
{
	BRect rect(Bounds());
	float w = StringWidth(Name()), h, x, y;
	font_height	fh;

	GetFontHeight(&fh);
	h = fh.ascent + fh.descent;

	x = rect.left + (rect.Width() - h) / 2.0 + fh.descent;
	y = rect.top + floor((rect.Height() - w) / 2.0);

	CBorderView::Draw(updateRect);

	SetHighColor(128, 128, 128, 255);
	SetLowColor(220, 220, 220, 255);
	DrawString(Name(), BPoint(x, y));
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

	m_contextMenu->AddItem(new BMenuItem("Hide",
										 new BMessage(CStripView::HIDE)));
}

void
CStripLabelView::ShowContextMenu(
	BPoint point)
{
	if (!m_contextMenu)
		return;

	BMenuItem *item = m_contextMenu->FindItem(CStripView::HIDE);
	if (item)
		item->SetEnabled(m_stripView->IsRemovable());

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

// END - StripLabelView.cpp
