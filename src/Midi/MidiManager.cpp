/* ===================================================================== *
 * MidiManager.cpp (MeV/Midi)
 * ===================================================================== */

#include "MidiManager.h"

#include "Destination.h"
#include "ReconnectingMidiProducer.h"

// Application Kit
#include <Messenger.h>
// Midi Kit
#include <MidiConsumer.h>
// Support Kit
#include <Debug.h>
// Standard C Library
#include <stdio.h>

// Debugging Macros
#define D_ALLOC(x) PRINT(x)	// Constructor/Destructor
#define D_ACCESS(x) PRINT(x)	// Accessors
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

CMidiManager *
CMidiManager::s_instance = NULL;

// ---------------------------------------------------------------------------
// Singleton Access

CMidiManager *
CMidiManager::Instance()
{
	D_ALLOC(("CMidiManager::Instance()\n"));

	if (s_instance == NULL)
	{	
		s_instance = new CMidiManager();
		s_instance->Run(); 			
	}

	return s_instance;
}

// ---------------------------------------------------------------------------
// Hidden Constructor

CMidiManager::CMidiManager()
	:	BLooper("MidiManager"),
		m_roster(NULL),
		m_internalSynth(NULL)
{
	D_ALLOC(("CMidiManager::CMidiManager()\n"));

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

CMidiManager::~CMidiManager()
{
	D_ALLOC(("CMidiManager::~CMidiManager()\n"));

	if (m_internalSynth != NULL)
	{
		m_internalSynth->Unregister();
		m_internalSynth->Release();
	}	
}

// ---------------------------------------------------------------------------
// Hook Functions

void
CMidiManager::DocumentOpened(
	CMeVDoc *document)
{
}

// ---------------------------------------------------------------------------
// Accessors

BMidiProducer *
CMidiManager::GetNextProducer(
	int32 *id) const
{
	D_ACCESS(("CMidiManager::GetNextProducer(%ld)\n", *id));
	ASSERT(m_roster != NULL);

	return (m_roster->NextProducer(id));
}

BMidiProducer *
CMidiManager::FindProducer(
	int32 id)
{
	D_ACCESS(("CMidiManager::FindProducer(%ld)\n", id));
	ASSERT(m_roster != NULL);

	return (m_roster->FindProducer(id));
}

BMidiConsumer *
CMidiManager::GetNextConsumer(
	int32 *id) const
{
	D_ACCESS(("CMidiManager::GetNextConsumer(%ld)\n", *id));
	ASSERT(m_roster != NULL);

	return (m_roster->NextConsumer(id));
}

BMidiConsumer *
CMidiManager::FindConsumer(
	int32 id)
{
	D_ACCESS(("CMidiManager::FindConsumer(%ld)\n", id));
	ASSERT(m_roster != NULL);

	return (m_roster->FindConsumer(id));
}

BMidiEndpoint *
CMidiManager::FindEndpoint(
	const BString &name)
{
	D_ACCESS(("CMidiManager::FindEndpoint(%s)\n", name.String()));
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
CMidiManager::FindProducer(
	const BString &name)
{
	D_ACCESS(("CMidiManager::FindProducer(%s)\n", name.String()));
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
CMidiManager::FindConsumer(
	const BString &name)
{
	D_ACCESS(("CMidiManager::FindConsumer(%s)\n", name.String()));
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
CMidiManager::GetIcon(
	icon_size which,
	BBitmap *outBitmap)
{
	D_ACCESS(("CMidiManager::GetIcon()\n"));

	if (which != B_MINI_ICON)
		return B_ERROR;

	outBitmap->SetBits(MINI_ICON_BITS, 256, 0, B_CMAP8);
	return B_OK;
}

status_t
CMidiManager::GetIconFor(
	BMidiEndpoint *endpoint,
	icon_size which,
	BBitmap *outBitmap)
{
	D_ACCESS(("CMidiManager::GetIconFor(%s)\n", endpoint->Name()));

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
// BLooper Implementation

void
CMidiManager::MessageReceived(
	BMessage *message)
{
	D_MESSAGE(("CMidiManager::MessageReceived()\n"));

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
			BLooper::MessageReceived(message);
		}
	}
}

// ---------------------------------------------------------------------------
// CObservable Implementation

void
CMidiManager::Released(
	CObservable *subject)
{
	if (Lock())
	{
		// do anything necessary to let go of the subject

		// if the subject points to a destination, we'll need to remove it
		// from the multimap and release it

		// if it's a document, just release it

		Unlock();
	}
}

void
CMidiManager::Updated(
	BMessage *message)
{
	PostMessage(message, this);
}

// ---------------------------------------------------------------------------
// Internal Operations

void
CMidiManager::_endpointRegistered(
	int32 id,
	const BString &type)
{
	D_ROSTER(("CMidiManager::_endpointRegistered(%ld, %s)\n",
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
CMidiManager::_endpointUnregistered(
	int32 id,
	const BString &type)
{
	D_ROSTER(("CMidiManager::_endpointUnregistered(%ld, %s)\n",
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
CMidiManager::_endpointConnected(
	int32 producerID,
	int32 consumerID)
{
	D_ROSTER(("CMidiManager::_endpointConnected(%ld, %ld)\n",
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
CMidiManager::_endpointDisconnected(
	int32 producerID,
	int32 consumerID)
{
	D_ROSTER(("CMidiManager::_endpointDisconnected(%ld, %ld)\n",
			  producerID, consumerID));

//	_disconnect(producerID, consumerID);
}

void
CMidiManager::_endpointChangedName(
	int32 id,
	const BString &type,
	const BString &name)
{
}

void
CMidiManager::_endpointChangedLatency(
	int32 id,
	const BString &type,
	bigtime_t latency)
{
}

void
CMidiManager::_endpointChangedProperties(
	int32 id,
	const BString &type,
	const BMessage *message)
{
}

// END - MidiManager.cpp
