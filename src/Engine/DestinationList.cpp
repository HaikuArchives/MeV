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
    	if ((m_tablerep[c])==NULL)
    	{
    		m_tablerep[c]=new Destination;
    		m_tablerep[c]->name << "Untitled ";
    		m_tablerep[c]->name << c+1;
    		m_tablerep[c]->m_producer=NULL;
    		m_tablerep[c]->producer_name="never name a midiport this";
    		m_tablerep[c]->channel	= 1;
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
void CDestinationList::RemoveVC (int id)
{
	count--;
	delete (m_tablerep[id]);
	m_tablerep[id]=NULL;
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
	if (m_tablerep[id]!=NULL)
	{
	
		return true;
	}
	else
	{
	
		return false;
	}
}

void CDestinationList::First()
{
	pos=0;
	while (m_tablerep[pos]==NULL)
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
	if (m_tablerep[pos]==NULL)
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
	while (m_tablerep[pos]==NULL)
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
					if (dest->producer_name==portname)
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
void CDestinationList::ReadVCTable (CIFFReader &reader)
{
	int32		i = 0;
	int32 portid=0;
	BString midiport;

	while (reader.BytesAvailable() > 0 )
	{
		reader >> portid;
		m_tablerep[portid]=new Destination;
		reader >> m_tablerep[portid]->channel >> m_tablerep[portid]->flags >> m_tablerep[portid]->velocityContour >> m_tablerep[portid]->initialTranspose;
		reader >> m_tablerep[portid]->fillColor.red;
		reader >> m_tablerep[portid]->fillColor.green;
		reader >> m_tablerep[portid]->fillColor.blue;
		m_tablerep[portid]==NULL;
		//reader >> midiport;
		printf ("reading %s\n",midiport.String());
		//printf ("midiport read %s\n",midiport.String());*/
	}
}
void CDestinationList::WriteVCTable (CIFFWriter &writer)
{
	for (First();!IsDone();Next())
	{
		writer << (int32)CurrentID(); 
		Destination *dest=CurrentDest();
		writer << dest->channel << dest->flags << dest->velocityContour << dest->initialTranspose;
		writer << dest->fillColor.red;
		writer << dest->fillColor.green;
		writer << dest->fillColor.blue;
		//writer << dest->m_producer->Name();
		printf ("writing %s\n",dest->m_producer->Name());
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
CDestinationList::SetDisableFor(
	int id,
	bool disable)
{
	Destination *dest = m_tablerep[id];
	if (disable)
	{
		if (dest->flags & Destination::disabled)
		{
		}
		else
		{
			dest->flags+=Destination::disabled;		
			dest->m_producer=NULL;	
			CUpdateHint hint;
			hint.AddInt8("channel",id);
			CObservableSubject::PostUpdate(&hint,NULL);
			m_doc->PostUpdateAllTracks(&hint);
		}
	}
	else
	{
		if (dest->flags & Destination::disabled)
		{
			dest->flags-=Destination::disabled;
			CUpdateHint hint;
			hint.AddInt8("channel",id);
			CObservableSubject::PostUpdate(&hint,NULL);
			m_doc->PostUpdateAllTracks(&hint);
			
		}
		else
		{
		}
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
