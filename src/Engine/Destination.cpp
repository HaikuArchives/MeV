/* ===================================================================== *
 * Destination.cpp (MeV/Engine)
 * ===================================================================== */

#include "Destination.h"

#include "IFFReader.h"
#include "IFFWriter.h"
#include "InternalSynth.h"
#include "MeVDoc.h"
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
		m_flags(0)
{
	m_channel = 0;
	m_name.SetTo(name);
	m_name << " " << m_id + 1;
	m_producer = new CReconnectingMidiProducer(m_name.String());
	m_producer->Register();
	m_fillColor = s_defaultColorTable[id % 15];
	
	_updateIcons();
}

CDestination::~CDestination()
{
	m_producer->Release();
}

// ---------------------------------------------------------------------------
// Serialization

//this in the writer
void WriteStr255( CAbstractWriter &outWriter, char *inBuffer, int32 inLength );
void WriteStr255( CAbstractWriter &outWriter, char *inBuffer, int32 inLength )
{
	if (inLength > 255) inLength = 255;
	outWriter << (uint8)inLength;
	outWriter.MustWrite( inBuffer, inLength );
}

void
CDestination::WriteDestination (CIFFWriter &writer)
{
	char buff[255];
	writer << (int32)GetID(); 
	writer << m_channel << m_flags;
	writer << m_fillColor.red;
	writer << m_fillColor.green;
	writer << m_fillColor.blue;
	m_name.CopyInto(buff,0,m_name.Length());
	WriteStr255 (writer,buff,m_name.Length());
	//write name of producer
	BString prod;
	prod << m_producer->Name();
	prod.CopyInto(buff,0,prod.Length());
	WriteStr255 (writer,buff,prod.Length());
	
	
	//write name of consumer
	BString cons;
	if (m_producer->Connections()->CountItems() > 0)
	{
		cons << ((BMidiConsumer *)m_producer->Connections()->ItemAt(0))->Name();
		cons.CopyInto(buff,0,cons.Length());
		WriteStr255 (writer,buff,cons.Length());
	}
	else
	{
		cons << "MEV NO PORT";
		cons.CopyInto(buff,0,cons.Length());
		WriteStr255 (writer,buff,cons.Length());
	}
}

// ---------------------------------------------------------------------------
// Accessors

bool
CDestination::IsValid() const
{
	return ((m_flags & CDestination::disabled) || (m_flags & CDestination::deleted));
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
		Document().SetModified();

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
		Document().SetModified();

		CUpdateHint hint;
		hint.AddInt32("DestID", GetID());
		hint.AddInt32("DestAttrs", Update_Flags);
		PostUpdate(&hint);
	}
}

void
CDestination::SetName(
	const char *name)
{
	if (BString(name) != m_name)
	{
		m_name.SetTo(name);
		m_producer->SetName(name);
		Document().SetModified();
		
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
	if (latency != m_latency)
	{
		m_latency = latency;
		Document().SetModified();
	
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
		Document().SetModified();
	
		CUpdateHint hint;
		hint.AddInt32("DestID", GetID());
		hint.AddInt32("DestAttrs", Update_Color);
		PostUpdate(&hint);
	}
}

void
CDestination::SetChannel(
	uint8 channel)
{
	if (channel != m_channel)
	{	
		m_channel = channel;
		Document().SetModified();

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
	if (sink)
	{
		CMidiManager *mm=CMidiManager::Instance();
		if (m_producer->IsConnected(mm->FindConsumer(m_consumer_id)))
		{
			m_producer->Disconnect(mm->FindConsumer(m_consumer_id));
		}
		m_consumer_id=sink->ID();
		BMessage props;
		bool flag;
		sink->GetProperties(&props);
		if (props.FindBool("mev:internalSynth",&flag)==B_OK)
		{
			((CInternalSynth *)sink)->Init();
		}
		if (connect)
		{
			m_producer->Connect(sink);
		}
		else
		{
			m_producer->Disconnect(sink);
		}
	}
}

bool
CDestination::IsConnected(
	BMidiConsumer *sink) const
{
	if ((sink->ID() == m_consumer_id)
	 && (m_producer->IsConnected(sink)))
		return true;

	return false;
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
	StSubjectLock lock(Document(), Lock_Shared);
	int32 originalIndex = Document().IndexOf(this);
	if (_addFlag(CDestination::deleted))
	{
		int32 index = 0;
		CDestination *next = Document().GetNextDestination(&index);
		if (next)
			Document().SetDefaultAttribute(EvAttr_Channel, next->GetID());

		Document().SetModified();
		
		CUpdateHint hint;
		hint.AddInt32("DestID", GetID());
		hint.AddInt32("DocAttrs", CMeVDoc::Update_DelDest);
		hint.AddInt32("original_index", originalIndex);
		Document().PostUpdate(&hint);
	}
}

void 
CDestination::Undelete(
	int32 originalIndex)
{
	if (_removeFlag(CDestination::deleted))
	{
		Document().SetModified();
		
		CUpdateHint hint;
		hint.AddInt32("DestID", GetID());
		hint.AddInt32("DocAttrs", CMeVDoc::Update_AddDest);
		Document().PostUpdate(&hint);
	}
}

// ---------------------------------------------------------------------------
// Internal Operations

void
CDestination::_addIcons(
	BMessage* msg,
	BBitmap* largeIcon,
	BBitmap* miniIcon) const
{
	if (! msg->HasData("be:large_icon", 'ICON')) {
		msg->AddData("be:large_icon", 'ICON', largeIcon->Bits(),
			largeIcon->BitsLength());
	} else {
		msg->ReplaceData("be:large_icon", 'ICON', largeIcon->Bits(),
			largeIcon->BitsLength());
	}
	if (! msg->HasData("be:mini_icon", 'MICN')) {
		msg->AddData("be:mini_icon", 'MICN', miniIcon->Bits(),
			miniIcon->BitsLength());
	} else {
		msg->ReplaceData("be:mini_icon", 'MICN', miniIcon->Bits(),
			miniIcon->BitsLength());
	}
}

BBitmap *
CDestination::_createIcon(
	BRect r)
{
	BBitmap	*icon = new BBitmap(r, B_CMAP8,true);
	BView *icon_view = new BView (r, "icon writer", B_FOLLOW_RIGHT,
								  B_FRAME_EVENTS);
	icon->Lock();
	icon->AddChild(icon_view);
	icon_view->SetHighColor(B_TRANSPARENT_COLOR);
	icon_view->FillRect(r, B_SOLID_HIGH);
	icon_view->SetHighColor(m_fillColor);
	if ((m_flags & CDestination::disabled) || (m_flags & CDestination::muted))
	{
		rgb_color contrast;
		contrast.red=(m_fillColor.red+128) % 255;
		contrast.green=(m_fillColor.green+128) % 255;
		contrast.blue=(m_fillColor.blue+128) % 255;
		icon_view->SetLowColor(contrast);
		pattern mixed_colors = {0xf0,0xf0,0xf0,0xf0,0x0f,0x0f,0x0f,0x0f};
		icon_view->FillEllipse(r,mixed_colors);
	}
	else
	{
		icon_view->FillEllipse(r);
	}
	icon_view->SetPenSize(1);
	rgb_color blk;
	blk.red=0;
	blk.green=0;
	blk.blue=0;
	icon_view->SetHighColor(blk);	
	icon_view->Sync();
	icon_view->RemoveSelf();
	icon->Unlock();
	delete (icon_view);
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

#if DEBUG
void
CDestination::PrintToStream()
{
	printf("\n");
	printf("Name=%s\n", Name());
	printf("ID=%ld\n", m_id);
	printf("IndexOf=%ld\n", Document().m_destinations.IndexOf(this));
	printf("Muted=%d\n", Muted());
	printf("Solod=%d\n", Solo());
	printf("Deleted=%d\n", Deleted());
	printf("Disabled=%d\n", Disabled());
	printf("Valid=%d\n", IsValid());
	printf("Producer=%s\n", m_producer->Name());	
}
#endif

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
