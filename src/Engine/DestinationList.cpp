#include "DestinationList.h"
#include "Messenger.h"
#include "MeVDoc.h"
#include "ReconnectingMidiProducer.h"
#include "InternalSynth.h"
#include <Debug.h>
enum ID {
	VCTM_NOTIFY='ntfy'
	};
const rgb_color CDestinationList::m_defaultColorTable[]= {
	{ 255, 128, 128 },			
	{ 255, 128,   0 },			
	{ 255, 255,   0 },		
	{ 128, 255,   0 },			
	{   0, 255, 128 },			
	{   0, 192,   0 },			
	{   0, 160, 160 },			
	{   0, 192, 160 },			
	{ 128, 255, 255 },			
	{  47, 130, 255 },			
	{ 128, 128, 255 },			
	{ 200,   0, 255 },			
	{ 255,   0, 255 },		
	{ 255, 128, 255 },	
	{ 192, 192, 192 },
	{ 128, 128,   0 },
};
CDestinationList::CDestinationList(CMeVDoc *inDoc) : CObservableSubject (),CObserver(* CMidiManager::Instance(),CMidiManager::Instance())
{
	m_selected_id=-1;
    count=0;
    pos=0;
    m_notifier=NULL;
    int c;
    for (c=0;c<Max_Destinations;c++)
    {	
    	m_tablerep[c]=NULL;
    }
   	
    m_doc=inDoc;
    m_midimanager=CMidiManager::Instance();
    SetSubject (m_midimanager);
    //BMessenger *msgr=new BMessenger (this,this->Window());
    //m_midimanager->Subscribe(msgr);
}
CDestinationList::~CDestinationList()
{
	//m_midimanager->Unsubscribe(msg);
}
int CDestinationList::NewDest()
{
	count++; 
	int c;
	for (c=0;c<Max_Destinations;c++)
    {
    	if (!IsDefined(c))
    	{
    		m_tablerep[c]=new Destination;
    		m_tablerep[c]->name << "Untitled Destination ";
    		m_tablerep[c]->name << c+1;
    		BString epname;
    		epname << "MeV: ";
    		epname << m_tablerep[c]->name;
    		m_tablerep[c]->m_producer=new CReconnectingMidiProducer(epname.String());
    		m_tablerep[c]->m_producer->Register();
    		m_tablerep[c]->producer_name="obsolete";
    		m_tablerep[c]->channel	= 0;
    		m_tablerep[c]->flags		= Destination::transposable;  
    		m_tablerep[c]->velocityContour=0;
    		m_tablerep[c]->VUMeter=0;
    		SetColorFor(c, m_defaultColorTable[c % 16]);
    		return c;
    	}
    }
    return -1;		
}
void CDestinationList::SetSelectedId(int id)
{
	m_selected_id=id;
}
int CDestinationList::SelectedId()
{
	return (m_selected_id);
}
void CDestinationList::RemoveVC (int id)
{
	count--;
	SetDeletedFor(id,true);
}
Destination *  CDestinationList::operator[](int i)
{
	return (m_tablerep)[i];
}
Destination *  CDestinationList::get(int i)
{
	return m_tablerep[i];
}
bool CDestinationList::IsDefined(int id)
{
	if (id >= Max_Destinations)
	{
		return false;
	}
	if (m_tablerep[id]==NULL)
	{
		return false;
	}
	else if (m_tablerep[id]->flags & Destination::deleted)
	{
		return false;
	}	
	return true;	

}

void CDestinationList::First()
{
	pos=0;
	while (!IsDefined(pos))
	{
		pos++;
		if (pos>=Max_Destinations)
		{
			return;
		}
	}
}
bool CDestinationList::IsDone()
{
	if (pos>=Max_Destinations)
	{
		return true;
	}
	else
	{
		return false;
	}
}
int CDestinationList::CurrentID()  
{
	return pos;
}
Destination * CDestinationList::CurrentDest()
{
	if (!IsDefined(pos))
	{
	    Destination *dest=new Destination;
   		dest->name.SetTo("blah");
   		dest->channel	= 1;
   		dest->flags = Destination::transposable;   
   		dest->velocityContour=0;
   		dest->VUMeter=0;
		rgb_color color = {128, 128, 128, 255};
   		SetColorFor(pos, color);
		return (dest);
	}

	if (pos < Max_Destinations)
	{
		return m_tablerep[pos];
	}

	return NULL;
}

void CDestinationList::Next()
{
	pos++;
	while (!IsDefined(pos))
	{
		pos++;
		if (pos>=Max_Destinations)
		{
				return;
		}
	}
}
void CDestinationList::OnUpdate(BMessage *msg)
{
	int32 op;
	BString portname;
	
	if (msg->FindInt32("midiop", &op) == B_OK)
	{
		switch (op)
		{
			case B_MIDI_UNREGISTERED:
			{
				/*msg->FindString("name",&portname);
				for (First();!IsDone();Next())
				{
					Destination *dest=CurrentDest();
					
					if (dest->producer_name==portname)
					{
						SetDisableFor(CurrentID(),true);
					}
				}*/
			}
			break;
			case B_MIDI_REGISTERED:
			{
				/*msg->FindString("name",&portname);
				for (First();!IsDone();Next())
				{
					Destination *dest=CurrentDest();
					if ((dest->producer_name==portname)&&(dest->flags & Destination::disabled))
					{
						dest->m_producer=m_midimanager->GetProducer(&dest->producer_name);
						SetDisableFor(CurrentID(),false);
					}
				}*/
			}
			break;
		}
	}
}
//this belongs in the reader.
int32 ReadStr255( CAbstractReader &inReader, char *outBuffer, int32 inMaxLength );
int32 ReadStr255( CAbstractReader &inReader, char *outBuffer, int32 inMaxLength )
{
	uint8			sLength;
	int32			actual;
	
	inReader >> sLength;
	actual = sLength < inMaxLength - 1 ? sLength : inMaxLength - 1;
	inReader.MustRead( outBuffer, actual );
	outBuffer[ actual ] = 0;
	if (actual < sLength) inReader.Skip( sLength - actual );
	return actual;
}
//this too..
void WriteStr255( CAbstractWriter &outWriter, char *inBuffer, int32 inLength );
void WriteStr255( CAbstractWriter &outWriter, char *inBuffer, int32 inLength )
{
	if (inLength > 255) inLength = 255;
	outWriter << (uint8)inLength;
	outWriter.MustWrite( inBuffer, inLength );
}

void CDestinationList::ReadVCTable (CIFFReader &reader)
{
	int32 portid=0;
	BString midiport;
	char buff[255];
	while (reader.BytesAvailable() > 0 )
	{
		reader >> portid;
		m_tablerep[portid]=new Destination;
		m_tablerep[portid]->m_producer=new CReconnectingMidiProducer("");
		reader >> m_tablerep[portid]->channel >> m_tablerep[portid]->flags >> m_tablerep[portid]->velocityContour >> m_tablerep[portid]->initialTranspose;
		rgb_color color;
		reader >> color.red;
		reader >> color.green;
		reader >> color.blue;
		SetColorFor(portid, color, false);
		ReadStr255(reader,buff, 255);
		m_tablerep[portid]->name.SetTo(buff);
		//set producer name
		ReadStr255(reader,buff,255);
		BString prod;
		prod.SetTo(buff);
		m_tablerep[portid]->m_producer->SetName(prod.String());
		
		//load and connect all connections 
		BString pname;
		while (pname.Compare("connection list end"))
		{
			ReadStr255( reader,buff, 255 );
			pname.SetTo(buff);
			//connect with name
			ToggleConnectFor  (portid,m_midimanager->FindConsumer(pname.String()));
		}
		
		
	}
	CUpdateHint hint;
	hint.AddInt8("channel",portid);
	CObservableSubject::PostUpdate(&hint,NULL);
}
void CDestinationList::WriteVCTable (CIFFWriter &writer)
{
	char buff[255];
	for (First();!IsDone();Next())
	{
		writer << (int32)CurrentID(); 
		Destination *dest=CurrentDest();
		writer << dest->channel << dest->flags << dest->velocityContour << dest->initialTranspose;
		writer << dest->fillColor.red;
		writer << dest->fillColor.green;
		writer << dest->fillColor.blue;
		dest->name.CopyInto(buff,0,dest->name.Length());
		WriteStr255 (writer,buff,dest->name.Length());
		//write name of producer
		BString prod;
		prod << dest->m_producer->Name();
		prod.CopyInto(buff,0,prod.Length());
		WriteStr255 (writer,buff,prod.Length());
		
		
		//write connections
		BList *connections=dest->m_producer->Connections();
		int c=connections->CountItems()-1;
		while (c>=0)
		{
			BString name;
			BMidiProducer *prod=(BMidiProducer *)connections->ItemAt(c--);
			name=prod->Name();
			name.CopyInto(buff,0,name.Length());
			WriteStr255( writer,buff,name.Length() );
		}
		WriteStr255 (writer,"connection list end",20);	
			
	}
}
BBitmap * CDestinationList::GetIconFor (int id,BRect r) const
{	
	BBitmap	*icon = new BBitmap( r, B_CMAP8,true );
	Destination *dest = m_tablerep[id];
	if (dest)
	{
		
		BView *icon_view = new BView (r,"icon writer",B_FOLLOW_RIGHT,B_FRAME_EVENTS);
		
		//BView::LockLooper();
		icon->Lock();
		icon->AddChild(icon_view);
		icon_view->SetHighColor(B_TRANSPARENT_COLOR);
		icon_view->FillRect(r, B_SOLID_HIGH);
		icon_view->SetHighColor(dest->fillColor);
		
		if ((dest->flags & Destination::disabled) || (dest->flags & Destination::mute))
		{
			rgb_color contrast;
			contrast.red=(dest->fillColor.red+128) % 255;
			contrast.green=(dest->fillColor.green+128) % 255;
			contrast.blue=(dest->fillColor.blue+128) % 255;
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
		//icon_view->StrokeRect(r);		
		icon_view->Sync();
		icon_view->RemoveSelf();
		icon->Unlock();
		
		delete (icon_view);
	}
	return icon;
}
void CDestinationList::SetNameFor(
	int id,
	BString name)
{
	Destination *dest = m_tablerep[id];
	if (dest)
	{
		dest->name=name;
	}
	CUpdateHint		hint;
	hint.AddInt8( "channel", id);
	CObservableSubject::PostUpdate( &hint, NULL );
	BString epname;
	epname << "MeV: ";
	epname << name;
	dest->m_producer->SetName(epname.String());
	
}
void
CDestinationList::SetColorFor(
	int id,
	rgb_color color,
	bool postUpdate)
{
	Destination *dest = m_tablerep[id];
	if (dest)
	{
		dest->fillColor = color;
		if ((color.red + color.green + color.blue) < 384)
			dest->highlightColor = tint_color(color, B_LIGHTEN_2_TINT);
		else
			dest->highlightColor = tint_color(color, B_DARKEN_2_TINT);
	}
	
	//producer icon set here
	BRect sr,lr;
	sr.Set(0,0,15,15);
	lr.Set(0,0,31,31);
	BMessage msg;
	dest->m_producer->GetProperties(&msg);
	m_midimanager->AddIcons(&msg,GetIconFor(id,lr),GetIconFor(id,sr));
	dest->m_producer->SetProperties(&msg);
	//end producer set
	
	if (postUpdate)
	{
		CUpdateHint hint;
		hint.AddInt8("channel", id);
		CObservableSubject::PostUpdate(&hint, NULL);
		m_doc->PostUpdateAllTracks(&hint);
	}
}

void 
CDestinationList::SetChannelFor(
	int id,
	int channel)
{
	Destination *dest = m_tablerep[id];
	if (dest)
	{
		dest->channel=channel;
	}
	CUpdateHint hint;
	hint.AddInt8("channel",id);
	CObservableSubject::PostUpdate(&hint,NULL);
}
void 
CDestinationList::SetPortFor(
	int id,
	BMidiLocalProducer *producer)
{
	

}
void
CDestinationList::SetMuteFor(
	int id,
	bool muted)
{
	Destination *dest = m_tablerep[id];
	if (muted)
	{
		dest->flags+=Destination::mute;
	}
	else
	{
		dest->flags-=Destination::mute;
	}
	
	BRect sr,lr;
	sr.Set(0,0,15,15);
	lr.Set(0,0,31,31);
	BMessage msg;
	dest->m_producer->GetProperties(&msg);
	m_midimanager->AddIcons(&msg,GetIconFor(id,sr),GetIconFor(id,lr));
	dest->m_producer->SetProperties(&msg);
	
	
	CUpdateHint hint;
	hint.AddInt8("channel",id);
	CObservableSubject::PostUpdate(&hint,NULL);
	m_doc->PostUpdateAllTracks(&hint);
}
void 
CDestinationList::SetDeletedFor(
	int id,
	bool deleted)
{
	Destination *dest = m_tablerep[id];
	if (deleted)
	{
		dest->flags+=Destination::deleted;		
		dest->m_producer->Unregister();
		rgb_color color = {150, 150, 150, 255};
		SetColorFor(id, color);
		
		CUpdateHint hint;
		hint.AddInt8("channel",id);
		CObservableSubject::PostUpdate(&hint,NULL);
		m_doc->PostUpdateAllTracks(&hint);
	}
}
void 
CDestinationList::SetDisableFor(
	int id,
	bool disable)
{
	Destination *dest = m_tablerep[id];
	if (disable) 
	{
		dest->flags+=Destination::disabled;		
		//dest->m_producer=NULL;	
		CUpdateHint hint;
		hint.AddInt8("channel",id);
		CObservableSubject::PostUpdate(&hint,NULL);
		m_doc->PostUpdateAllTracks(&hint);
	}
	else if (dest->flags & Destination::disabled)
	{
		dest->flags-=Destination::disabled;
		CUpdateHint hint;
		hint.AddInt8("channel",id);
		CObservableSubject::PostUpdate(&hint,NULL);
		m_doc->PostUpdateAllTracks(&hint);			
	}	
}
void
CDestinationList::ToggleConnectFor(
	int id,
	BMidiConsumer *sink)
{
	BMessage props;
	Destination *dest = m_tablerep[id];
	sink->GetProperties(&props);
	bool flag;
	if (props.FindBool("mev:internalSynth", &flag) == B_OK)
	{
		m_midimanager->PostMessage(CInternalSynth::INIT_INTERNAL_SYNTH);
	}
	if (dest->m_producer->IsConnected(sink))
	{
		dest->m_producer->Disconnect(sink);
	}
	else
	{
		dest->m_producer->Connect(sink);
	}
}
void
CDestinationList::SetSoloFor( 
	int id,
	bool solo)
{
	if (solo)
	{
	//go though every id and set muteFromSolo
	//set unique id as solo
	}
	else
	{
	//unset unique id solo
	//un mutefromsolor everyone
	}
	
}


// END - DestinationList.cpp
