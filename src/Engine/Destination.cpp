/* ===================================================================== *
 * Destination.cpp (MeV/Engine)
 * ===================================================================== */
#include "MidiManager.h"
#include "Destination.h"
#include "MeVDoc.h"
#include "IFFWriter.h"
#include "IFFReader.h"
#include "TrackWindow.h"
#include "ReconnectingMidiProducer.h"
#include "InternalSynth.h"
#include "Observer.h"
#include <MidiConsumer.h>
#include <MidiProducer.h>
#include <Rect.h>
#include <Bitmap.h>
#include <View.h>
#include <Debug.h>
#define D_ALLOC(x) //PRINT(x)		// Constructor/Destructor
#define D_HOOK(x) //PRINT(x)		// Hook Functions
#define D_ACCESS(x) //PRINT(x)		// Accessors
#define D_OPERATION(x) // PRINT(x)	// Operations
#define D_SERIALIZE(x) //PRINT(x)		// Serialization
#include <iostream.h>
// ---------------------------------------------------------------------------
// Constructor/Destructor
const rgb_color CDestination::m_defaultColorTable[]= {
	{255, 64, 64},
	{64,128,64},
	{164,32,164},
	{164,164,32},			
	{   0, 255, 64 },			
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

CDestination::CDestination (
			int32 id,
			CMeVDoc &inDoc,
			char *name,
			bool notify)
			:	
				m_latency(0)
				
{
	
	m_doc=&inDoc;
	m_flags=0;
	m_channel=0;
	m_id=id;
	m_name.SetTo(name);
	m_name << " " << m_id+1;
	m_producer=new CReconnectingMidiProducer (m_name.String());
	m_producer->Register();
	m_fillColor = m_defaultColorTable[(id) % 15];
	
	BRect sr,lr;
	sr.Set(0,0,15,15);
	lr.Set(0,0,31,31);

	BMessage msg;
	m_producer->GetProperties(&msg);
	_addIcons(&msg,CreateIcon(lr),CreateIcon(sr));
	m_producer->SetProperties(&msg);
	//send notification message
	if (notify)
	{				
			inDoc.NotifyUpdate(CMeVDoc::Update_AddDest,NULL);
	}
}

CDestination::~CDestination()
{
	m_producer->Release();
}

// serialization

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

// Accessors

bool
CDestination::IsValid () const
{
	return ((m_flags & CDestination::disabled) || (m_flags & CDestination::deleted));
}
BBitmap *
CDestination::CreateIcon (BRect r)
{
	BBitmap	*icon = new BBitmap( r, B_CMAP8,true );
	BView *icon_view = new BView (r,"icon writer",B_FOLLOW_RIGHT,B_FRAME_EVENTS);
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

void CDestination::_addIcons(BMessage* msg, BBitmap* largeIcon, BBitmap* miniIcon) const
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


void
CDestination::SetMuted (bool muted) 
{
	if (muted)
	{
		_addFlag (CDestination::muted);
	}
	else
	{
		_removeFlag (CDestination::muted);
	}
	BRect sr,lr;
	sr.Set(0,0,15,15);
	lr.Set(0,0,31,31);
	
	BMessage msg;
	m_producer->GetProperties(&msg);
	BBitmap *lg=CreateIcon(lr);
	BBitmap *sm=CreateIcon(sr);
	_addIcons(&msg,lg,sm);
	delete lg;
	delete sm;
	m_producer->SetProperties(&msg);
	CUpdateHint hint;
	hint.AddInt8("channel",GetID());
	hint.AddInt32("DestAttrs",Update_Flags);
	hint.AddInt32("DestID",GetID());
	Document().PostUpdateAllTracks(&hint);
	Document().PostUpdate(&hint);
}
bool
CDestination::Muted() const
{
	return (m_flags & CDestination::muted);
}
bool
CDestination::Solo() const
{
	return (m_flags & CDestination::solo);
}

bool
CDestination::Deleted() const
{
	return (m_flags & CDestination::deleted);
}
bool
CDestination::Disabled() const
{
	return (m_flags & CDestination::disabled);
}
bool
CDestination::MutedFromSolo () const
{
}

void
CDestination::SetSolo (bool solo)
{
}

void
CDestination::SetName (const char *name)
{
	m_name.SetTo(name);
	m_producer->SetName(name);
	CUpdateHint hint;
	hint.AddInt32 ("DestID",GetID());
	hint.AddInt32 ("DestAttrs",Update_Name);
	Document().PostUpdate(&hint,NULL);
}
void 
CDestination::SetLatency (int32 microseconds)
{
	m_latency=microseconds/1000;
	CUpdateHint hint;
	hint.AddInt32 ("DestID",GetID());
	hint.AddInt32 ("DestAttrs",Update_Latency);
	Document().SetDestinationLatency(GetID(),m_latency);
}
int32
CDestination::Latency (uint8 clockType)
{
//	m_producer->Latency();
}
void
CDestination::SetColor (rgb_color color)
{
	m_fillColor=color;
	
	if ((color.red + color.green + color.blue) < 384)
		m_highlightColor = tint_color(color, B_LIGHTEN_2_TINT);
	else
		m_highlightColor = tint_color(color, B_DARKEN_2_TINT);
	
	BRect sr,lr;
	sr.Set(0,0,15,15);
	lr.Set(0,0,31,31);

	BMessage msg;
	m_producer->GetProperties(&msg);
	_addIcons(&msg,CreateIcon(lr),CreateIcon(sr));
	m_producer->SetProperties(&msg);
	
	CUpdateHint hint;
	hint.AddInt32("channel",GetID());
	hint.AddInt32("DestAttrs",Update_Color);
	Document().PostUpdateAllTracks(&hint);
	Document().PostUpdate(&hint);
}
rgb_color
CDestination::GetFillColor ()
{
	return m_fillColor;
}
 
rgb_color
CDestination::GetHighlightColor ()
{
	return m_highlightColor;
}	

void
CDestination::SetChannel (uint8 channel)
{
	m_channel=channel;
	//updates anyone?
}

void
CDestination::SetConnect (BMidiConsumer *sink,bool connect)
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
		//updates ?
	}
}

bool
CDestination::IsConnected (BMidiConsumer *sink) const
{
	if ((sink->ID()==m_consumer_id) && (m_producer->IsConnected(sink)))
	{
		return true;
	}
	return false;
}

void
CDestination::SetDisable (bool disable)
{
	bool modify;
	if (disable)
	{
		modify=_addFlag (CDestination::disabled);
	}
	else
	{
		modify=_removeFlag (CDestination::disabled);
	}
	if (modify)
	{
		CUpdateHint hint;
		hint.AddInt32("DestID",GetID());
		hint.AddInt32("DestAttrs",Update_Flags);
		Document().PostUpdateAllTracks(&hint);
		//Document().PostUpdate(&hint,NULL);
	}
}
void
CDestination::Delete ()
{
//	if (m_doc->m_destinations.ReplaceItem(GetID(),NULL))
	{
		_addFlag(CDestination::deleted);
		Document().SetModified();
		
		CUpdateHint hint;
		hint.AddInt32 ("DestID", GetID());
		hint.AddInt32 ("DestAttrs", Update_Flags);
		m_doc->PostUpdateAllTracks(&hint);
		m_doc->NotifyUpdate(CMeVDoc::Update_DelDest,NULL);
	}
}
void 
CDestination::Undelete(
	int32 originalIndex)
{
	//if (Document().m_destinations.AddItem(this,originalIndex))
	{
		_removeFlag(CDestination::deleted);
		Document().SetModified();
		
		CUpdateHint hint;
		hint.AddInt32 ("DestID", GetID());
		hint.AddInt32 ("DestAttrs", Update_Flags);
		Document().PostUpdateAllTracks(&hint);
		Document().NotifyUpdate(CMeVDoc::Update_AddDest,NULL);
	
	}
}

bool 
CDestination::_addFlag(int32 flag)
{
	if (m_flags & flag)
	{
	//set
	}
	else
	{
		m_flags^=flag;
		return true;
	}	
}
bool 
CDestination::_removeFlag (int32 flag)
{
	if (m_flags & flag)
	{
		m_flags^=flag;
		return true;
	}
	else
	{
	//unset
	}
}
// debug
void
CDestination::PrintSelf()
{
	printf ("\n");
	printf ("Name=%s\n",Name());
	printf ("ID=%d\n",m_id);
	printf ("IndexOf=%d\n",Document().m_destinations.IndexOf(this));
	printf ("Muted=%d\n",Muted());
	printf ("Solod=%d\n",Solo());
	printf ("Deleted=%d\n",Deleted());
	printf ("Disabled=%d\n",Disabled());
	printf ("Valid=%d\n",IsValid());
	printf ("Producer=%s\n",m_producer->Name());	
}
CDestinationDeleteUndoAction::CDestinationDeleteUndoAction(
	CDestination *dest)
	:	m_dest(dest),
		m_index(-1)
{
	D_ALLOC("CDestinationDeleteUndoAction::CDestinationDeleteUndoAction()");	
	dest->Delete();
}

CDestinationDeleteUndoAction::~CDestinationDeleteUndoAction()
{
	D_ALLOC("CDestinationDeleteUndoAction::~CDestinationDeleteUndoAction()");	
}


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
