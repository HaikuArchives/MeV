/* ===================================================================== *
 * StringEditView.cpp (MeV/UI)
 * ===================================================================== */

#include "StringEditView.h"

// Interface Kit
#include <TextView.h>
// Support Kit
#include <Debug.h>

// Debugging Macros
#define D_ALLOC(x) //PRINT (x)		// Constructor/Destructor
#define D_HOOK(x) //PRINT (x)		// BMenuItem Implementation

// ---------------------------------------------------------------------------
// Constructor/Destructor

CStringEditView::CStringEditView(
	BRect frame,
	BString text,
	BMessage *message,
	BMessenger messenger)
	:	BView(frame.InsetByCopy(-2.0, -1.0), "StringEditView",
			  B_FOLLOW_NONE, B_WILL_DRAW),
		BInvoker(message, messenger),
		m_text(text)
{
	D_ALLOC(("CStringEditView::CStringEditView()\n", label));

	BRect rect(Bounds());
	rect.InsetBy(1.0, 1.0);
	m_textView = new BTextView(rect, "StringEditTextView", rect,
							   be_plain_font, 0, B_FOLLOW_ALL, B_WILL_DRAW);
	m_textView->SetWordWrap(false);

	rect.right = rect.left + m_textView->LineWidth() + 3.0;
	rect.bottom = rect.top + m_textView->LineHeight() + 1.0;
	m_textView->ResizeTo(rect.Width(), rect.Height());
	rect.left += 1.0;
	rect.right -= 3;
	rect.bottom--;
	m_textView->SetTextRect(rect);
	AddChild(m_textView);
	ResizeTo(m_textView->Bounds().Width() + 2.0,
			 m_textView->Bounds().Height() + 2.0);
	MoveBy(-1.0, -1.0);

	m_textView->MakeResizable(true, this);
	m_textView->SetText(m_text.String());
//	m_textView->SelectAll();
//	m_textView->MakeFocus(true);
}

CStringEditView::~CStringEditView()
{
	D_ALLOC(("CStringEditView::~CStringEditView()\n"));

}

// ---------------------------------------------------------------------------
// BView Implementation

void
CStringEditView::AllAttached()
{
	D_HOOK(("CStringEditView::AttachedToWindow()\n"));

	m_textView->MakeFocus(true);
	m_textView->SelectAll();
}

void
CStringEditView::AttachedToWindow()
{
	D_HOOK(("CStringEditView::AttachedToWindow()\n"));

	SetEventMask(B_POINTER_EVENTS | B_KEYBOARD_EVENTS, B_NO_POINTER_HISTORY);
}

void
CStringEditView::Draw(
	BRect updateRect)
{
	D_HOOK(("CStringEditView::Draw()\n"));

	SetHighColor(0, 0, 0, 255);
	SetDrawingMode(B_OP_COPY);
	StrokeRect(Bounds(), B_SOLID_HIGH);
}

void
CStringEditView::KeyDown(
	const char *bytes,
	int32 numBytes)
{
	D_HOOK(("CStringEditView::KeyDown()\n"));

	if (bytes[0] == B_RETURN)
	{
		SetEventMask(0);
		RemoveSelf();
		BMessage message(*Message());
		BString text = m_textView->Text();
		text.RemoveLast("\n");
		message.AddString("text", text.String());
		Invoke(&message);
	}
	if (bytes[0] == B_ESCAPE)
	{
		SetEventMask(0);
		RemoveSelf();
		// send an empty message, which equals to a cancel notification
		Invoke(Message());
	}
}

void
CStringEditView::MouseDown(
	BPoint point)
{
	if (!Bounds().Contains(point))
	{
		SetEventMask(0);
		RemoveSelf();
		BMessage message(*Message());
		message.AddString("text", m_textView->Text());
		Invoke(Message());
	}
}

// END - StringEditView.cpp
