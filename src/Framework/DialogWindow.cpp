/* ===================================================================== *
 * DialogWindow.cpp (MeV/Framework)
 * ===================================================================== */

#include "DialogWindow.h"

#include "ScreenUtils.h"

// Interface
#include <Button.h>
#include <View.h>
// Support Kit
#include <Debug.h>

// Debugging Macros
#define D_ALLOC(x) //PRINT(x)		// Constructor/Destructor
#define D_HOOK(x) //PRINT(x)		// BWindow/CObserver Implementation
#define D_MESSAGE(x) //PRINT(x)		// MessageReceived()

// ---------------------------------------------------------------------------
// CDialogBackgroundView class

class CDialogBackgroundView
	:	public BView
{
	friend class CDialogWindow;

public:							// Constructor/Destructor

								CDialogBackgroundView(
									BRect rect);

public:							// Accessors

	BRect						ContentFrame() const;

public:							// BView Implementation

	virtual void				Draw(
									BRect updateRect);

private:						//  Instance Data

	float						m_separatorOffset;
};

// ---------------------------------------------------------------------------
// Constructor/Destructor

CDialogWindow::CDialogWindow(
	BRect frame,
	BWindow *parent)
	:	BWindow(frame, "Dialog", B_MODAL_WINDOW_LOOK,
				B_MODAL_SUBSET_WINDOW_FEEL,
				B_NOT_RESIZABLE | B_NOT_ZOOMABLE | B_NOT_CLOSABLE),
		m_parent(parent),
		m_buttonOffset(10.0)
{
	D_ALLOC(("CDialogWindow::CDialogWindow()\n"));

	status_t error = AddToSubset(m_parent);
	if (error)
		D_ALLOC((" -> could not add self to parent windows subset\n"));

	m_backgroundView = new CDialogBackgroundView(Bounds());
	AddChild(m_backgroundView);
}

CDialogWindow::~CDialogWindow()
{
	D_ALLOC(("CDialogWindow::~CDialogWindow()\n"));
}

// ---------------------------------------------------------------------------
// Accessors

BView *
CDialogWindow::BackgroundView() const
{
	return m_backgroundView;
}

BRect
CDialogWindow::ContentFrame() const
{
	return m_backgroundView->ContentFrame();
}

BWindow *
CDialogWindow::Parent() const
{
	return m_parent;
}

// ---------------------------------------------------------------------------
// Operations

BButton *
CDialogWindow::AddButton(
	const char *label,
	BMessage *message)
{
	BRect rect(Bounds());
	BButton *button = new BButton(rect, label, label, message,
								  B_FOLLOW_LEFT | B_FOLLOW_BOTTOM);
	m_backgroundView->AddChild(button);
	button->ResizeToPreferred();
	button->MoveTo(m_buttonOffset,
				   rect.bottom - button->Frame().Height() - 10.0);
	m_backgroundView->m_separatorOffset = rect.bottom - button->Frame().top + 10.0;
	m_buttonOffset = button->Frame().right + 10.0;

	return button;
}

// ---------------------------------------------------------------------------
// BWindow Implementation

void
CDialogWindow::Show()
{
	// center over parent
	BRect rect = UScreenUtils::CenterOnWindow(Frame().Width(), Frame().Height(),
											  m_parent);
	MoveTo(rect.left, rect.top);

	BWindow::Show();
}

// ---------------------------------------------------------------------------
// CDialogBackgroundView Implementation

CDialogBackgroundView::CDialogBackgroundView(
	BRect frame)
	:	BView(frame, "", B_FOLLOW_ALL_SIDES, B_WILL_DRAW),
		m_separatorOffset(0.0)
{
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
}

BRect
CDialogBackgroundView::ContentFrame() const
{
	BRect rect(Bounds());
	rect.bottom -= m_separatorOffset;
	rect.InsetBy(10.0, 5.0);

	return rect;
}

void
CDialogBackgroundView::Draw(
	BRect updateRect)
{
	BRect rect(ContentFrame());
	rect.bottom += 5.0;
	rect.top = rect.bottom - 1.0;
	SetHighColor(tint_color(ViewColor(), B_LIGHTEN_2_TINT));
	StrokeLine(rect.LeftTop(), rect.RightTop(), B_SOLID_HIGH);
	SetHighColor(tint_color(ViewColor(), B_DARKEN_2_TINT));
	StrokeLine(rect.LeftBottom(), rect.RightBottom(), B_SOLID_HIGH);
}

// END - DialogWindow.cpp
