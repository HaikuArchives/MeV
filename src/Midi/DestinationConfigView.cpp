/* ===================================================================== *
 * DestinationConfigView.cpp (MeV/Midi)
 * ===================================================================== */

#include "DestinationConfigView.h"

#include "IconMenuItem.h"
#include "InternalSynth.h"
#include "MidiDestination.h"
#include "MidiModule.h"
#include "MidiPortsMenu.h"

// Interface Kit
#include <Bitmap.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <Region.h>
#include <StringView.h>
#include <TextControl.h>
// Support Kot
#include <Debug.h>

// Debugging Macros
#define D_ALLOC(x) //PRINT(x)		// Constructor/Destructor
#define D_HOOK(x) //PRINT(x)		// BView Implementation
#define D_MESSAGE(x) //PRINT(x)		// MessageReceived()

using namespace Midi;

// ---------------------------------------------------------------------------
// Constructor/Destructor

CDestinationConfigView::CDestinationConfigView(
	BRect frame,
	CMidiDestination *destination)
	:	CConsoleView(frame, "Configuration"),
		m_destination(destination),
		m_portMenuLabel(NULL),
		m_portMenuField(NULL),
		m_portMenu(NULL),
		m_channelMenuField(NULL),
		m_channelMenu(NULL),
		m_consumerIcon(NULL)
{
	D_ALLOC(("CDestinationConfigView::CDestinationConfigView()\n"));

	MakeExpandable(true);
	MakeSelectable(false);

	Destination()->AddObserver(this);

	Expanded(true);
}

CDestinationConfigView::~CDestinationConfigView()
{
	D_ALLOC(("CDestinationConfigView::~CDestinationConfigView()\n"));

	if (m_destination != NULL)
	{
		m_destination->RemoveObserver(this);
		m_destination = NULL;
	}
}

// ---------------------------------------------------------------------------
// CConsoleView Implementation

void
CDestinationConfigView::AttachedToWindow()
{
	D_HOOK(("CDestinationConfigView::AttachedToWindow()\n"));

	CConsoleView::AttachedToWindow();

	if (IsExpanded())
	{
		BMessenger messenger(this);
		m_channelMenu->SetTargetForItems(messenger);
		m_portMenu->SetTarget(messenger);
	}
}

void
CDestinationConfigView::Draw(
	BRect updateRect)
{
	CConsoleView::Draw(updateRect);

	if (!IsExpanded())
	{
		SetDrawingMode(B_OP_OVER);
		BPoint offset(Bounds().LeftTop());
		offset.x = Bounds().Width() / 2.0 - B_MINI_ICON / 2.0;
		offset.y += 5.0;
		if (m_consumerIcon != NULL)
			DrawBitmapAsync(m_consumerIcon, offset);
		offset.y += B_MINI_ICON + 5.0;

		BFont font;
		GetFont(&font);
		font_height fh;
		font.GetHeight(&fh);
		offset.y += fh.ascent;

		BString channelStr;
		channelStr << Destination()->Channel() + 1;
		offset.x = Bounds().Width() / 2.0
				   - font.StringWidth(channelStr.String()) / 2.0;
		SetHighColor(tint_color(HighColor(), B_LIGHTEN_1_TINT));
		DrawString(channelStr.String(), offset);
	}
}

void
CDestinationConfigView::Expanded(
	bool expanded)
{
	if (expanded)
	{
		BFont font;
		GetFont(&font);
		font_height fh;
		font.GetHeight(&fh);
		BRect labelRect, fieldRect;

		if (m_portMenu == NULL)
		{
			// add "Ports" menu
			labelRect = Bounds();
			labelRect.InsetBy(2.0, 2.0);
			labelRect.bottom = labelRect.top + fh.ascent + fh.descent;
			m_portMenuLabel = new BStringView(labelRect, "", "Port:");
			m_portMenuLabel->SetHighColor(tint_color(HighColor(),
													 B_LIGHTEN_1_TINT));
			m_portMenu = new CMidiPortsMenu(Destination());
			fieldRect = Bounds();
			fieldRect.InsetBy(2.0, 2.0);
			fieldRect.top = labelRect.bottom + 2.0;
			fieldRect.bottom = fieldRect.top + 2 * (fh.ascent + fh.descent);
			m_portMenuField = new BMenuField(fieldRect, "MIDI Port", "",
											 m_portMenu);
			m_portMenuField->SetDivider(0.0);
		}
		AddChild(m_portMenuLabel);
		AddChild(m_portMenuField);

		if (m_channelMenu == NULL)
		{
			// add "Channels" menu
			m_channelMenu = new BPopUpMenu("Channel");
			m_channelMenu->SetLabelFromMarked(true);
			for (int32 c = 0; c <= 15; c++)
			{
				BMessage *message = new BMessage(CHANNEL_SELECTED);
				message->AddInt8("value", c);
				BString channelName;
				channelName << (c + 1);
				BMenuItem *item;
				m_channelMenu->AddItem(item = new BMenuItem(channelName.String(),
															 message));
				if (c == Destination()->Channel())
					item->SetMarked(true);
			}
			fieldRect.OffsetTo(fieldRect.left, fieldRect.bottom + 2.0);
			m_channelMenuField = new BMenuField(fieldRect, "Channel", "Channel: ",
									   m_channelMenu);
			m_channelMenuField->SetHighColor(tint_color(HighColor(),
														B_LIGHTEN_1_TINT));
			m_channelMenuField->SetDivider(font.StringWidth("Channel:  "));
		}
		AddChild(m_channelMenuField);
	}
	else
	{
		RemoveChild(m_portMenuLabel);
		RemoveChild(m_portMenuField);
		RemoveChild(m_channelMenuField);

		if (m_consumerIcon != NULL)
			delete m_consumerIcon;

		m_consumerIcon = new BBitmap(BRect(0.0, 0.0, B_MINI_ICON - 1.0,
										   B_MINI_ICON - 1.0), B_CMAP8);
		if (CMidiModule::Instance()->GetIconFor(Destination()->ConnectedTo(),
												B_MINI_ICON,
												m_consumerIcon) != B_OK)
		{
			delete m_consumerIcon;
			m_consumerIcon = NULL;
		}
	}
}

void
CDestinationConfigView::GetPreferredSize(
	float *width,
	float *height)
{
	D_HOOK(("CDestinationConfigView::GetPreferredSize()\n"));

	if (IsExpanded())
	{
		*width = Bounds().Width();
		*height = FindView("Channel")->Frame().bottom + 7.0;
	}
	else
	{
		BFont font;
		GetFont(&font);
		font_height fh;
		font.GetHeight(&fh);

		*width = 10.0 + B_MINI_ICON + 10.0;
		*height = 5.0 + B_MINI_ICON + 5.0 + fh.ascent + fh.descent + 5.0;
	}
}

void
CDestinationConfigView::MessageReceived(
	BMessage *message)
{
	D_MESSAGE(("CDestinationConfigView::MessageReceived()\n"));

	switch (message->what)
	{
		case CMidiPortsMenu::CONSUMER_SELECTED:
		{
			D_MESSAGE((" -> CMidiPortsMenu::CONSUMER_SELECTED:\n"));

			CWriteLock lock(Destination());
			int32 consumerID;
			if (message->FindInt32("consumer", &consumerID) != B_OK)
				return;

			if (consumerID == 0)
			{
				Destination()->Disconnect();
			}
			else
			{
				BMidiConsumer *consumer = CMidiModule::Instance()->FindConsumer(consumerID);
				Destination()->ConnectTo(consumer);
			}
			break;
		}
		case CHANNEL_SELECTED:
		{
			D_MESSAGE((" -> CHANNEL_SELECTED\n"));

			CWriteLock lock(Destination());
			int8 channel;
			if (message->FindInt8("value", &channel) != B_OK)
				return;
			BMenuItem *item;
			if (message->FindPointer("source", (void **)&item) != B_OK)
				return;
			item->SetMarked(true);

			Destination()->SetChannel(static_cast<uint8>(channel));
			break;
		}
		default:
		{
			CConsoleView::MessageReceived(message);
		}
	}
}

bool
CDestinationConfigView::SubjectReleased(
	CObservable *subject)
{
	D_OBSERVE(("CDestinationConfigView::SubjectReleased()\n"));

	if (subject == m_destination)
	{
		Destination()->RemoveObserver(this);
		m_destination = NULL;
		return true;
	}

	return false;
}

void
CDestinationConfigView::SubjectUpdated(
	BMessage *message)
{
	D_OBSERVE(("CDestinationConfigView::SubjectUpdated()\n"));

	int32 destAttrs;
	if (message->FindInt32("DestAttrs", &destAttrs) != B_OK)
		return;
}

// END - DestinationConfigView.cpp
