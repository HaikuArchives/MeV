/* ===================================================================== *
 * MidiModule.cpp (MeV/Midi)
 * ===================================================================== */

#include "MidiModule.h"

#include "InternalSynth.h"
#include "MeVDoc.h"
#include "MidiDestination.h"

// Application Kit
#include <Messenger.h>
// Interface Kit
#include <Bitmap.h>
// Midi Kit
#include <MidiConsumer.h>
#include <MidiProducer.h>
#include <MidiRoster.h>
// Support Kit
#include <Debug.h>
// Standard C Library
#include <stdio.h>

// Debugging Macros
#define D_ALLOC(x) //PRINT(x)	// Constructor/Destructor
#define D_ACCESS(x) //PRINT(x)	// Accessors
#define D_HOOK(x) //PRINT(x)	// CMeVModule Implementation
#define D_MESSAGE(x) //PRINT(x)	// MessageReceived()
#define D_ROSTER(x) //PRINT(x)	// BMidiRoster Interaction

using namespace Midi;

// ---------------------------------------------------------------------------
// Module Icons

const unsigned char MINI_ICON_BITS [] = {
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xff,
	0xff,0x00,0x3f,0x3f,0x3f,0x3f,0x3f,0x3f,0x3f,0x3f,0x3f,0x3f,0x1b,0x00,0xff,0xff,
	0xff,0x00,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x3f,0x3f,0x3f,0x1b,0x00,0x0f,0xff,
	0xff,0x00,0x0f,0x1b,0x0f,0x00,0x00,0x00,0x00,0x0f,0x1b,0x1b,0x1b,0x00,0x0f,0xff,
	0xff,0x00,0x0f,0x0f,0x00,0x00,0x00,0x00,0x00,0x0f,0x00,0x00,0x00,0x0f,0x0f,0xff,
	0xff,0x00,0x0f,0x00,0x00,0x00,0x00,0x00,0x00,0x0f,0x3f,0x3f,0x1b,0x00,0x0f,0xff,
	0xff,0x00,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x3f,0x3f,0x1b,0x00,0x0f,0xff,
	0xff,0x00,0x0f,0x1b,0x0f,0x00,0x00,0x00,0x00,0x0f,0x1b,0x1b,0x1b,0x00,0x0f,0xff,
	0xff,0x00,0x0f,0x0f,0x00,0x00,0x00,0x00,0x00,0x0f,0x00,0x00,0x00,0x0f,0x0f,0xff,
	0xff,0x00,0x0f,0x00,0x00,0x00,0x00,0x00,0x00,0x0f,0x3f,0x3f,0x1b,0x00,0x0f,0xff,
	0xff,0x00,0x3f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x3f,0x3f,0x1b,0x00,0x0f,0xff,
	0xff,0x00,0x1b,0x1b,0x1b,0x1b,0x1b,0x1b,0x1b,0x1b,0x1b,0x1b,0x1b,0x00,0x0f,0xff,
	0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0f,0x0f,0xff,
	0xff,0xff,0xff,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff
};

// ---------------------------------------------------------------------------
// Class Data Initialization

CMidiModule *
CMidiModule::s_instance = NULL;

// ---------------------------------------------------------------------------
// Singleton Access

CMidiModule *
CMidiModule::Instance()
{
	D_ALLOC(("CMidiModule::Instance()\n"));

	if (s_instance == NULL)
	{	
		s_instance = new CMidiModule();
		s_instance->Run(); 			
	}

	return s_instance;
}

// ---------------------------------------------------------------------------
// Hidden Constructor

CMidiModule::CMidiModule()
	:	CMeVModule('MIDI', "MeV MIDI Module"),
		m_roster(NULL),
		m_internalSynth(NULL)
{
	D_ALLOC(("CMidiModule::CMidiModule()\n"));

	m_roster = BMidiRoster::MidiRoster();
	if (m_roster == NULL)
	{
		D_ALLOC((" -> couldn't get BMidiRoster !!\n"));
		return;
	}

	BMessenger messenger(this);
	m_roster->StartWatching(&messenger);

	m_internalSynth = new CInternalSynth();
	m_internalSynth->Register();
}

CMidiModule::~CMidiModule()
{
	D_ALLOC(("CMidiModule::~CMidiModule()\n"));

	if (m_internalSynth != NULL)
	{
		m_internalSynth->Unregister();
		m_internalSynth->Release();
	}	
}

// ---------------------------------------------------------------------------
// Accessors

BMidiProducer *
CMidiModule::GetNextProducer(
	int32 *id) const
{
	D_ACCESS(("CMidiModule::GetNextProducer(%ld)\n", *id));
	ASSERT(m_roster != NULL);

	return (m_roster->NextProducer(id));
}

BMidiProducer *
CMidiModule::FindProducer(
	int32 id)
{
	D_ACCESS(("CMidiModule::FindProducer(%ld)\n", id));
	ASSERT(m_roster != NULL);

	return (m_roster->FindProducer(id));
}

BMidiConsumer *
CMidiModule::GetNextConsumer(
	int32 *id) const
{
	D_ACCESS(("CMidiModule::GetNextConsumer(%ld)\n", *id));
	ASSERT(m_roster != NULL);

	return (m_roster->NextConsumer(id));
}

BMidiConsumer *
CMidiModule::FindConsumer(
	int32 id)
{
	D_ACCESS(("CMidiModule::FindConsumer(%ld)\n", id));
	ASSERT(m_roster != NULL);

	return (m_roster->FindConsumer(id));
}

BMidiEndpoint *
CMidiModule::FindEndpoint(
	const BString &name)
{
	D_ACCESS(("CMidiModule::FindEndpoint(%s)\n", name.String()));
	ASSERT(m_roster != NULL);

	int32 id = 0;
	BMidiEndpoint* bme = m_roster->NextEndpoint(&id);
	while (bme)
	{
		if (name == bme->Name())
			break;

		// Don't forget to decrement the refcount
		// from NextEndpoint!			
		bme->Release();
		bme = m_roster->NextEndpoint(&id);
	}

	return bme;
}

BMidiProducer *
CMidiModule::FindProducer(
	const BString &name)
{
	D_ACCESS(("CMidiModule::FindProducer(%s)\n", name.String()));
	ASSERT(m_roster != NULL);

	int32 id = 0;
	BMidiProducer* bmp = m_roster->NextProducer(&id);
	while (bmp)
	{
		if (name == bmp->Name())
			break;
			
		// Don't forget to decrement the refcount
		// from NextProducer!			
		bmp->Release();
		bmp = m_roster->NextProducer(&id);
	}

	return bmp;
}

BMidiConsumer *
CMidiModule::FindConsumer(
	const BString &name)
{
	D_ACCESS(("CMidiModule::FindConsumer(%s)\n", name.String()));
	ASSERT(m_roster != NULL);

	if (name == "Internal Synth")
		return InternalSynth();

	int32 id = 0;
	BMidiConsumer* bmc = m_roster->NextConsumer(&id);
	while (bmc)
	{
		if (name == bmc->Name())
			break;

		// Don't forget to decrement the refcount
		// from NextConsumer!			
		bmc->Release();
		bmc = m_roster->NextConsumer(&id);
	}

	return bmc;
}

status_t
CMidiModule::GetIconFor(
	BMidiEndpoint *endpoint,
	icon_size which,
	BBitmap *outBitmap)
{
	D_ACCESS(("CMidiModule::GetIconFor(%s)\n", endpoint->Name()));

	BMessage props;
	if (endpoint && (endpoint->GetProperties(&props) != B_OK))
		return B_NAME_NOT_FOUND;

	const void *data;
	ssize_t size;
	status_t error = B_NO_ERROR;
	switch (which)
	{
		case B_LARGE_ICON:
		{
			error = props.FindData("be:large_icon", 'ICON', &data, &size);
			break;
		}
		case B_MINI_ICON:
		{
			error = props.FindData("be:mini_icon", 'MICN', &data, &size);
			// some evil endpoints use 'small_icon' instead :P
			if (error)
				error = props.FindData("be:small_icon", 'MICN', &data, &size);
			break;
		}
	}
	if (!error)
		memcpy(outBitmap->Bits(), data, size);

	return error;
}

// ---------------------------------------------------------------------------
// CMeVModule Implementation

CDestination *
CMidiModule::CreateDestination(
	CMeVDoc *document,
	int32 *id,
	const char *name)
{
	D_HOOK(("CMidiModule::CreateDestination()\n"));

	CDestination *dest;
	if ((id == NULL) || (name == NULL))
		dest = new CMidiDestination(document);
	else
		dest = new CMidiDestination(*id, name, document);
	dest->AddObserver(this);

	return dest;
}

status_t
CMidiModule::GetIcon(
	icon_size which,
	BBitmap *outBitmap)
{
	D_ACCESS(("CMidiModule::GetIcon()\n"));

	if (which != B_MINI_ICON)
		return B_ERROR;

	outBitmap->SetBits(MINI_ICON_BITS, 256, 0, B_CMAP8);
	return B_OK;
}

void
CMidiModule::MessageReceived(
	BMessage *message)
{
	D_MESSAGE(("CMidiModule::MessageReceived()\n"));

	switch (message->what)
	{
		case B_MIDI_EVENT:
		{
			D_MESSAGE((" -> B_MIDI_EVENT\n"));

			int32 op;
			if (message->FindInt32("be:op", &op) != B_OK)
				return;

			int32 id;
			BString type;
			switch (op)
			{
				case B_MIDI_REGISTERED:
				{
					if (message->FindInt32("be:id", &id) != B_OK)
						break;
					if (message->FindString("be:type", &type) != B_OK)
						break;
					_endpointRegistered(id, type);
					break;
				}
				case B_MIDI_UNREGISTERED:
				{
					if (message->FindInt32("be:id", &id) != B_OK)
						break;
					if (message->FindString("be:type", &type) != B_OK)
						break;
					_endpointUnregistered(id, type);
					break;
				}
				case B_MIDI_CONNECTED:
				{
					int32 producerID, consumerID;
					if (message->FindInt32("be:producer", &producerID) != B_OK)
						break;
					if (message->FindInt32("be:consumer", &consumerID) != B_OK)
						break;
					_endpointConnected(producerID, consumerID);
					break;
				}
				case B_MIDI_DISCONNECTED:
				{
					int32 producerID, consumerID;
					if (message->FindInt32("be:producer", &producerID) != B_OK)
						break;
					if (message->FindInt32("be:consumer", &consumerID) != B_OK)
						break;
					_endpointDisconnected(producerID, consumerID);
					break;
				}
				case B_MIDI_CHANGED_NAME:
				{
					BString name;
					if (message->FindInt32("be:id", &id) != B_OK)
						break;
					if (message->FindString("be:type", &type) != B_OK)
						break;
					if (message->FindString("be:name", &name) != B_OK)
						break;
					_endpointChangedName(id, type, name);
					break;
				}
				case B_MIDI_CHANGED_LATENCY:
				{
					bigtime_t latency;
					if (message->FindInt32("be:id", &id) != B_OK)
						break;
					if (message->FindString("be:type", &type) != B_OK)
						break;
					if (message->FindInt64("be:latency", &latency) != B_OK)
						break;
					_endpointChangedLatency(id, type, latency);
					break;
				}
				case B_MIDI_CHANGED_PROPERTIES:
				{
					BMessage props;
					if (message->FindInt32("be:id", &id) != B_OK)
						break;
					if (message->FindString("be:type", &type) != B_OK)
						break;
					if (message->FindMessage("be:properties", &props) != B_OK)
						break;
					_endpointChangedProperties(id, type, &props);
					break;
				}
			}
			break;
		}
		default:
		{
			CMeVModule::MessageReceived(message);
		}
	}
}

bool
CMidiModule::SubjectReleased(
	CObservable *subject)
{
	D_HOOK(("CMidiModule::SubjectReleased()\n"));	

	return false;
}

void
CMidiModule::SubjectUpdated(
	BMessage *message)
{
	D_HOOK(("CMidiModule::SubjectUpdated()\n"));	
}

// ---------------------------------------------------------------------------
// Internal Operations

void
CMidiModule::_endpointRegistered(
	int32 id,
	const BString &type)
{
	D_ROSTER(("CMidiModule::_endpointRegistered(%ld, %s)\n",
			  id, type.String()));

	if (type == "producer")
	{
//		_addProducer(id);
	}
	else if (type == "consumer")
	{
//		_addConsumer(id);
	}
}

void
CMidiModule::_endpointUnregistered(
	int32 id,
	const BString &type)
{
	D_ROSTER(("CMidiModule::_endpointUnregistered(%ld, %s)\n",
			  id, type.String()));

	if (type == "producer")
	{
//		_removeProducer(id);
	}
	else if (type == "consumer")
	{
//		_removeConsumer(id);
	}
	
}

void
CMidiModule::_endpointConnected(
	int32 producerID,
	int32 consumerID)
{
	D_ROSTER(("CMidiModule::_endpointConnected(%ld, %ld)\n",
			  producerID, consumerID));

	BMidiConsumer *consumer = FindConsumer(consumerID);
	if (consumer)
	{
		BMessage props;
		if ((consumer->GetProperties(&props) == B_OK)
		 && (props.HasBool("mev:internal_synth")))
		{
			D_ROSTER((" -> init internal synth!\n"));
			m_internalSynth->Init();
		}
	}

//	_connect(producerID, consumerID);
}

void
CMidiModule::_endpointDisconnected(
	int32 producerID,
	int32 consumerID)
{
	D_ROSTER(("CMidiModule::_endpointDisconnected(%ld, %ld)\n",
			  producerID, consumerID));

//	_disconnect(producerID, consumerID);
}

void
CMidiModule::_endpointChangedName(
	int32 id,
	const BString &type,
	const BString &name)
{
}

void
CMidiModule::_endpointChangedLatency(
	int32 id,
	const BString &type,
	bigtime_t latency)
{
}

void
CMidiModule::_endpointChangedProperties(
	int32 id,
	const BString &type,
	const BMessage *message)
{
}

// END - MidiModule.cpp
