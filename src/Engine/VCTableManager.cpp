#include "VCTableManager.h"
#include "MidiManager.h"
enum ID {
	VCTM_NOTIFY='ntfy'
	};
#include <stdio.h>

CVCTableManager::CVCTableManager()
{
	count=0;
    pos=0;
    m_notifier=NULL;
    int c;
    for (c=0;c<Max_VChannels;c++)
    {	
    	m_tablerep[c]=NULL;
    }
}
void CVCTableManager::AddClient(BHandler *nhandler)
{
	m_notifier=new BMessenger(nhandler);
}
void CVCTableManager::NotifyClients()
{
	_notifyClients();
}
void CVCTableManager::_notifyClients()
{
if (m_notifier!=NULL)
	{
		m_notifier->SendMessage (new BMessage (VCTM_NOTIFY));
	}
}
int CVCTableManager::NewVC(char *name)
{
	count++; 
	int c;
	for (c=0;c<Max_VChannels;c++)
    {
    	if ((m_tablerep[c])==NULL)
    	{
    		CMidiManager *midiManager=CMidiManager::Instance();
    		m_tablerep[c]=new VChannelEntry;
    		m_tablerep[c]->name.SetTo(name);
    		m_tablerep[c]->m_producer=midiManager->InternalSynth();
    		m_tablerep[c]->channel	= 1;
    		m_tablerep[c]->flags		= VChannelEntry::transposable;   
    		m_tablerep[c]->velocityContour=0;
    		m_tablerep[c]->VUMeter=0;
    		m_tablerep[c]->fillColor.red=100;
    		m_tablerep[c]->fillColor.green=100;
    		m_tablerep[c]->fillColor.blue=100;
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
	return (m_tablerep[i]);
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
    		vc->flags		= VChannelEntry::transposable;   
    		vc->velocityContour=0;
    		vc->VUMeter=0;
    		vc->fillColor.red=100;
    		vc->fillColor.green=100;
    		vc->fillColor.blue=100;
			return (vc);
	}
	if (pos<Max_VChannels)
	{
		return m_tablerep[pos];
	}
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