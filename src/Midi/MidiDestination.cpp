/* ===================================================================== *
 * MidiDestination.cpp (MeV/Midi)
 * ===================================================================== */

#include "MidiDestination.h"

#include "GeneralMidi.h"
#include "InternalSynth.h"
#include "MeVDoc.h"
#include "MidiManager.h"
#include "ReconnectingMidiProducer.h"

// Interface Kit
#include <Rect.h>
#include <Bitmap.h>
#include <View.h>
// Midi Kit
#include <MidiConsumer.h>
// Support Kit
#include <Debug.h>

#define D_ALLOC(x) //PRINT(x)		// Constructor/Destructor
#define D_HOOK(x) //PRINT(x)		// Hook Functions
#define D_ACCESS(x) //PRINT(x)		// Accessors
#define D_OPERATION(x) // PRINT(x)	// Operations
#define D_SERIALIZE(x) //PRINT(x)	// Serialization

using namespace Midi;

// ---------------------------------------------------------------------------
// Constants

const char *NOTE_NAMES[] =
{ "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

// ---------------------------------------------------------------------------
// Constructor/Destructor

CMidiDestination::CMidiDestination(
	long id,
	const char *name,
	CMeVDoc *document)
	:	CDestination('MIDI', id, name, document),
		m_producer(NULL),
		m_consumerID(0),
		m_channel(0),
		m_generalMidi(false)
{
	D_ALLOC(("CMidiDestination::CMidiDestination()\n"));

	m_producer = new Midi::CReconnectingMidiProducer(Name());
	_updateIcons();
	m_producer->Register();
}

CMidiDestination::CMidiDestination(
	CMeVDoc *document)
	:	CDestination('MIDI', document),
		m_producer(NULL),
		m_consumerID(0),
		m_channel(0),
		m_generalMidi(false)
{
	D_ALLOC(("CMidiDestination::CMidiDestination(deserialize)\n"));

	m_producer = new Midi::CReconnectingMidiProducer("");
	_updateIcons();
	m_producer->Register();
}

CMidiDestination::~CMidiDestination()
{
	D_ALLOC(("CMidiDestination::~CMidiDestination()\n"));

	m_producer->Release();
}

// ---------------------------------------------------------------------------
// Accessors

bool
CMidiDestination::GetNoteName(
	unsigned char note,
	char *outName)
{
	if (m_generalMidi && (m_channel == GeneralMidi::DRUM_KIT_CHANNEL))
	{
		strncpy(outName, GeneralMidi::GetDrumSoundNameFor(note),
				NOTE_NAME_LENGTH);
		return true;
	}
	else
	{
		unsigned char octave = note / 12;
		unsigned char key = note % 12;
		snprintf(outName, NOTE_NAME_LENGTH, "%s%d (%d)",
				 NOTE_NAMES[key], octave - 2, note);
		return true;
	}

	return false;
}

bool
CMidiDestination::GetProgramName(
	unsigned short bank,
	unsigned char program,
	char *outName)
{
	if (m_generalMidi)
	{
		if (m_channel == GeneralMidi::DRUM_KIT_CHANNEL)
			snprintf(outName, PROGRAM_NAME_LENGTH, "Drum Kit");
		else
			strncpy(outName, GeneralMidi::GetProgramNameFor(program),
					PROGRAM_NAME_LENGTH);
		return true;
	}

	return false;
}

void
CMidiDestination::SetChannel(
	uint8 channel)
{
	if (channel != m_channel)
	{	
		m_channel = channel;
		Document()->SetModified();

		CUpdateHint hint;
		hint.AddInt32("DestID", ID());
		hint.AddInt32("DestAttrs", Update_Channel);
		PostUpdate(&hint);
	}
}

void
CMidiDestination::SetConnect(
	BMidiConsumer *sink,
	bool connect)
{
	D_ACCESS(("CMidiDestination::SetConnect(%s)\n", sink ? sink->Name() : "(none)"));

	if (sink)
	{
		Midi::CMidiManager *mm = Midi::CMidiManager::Instance();
		if (m_producer->IsConnected(mm->FindConsumer(m_consumerID)))
		{
			m_producer->Disconnect(mm->FindConsumer(m_consumerID));
			m_generalMidi = false;
		}

		if (connect)
		{
			BMessage props;
			if (sink->GetProperties(&props) == B_OK)
			{
				if (props.HasBool("mev:internal_synth"))
					// init internal synth
					((Midi::CInternalSynth *)sink)->Init();
				if (props.HasBool("mev:general_midi"))
					m_generalMidi = true;
			}
			m_producer->Connect(sink);
			m_consumerID = sink->ID();
			SetLatency(sink->Latency());
		}
		else
		{
			m_producer->Disconnect(sink);
			m_consumerID = 0;
			SetLatency(0);
		}
	}
}

bool
CMidiDestination::IsConnected(
	BMidiConsumer *sink) const
{
	if ((sink->ID() == m_consumerID)
	 && (m_producer->IsConnected(sink)))
		return true;

	return false;
}

// ---------------------------------------------------------------------------
// CDestination Implementation

void
CMidiDestination::ColorChanged(
	rgb_color color)
{
	_updateIcons();
}

void
CMidiDestination::Deleted()
{
	// +++ disconnect
}

void
CMidiDestination::Disabled(
	bool disabled)
{
	_updateIcons();
}

status_t
CMidiDestination::GetIcon(
	icon_size which,
	BBitmap *outBitmap)
{
	ASSERT(outBitmap != NULL);

	BMessage props;
	if ((m_producer == NULL) || (m_producer->GetProperties(&props) != B_OK))
		return B_NAME_NOT_FOUND;

	const void *data;
	ssize_t size;
	status_t error;
	switch (which)
	{
		case B_LARGE_ICON:
		{
			error = props.FindData("be:large_icon", 'ICON', &data, &size);
			if (error)
				return error;
			break;
		}
		case B_MINI_ICON:
		{
			error = props.FindData("be:mini_icon", 'MICN', &data, &size);
			if (error)
				return error;
			break;
		}
	}
	memcpy(outBitmap->Bits(), data, size);

	return B_OK;
}

void
CMidiDestination::Muted(
	bool muted)
{
	_updateIcons();
}

void
CMidiDestination::NameChanged(
	const char *name)
{
	// +++ update producer name
}

void
CMidiDestination::ReadChunk(
	CIFFReader &reader)
{
	ASSERT(IsWriteLocked());

	switch (reader.ChunkID())
	{
		case CONNECTION_NAME_CHUNK:
		{
			char buffer[Midi::CONNECTION_NAME_LENGTH];
			reader.MustRead(buffer, MIN((size_t)reader.ChunkLength(),
										Midi::CONNECTION_NAME_LENGTH));
			SetConnect(Midi::CMidiManager::Instance()->FindConsumer(buffer),
					   true);
			break;
		}
		case DESTINATION_SETTINGS_CHUNK:
		{
			reader >> m_channel;
			break;
		}
		default:
		{
			CDestination::ReadChunk(reader);
		}
	}
}

void
CMidiDestination::Serialize(
	CIFFWriter &writer)
{
	ASSERT(IsReadLocked());

	CDestination::Serialize(writer);

	BMidiConsumer *consumer = (BMidiConsumer *)m_producer->Connections()->ItemAt(0);
	if (consumer)
	{
		const char *consumerName = consumer->Name();
		writer.WriteChunk(Midi::CONNECTION_NAME_CHUNK, consumerName,
						  MIN(strlen(consumerName) + 1,
						  	  Midi::CONNECTION_NAME_LENGTH));
	}

	writer.Push(Midi::DESTINATION_SETTINGS_CHUNK);
	writer << m_channel;
	writer.Pop();
}

void
CMidiDestination::Soloed(
	bool solo)
{
	_updateIcons();
}

void
CMidiDestination::Undeleted()
{
	// +++ reconnect
}

// ---------------------------------------------------------------------------
// Internal Operations

void
CMidiDestination::_addIcons(
	BMessage *message,
	BBitmap *largeIcon,
	BBitmap *miniIcon) const
{
	if (!message->HasData("be:large_icon", 'ICON'))
		message->AddData("be:large_icon", 'ICON', largeIcon->Bits(),
						 largeIcon->BitsLength());
	else
		message->ReplaceData("be:large_icon", 'ICON', largeIcon->Bits(),
							 largeIcon->BitsLength());
	if (!message->HasData("be:mini_icon", 'MICN'))
		message->AddData("be:mini_icon", 'MICN', miniIcon->Bits(),
						 miniIcon->BitsLength());
	else
		message->ReplaceData("be:mini_icon", 'MICN', miniIcon->Bits(),
							 miniIcon->BitsLength());
}

BBitmap *
CMidiDestination::_createIcon(
	BRect r)
{
	BBitmap	*icon = new BBitmap(r, B_CMAP8,true);
	BView *iconView = new BView(r, "", B_FOLLOW_RIGHT, B_FRAME_EVENTS);
	icon->Lock();
	icon->AddChild(iconView);
	iconView->SetHighColor(B_TRANSPARENT_COLOR);
	iconView->FillRect(r, B_SOLID_HIGH);
	iconView->SetHighColor(Color());
	if (IsDisabled() || IsMuted())
	{
		rgb_color contrast;
		contrast.red = (Color().red + 128) % 255;
		contrast.green = (Color().green + 128) % 255;
		contrast.blue = (Color().blue + 128) % 255;
		iconView->SetLowColor(contrast);
		pattern mixedColors = { 0xf0, 0xf0, 0xf0, 0xf0,
								0x0f, 0x0f, 0x0f, 0x0f };
		iconView->FillEllipse(r, mixedColors);
	}
	else
	{
		iconView->FillEllipse(r);
	}
	iconView->Sync();
	iconView->RemoveSelf();
	delete iconView;
	icon->Unlock();
	return icon;
}

void
CMidiDestination::_updateIcons()
{
	BRect sr, lr;
	sr.Set(0.0, 0.0, 15.0, 15.0);
	lr.Set(0.0, 0.0, 31.0, 31.0);
	BMessage msg;
	m_producer->GetProperties(&msg);
	BBitmap *lg = _createIcon(lr);
	BBitmap *sm = _createIcon(sr);
	_addIcons(&msg, lg, sm);
	delete lg;
	delete sm;
	m_producer->SetProperties(&msg);
}

// END - MidiDestination.cpp
