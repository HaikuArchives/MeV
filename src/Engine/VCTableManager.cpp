#include "VCTableManager.h"
#include <stdio.h>
CVCTableManager::CVCTableManager(VChannelTable *table)
{
	m_tablerep=table;
    pos=0;

}

void CVCTableManager::AddVC(VChannelEntry &vc)
{
	int c=0;
	while ((*m_tablerep)[c].defined==1)
	{
		c++;
	}
	
	(*m_tablerep)[c]=vc;
}
void CVCTableManager::RemoveVC (int id)
{
	(*m_tablerep)[id].defined=0;
}

bool CVCTableManager::IsDefined(int id)
{
	if ((*m_tablerep)[id].defined==1)
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
	while ((*m_tablerep)[pos].defined!=1)
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
	if (pos<Max_VChannels)
	{
		return &((*m_tablerep)[pos]);
	}
}
void CVCTableManager::Next()
{
	pos++;
	while ((*m_tablerep)[pos].defined!=1)
	{
		pos++;
		if (pos>=Max_VChannels)
			{
			return;
			}
	}
}