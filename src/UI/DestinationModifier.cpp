/* ===================================================================== *
 * DestinationModifier.cpp (MeV/UI)
 * ===================================================================== */

#include "DestinationModifier.h"

#include "Destination.h"
#include "MeVDoc.h"
#include "DestinationListView.h"
#include "MidiManager.h"
#include "IconMenuItem.h"

#include <stdio.h>
// Interface Kit
#include <MenuField.h>
// Midi Kit
#include <MidiConsumer.h>

// ---------------------------------------------------------------------------
// Constructor/Destructor

CDestinationModifier::CDestinationModifier(
	BRect frame,
	int32 id,
	CMeVDoc *doc,
	BHandler *parent)
	:	BWindow(frame, "CDestination Modifier", B_TITLED_WINDOW,
				B_NOT_ZOOMABLE | B_NOT_RESIZABLE),
		CObserver(*this, doc),
		m_dest(doc->FindDestination(id)),
		m_id(id),
		m_midiManager(CMidiManager::Instance()),
		m_parent(parent)
{
	BString title;
	title << "Destination: ";
	title << m_dest->Name();
	SetTitle(title.String());

	_buildUI();
}

// ---------------------------------------------------------------------------
// Operations

void
CDestinationModifier::Update()
{

}

// ---------------------------------------------------------------------------
// BWindow Implementation

void
CDestinationModifier::MenusBeginning()
{
	int i = m_midiPorts->CountItems();
	while (i >= 0)
	{
		BMenuItem *item = (m_midiPorts->RemoveItem(i));
		if (item)
			delete item;
		i--;
	}
	_populatePortsMenu();
}

void
CDestinationModifier::MenusEnded()
{
	int i = m_midiPorts->CountItems();
	while (i >= 0)
	{
		BMenuItem *item = (m_midiPorts->RemoveItem(i));
		if (item)
			delete item;
		i--;
	}
}

void
CDestinationModifier::MessageReceived(
	BMessage *message)
{
	switch (message->what)
	{
		case NAME_CHANGED:
		{
			BString name;
			name << m_name->Text();
			m_dest->SetName(name.String());
			BString title;
			title << "Destination: ";
			title << name;
			SetTitle(title.String());
			break;
		}
		case CHANNEL_SELECTED:
		{
			m_dest->SetChannel(message->FindInt8("value"));	
			break;
		}
		case PORT_SELECTED:
		{
			int32 portID;
			if (message->FindInt32("port_id", &portID) != B_OK)
				return;
			m_dest->SetConnect(m_midiManager->FindConsumer(portID),1);
			break;
		}
		case MUTED:
		{
			m_dest->SetMuted(m_muted->Value());
			break;
		}
		case COLOR_CHANGED:
		{
			m_dest->SetColor(m_colors->ValueAsColor());
			break;
		}
		case Update_ID:
		case Delete_ID:
		{
			CObserver::MessageReceived(message);
			break;
		}
		default:
		{
			BWindow::MessageReceived(message);
		}
	}
}

bool 
CDestinationModifier::QuitRequested()
{
	BMessage *msg = new BMessage(WINDOW_CLOSED);
	msg->AddInt32("destination_id", m_id);
	BMessenger *amsgr = new BMessenger(m_parent);
	amsgr->SendMessage(msg);

	return false;
}

// ---------------------------------------------------------------------------
// CObserver Implementation

void
CDestinationModifier::OnUpdate(BMessage *msg)
{
	//i don't really know why this would happen.
}

// ---------------------------------------------------------------------------
// Internal Operations

void
CDestinationModifier::_buildUI()
{
	BMenuField *menuField;
	float maxLabelWidth = be_plain_font->StringWidth("Channel:  ");

	BRect rect(Bounds());
	m_background = new BView(rect, "", B_FOLLOW_ALL_SIDES, B_WILL_DRAW);
	m_background->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	AddChild(m_background);

	font_height fh;
	be_plain_font->GetHeight(&fh);

	// add "Name" text-control
	rect.InsetBy(5.0, 5.0);
	rect.bottom = rect.top + fh.ascent + fh.descent + fh.leading + 6.0;
	m_name = new BTextControl(rect, "Name", "Name:", "",
							  new BMessage(NAME_CHANGED));
	m_name->SetDivider(maxLabelWidth);
	m_name->SetText(m_dest->Name());
	m_background->AddChild(m_name);

	// add "Ports" menu
	rect.OffsetBy(0.0, rect.Height() + 10.0);
	m_midiPorts = new BPopUpMenu("Ports");
	m_midiPorts->SetLabelFromMarked(true);
	menuField = new BMenuField(rect, "Ports", "Ports:", m_midiPorts);
	menuField->SetDivider(maxLabelWidth);
	m_background->AddChild(menuField);
	
	// add "Channels" menu
	rect.OffsetBy(0.0, rect.Height() + 5.0);
	rect.right = Bounds().Width() / 2.0;
	m_channels = new BPopUpMenu("Channel");
	for (int c = 0;c <= 15; c++)
	{
		BMessage *msg = new BMessage(CHANNEL_SELECTED);
		msg->AddInt8("value", c);
		BString cname;
		cname << (c + 1);
		BMenuItem *item = new BMenuItem(cname.String(), msg);
		m_channels->AddItem(item);
	}
	(m_channels->ItemAt((m_dest->Channel())))->SetMarked(true);
	menuField = new BMenuField(rect, "Channel", "Channel:", m_channels);
	menuField->SetDivider(maxLabelWidth);
	m_background->AddChild(menuField);

	// add "Mute" check-box
	rect.left = rect.right;
	rect.right = Bounds().Width() * 0.75;
	m_muted = new BCheckBox(rect, "Mute", "Mute", new BMessage(MUTED));
	m_background->AddChild(m_muted);
	if (m_dest->Muted())
	{
		m_muted->SetEnabled(false);
	}
	// add "Solo" check-box
	rect.left = rect.right;
	rect.right = Bounds().Width() - 5.0;
	m_solo = new BCheckBox(rect, "Solo", "Solo", new BMessage(SOLOED));
	m_solo->SetEnabled(false);
	m_background->AddChild(m_solo);

	// add CDestination color-control
	rect.left = Bounds().left + 5.0;
	rect.top = rect.bottom + 15.0;
	rect.bottom = Bounds().bottom - 5.0;
	m_colors = new BColorControl(rect.LeftTop(), B_CELLS_32x8, 4.0,
								 "Destination Color",
								 new BMessage(COLOR_CHANGED));
	m_colors->SetValue(m_dest->GetFillColor());
	m_background->AddChild(m_colors);

	// resize the window to fit the color control
	if (m_colors->Frame().right > (Bounds().right - 5.0))
	{
		ResizeBy(m_colors->Frame().right - (Bounds().right - 5.0),
				 0.0);
	}

	if (m_colors->Frame().bottom < (Bounds().bottom - 5.0))
	{
		ResizeBy(0.0, m_colors->Frame().bottom - (Bounds().bottom - 5.0));
	}
	_populatePortsMenu();
}

void
CDestinationModifier::_populatePortsMenu()
{
	BMidiConsumer *con = NULL;

	int32 id = 0;
	while ((con = m_midiManager->NextConsumer(&id)) != NULL)
	{
		if (con->IsValid())
		{
			BMessage *msg = new BMessage(PORT_SELECTED);
			msg->AddInt32("port_id", id);
			BBitmap *icon = m_midiManager->ConsumerIcon(id, B_MINI_ICON);
			CIconMenuItem *item = new CIconMenuItem(con->Name(), msg, icon);
			m_midiPorts->AddItem(item);
			if (m_dest->IsConnected(con))
			{
				item->SetMarked(true);
			}
			con->Release();
		}
	}

	CInternalSynth *internalSynth = m_midiManager->InternalSynth();
	BMessage *msg = new BMessage(PORT_SELECTED);
	msg->AddInt32("port_id", internalSynth->ID());
	CIconMenuItem *item = new CIconMenuItem(internalSynth->Name(), msg,
											m_midiManager->ConsumerIcon(internalSynth->ID(),
											B_MINI_ICON));
	m_midiPorts->AddItem(item);
	if (m_dest->GetProducer()->IsConnected(internalSynth))
	{
		item->SetMarked(true);
	}
}

void
CDestinationModifier::_updateStatus()
{
}

// END - DestinationModifier.cpp
