#include "DestinationList.h"
#include "Messenger.h"
#include "MeVDoc.h"
#include <stdio.h>
enum ID {
	VCTM_NOTIFY='ntfy'
	};
const rgb_color CDestinationList::m_defaultColorTable[]= {
	{ 255, 128, 128 },			// Midi Channel 1
	{ 255, 128,   0 },			// Midi Channel 2
	{ 255, 255,   0 },			// Midi Channel 3
	{ 128, 255,   0 },			// Midi Channel 4
	{   0, 255, 128 },			// Midi Channel 5
	{   0, 192,   0 },			// Midi Channel 6
	{   0, 160, 160 },			// Midi Channel 7
	{   0, 192, 160 },			// Midi Channel 8
	{ 128, 255, 255 },			// Midi Channel 9
	{  47, 130, 255 },			// Midi Channel 10
	{ 128, 128, 255 },			// Midi Channel 11
	{ 200,   0, 255 },			// Midi Channel 12
	{ 255,   0, 255 },			// Midi Channel 13
	{ 255, 128, 255 },			// Midi Channel 14
	{ 192, 192, 192 },			// Midi Channel 15
	{ 128, 128,   0 },			// Midi Channel 16
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
    		m_tablerep[c]->name << "Untitled ";
    		m_tablerep[c]->name << c+1;
    		m_tablerep[c]->m_producer=NULL;
    		m_tablerep[c]->producer_name="never name a midiport this";
    		m_tablerep[c]->channel	= 0;
    		m_tablerep[c]->flags		= Destination::transposable;  
    		//m_tablerep[c]->flags		= Destination::mute; 
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
   		dest->m_producer=NULL;
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
				msg->FindString("name",&portname);
				for (First();!IsDone();Next())
				{
					Destination *dest=CurrentDest();
					
					if (dest->producer_name==portname)
					{
						SetDisableFor(CurrentID(),true);
					}
				}
			}
			break;
			case B_MIDI_REGISTERED:
			{
				msg->FindString("name",&portname);
				for (First();!IsDone();Next())
				{
					Destination *dest=CurrentDest();
					if ((dest->producer_name==portname)&&(dest->flags & Destination::disabled))
					{
						dest->m_producer=m_midimanager->GetProducer(&dest->producer_name);
						SetDisableFor(CurrentID(),false);
					}
				}
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
	int32		i = 0;
	int32 portid=0;
	BString midiport;
	char buff[255];
	while (reader.BytesAvailable() > 0 )
	{
		reader >> portid;
		m_tablerep[portid]=new Destination;
		reader >> m_tablerep[portid]->channel >> m_tablerep[portid]->flags >> m_tablerep[portid]->velocityContour >> m_tablerep[portid]->initialTranspose;
		reader >> m_tablerep[portid]->fillColor.red;
		reader >> m_tablerep[portid]->fillColor.green;
		reader >> m_tablerep[portid]->fillColor.blue;
		ReadStr255( reader,buff, 255 );
		m_tablerep[portid]->name.SetTo(buff);
		ReadStr255( reader,buff, 255 );
		m_tablerep[portid]->producer_name.SetTo(buff);
		m_tablerep[portid]->m_producer=m_midimanager->GetProducer(&m_tablerep[portid]->producer_name);
		//m_tablerep[portid]->m_producer=NULL;
		if (m_tablerep[portid]->m_producer==NULL)
		{
			//SetDisableFor(portid,true);
			m_tablerep[portid]->flags+=Destination::disabled;		
			m_tablerep[portid]->m_producer=NULL;	
			CUpdateHint hint;
			hint.AddInt8("channel",portid);
			CObservableSubject::PostUpdate(&hint,NULL);
		}
	}
	CUpdateHint hint;
	hint.AddInt8("channel",portid);
	CObservableSubject::PostUpdate(&hint,NULL);
}
void CDestinationList::WriteVCTable (CIFFWriter &writer)
{
	char buff[255];
	char buff2[255];
	for (First();!IsDone();Next())
	{
		writer << (int32)CurrentID(); 
		Destination *dest=CurrentDest();
		writer << dest->channel << dest->flags << dest->velocityContour << dest->initialTranspose;
		writer << dest->fillColor.red;
		writer << dest->fillColor.green;
		writer << dest->fillColor.blue;
		dest->name.CopyInto(buff,0,dest->name.Length());
		WriteStr255( writer,buff,dest->name.Length() );
		if (dest->producer_name.Length()>0)
		{
			dest->producer_name.CopyInto(buff,0,dest->producer_name.Length());
			WriteStr255( writer,buff,dest->producer_name.Length() );		
		}
		else
		{
			BString s="no port";
			s.CopyInto(buff,0,s.Length());
			WriteStr255( writer,buff,s.Length() );
		}	
			
	}
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
}
void
CDestinationList::SetColorFor(
	int id,
	rgb_color color)
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
	CUpdateHint		hint;
	hint.AddInt8( "channel", id);
	CObservableSubject::PostUpdate( &hint, NULL );
	m_doc->PostUpdateAllTracks(&hint);
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
	if (producer!=NULL)
	{
	
		Destination *dest = m_tablerep[id];
		if (dest)
		{
			dest->m_producer=producer;
			dest->producer_name=producer->Name();
			CUpdateHint hint;
			hint.AddInt8("channel",id);
			CObservableSubject::PostUpdate(&hint,NULL);
			if (dest->flags & Destination::disabled)
			{
			//if disabled, let everyone know about it...no longer disabled.
					dest->flags-=Destination::disabled;
					m_doc->PostUpdateAllTracks(&hint);
			}
		}

	}

}
void
CDestinationList::SetMuteFor(
	int id,
	bool muted)
{
	if (muted)
	{
		Destination *dest = m_tablerep[id];
		dest->flags+=Destination::mute;
		CUpdateHint hint;
		hint.AddInt8("channel",id);
		CObservableSubject::PostUpdate(&hint,NULL);
		m_doc->PostUpdateAllTracks(&hint);
	}
	else
	{
		Destination *dest = m_tablerep[id];
		dest->flags-=Destination::mute;
		CUpdateHint hint;
		hint.AddInt8("channel",id);
		CObservableSubject::PostUpdate(&hint,NULL);
		m_doc->PostUpdateAllTracks(&hint);
	}

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
		dest->m_producer=NULL;	
		dest->fillColor.red=150;
		dest->fillColor.green=150;
		dest->fillColor.blue=150;
		
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
		dest->m_producer=NULL;	
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
