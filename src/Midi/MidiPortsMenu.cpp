/* ===================================================================== *
 * MidiPortsMenu.cpp (MeV/Midi)
 * ===================================================================== */

#include "MidiPortsMenu.h"

#include "IconMenuItem.h"
#include "InternalSynth.h"
#include "MidiDestination.h"
#include "MidiModule.h"

// Interface Kit
#include <Bitmap.h>
#include <MenuItem.h>
// Midi Kit
#include <MidiConsumer.h>
#include <MidiProducer.h>
// Support Kot
#include <Debug.h>

// Debugging Macros
#define D_ALLOC(x) //PRINT(x)		// Constructor/Destructor
#define D_HOOK(x) //PRINT(x)		// BView Implementation
#define D_INTERNAL(x) PRINT(x)	// Internal Operations

using namespace Midi;

// ---------------------------------------------------------------------------
// Constructor/Destructor

CMidiPortsMenu::CMidiPortsMenu(
	CMidiDestination *destination)
	:	BPopUpMenu("(None)"),
		m_destination(destination)
{
	D_ALLOC(("CMidiPortsMenu::CMidiPortsMenu()\n"));

	SetFont(be_plain_font);
	_update();
}

CMidiPortsMenu::~CMidiPortsMenu()
{
	D_ALLOC(("CMidiPortsMenu::~CMidiPortsMenu()\n"));
}

// ---------------------------------------------------------------------------
// BPopUpMenu Implementation

void
CMidiPortsMenu::AttachedToWindow()
{
	D_HOOK(("CMidiPortsMenu::AttachedToWindow()\n"));

	_update();

	BPopUpMenu::AttachedToWindow();
}

// ---------------------------------------------------------------------------
// Internal Operations

void
CMidiPortsMenu::_update()
{
	D_INTERNAL(("CMidiPortsMenu::_update()\n"));
	ASSERT(m_destination != NULL);

	int32 oldCount = CountItems();

	if (m_destination != NULL)
	{
		BMidiConsumer *consumer = NULL;
		BMenuItem *item;

		BMessage *message = new BMessage(CONSUMER_SELECTED);
		message->AddInt32("consumer", 0);
		AddItem(item = new BMenuItem("(None)", message));
		item->SetMarked(true);
		AddSeparatorItem();

		// Add the internal Synth
		consumer = CMidiModule::Instance()->InternalSynth();
		message = new BMessage(*message);
		message->ReplaceInt32("consumer", consumer->ID());
		BBitmap *icon = new BBitmap(BRect(0.0, 0.0, B_MINI_ICON - 1.0,
										  B_MINI_ICON - 1.0), B_CMAP8);
		if (CMidiModule::Instance()->GetIconFor(consumer, B_MINI_ICON,
												 icon) != B_OK)
		{
			delete icon;
			icon = NULL;
		}
		AddItem(item = new CIconMenuItem(consumer->Name(), message,
													  icon));
		if (m_destination->IsConnectedTo(consumer))
			item->SetMarked(true);
	
		int32 id = 0;
		while ((consumer = CMidiModule::Instance()->GetNextConsumer(&id)) != NULL)
		{
			if (consumer->IsValid())
			{
				message = new BMessage(*message);
				message->ReplaceInt32("consumer", consumer->ID());
				BBitmap *icon = new BBitmap(BRect(0.0, 0.0, B_MINI_ICON - 1.0,
												  B_MINI_ICON - 1.0), B_CMAP8);
				if (CMidiModule::Instance()->GetIconFor(consumer, B_MINI_ICON, icon) != B_OK)
				{
					delete icon;
					icon = NULL;
				}
				AddItem(item = new CIconMenuItem(consumer->Name(),
												 message, icon));
				if (m_destination->IsConnectedTo(consumer))
					item->SetMarked(true);
				consumer->Release();
			}
		}
	}

	if (oldCount > 0)
		RemoveItems(0, oldCount, true);

	SetTargetForItems(m_target);
}

// END - MidiPortsMenu.cpp
