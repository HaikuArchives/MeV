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

const rgb_color
CDestination::s_defaultColorTable[] = 
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

// ---------------------------------------------------------------------------
// Constructor/Destructor

CDestination::CDestination(
	int32 id,
	CMeVDoc *document,
	const char *name)
	:	m_doc(document),
		m_id(id),
		m_name(name),
		m_latency(0),
		m_flags(0),
		m_channel(0)
{
	m_name.SetTo(name);
	m_name << " " << m_id + 1;
	m_producer = new Midi::CReconnectingMidiProducer(m_name.String());
	m_producer->Register();

	SetColor(s_defaultColorTable[id % 15]);
}

CDestination::CDestination(
	CIFFReader &reader,
	CMeVDoc *document)
	:	m_doc(document),
		m_id(0),
		m_latency(0),
		m_flags(0),
		m_channel(0)
{
	m_producer = new Midi::CReconnectingMidiProducer("");

	char buffer[255];
	rgb_color color;

	reader >> m_id;
	reader.ReadStr255(buffer, 255);
	SetName(buffer);
	reader >> m_latency;
	reader >> m_flags;
	reader >> color;

	// connect with name
	if (reader.ReadStr255(buffer, 255) > 0)
		SetConnect(Midi::CMidiManager::Instance()->FindConsumer(buffer), true);
	reader >> m_channel;

	// need the icons first, so...
	SetColor(color);
}

CDestination::~CDestination()
{
	m_producer->Release();
}

// ---------------------------------------------------------------------------
// Serialization

void
CDestination::WriteDestination(
	CIFFWriter &writer)
{
	char buffer[255];

	writer.Push(DESTINATION_CHUNK);

	writer.WriteStr255("MIDI", 4);

	writer << m_id;
	m_name.CopyInto(buffer, 0, m_name.Length());
	writer.WriteStr255(buffer, m_name.Length());
	writer << m_latency;
	writer << m_flags;
	writer << m_fillColor;

	// MIDI specific stuff
	// +++ move to CMidiDestination
	if (m_producer->Connections()->CountItems() > 0)
	{
		BString cons;
		cons << ((BMidiConsumer *)m_producer->Connections()->ItemAt(0))->Name();
		cons.CopyInto(buffer, 0, cons.Length());
		writer.WriteStr255(buffer, cons.Length());
	}
	else
	{
		writer.WriteStr255(buffer, 0);
	}
	writer << m_channel;

	writer.Pop();
}

// ---------------------------------------------------------------------------
// Accessors

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
	return ((m_flags & CDestination::disabled)
		 || (m_flags & CDestination::deleted));
}

bool
CDestination::Muted(
	bool *fromSolo) const {

	if (fromSolo) {
		*fromSolo = m_flags & mutedFromSolo;
	}

	return m_flags & muted;
}

void
CDestination::SetMuted(
	bool muted)
{
	bool changed = false;
	if (muted)
		changed = _addFlag(CDestination::muted);
	else
		changed = _removeFlag(CDestination::muted);

	if (changed)
	{
		_updateIcons();
		Document()->SetModified();

		CUpdateHint hint;
		hint.AddInt32("DestID", GetID());
		hint.AddInt32("DestAttrs", Update_Flags);
		PostUpdate(&hint);
	}
}

void
CDestination::SetSolo(
	bool solo)
{
	bool changed = false;
	if (muted)
		changed = _addFlag(CDestination::solo);
	else
		changed = _removeFlag(CDestination::solo);

	if (changed)
	{
		_updateIcons();
		Document()->SetModified();

		CUpdateHint hint;
		hint.AddInt32("DestID", GetID());
		hint.AddInt32("DestAttrs", Update_Flags);
		PostUpdate(&hint);
	}
}

void
CDestination::SetName(
	const BString &name)
{
	if (name != m_name)
	{
		m_name = name;
		BString producerName = "MeV: ";
		producerName << m_name;
		m_producer->SetName(producerName.String());
		Document()->SetModified();
		
		CUpdateHint hint;
		hint.AddInt32("DestID", GetID());
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
		hint.AddInt32("DestID", GetID());
		hint.AddInt32("DestAttrs", Update_Latency);
		PostUpdate(&hint);
	}
}

void
CDestination::SetColor(
	rgb_color color)
{
	if ((color.red != m_fillColor.red)
	 || (color.green != m_fillColor.green)
	 || (color.blue != m_fillColor.blue))
	{
		m_fillColor = color;
		
		if ((color.red + color.green + color.blue) < 384)
			m_highlightColor = tint_color(color, B_LIGHTEN_2_TINT);
		else
			m_highlightColor = tint_color(color, B_DARKEN_2_TINT);
	
		_updateIcons();
		Document()->SetModified();
	
		CUpdateHint hint;
		hint.AddInt32("DestID", GetID());
		hint.AddInt32("DestAttrs", Update_Color);
		PostUpdate(&hint);
	}
}

void
CDestination::SetDisabled(
	bool disabled)
{
	bool changed = false;
	if (disabled)
		changed = _addFlag(CDestination::disabled);
	else
		changed = _removeFlag(CDestination::disabled);

	if (changed)
	{
		CUpdateHint hint;
		hint.AddInt32("DestID",GetID());
		hint.AddInt32("DestAttrs",Update_Flags);
		PostUpdate(&hint);
	}
}

// ---------------------------------------------------------------------------
// Operations

void
CDestination::Delete ()
{
	StSubjectLock lock(*Document(), Lock_Shared);
	int32 originalIndex = Document()->IndexOf(this);
	if (_addFlag(CDestination::deleted))
	{
		int32 index = 0;
		CDestination *next = Document()->GetNextDestination(&index);
		if (next)
			Document()->SetDefaultAttribute(EvAttr_Channel, next->GetID());

		Document()->SetModified();
		
		CUpdateHint hint;
		hint.AddInt32("DestID", GetID());
		hint.AddInt32("DocAttrs", CMeVDoc::Update_DelDest);
		hint.AddInt32("original_index", originalIndex);
		Document()->PostUpdate(&hint);
	}
}

void 
CDestination::Undelete(
	int32 originalIndex)
{
	if (_removeFlag(CDestination::deleted))
	{
		Document()->SetModified();
		
		CUpdateHint hint;
		hint.AddInt32("DestID", GetID());
		hint.AddInt32("DocAttrs", CMeVDoc::Update_AddDest);
		Document()->PostUpdate(&hint);
	}
}

// ---------------------------------------------------------------------------
// Operations

bool
CDestination::GetProgramName(
	uint16 bank,
	uint8 program,
	char *outName)
{
	if (m_generalMidi)
	{
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
		hint.AddInt32("DestID", GetID());
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
			m_producer->Disconnect(mm->FindConsumer(m_consumerID));

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
	iconView->SetHighColor(m_fillColor);
	if ((m_flags & CDestination::disabled) || (m_flags & CDestination::muted))
	{
		rgb_color contrast;
		contrast.red = (m_fillColor.red+128) % 255;
		contrast.green = (m_fillColor.green+128) % 255;
		contrast.blue = (m_fillColor.blue+128) % 255;
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
