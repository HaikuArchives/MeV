#include "VCTableManager.h"
#include "MidiManager.h"
#include "Messenger.h"
#include "MeVDoc.h"
#include <stdio.h>
enum ID {
	VCTM_NOTIFY='ntfy'
	};
const rgb_color CVCTableManager::m_defaultColorTable[]= {
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
CVCTableManager::CVCTableManager(CMeVDoc *inDoc) : CObservableSubject ()
{
	count=0;
    pos=0;
    m_notifier=NULL;
    int c;
    for (c=0;c<Max_VChannels;c++)
    {	
    	m_tablerep[c]=NULL;
    }
    m_doc=inDoc;
}

int CVCTableManager::NewVC()
{
	count++; 
	int c;
	//we need to find the highest value of 'untitled'
	for (c=0;c<Max_VChannels;c++)
    {
    	if ((m_tablerep[c])==NULL)
    	{
    		CMidiManager *midiManager=CMidiManager::Instance();
    		m_tablerep[c]=new VChannelEntry;
    		m_tablerep[c]->name << "Untitled ";
    		m_tablerep[c]->name << c+1;
    		m_tablerep[c]->m_producer=NULL;
    		m_tablerep[c]->channel	= 1;
    		m_tablerep[c]->flags		= VChannelEntry::transposable;  
    		//m_tablerep[c]->flags		= VChannelEntry::mute; 
    		m_tablerep[c]->velocityContour=0;
    		m_tablerep[c]->VUMeter=0;
    		SetColorFor(c, m_defaultColorTable[c % 16]);
    		return c;
    	}
    }
    return -1;		
}
void CVCTableManager::RemoveVC (int id)
{
	count--;
	delete (m_tablerep[id]);
	m_tablerep[id]=NULL;
}
VChannelEntry *  CVCTableManager::operator[](int i)
{
	return (m_tablerep)[i];
}
VChannelEntry *  CVCTableManager::get(int i)
{
	return m_tablerep[i];
}
bool CVCTableManager::IsDefined(int id)
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

void CVCTableManager::First()
{
	pos=0;
	while (m_tablerep[pos]==NULL)
	{
		pos++;
		if (pos>=Max_VChannels)
		{
			return;
		}
	}
}
bool CVCTableManager::IsDone()
{
	if (pos>=Max_VChannels)
	{
		return true;
	}
	else
	{
		return false;
	}
}
int CVCTableManager::CurrentID()  
{
	return pos;
}
VChannelEntry * CVCTableManager::CurrentVC()
{
	if (m_tablerep[pos]==NULL)
	{
	    VChannelEntry *vc=new VChannelEntry;
   		vc->name.SetTo("blah");
   		vc->m_producer=NULL;
   		vc->channel	= 1;
   		vc->flags = VChannelEntry::transposable;   
   		vc->velocityContour=0;
   		vc->VUMeter=0;
		rgb_color color = {128, 128, 128, 255};
   		SetColorFor(pos, color);
		return (vc);
	}

	if (pos < Max_VChannels)
	{
		return m_tablerep[pos];
	}

	return NULL;
}

void CVCTableManager::Next()
{
	pos++;
	while (m_tablerep[pos]==NULL)
	{
		pos++;
		if (pos>=Max_VChannels)
		{
				return;
		}
	}
}
void CVCTableManager::SetNameFor(
	int id,
	BString name)
{
	VChannelEntry *vce = m_tablerep[id];
	if (vce)
	{
		vce->name=name;
	}
	CUpdateHint		hint;
	hint.AddInt8( "channel", id);
	PostUpdate( &hint, NULL );
}
void
CVCTableManager::SetColorFor(
	int id,
	rgb_color color)
{
	VChannelEntry *vce = m_tablerep[id];
	if (vce)
	{
		vce->fillColor = color;
		if ((color.red + color.green + color.blue) < 384)
			vce->highlightColor = tint_color(color, B_LIGHTEN_2_TINT);
		else
			vce->highlightColor = tint_color(color, B_DARKEN_2_TINT);
	}
	CUpdateHint		hint;
	hint.AddInt8( "channel", id);
	PostUpdate( &hint, NULL );
	m_doc->PostUpdateAllTracks(&hint);
}
void 
CVCTableManager::SetChannelFor(
	int id,
	int channel)
{
	VChannelEntry *vce = m_tablerep[id];
	if (vce)
	{
		vce->channel=channel;
	}
	CUpdateHint hint;
	hint.AddInt8("channel",id);
	PostUpdate(&hint,NULL);
}
void 
CVCTableManager::SetPortFor(
	int id,
	BMidiLocalProducer *producer)
{
	VChannelEntry *vce = m_tablerep[id];
	if (vce)
	{
		vce->m_producer=producer;
	}
	CUpdateHint hint;
	hint.AddInt8("channel",id);
	PostUpdate(&hint,NULL);
}
void
CVCTableManager::SetMuteFor(
	int id,
	bool muted)
{
	if (muted)
	{
		VChannelEntry *vce = m_tablerep[id];
		vce->flags=VChannelEntry::mute;
		CUpdateHint hint;
		hint.AddInt8("channel",id);
		PostUpdate(&hint,NULL);
		m_doc->PostUpdateAllTracks(&hint);
	}
	else
	{
		VChannelEntry *vce = m_tablerep[id];
		vce->flags=0;
		CUpdateHint hint;
		hint.AddInt8("channel",id);
		PostUpdate(&hint,NULL);
		m_doc->PostUpdateAllTracks(&hint);
	}

}


// END - VCTableManager.cpp
