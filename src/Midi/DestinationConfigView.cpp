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
		m_destination(destination)
{
	D_ALLOC(("CDestinationConfigView::CDestinationConfigView()\n"));

	MakeExpandable(false);
	MakeSelectable(false);

	Destination()->AddObserver(this);

	BFont font;
	GetFont(&font);
	font_height fh;
	font.GetHeight(&fh);
	BRect labelRect, fieldRect;
	BMenuField *menuField;
	BStringView *stringView;

	// add "Ports" menu
	labelRect = Bounds();
	labelRect.InsetBy(2.0, 2.0);
	labelRect.bottom = labelRect.top + fh.ascent + fh.descent;
	stringView = new BStringView(labelRect, "", "Port:");
	stringView->SetHighColor(tint_color(HighColor(), B_LIGHTEN_1_TINT));
	AddChild(stringView);
	m_portMenu = new CMidiPortsMenu(Destination());
	fieldRect = Bounds();
	fieldRect.InsetBy(2.0, 2.0);
	fieldRect.top = labelRect.bottom + 2.0;
	fieldRect.bottom = fieldRect.top + 2 * (fh.ascent + fh.descent);
	menuField = new BMenuField(fieldRect, "MIDI Port", "", m_portMenu);
	menuField->SetDivider(0.0);
	AddChild(menuField);

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
		if (c == m_destination->Channel())
			item->SetMarked(true);
	}
	fieldRect.OffsetTo(fieldRect.left, fieldRect.bottom + 2.0);
	menuField = new BMenuField(fieldRect, "Channel", "Channel: ",
							   m_channelMenu);
	menuField->SetHighColor(tint_color(HighColor(), B_LIGHTEN_1_TINT));
	menuField->SetDivider(font.StringWidth("Channel:  "));
	AddChild(menuField);
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

	BMessenger messenger(this);
	m_channelMenu->SetTargetForItems(messenger);
	m_portMenu->SetTarget(messenger);
}

void
CDestinationConfigView::GetPreferredSize(
	float *width,
	float *height)
{
	D_HOOK(("CDestinationConfigView::GetPreferredSize()\n"));

	*width = Bounds().Width();
	*height = FindView("Channel")->Frame().bottom + 7.0;
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
