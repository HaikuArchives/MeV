/* ===================================================================== *
 * ItemList.h (MeV)
 * ---------------------------------------------------------------------
 * License:
 *  The contents of this file are subject to the Mozilla Public
 *  License Version 1.1 (the "License"); you may not use this file
 *  except in compliance with the License. You may obtain a copy of
 *  the License at http://www.mozilla.org/MPL/
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
 *  Container class which keeps undo history of changes
 * ---------------------------------------------------------------------
 * History:
 *	1997		Talin
 *		Original implementation
 *	04/08/2000	cell
 *		General cleanup in preparation for initial SourceForge checkin
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __C_ItemList_H__
#define __C_ItemList_H__

#include "Undo.h"

	// Forward declare undo action.
class ItemListUndoAction_Base;
template<class Item> class ItemListUndoAction ;

/*	Issues:
	1.	Add in overloading of the equals operator for markers and others.
*/

#pragma overload	void *operator new( size_t size );
inline void* operator new( size_t, void* p ) { return p; }

/* ===================================================================== *
   A template used for copying, constructing, and destructing unknown types
 * ===================================================================== */

	// This supplies the lowest-common-denominator functions for moving and copying
	// arbitrary classes including integral types. However, these functions are not
	// particularly optimal, and one would hope that they be replaced for a given type.
template <class T> class ItemFuncs
{
public:
		// Construct items (default constructor with no params)
	static void Construct(	T* inPointer )				{ new ( inPointer ) T; }
	static void Construct(	T* inPointer, long inCount )
		{ while (inCount--) Construct( inPointer++ ); }

		// Copy constructor
	static void Construct(	T* outDst, T* inSrc )		{ new ( outDst ) T( *inSrc ); }
	static void Construct(	T* outDst, T* inSrc, long inCount )
		{ while (inCount--) Copy( outDst++, inSrc++ ); }

		// Copy operator
	static void Copy(		T* outDst, T* inSrc )		{ *outDst = *inSrc; }
	static void Copy(		T* outDst, T* inSrc, long inCount )
		{ while (inCount--) Copy( outDst++, inSrc++ ); }

		// Destructor
	static void Destroy(	T* inPointer )				{ inPointer->~T(); }
	static void Destroy(	T* inPointer, long inCount )
		{ while (inCount--) Destroy( inPointer++ ); }

		// Move operator -- used to reallocate the item, constructing in a new location
		// and destructing in the old. Note: Copies may overlap!
	static void Move(		T* outDst, T* inSrc )		{ Construct( outDst, inSrc ); Destroy( inSrc ); }
	static void Move(		T* outDst, T* inSrc, long inCount )
	{
		if (inSrc > outDst)	{ while (inCount--) Move( outDst++, inSrc++ ); }
		else				{ while (inCount--) Move( &outDst[ inCount ], &inSrc[ inCount ] ); }
	}
};

/* ===================================================================== *
   ItemBlock_Base - A Block which holds a number of items
 * ===================================================================== */

class ItemBlock_Base : private DNode {

	friend class	ItemList_Base;
	friend class	ItemMarker_Base;

protected:
	uint16			count;					// number of items in block
	DList			markers;				// markers pointing to this block
	
		// Member functions to iterate through list of blocks.
	ItemBlock_Base *Next() const { return (ItemBlock_Base *)DNode::Next(); }
	ItemBlock_Base *Prev() const { return (ItemBlock_Base *)DNode::Prev(); }
	ItemMarker_Base *FirstMarker() const
		{	return (ItemMarker_Base *)markers.First(); }

		// The actual array of items does not exist here, but rather exists
		// in the derived template classes.
};

	// A non-template base class representing a list of item blocks.

class ItemList_Base {
	friend class	UndoItem;
	friend class	ItemBlock_Base;
	friend class	ItemMarker_Base;
	friend class ItemListUndoAction_Base;

	int16			itemSize,				// Size of items
					itemsPerBlock;			// Number of items per block

protected:
	int32			count,					// number of items
					blockCount;				// number of blocks

	DList			blocks;					// list of item blocks

protected:
		// constructor
	ItemList_Base( short inItemSize, short inItemsPerBlock );
	virtual ~ItemList_Base();				// destructor (has to be 1 level up!)

		// Block manipulation
	void Compact(	ItemBlock_Base *inBlock );
	bool Split(	ItemBlock_Base *inBlock, short inSplitPoint );
	
		// Item insertion and deletion
	bool Insert(		ItemMarker_Base	*inListPos,			// Insertion point
					void				*inItemArray,		// Items to insert
					long				inNumItems,			// Number of items to insert
					ItemListUndoAction_Base *inSaveUndo );		// non-NULL if undo info should be saved.

	void Remove(		ItemMarker_Base	*inListPos,			// First item to remove
					long				inItemCount,			// # of items to remove
					ItemListUndoAction_Base *inSaveUndo );		// non-NULL if undo should be saved

	long Replace(	ItemMarker_Base *inListPos,			// Replacement point
					void				*inItemArray,		// List of new items values
					long				inItemCount,			// # of items to replace
					ItemListUndoAction_Base *inaveUndo );		// non-NULL if undo should be saved

	long Swap(		ItemMarker_Base *inListPos,			// Beginning of swap location
					void				*inItemArray,		// List of items to swap
					long				inItemCount );		// # of items in list
	
	ItemBlock_Base *FirstBlock() const
		{ return (ItemBlock_Base *)blocks.First(); }
	ItemBlock_Base *LastBlock() const
		{ return (ItemBlock_Base *)blocks.Last(); }
	
		// Allocate a new block. Must be overridden.
	virtual void *NewBlock() = 0;
	
		// Notifies subclasses that a block has changed. This can be used in case
		// of extra information associated with a block that needs to be changed.
	virtual void OnBlockChanged( ItemBlock_Base * ) {}
		
private:
		// These functions are used in the management of items. Since we
		// have problems using real destructors, these serve as "fake"
		// constructors and destructors which can be over-ridden by
		// subclasses to properly initialize the items in the container.

		// Copy items to new uninitialized memory
	virtual void ConstructItems( void *outDst, void *inSrc, long inItemCount ) = 0;

		// Copy items over old items
	virtual void CopyItems( void *outDst, void *inSrc, long inItemCount ) = 0;

		// Copy items to new uninitalized memory, destructing source items
	virtual void MoveItems( void *outDst, void *inSrc, long inItemCount ) = 0;

		// Destroy items
	virtual void DestroyItems( void *outDst, long inItemCount ) = 0;

public:
	long TotalItems() const { return count; }
	
		/**	Returns TRUE if item list is empty. */
	bool IsEmpty() const ;

#if DEBUG
	virtual void Validate();
#endif
};

/* ===================================================================== *
   Template class representing a block of specific-sized items
 * ===================================================================== */

	// The parameters to the template are:
	// K	--	the type of items stored in the block
	// cnt	--	the number of items per block

template<class K,int cnt> class ItemBlock : protected ItemBlock_Base {
	friend class	ItemMarker_Base;
//	friend class	ItemMarker<ItemBlock,K>;
//	friend class	ItemList  <ItemBlock,K>;

private:
	uint8			array[ cnt * sizeof( K ) ];		// Array of items in the block

protected:
	const K *ItemAddress( int num ) const { return (K *)&array[ num * sizeof( K ) ]; }

	ItemBlock *Next() const { return (ItemBlock *)ItemBlock_Base::Next(); }
	ItemBlock *Prev() const { return (ItemBlock *)ItemBlock_Base::Prev(); }

public:
	typedef K		Item;
	enum { Cnt = cnt };

	static ItemBlock* Upcast( ItemBlock_Base *b ) { return (ItemBlock *)b; }
	static ItemBlock_Base* Downcast( ItemBlock *b ) { return (ItemBlock_Base *)b; }
};

/* ===================================================================== *
   ItemList Template - A list of blocks
 * ===================================================================== */

template<class BK,class Item> class ItemList : public ItemList_Base {
//	friend class	ItemMarker<BK,Item>;

// typedef BK::Item Item;

	void *NewBlock() { return new BK; }

		// Copy items to new uninitialized memory, keeping old copies
	void ConstructItems( void *outDst, void *inSrc, long inItemCount )
	{
		ItemFuncs<Item>::Construct( (Item *)outDst, (Item *)inSrc, inItemCount );
	}

		// Copy items over old items, but keeping old copies
	void CopyItems( void *outDst, void *inSrc, long inItemCount )
	{
		ItemFuncs<Item>::Copy( (Item *)outDst, (Item *)inSrc, inItemCount );
	}

		// Copy items to new uninitalized memory, destructing source items
	void MoveItems( void *outDst, void *inSrc, long inItemCount )
	{
		ItemFuncs<Item>::Move( (Item *)outDst, (Item *)inSrc, inItemCount );
	}

		// Destroy items
	void DestroyItems( void *outDst, long inItemCount )
	{
		ItemFuncs<Item>::Destroy( (Item *)outDst, inItemCount );
	}
	
protected:
	BK *FirstBlock() const
		{ return (BK *)BK::Upcast( ItemList_Base::FirstBlock() ); }
	BK *LastBlock() const
		{ return (BK *)BK::Upcast( ItemList_Base::LastBlock() ); }

public:
	ItemList() : ItemList_Base( sizeof (Item), BK::Cnt ) {}
	~ItemList();

	long TotalItems() const { return ItemList_Base::TotalItems(); }
	
		/**	Returns TRUE if item list is empty. */
	bool IsEmpty() const { return ItemList_Base::IsEmpty(); }

#if 1
	void Validate() { ItemList_Base::Validate(); }
#endif
};

template<class BK,class Item>
ItemList<BK,Item>::~ItemList()
{
	DNode			*d;

		// Delete any remaining blocks
	while (	(d = blocks.RemHead()) != NULL )
	{
		BK		*bk = bk = (BK *)d;

		ItemFuncs<Item>::Destroy( (Item *)bk->ItemAddress( 0 ), bk->count );
		delete bk;
	}
}

/* ===================================================================== *
   ItemMarker_Base -- non-template base class for ItemMarker
 * ===================================================================== */

class ItemMarker_Base : private DNode {
	friend class	ItemList_Base;
	friend class ItemListUndoAction_Base;
	
public:

		// If data is inserted at this position, it controls whether the
		// position marker moves to the beginning or the end of the insertion.
	enum trackingTypes {
		Track_Previous = 0,					// track previous event
		Track_Next							// track next event
	};

protected:
	void			*item;
	ItemBlock_Base	*block;					// pointer to block item is in
	ItemList_Base	*blockList;				// pointer to block item list
	int16			index;					// index within block
	int16			trackType;				// tracking type

	void SetBlock( ItemBlock_Base *bk );	// change the block field
	void SetIndex( short index ); 			// change the index and ptr
	long AbsIndex() const;					// return absolute item number

	ItemBlock_Base *FirstBlock() const
		{ return blockList ? blockList->FirstBlock() : NULL; }

	ItemBlock_Base *LastBlock() const
		{ return blockList ? blockList->LastBlock() : NULL; }

		// Functions to navigate in the list
	void *Peek( int32 inOffset ) const;		// peek forward or back
	void *Seek( int32 inOffset );			// go to next N items in list
	void *First();							// go to first item in list
	void *Last();							// go to last item in list

		// point the position marker to a specific item
	void *Set( long inAbsIndex );

		// get items into flat array
	long Get( void *outItemArray, long inItemCount ) const;

	bool Insert(	void		*inItemArray,
					long		inItemCount,
					ItemListUndoAction_Base *inSaveUndo )
	{
		if (!blockList) return FALSE;
		return blockList->Insert( this, inItemArray, inItemCount, inSaveUndo );
	}

	void Remove(	long		inItemCount,
				ItemListUndoAction_Base *inSaveUndo )
	{
		if (blockList) blockList->Remove( this, inItemCount, inSaveUndo );
	}

	long Replace(	void		*inItemArray,
					long		inItemCount,
					ItemListUndoAction_Base *inSaveUndo )
	{
		if (!blockList) return 0;
		return blockList->Replace( this, inItemArray, inItemCount, inSaveUndo );
	}

		// Copy constructor
	ItemMarker_Base( ItemList_Base &l );

public:
		// constructors
	ItemMarker_Base( const ItemMarker_Base &r );	// copy constructor
	ItemMarker_Base();						// NULL constructor

	~ItemMarker_Base();						// destructor
	
		// indicate that the marker is pointing to "no item".
	void Clear()
		{ item = NULL; block = NULL; }

		// Set the tracking type
	void Track( short t ) { trackType = t; }	// set position
	
		// position query functions
	bool IsAtStart();						// true if at start of list
	bool IsAtEnd();						// true if at end of list

	void SetList( ItemList_Base *inItemList )
	{
		if (inItemList != blockList)
		{
			Clear();
			blockList = inItemList;
		}
	}
};

/* ===================================================================== *
   ItemMarker -- pointer to an item within a block. This will auto-adjust
   as edits are made.
 * ===================================================================== */

template<class BK,class Item> class ItemMarker : public ItemMarker_Base {
protected:

	ItemMarker( const ItemMarker &inSrcMarker )		// copy constructor
		: ItemMarker_Base( inSrcMarker ) {}

public:
		// A convenient typedef, grabs the item type out of the BK class
// typedef BK::Item Item;

		// constructors
	ItemMarker( ItemList<BK,Item> &inItemList )		// constructor
		: ItemMarker_Base( inItemList ) {}

	ItemMarker() : ItemMarker_Base(){}		// null constructor

		// point the position marker to a specific item
	Item *Set( long inIndex )
	{
		return (Item *)ItemMarker_Base::Set( inIndex );
	}

		// As above, but also set the tracktype
	Item *Set( int32 inIndex, short inTrackType )
	{
		Track( inTrackType );
		return (Item *)ItemMarker_Base::Set( inIndex );
	}

		// Operators to overload:
		// 	*
		// 	[]
		// 	=
		// 	== != (compare indexes)
		// 	++ --
		// 	+ - += -=
		// 	() (function call???)

		// peek forward or back from marker
	const Item *Peek( int32 inOffset ) const
	{
		return (Item *)ItemMarker_Base::Peek( inOffset );
	}

		// Synonym for peek()
	const Item &operator[]( int32 inOffset ) const
	{ return *((Item *)ItemMarker_Base::Peek( inOffset )); }

		// Seek forwards or backwards in the list
	const Item *Seek( int32 inOffset )
	{
		return (Item *)ItemMarker_Base::Seek( inOffset );
	}

		// go to first item in list
	const Item *First()
	{
		return (Item *)ItemMarker_Base::First();
	}

		// go to last item in list
	const Item *Last()
	{
		return (Item *)ItemMarker_Base::Last();
	}

		// An operator to quickly get item pointer
	const Item &operator->() const { return *(Item *)item; }	// return pointer to item
	const Item &operator* () const { return *(Item *)item; }	// return pointer to item

		// Get the address of the block
	BK *BlockAddr() { return (BK *)(BK::Upcast( block )); }

		// Insert items into list
	bool Insert( Item *inItemArray, long inNumItems, ItemListUndoAction<Item> *inSaveUndo )
	{
		return ItemMarker_Base::Insert(	inItemArray, inNumItems, inSaveUndo );
	}

		// Delete items from list
	void Remove( long inNumItems, ItemListUndoAction<Item> *inSaveUndo )		// delete items in list
	{
		ItemMarker_Base::Remove( inNumItems, inSaveUndo );
	}

		// get items into flat array
	long Get( Item *inItemArray, long inNumItems = 1 ) const
	{
		return ItemMarker_Base::Get( inItemArray, inNumItems );
	}

		// take items from flat array and write over existing items
	long Replace( Item *inItemArray, long inNumItems, ItemListUndoAction<Item> *inSaveUndo )
	{
		return ItemMarker_Base::Replace( inItemArray, inNumItems, inSaveUndo );
	}

	void SetList( ItemList<BK,Item> *inList ) { ItemMarker_Base::SetList( inList ); }

protected:
	void SetBlock( BK *inBlock )	{ ItemMarker_Base::SetBlock( BK::Downcast( inBlock ) ); }
	void SetIndex( short inIndex )	{ ItemMarker_Base::SetIndex( inIndex ); }
};

/** ===================================================================== *
	UndoItem -- stores UNDO information.
*/

class ItemListUndoAction_Base : public UndoAction {
	class CUndoIterator;

	friend class		ItemList_Base;
	friend class		UndoItem;
	friend class		CUndoIterator;

	int32			size;
	
protected:
	ItemList_Base	&list;
	DList			editList;
	
		/**	An embedded class used by subclasses to iterate through the
			undo data. The main purpose is to allow for smarter screen updates
			by peeking at the undo data. */
	class CUndoIterator {
		UndoItem		*item;
		int32		index;
		int32		itemSize;
	public:
			/**	Constructor */
		CUndoIterator( int32 inItemSize )
		{
			item = NULL;
			index = 0;
			itemSize = inItemSize;
		}
		
			/**	Returns pointer to first item. */
		void *First( ItemListUndoAction_Base &inBase )
		{
			if (inBase.editList.First() != NULL)
				item = (UndoItem *)inBase.editList.First();
			else item = NULL;
			index = 0;
			return Item();
		}
		
			/**	returns pointer to next item. */
		void *Next();
		
			/**	Returns pointer to current item. */
		void *Item();
	};
	
		/**	Return the estimated size of this undo action. */
	virtual int32 Size() { return size; }
	
		/**	Apply this undo action. */
	virtual void Undo();
	
		/**	Apply this redo action. */
	virtual void Redo();
	
public:

		/**	Constructor -- takes a list to apply changes to. */
	ItemListUndoAction_Base( ItemList_Base &inList )
		: list( inList )
	{
		size = sizeof *this;
	}

		/**	Destroy all child undo items. */
	virtual ~ItemListUndoAction_Base()
	{
	}
	
	void Rollback() { Undo(); }
};

class UndoItem : public DNode {
	friend class		ItemList_Base;
	friend class		ItemListUndoAction_Base;
	friend class		ItemListUndoAction_Base::CUndoIterator;

	enum {
		Action_Insert,
		Action_Delete,
		Action_Change
	};
	
	short			actionType;				// add, delete, or change

		// This field will be TRUE if this undo record represents the
		// first edit within a compound command.
// short			beginAction;

		// The absolute position within the item list of where the edit
		// occured.
	int32			index;
	int32			numItems;				// number of items in this set
	void				*undoData;				// actual saved undo data

	UndoItem( ItemListUndoAction_Base &inAction, long inNumItems, size_t inItemSize );

// UndoItem *Next() { return (UndoItem *)DNode::Next(); }
// UndoItem *Prev() { return (UndoItem *)DNode::Prev(); }

		// I don't like these functions being public, but I need to be able
		// to access them from the template.
public:
	~UndoItem();

	void *UndoData() { return undoData; }
	long NumItems()  { return numItems; }
};

template<class Item>
class ItemListUndoAction : public ItemListUndoAction_Base {
public:
		/**	Constructor -- takes a list to apply changes to. */
	ItemListUndoAction( ItemList_Base &inList )
		: ItemListUndoAction_Base( inList )
	{
	}

	~ItemListUndoAction()
	{
		UndoItem		*ui;
	
			// Delete any remaining undo records
		while (	(ui = (UndoItem *)editList.First() ) != NULL )
		{
			ItemFuncs<Item>::Destroy( (Item *)ui->UndoData(), ui->NumItems() );
			delete ui;
		}
	}
};

#endif /* __C_ItemList_H__ */