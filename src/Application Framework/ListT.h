/* ===================================================================== *
 * LisT.h (MeV/Application Framework)
 * ---------------------------------------------------------------------
 * License:
 *  The contents of this file are subject to the Netscape Public
 *  License Version 1.1 (the "License"); you may not use this file
 *  except in compliance with the License. You may obtain a copy of
 *  the License at http://www.mozilla.org/NPL/
 *
 *  Software distributed under the License is distributed on an "AS
 *  IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 *  implied. See the License for the specific language governing
 *  rights and limitations under the License.
 *
 *  The Original Code is MeV (Musical Environment) code.
 *
 *  The Initial Developer of the Original Code is Sylvan Technical 
 *  Arts. Portions created by Sylvan are Copyright (C) 1997 Sylvan 
 *  Technical Arts. All Rights Reserved.
 *
 *  Contributor(s): 
 *		Christopher Lenz (cell)
 *
 * ---------------------------------------------------------------------
 * Purpose:
 *  Template list class
 * ---------------------------------------------------------------------
 * History:
 *	1997		Talin
 *		Original implementation Talin, 1997
 *	04/08/2000	cell
 *		General cleanup in preparation for initial SourceForge checkin
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#pragma once

#include <List.h>

template <class T> class BListT : public BList {
public:
	BListT(int32 itemsPerBlock = 20) : BList( itemsPerBlock ) {}
	BListT(const BListT &inList ) : BList( inList ) {}
	~BListT();

	BList	&operator=(const BListT &from) { BList::operator=( from ); } 
	bool	AddItem(T *item) { return BList::AddItem( item ); }
	bool	AddItem(T *item, int32 atIndex) { return BList::AddItem( item, atIndex ); }
	bool	AddList( BListT *newItems ) { return BList::AddList( newItems ); }
	bool	AddList( BListT *newItems, int32 atIndex ) { return BList::AddList( newItems, atIndex ); }
	bool	RemoveItem( T *item ) { return BList::RemoveItem( item ); }
	T *RemoveItem(int32 index) { return (T*)BList::RemoveItem( index ); }
	bool	RemoveItems(int32 index, int32 count) { return BList::RemoveItems( index, count ); }
	T *ItemAt(int32 index) const { return (T*)BList::ItemAt( index ); }
	T *ItemAtFast(int32 index) const { return (T*)BList::ItemAtFast( index ); }
	int32 IndexOf(T *item) const { return BList::IndexOf( item ); }
	T *FirstItem() const { return (T*)BList::FirstItem(); }
	T*LastItem() const { return (T*)BList::LastItem(); }
	bool	HasItem(T *item) const { return BList::HasItem( T* item ); }
	void	DoForEach(bool (*func)(T *)) { return BList::DoForEach( (bool (*func)(void *))func ); }
	void	DoForEach(bool (*func)(T *, void *), void *p) { return BList::DoForEach( (bool (*func)(void *)func, p ); }
	T *Items() const { return (T*)BList::Items(); }
	void	SortItems(int (*cmp)(const T *, const T *)) { return BList::SortItem( (int (*cmp)(const void *, const void *))cmp ); }
	
	void DestroyItems()
	{
		for (int i = CountItem() - 1; i >= 0; i--) delete ItemAt( i );
		MakeEmpty();
	}
};
