/* ===================================================================== *
 * Destination.cpp (MeV/Engine)
 * ===================================================================== */

#include "Destination.h"

#include "GeneralMidi.h"
#include "IFFReader.h"
#include "IFFWriter.h"
#include "InternalSynth.h"
#include "MeVDoc.h"
#include "MeVFileID.h"
#include "MidiManager.h"
#include "Observer.h"
#include "ReconnectingMidiProducer.h"
#include "TrackWindow.h"

// Interface Kit
#include <Rect.h>
#include <Bitmap.h>
#include <View.h>
// Midi Kit
#include <MidiConsumer.h>
#include <MidiProducer.h>
// Support Kit
#include <Debug.h>
// Standard C++ Library
#include <iostream>

#define D_ALLOC(x) //PRINT(x)		// Constructor/Destructor
#define D_HOOK(x) //PRINT(x)		// Hook Functions
#define D_ACCESS(x) //PRINT(x)		// Accessors
#define D_OPERATION(x) // PRINT(x)	// Operations
#define D_SERIALIZE(x) //PRINT(x)		// Serialization

// ---------------------------------------------------------------------------
// Constants Initialization

const rgb_color DEFAULT_COLORS[16] = 
{
	{ 255,  64,  64 },
	{  64, 128,  64 },
	{ 164,  32, 164 },
	{ 164, 164,  32 },			
	{   0, 255,  64 },			
	{   0, 255,   0 },			
	{   0, 160, 160 },			
	{   0, 255, 160 },			
	{ 64, 255, 255 },			
	{  47, 130, 255 },			
	{ 128, 128, 255 },			
	{ 200,   0, 255 },			
	{ 255,   0, 255 },		
	{ 255, 128, 255 },	
	{ 192, 192, 192 },
	{ 128, 128,   0 }
};

enum destinationFlags
{		
	MUTED			= (1<<0),

	MUTED_FROM_SOLO	= (1<<1),

	SOLO			= (1<<2),

	DISABLED		= (1<<3),

	DELETED			= (1<<4)
};

// +++ move this to CMidiDestination
const char *NOTE_NAMES[] =
{ "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

// ---------------------------------------------------------------------------
// Constructor/Destructor

CDestination::CDestination(
	unsigned long type,
	long id,
	const char *name,
	CMeVDoc *document)
	:	m_doc(document),
		m_type(type),
		m_id(id),
		m_latency(0),
		m_flags(0),
		m_color(DEFAULT_COLORS[id % 15]),
		m_channel(0),
		m_generalMidi(false)
{
	D_ALLOC(("CDestination::CDestination()\n"));

	snprintf(m_name, DESTINATION_NAME_LENGTH, "%s %ld", name, id + 1);

	m_producer = new Midi::CReconnectingMidiProducer(m_name);
	m_producer->Register();
}

CDestination::CDestination(
	CMeVDoc *document)
	:	m_doc(document),
		m_type('MIDI'),
		m_id(0),
		m_latency(0),
		m_flags(0),
		m_channel(0),
		m_generalMidi(false)
{
	D_ALLOC(("CDestination::CDestination(deserialize)\n"));

	m_producer = new Midi::CReconnectingMidiProducer("");
}

CDestination::~CDestination()
{
	D_ALLOC(("CDestination::~CDestination()\n"));

	m_producer->Release();
}

// ---------------------------------------------------------------------------
// Accessors

unsigned long
CDestination::Type() const
{
	return m_type;
}

long
CDestination::ID() const
{
	return m_id;
}

status_t
CDestination::GetIcon(
	icon_size which,
	BBitmap *outBitmap)
{
	ASSERT(outBitmap != NULL);

	BMessage props;
	if (m_producer && (m_producer->GetProperties(&props) != B_OK))
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

	return B_ERROR;
}

bool
CDestination::IsValid() const
{
	return ((m_flags & DISABLED) || (m_flags & DELETED));
}

bool
CDestination::IsMuted(
	bool *fromSolo) const {

	if (fromSolo) {
		*fromSolo = m_flags & MUTED_FROM_SOLO;
	}

	return m_flags & MUTED;
}

void
CDestination::SetMuted(
	bool muted)
{
	bool changed = false;
	if (muted)
		changed = _addFlag(MUTED);
	else
		changed = _removeFlag(MUTED);

	if (changed)
	{
		_updateIcons();
		Document()->SetModified();

		CUpdateHint hint;
		hint.AddInt32("DestID", m_id);
		hint.AddInt32("DestAttrs", Update_Flags);
		PostUpdate(&hint);
	}
}

bool
CDestination::IsSolo() const
{
	return m_flags & SOLO;
}

void
CDestination::SetSolo(
	bool solo)
{
	bool changed = false;
	if (solo)
		changed = _addFlag(SOLO);
	else
		changed = _removeFlag(SOLO);

	if (changed)
	{
		_updateIcons();
		Document()->SetModified();

		CUpdateHint hint;
		hint.AddInt32("DestID", m_id);
		hint.AddInt32("DestAttrs", Update_Flags);
		PostUpdate(&hint);
	}
}

void
CDestination::SetName(
	const char *name)
{
	if (strcmp(m_name, name) != 0)
	{
		strncpy(m_name, name, DESTINATION_NAME_LENGTH);

		BString producerName = "MeV: ";
		producerName << m_name;
		m_producer->SetName(producerName.String());

		Document()->SetModified();
		
		CUpdateHint hint;
		hint.AddInt32("DestID", m_id);
		hint.AddInt32("DestAttrs", Update_Name);
		PostUpdate(&hint);
	}
}

void
CDestination::SetLatency(
	bigtime_t latency)
{
	D_ACCESS(("CDestination::SetLatency(%Ld)\n", latency));

	if (latency != m_latency)
	{
		m_latency = latency;
		Document()->SetModified();
	
		CUpdateHint hint;
		hint.AddInt32("DestID", m_id);
		hint.AddInt32("DestAttrs", Update_Latency);
		PostUpdate(&hint);
	}
}

void
CDestination::SetColor(
	rgb_color color)
{
	if ((color.red != m_color.red)
	 || (color.green != m_color.green)
	 || (color.blue != m_color.blue))
	{
		m_color = color;
		
		_updateIcons();
		Document()->SetModified();
	
		CUpdateHint hint;
		hint.AddInt32("DestID", m_id);
		hint.AddInt32("DestAttrs", Update_Color);
		PostUpdate(&hint);
	}
}

bool
CDestination::IsDisabled() const
{
	return m_flags & DISABLED;
}

void
CDestination::SetDisabled(
	bool disabled)
{
	bool changed = false;
	if (disabled)
		changed = _addFlag(DISABLED);
	else
		changed = _removeFlag(DISABLED);

	if (changed)
	{
		CUpdateHint hint;
		hint.AddInt32("DestID", m_id);
		hint.AddInt32("DestAttrs",Update_Flags);
		PostUpdate(&hint);
	}
}

// ---------------------------------------------------------------------------
// Operations

bool
CDestination::IsDeleted() const
{
	return m_flags & DELETED;
}

void
CDestination::Delete ()
{
	StSubjectLock lock(*Document(), Lock_Shared);
	int32 originalIndex = Document()->IndexOf(this);
	if (_addFlag(DELETED))
	{
		int32 index = 0;
		CDestination *next = Document()->GetNextDestination(&index);
		if (next)
			Document()->SetDefaultAttribute(EvAttr_Channel, next->m_id);

		Document()->SetModified();
		
		CUpdateHint hint;
		hint.AddInt32("DestID", m_id);
		hint.AddInt32("DocAttrs", CMeVDoc::Update_DelDest);
		hint.AddInt32("original_index", originalIndex);
		Document()->PostUpdate(&hint);
	}
}

void 
CDestination::Undelete(
	int32 originalIndex)
{
	if (_removeFlag(DELETED))
	{
		Document()->SetModified();
		
		CUpdateHint hint;
		hint.AddInt32("DestID", m_id);
		hint.AddInt32("DocAttrs", CMeVDoc::Update_AddDest);
		Document()->PostUpdate(&hint);
	}
}

// ---------------------------------------------------------------------------
// CSerializable Implementation

void
CDestination::ReadChunk(
	CIFFReader &reader)
{
	switch (reader.ChunkID())
	{
		case DESTINATION_HEADER_CHUNK:
		{
			reader >> m_id;
			reader >> m_latency;
			reader >> m_flags;
			reader >> m_color;

			break;
		}
		case DESTINATION_NAME_CHUNK:
		{
			reader.MustRead(m_name, MIN((size_t)reader.ChunkLength(),
										DESTINATION_NAME_LENGTH));
			break;
		}
		case Midi::CONNECTION_NAME_CHUNK:
		{
			// +++ move to CMidiDestination
			char buffer[Midi::CONNECTION_NAME_LENGTH];
			reader.MustRead(buffer, MIN((size_t)reader.ChunkLength(),
										Midi::CONNECTION_NAME_LENGTH));
			SetConnect(Midi::CMidiManager::Instance()->FindConsumer(buffer),
					   true);
			break;
		}
		case Midi::DESTINATION_SETTINGS_CHUNK:
		{
			reader >> m_channel;
			break;
		}
		default:
		{
			CSerializable::ReadChunk(reader);
		}
	}
}

void
CDestination::Serialize(
	CIFFWriter &writer)
{
	writer.Push(DESTINATION_HEADER_CHUNK);
	writer << m_id;
	writer << m_latency;
	writer << m_flags;
	writer << m_color;
	writer.Pop();

	writer.WriteChunk(DESTINATION_NAME_CHUNK, m_name,
					  MIN(strlen(m_name) + 1, DESTINATION_NAME_LENGTH));

	// MIDI specific stuff
	// +++ move to CMidiDestination
	BMidiConsumer *consumer = (BMidiConsumer *)m_producer->Connections()->ItemAt(0);
	const char *consumerName = consumer->Name();
	writer.WriteChunk(Midi::CONNECTION_NAME_CHUNK, consumerName,
					  MIN(strlen(consumerName) + 1,
					  	  Midi::CONNECTION_NAME_LENGTH));

	writer.Push(Midi::DESTINATION_SETTINGS_CHUNK);
	writer << m_channel;
	writer.Pop();
}

// ---------------------------------------------------------------------------
// +++ move these to CMidiDestination

bool
CDestination::GetNoteName(
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
CDestination::GetProgramName(
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
CDestination::SetChannel(
	uint8 channel)
{
	if (channel != m_channel)
	{	
		m_channel = channel;
		Document()->SetModified();

		CUpdateHint hint;
		hint.AddInt32("DestID", m_id);
		hint.AddInt32("DestAttrs", Update_Channel);
		PostUpdate(&hint);
	}
}

void
CDestination::SetConnect(
	BMidiConsumer *sink,
	bool connect)
{
	D_ACCESS(("CDestination::SetConnect(%s)\n", sink ? sink->Name() : "(none)"));

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
CDestination::IsConnected(
	BMidiConsumer *sink) const
{
	if ((sink->ID() == m_consumerID)
	 && (m_producer->IsConnected(sink)))
		return true;

	return false;
}

// ---------------------------------------------------------------------------
// Internal Operations

void
CDestination::_addIcons(
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
CDestination::_createIcon(
	BRect r)
{
	BBitmap	*icon = new BBitmap(r, B_CMAP8,true);
	BView *iconView = new BView(r, "icon writer", B_FOLLOW_RIGHT,
								B_FRAME_EVENTS);
	icon->Lock();
	icon->AddChild(iconView);
	iconView->SetHighColor(B_TRANSPARENT_COLOR);
	iconView->FillRect(r, B_SOLID_HIGH);
	iconView->SetHighColor(m_color);
	if (IsDisabled() || IsMuted())
	{
		rgb_color contrast;
		contrast.red = (m_color.red + 128) % 255;
		contrast.green = (m_color.green + 128) % 255;
		contrast.blue = (m_color.blue + 128) % 255;
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
CDestination::_updateIcons()
{
	BRect sr,lr;
	sr.Set(0,0,15,15);
	lr.Set(0,0,31,31);
	BMessage msg;
	m_producer->GetProperties(&msg);
	BBitmap *lg = _createIcon(lr);
	BBitmap *sm = _createIcon(sr);
	_addIcons(&msg,lg,sm);
	delete lg;
	delete sm;
	m_producer->SetProperties(&msg);
}

bool 
CDestination::_addFlag(int32 flag)
{
	if (!(m_flags & flag))
	{
		m_flags ^= flag;
		return true;
	}

	return false;
}
bool 
CDestination::_removeFlag (int32 flag)
{
	if (m_flags & flag)
	{
		m_flags ^= flag;
		return true;
	}

	return false;
}

// ---------------------------------------------------------------------------
// CDestinationDeleteUndoAction: Constructor/Destructor

CDestinationDeleteUndoAction::CDestinationDeleteUndoAction(
	CDestination *dest)
	:	m_dest(dest),
		m_index(-1)
{
	D_ALLOC("CDestinationDeleteUndoAction::CDestinationDeleteUndoAction()");	
	m_dest->Delete();
}

CDestinationDeleteUndoAction::~CDestinationDeleteUndoAction()
{
	D_ALLOC("CDestinationDeleteUndoAction::~CDestinationDeleteUndoAction()");	
}

// ---------------------------------------------------------------------------
// CDestinationDeleteUndoAction: CUndoAction Implementation

void
CDestinationDeleteUndoAction::Redo()
{
	D_HOOK(("CDestinationDeleteUndoAction::Redo()\n"));
	m_dest->Delete();
}

void
CDestinationDeleteUndoAction::Undo()
{
	D_HOOK(("CDestinationDeleteUndoAction::Undo()\n"));
	m_dest->Undelete(m_index);
}

// END - Destination.cpp
