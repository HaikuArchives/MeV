/* ===================================================================== *
 * ColorDialogWindow.cpp (MeV/UI)
 * ===================================================================== */

#include "ColorDialogWindow.h"

// Interface
#include <Button.h>
#include <ColorControl.h>
#include <View.h>
// Support Kit
#include <Debug.h>

// Debugging Macros
#define D_ALLOC(x) //PRINT(x)		// Constructor/Destructor
#define D_HOOK(x) //PRINT(x)		// BWindow/CObserver Implementation
#define D_MESSAGE(x) //PRINT(x)		// MessageReceived()

// ---------------------------------------------------------------------------
// Constructor/Destructor

CColorDialogWindow::CColorDialogWindow(
	BRect frame,
	rgb_color color,
	BMessage *message,
	const BMessenger &target,
	BWindow *parent)
	:	CDialogWindow(frame, parent),
		m_colorControl(NULL),
		m_initialColor(color),
		m_message(message),
		m_target(target)
{
	D_ALLOC(("CColorDialogWindow::CColorDialogWindow()\n"));

	BButton *button = AddButton("Close", new BMessage(B_QUIT_REQUESTED));
	button->MakeDefault(true);
	AddButton("Revert", new BMessage(REVERT_REQUESTED));

	BRect rect(ContentFrame());
	rect.InsetBy(5.0, 5.0);
	m_colorControl = new BColorControl(rect.LeftTop(), B_CELLS_32x8, 4.0,
									   "", new BMessage(VALUE_CHANGED));
	m_colorControl->SetValue(color);
	BackgroundView()->AddChild(m_colorControl);

	float width, height;
	m_colorControl->GetPreferredSize(&width, &height);
	width += 20.0;
	height += Bounds().Height() - rect.Height();
	ResizeTo(width, height);
}

CColorDialogWindow::~CColorDialogWindow()
{
	D_ALLOC(("CColorDialogWindow::~CColorDialogWindow()\n"));

	delete m_message;
}

// ---------------------------------------------------------------------------
// BWindow Implementation

void
CColorDialogWindow::MessageReceived(
	BMessage *message)
{
	D_MESSAGE(("CColorDialogWindow::MessageReceived()\n"));

	switch (message->what)
	{
		case REVERT_REQUESTED:
		{
			D_MESSAGE((" -> REVERT_REQUESTED\n"));

			m_colorControl->SetValue(m_initialColor);
			PostMessage(VALUE_CHANGED);
			break;
		}
		case VALUE_CHANGED:
		{
			D_MESSAGE((" -> VALUE_CHANGED\n"));

			rgb_color color = m_colorControl->ValueAsColor();
			if (m_message->ReplaceData("color", B_RGB_COLOR_TYPE, &color,
									   sizeof(color)) != B_OK)
				m_message->AddData("color", B_RGB_COLOR_TYPE, &color,
								   sizeof(color));
			m_target.SendMessage(m_message);
			break;
		}
		default:
		{
			BWindow::MessageReceived(message);
		}
	}
}

// END - ColorDialogWindow.cpp
