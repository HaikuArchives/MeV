/* ===================================================================== *
 * DynamicMenu.cpp (MeV/Application Framework)
 * ===================================================================== */

#include "DynamicMenu.h"

CDynamicMenuDef::CDynamicMenuDef()
{
}

		/**	Destructor. */
CDynamicMenuDef::~CDynamicMenuDef()
{
	for (int i = 0; i < itemList.CountItems(); i++)
	{
		CItemDef		*item = (CItemDef *)itemList.ItemAt( i );
		
		delete item;
	}
}

int32 CDynamicMenuDef::AddItem(
	char			*label,
	BMessage		*msg,
	BHandler		*handler,
	char			shortcut,
	uint32		modifiers )
{
	Lock();
	int32		index = itemList.CountItems();
	CItemDef		*item = new CItemDef( label, msg, handler, shortcut, modifiers );
	
	itemList.AddItem( item );
	itemListChanged = true;
	
	Unlock();
	return index;
}

int32 CDynamicMenuDef::AddItem(
	int32		index,
	char			*label,
	BMessage		*msg,
	BHandler		*handler,
	char			shortcut,
	uint32		modifiers )
{
	Lock();
	int32		ct = itemList.CountItems();
	CItemDef		*item = new CItemDef( label, msg, handler, shortcut, modifiers );
	
	if (index >= ct) index = ct;
	if (index < 0) index = 0;

	itemList.AddItem( item, index );
	itemListChanged = true;
	
	Unlock();
	return index;
}

bool CDynamicMenuDef::ReplaceItem(
	int32		index,
	char			*label,
	BMessage		*msg,
	BHandler		*handler,
	char			shortcut,
	uint32		modifiers )
{
	Lock();
	if (index < 0 || index >= itemList.CountItems())
	{
		Unlock();
		return false;
	}
	CItemDef		*item = (CItemDef *)itemList.ItemAt( index );
	
	delete item->name;
	delete item->msg;
	
	item->name		= strdup( label );
	item->msg		= msg;
	item->handler	= handler;
	item->shortcut	= shortcut;
	item->modifiers	= modifiers;
	itemChanged = true;

	Unlock();
	return true;
}

bool CDynamicMenuDef::DeleteItem( int32 inIndex )
{
	Lock();
	if (inIndex < 0 || inIndex >= itemList.CountItems())
	{
		Unlock();
		return false;
	}
	CItemDef		*item = (CItemDef *)itemList.ItemAt( inIndex );
	
	delete item;
	itemList.RemoveItem( inIndex );
	itemListChanged = true;
	Unlock();
	return true;
}

char *CDynamicMenuDef::NameAt( int32 index )
{
	if (index < 0 || index >= itemList.CountItems()) return NULL;
	CItemDef		*item = (CItemDef *)itemList.ItemAt( index );

	return item->name;
}

BMessage *CDynamicMenuDef::MessageAt( int32 index )
{
	if (index < 0 || index >= itemList.CountItems()) return NULL;
	CItemDef		*item = (CItemDef *)itemList.ItemAt( index );

	return item->msg;
}

BHandler *CDynamicMenuDef::HandlerAt( int32 index )
{
	if (index < 0 || index >= itemList.CountItems()) return NULL;
	CItemDef		*item = (CItemDef *)itemList.ItemAt( index );

	return item->handler;
}

void CDynamicMenuDef::Unlock()
{
	if (itemChanged || itemListChanged)
	{
		for (int i = 0; i < instanceList.CountItems(); i++)
		{
			CDynamicMenuInstance	*inst = (CDynamicMenuInstance *)instanceList.ItemAt( i );
			
			if (itemChanged)		inst->itemChanged = true;
			if (itemListChanged)	inst->itemListChanged = true;
		}
		itemChanged = itemListChanged = false;
	}
	lock.Unlock();
}

CDynamicMenuInstance::CDynamicMenuInstance(
	CDynamicMenuDef	&inDef )
		: def( inDef )
{
	baseMenu = NULL;
	itemChanged = false;
	itemListChanged = true;
	modelMessage = NULL;
}

CDynamicMenuInstance::~CDynamicMenuInstance()
{
	def.Lock();
	def.instanceList.RemoveItem( this );
	def.Unlock();
	delete modelMessage;
}

void CDynamicMenuInstance::SetBaseMenu( BMenu *inMenu, int32 inIndex )
{
	int32			count = inMenu->CountItems();

	if (inIndex < 0 || inIndex >= count)	startIndex = count;
	else startIndex = inIndex;
	baseMenu = inMenu;
}
	
void CDynamicMenuInstance::SetMessageAttributes( BMessage *inModelMessage )
{
	delete modelMessage;
	modelMessage = inModelMessage;
	itemListChanged = true;
}
	
void CDynamicMenuInstance::CheckMenusChanged()
{
	if (baseMenu == false) return;

	def.Lock();
	if (itemListChanged)
	{
		int		i;
		
			//	Remove all items from the list.
		for (i = 0; i < itemList.CountItems(); i++)
		{
			BMenuItem	*mi = (BMenuItem *)itemList.ItemAt( i );
			
			baseMenu->RemoveItem( mi );
			delete mi;
		}
		itemList.MakeEmpty();
		
		for (i = 0; i < def.itemList.CountItems(); i++)
		{
			CDynamicMenuDef::CItemDef	*id;
			BMenuItem	*mi;
			BMessage		*msg;
			
			id = (CDynamicMenuDef::CItemDef *)def.itemList.ItemAt( i );
			msg = new BMessage( *id->msg );
			
			if (modelMessage)
			{
				char		*name;
				uint32	type;
				int32	count;
				const void	*data;
				ssize_t	size;
			
				for (	int32 j = 0; 
						modelMessage->GetInfo( B_ANY_TYPE, j, &name, &type, &count ) == B_OK;
						j++)
				{
					for (int k = 0; k < count; k++)
					{
						if (modelMessage->FindData( name, type, k, &data, &size ) == B_OK)
							msg->AddData( name, type, data, size, false, 1 );
					}
				}
			}
								
			mi = new BMenuItem( id->name, msg, id->shortcut, id->modifiers );
								
			mi->SetTarget( id->handler );
			itemList.AddItem( mi );

			baseMenu->AddItem( mi, startIndex + i );
		}
	
		itemListChanged = itemChanged = false;
	}
	else if (itemChanged)
	{
			//	Update all the items...
		for (int i = 0; i < itemList.CountItems(); i++)
		{
			BMenuItem	*mi = (BMenuItem *)itemList.ItemAt( i );
			CDynamicMenuDef::CItemDef	*id;

			id = (CDynamicMenuDef::CItemDef *)def.itemList.ItemAt( i );
			
			mi->SetLabel( id->name );
			mi->SetMessage( new BMessage( *id->msg ) );
			mi->SetTarget( id->handler );
			mi->SetShortcut( id->shortcut, id->modifiers );
		}
	
		itemChanged = false;
	}
	def.Unlock();
}
