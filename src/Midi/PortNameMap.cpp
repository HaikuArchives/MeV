/* ===================================================================== *
 * PortNameMap.cpp (MeV/Midi)
 * ===================================================================== */

#include "PortNameMap.h"

#include <stdio.h>
#include <iostream.h>

CPortNameMap::CPortNameMap()
{
m_map=new BList();

//this is kinda ugly right now...
/*
CStringTuple * amap=new CStringTuple;
amap->name=new BString("/dev/midi/awe64/1");
amap->map=new BString("SoundBlaster AWE64 1");
m_map->AddItem(amap);
amap=new CStringTuple;
amap->name=new BString("/dev/midi/awe64/2");
amap->map=new BString("SoundBlaster AWE64 2");
m_map->AddItem(amap);
amap=new CStringTuple;
amap->name=new BString("/dev/midi/awe64/3");
amap->map=new BString("SoundBlaster AWE64 3");
m_map->AddItem(amap);
amap=new CStringTuple;
amap->name=new BString("/dev/midi/awe64/4");
amap->map=new BString("SoundBlaster AWE64 4");
m_map->AddItem(amap);

amap=new CStringTuple;
amap->name=new BString("/dev/midi/awe64/1");
amap->map=new BString("SoundBlaster AWE64 1");
m_map->AddItem(amap);
*/


}
void
CPortNameMap::AddMap(CStringTuple *tuple)
{
	m_map->AddItem(tuple);
}
bool 
CPortNameMap::HasMap(BString *real_name)
{
	int c = m_map->CountItems()-1;
	while (c>=0)
	{
		CStringTuple *atuple;
		atuple=(CStringTuple *)m_map->ItemAt(c);
		if (real_name->FindFirst(atuple->name->String())==0)
		{
			return true;
		}
		c--;
	}
	return false;
	
}

BString * CPortNameMap::Map(BString *real_name)
{
	int c = m_map->CountItems()-1;
	while (c>=0)
	{
		CStringTuple *atuple;
		atuple=(CStringTuple *)m_map->ItemAt(c);
		if (real_name->FindFirst(atuple->name->String())==0)
		{
			return (atuple->map);
		}
		c--;
	}
	return (real_name);
}
