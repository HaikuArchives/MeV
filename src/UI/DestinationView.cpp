/* ===================================================================== *
 * DestinationView.cpp (MeV/UI)
 * ===================================================================== */

#include "DestinationView.h"

#include "BorderView.h"
#include "ColorDialogWindow.h"
#include "ColorWell.h"
#include "ConsoleView.h"
#include "Destination.h"
#include "Idents.h"
#include "MeVDoc.h"
#include "StringEditView.h"

// Interface Kit
#include <Bitmap.h>
#include <CheckBox.h>
#include <ColorControl.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <StringView.h>
#include <TextControl.h>
#include <Window.h>
// Support Kit
#include <Debug.h>

#define D_ALLOC(x) //PRINT(x)	// Constructor/Destructor
#define D_HOOK(x) //PRINT(x)	// BView Implementation
#define D_MESSAGE(x) //PRINT(x)	// MessageReceived()

// ---------------------------------------------------------------------------
// Constructor/Destructor

CDestinationView::CDestinationView(
	BRect frame,
	CDestination *destination)
	:	CConsoleView(frame, destination->Name()),
		m_destination(destination),
		m_editingName(false),
		m_icon(NULL),
		m_iconOffset(2.0, 2.0),
		m_mutedCheckBox(NULL),
		m_soloCheckBox(NULL),
		m_configView(NULL),
		m_monitorView(NULL),
		m_contextMenu(NULL)
{
	MakeExpandable(false);
	MakeSelectable(false);
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	SetFont(be_plain_font);

	CWriteLock lock(Destination());
	Destination()->AddObserver(this);

	BFont font;
	GetFont(&font);
	font_height fh;
	font.GetHeight(&fh);
	float controlOffset = font.StringWidth("Latency:   ");

	_updateIcon();
	_updateName();

	// add "Mute" check-box
	BRect rect(Bounds());
	rect.left = controlOffset + 2.0;
	rect.top = m_nameFrame.bottom + 8.0;
	rect.right -= 2.0;
	rect.bottom = rect.top + fh.ascent + fh.descent + fh.leading;
	m_mutedCheckBox = new BCheckBox(rect, "Mute", "Mute", new BMessage(MUTED));
	AddChild(m_mutedCheckBox);
	if (Destination()->IsMuted())
		m_mutedCheckBox->SetValue(B_CONTROL_ON);
	m_mutedCheckBox->ResizeToPreferred();

	// add "Solo" check-box
	rect.OffsetBy(0.0, rect.Height() + 5.0);
	m_soloCheckBox = new BCheckBox(rect, "Solo", "Solo", new BMessage(SOLO));
	m_soloCheckBox->SetEnabled(false);
	AddChild(m_soloCheckBox);
	if (Destination()->IsSolo())
		m_soloCheckBox->SetValue(B_CONTROL_ON);
	m_soloCheckBox->ResizeToPreferred();

	// add color-well
	BRect wellRect(rect);
	wellRect.left = 5.0;
	wellRect.right = controlOffset - 5.0;
	wellRect.top = m_nameFrame.bottom + 8.0;
	wellRect.bottom = wellRect.top + wellRect.Width();
	wellRect.InsetBy(5.0, 5.0);
	m_colorWell = new CColorWell(wellRect, new BMessage(COLOR_CHANGED));
	m_colorWell->SetColor(Destination()->Color());
	AddChild(m_colorWell);

	// add "Latency" text field
	rect.OffsetBy(0.0, rect.Height() + 10.0);
	rect.left = Bounds().left + 2.0;
	rect.right -= font.StringWidth("  ms  ");
	m_latencyControl = new BTextControl(rect, "Latency", "Latency:", "",
										new BMessage(LATENCY_CHANGED));
	m_latencyControl->SetDivider(controlOffset - 2.0);
	m_latencyControl->SetAlignment(B_ALIGN_RIGHT, B_ALIGN_RIGHT);
	AddChild(m_latencyControl);
	rect.OffsetTo(rect.right + 2.0,
				  rect.top + 5.0 + rect.Height() / 2.0 - fh.ascent / 2.0);
	rect.right = Bounds().right - 2.0;
	m_msLabel = new BStringView(rect, "", " ms ");
	AddChild(m_msLabel);

	// add configuration view
	rect.top = rect.bottom + 12.0;
	rect.left = Bounds().left + 4.0;
	rect.right = Bounds().right - 4.0;
	rect.bottom = Bounds().bottom - 4.0;
	m_configView = Destination()->MakeConfigurationView(rect);
	AddChild(m_configView);
	m_configView->ResizeToPreferred();
	rect = m_configView->Frame();

	_updateLatency();
}

CDestinationView::~CDestinationView()
{
	if (Destination() != NULL)
		Destination()->RemoveObserver(this);

	delete m_icon;
}

// ---------------------------------------------------------------------------
// CConsoleView Implementation

void
CDestinationView::AttachedToWindow()
{
	m_colorWell->SetTarget(this);
	m_mutedCheckBox->SetTarget(this);
	m_soloCheckBox->SetTarget(this);
	m_latencyControl->SetTarget(this);
}

void
CDestinationView::Draw(
	BRect updateRect)
{
	D_HOOK(("CDestinationView::Draw()\n"));

	CConsoleView::Draw(updateRect);

	if (m_icon != NULL)
	{
		SetDrawingMode(B_OP_OVER);
		DrawBitmapAsync(m_icon, m_iconOffset);
	}

	if (!m_editingName)
	{
		SetDrawingMode(B_OP_OVER);
		font_height fh;
		GetFontHeight(&fh);
		DrawString(m_truncatedName.String(),
				   m_nameFrame.LeftBottom() - BPoint(0.0, fh.descent));
	}
}

void
CDestinationView::GetPreferredSize(
	float *width,
	float *height)
{
	D_HOOK(("CDestinationView::GetPreferredSize()\n"));

	*width = m_configView->Frame().right + 5.0;
	*height = m_configView->Frame().bottom + 5.0;
}

void
CDestinationView::MessageReceived(
	BMessage *message)
{
	D_MESSAGE(("CDestinationView::MessageReceived()\n"));

	switch (message->what)
	{
		case RENAME:
		{
			D_MESSAGE((" -> RENAME\n"));

			BMessage *message = new BMessage(NAME_CHANGED);
			BMessenger messenger(this, Window());
			CStringEditView *view = new CStringEditView(m_nameFrame, m_destination->Name(),
														message, messenger);
			AddChild(view);
		
			m_editingName = true;
			break;
		}
		case NAME_CHANGED:
		{
			D_MESSAGE((" -> NAME_CHANGED\n"));
			
			CWriteLock lock(Destination());
			m_editingName = false;
			BString newName;
			if (message->FindString("text", &newName) != B_OK)
				return;
			Destination()->SetName(newName.String());
			break;
		}
		case MUTED:
		{
			D_MESSAGE((" -> MUTED\n"));

			CWriteLock lock(Destination());
			Destination()->SetMuted(m_mutedCheckBox->Value());
			break;
		}
		case SOLO:
		{
			D_MESSAGE((" -> SOLO\n"));

			CWriteLock lock(Destination());
			Destination()->SetSolo(m_soloCheckBox->Value());
			break;
		}
		case LATENCY_CHANGED:
		{
			D_MESSAGE((" -> LATENCY_CHANGED\n"));

			CWriteLock lock(Destination());
			float latency;
			if ((sscanf(m_latencyControl->Text(), "%f", &latency) == 1)
			 || (latency >= 0.0f))
				Destination()->SetLatency(static_cast<bigtime_t>(latency) * 1000);
			_updateLatency();
			m_latencyControl->MakeFocus(false);
			break;
		}
		case COLOR_CHANGED:
		{
			D_MESSAGE((" -> COLOR_CHANGED\n"));

			CWriteLock lock(Destination());
			const void *data;
			ssize_t size;
			if (message->FindData("color", B_RGB_COLOR_TYPE, &data, &size) != B_OK)
				return;
			const rgb_color *color = (const rgb_color *)data;
			Destination()->SetColor(*color);
			m_colorWell->SetColor(*color);
			m_colorWell->Invalidate();
			break;
		}
		case CHANGE_COLOR:
		{
			D_MESSAGE((" -> CHANGE_COLOR\n"));

			CReadLock lock(Destination());
			BRect rect(100, 100, 300, 200);
			CColorDialogWindow *window;
			window = new CColorDialogWindow(rect, Destination()->Color(),
											new BMessage(COLOR_CHANGED),
											this, Window());
			window->Show();
			break;
		}
		default:
		{
			CConsoleView::MessageReceived(message);
		}
	}
}

void
CDestinationView::MouseDown(
	BPoint point)
{
	D_HOOK(("CDestinationView::MouseDown()\n"));

	int32 buttons = B_PRIMARY_MOUSE_BUTTON;
	Window()->CurrentMessage()->FindInt32("buttons", &buttons);

	if ((buttons == B_PRIMARY_MOUSE_BUTTON) && m_nameFrame.Contains(point))
		Window()->PostMessage(RENAME, this);
	else if (buttons == B_SECONDARY_MOUSE_BUTTON)
		_showContextMenu(point);
	else
		CConsoleView::MouseDown(point);
}

bool
CDestinationView::SubjectReleased(
	CObservable *subject)
{
	D_HOOK(("CDestinationView::SubjectReleased()\n"));

	if (subject == m_destination)
	{
		m_destination->RemoveObserver(this);
		m_destination = NULL;
		return true;
	}

	return false;
}

void
CDestinationView::SubjectUpdated(
	BMessage *message)
{
	D_HOOK(("CDestinationView::SubjectUpdated()\n"));

	int32 destAttrs;
	if (message->FindInt32("DestAttrs", &destAttrs) != B_OK)
		return;
	if (destAttrs & CDestination::Update_Latency)
		_updateLatency();
	if (destAttrs & CDestination::Update_Name)
		_updateName();
	if (destAttrs & (CDestination::Update_Flags | CDestination::Update_Color))
		_updateIcon();
	Invalidate();
}

// ---------------------------------------------------------------------------
// Internal Operations

void
CDestinationView::_showContextMenu(
	BPoint point)
{
	if (m_contextMenu == NULL)
	{
		m_contextMenu = new BPopUpMenu("", false, false);
		m_contextMenu->SetFont(be_plain_font);

		BMenuItem *item;

		item = new BMenuItem("Collapse", NULL);
		item->SetEnabled(false);
		m_contextMenu->AddItem(item);
		item = new BMenuItem("Expand", NULL);
		item->SetEnabled(false);
		m_contextMenu->AddItem(item);
		m_contextMenu->AddSeparatorItem();

		item = new BMenuItem("Rename", new BMessage(RENAME));
		m_contextMenu->AddItem(item);
		item = new BMenuItem("Change Color", new BMessage(CHANGE_COLOR));
		m_contextMenu->AddItem(item);
	}

	m_contextMenu->SetTargetForItems(this);
	ConvertToScreen(&point);
	point -= BPoint(1.0, 1.0);
	m_contextMenu->Go(point, true, true, true);
}

void
CDestinationView::_updateIcon()
{
	CReadLock lock(Destination());

	BRect rect(0.0, 0.0, B_MINI_ICON - 1.0, B_MINI_ICON - 1.0);
	if (m_icon == NULL)
		m_icon = new BBitmap(rect, B_CMAP8);
	Destination()->GetIcon(B_MINI_ICON, m_icon);
	rect.OffsetTo(m_iconOffset);
	Invalidate(rect);
}

void
CDestinationView::_updateLatency()
{
	CReadLock lock(Destination());

	bigtime_t latency = Destination()->Latency();
	float msLatency = static_cast<float>(latency / 1000.0);
	char latencyStr[16];
	snprintf(latencyStr, 16, "%.3f", msLatency);
	m_latencyControl->SetText(latencyStr);
}

void
CDestinationView::_updateName()
{
	CReadLock lock(Destination());

	BFont font;
	GetFont(&font);
	font_height fh;
	font.GetHeight(&fh);

	// calculate name rect and truncate name if necessary
	m_nameFrame.left = m_iconOffset.x + B_MINI_ICON + 5.0;
	m_nameFrame.right = Bounds().right - 2.0;
	m_nameFrame.top = m_iconOffset.y + B_MINI_ICON / 2.0
					  - (fh.ascent + fh.descent) / 2.0;
	m_nameFrame.bottom = m_nameFrame.top + fh.ascent + fh.descent;
	m_truncatedName = Destination()->Name();
	font.TruncateString(&m_truncatedName, B_TRUNCATE_MIDDLE, m_nameFrame.Width());
	m_nameFrame.right = m_nameFrame.left
						+ font.StringWidth(m_truncatedName.String());
}

// END - DestinationView.cpp
