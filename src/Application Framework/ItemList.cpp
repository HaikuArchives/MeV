/* ===================================================================== *
 * ItemList.cpp (MeV/Application Framework)
 * ===================================================================== */

#include "ItemList.h"

/* ===================================================================== *
   Dummy class for measuring offsets
 * ===================================================================== */

	// A dummy class who's sole purpose is to figure out where the array of items
	// would be in a real class derived from ItemBlock_Base;

class ItemBlock_Metric : private ItemBlock_Base {
private:
	ItemBlock_Metric( void ) {}		// private constructor, can't make one of these
	char			dummyArray[ 1 ];

// const int		offset = offsetof( ItemBlock_Metric, dummyArray );
public:
	static char *address( ItemBlock_Base *b, long index, long itemSize )
	{
		return ( &((ItemBlock_Metric *)b)->dummyArray[ index * itemSize ] );
	}
};

/* ===================================================================== *
   ItemList_Base member functions
 * ===================================================================== */

	// constructor / destructor
ItemList_Base::ItemList_Base( short inItemSize, short inItemsPerBlock )
{
	itemSize = inItemSize;
	itemsPerBlock = inItemsPerBlock;
	count = blockCount = 0;
}

ItemList_Base::~ItemList_Base()
{
	ItemBlock_Base	*bk;

		// Delete any remaining blocks
	while (	(bk = (ItemBlock_Base *)blocks.RemHead()) != NULL )
	{
		ItemMarker_Base *marker;

			// Unlink all markers to this block, and clear them
		while ( (marker = (ItemMarker_Base *)bk->markers.RemHead()) != NULL )
		{
			marker->Clear();
		}

/*		DestroyItems( ItemBlock_Metric::address( bk, 0, 0 ), bk->count );
		delete bk; */
	}
}
	
	// This routine moves as much data from the next few blocks as
	// possible. If the blocks are emptied as a result of this, then
	// they are deleted. Markers are also adjusted.

void ItemList_Base::Compact( ItemBlock_Base *block )
{
		// While there is data to move:
	for (;;)
	{
			// Amount of space left in the block.
		long				availSpace = itemsPerBlock - block->count,
						copyCount;
		ItemMarker_Base	*next;

			// Address of next block
		ItemBlock_Base	*nextBlock = block->Next();
		ItemMarker_Base	*marker;
		
			// If no space, or no next block, then break.
		if (availSpace <= 0 || nextBlock == NULL) break;
		
			// Can't copy more data than what is in the block.
		copyCount = MIN( availSpace, nextBlock->count );
		if (copyCount <= 0) break;
		
			// Copy the data from the start of the next block to the
			// end of this one.
		MoveItems(	ItemBlock_Metric::address( block, block->count, itemSize ),
					ItemBlock_Metric::address( nextBlock, 0, itemSize ),
					copyCount );
					
		block->count += (short)copyCount;
		OnBlockChanged( block );

			// Copy the data from the end of the next block to the
			// start of the next block
		nextBlock->count -= (short)copyCount;
		if (nextBlock->count > 0)
		{
			MoveItems(	ItemBlock_Metric::address( nextBlock, 0, itemSize ),
						ItemBlock_Metric::address( nextBlock, copyCount, itemSize ),
						nextBlock->count );
		}

			// Update all marker positions from the next block
		for (	marker = nextBlock->FirstMarker(); marker != NULL; marker = next)
		{
			next = (ItemMarker_Base *)marker->Next();
		
				// If marker points to data that was copied
			if (marker->index < copyCount)
			{
					// Move marker to this block, and set index to copied data
				marker->SetBlock( block );
				marker->SetIndex( marker->index + block->count - (short)copyCount );
			}
			else
			{		// If marker is still in next block

					// If next block is going to be deleted
				if (nextBlock->count == 0)
				{
						// If there's a next block after that
					if (nextBlock->Next())
					{
							// Set to start of next block
						marker->SetBlock( nextBlock->Next() );
						marker->SetIndex( 0 );
					}
					else	// There's no next block
					{
							// Set to end of this block
						marker->SetBlock( block );
						marker->SetIndex( block->count );
					}
				}
				else
				{
						// Next block isn't empty, so subtract the
						// amount copied from the marker's position.
					marker->SetIndex( marker->index - (short)copyCount );
				}
			}
		}

		if (nextBlock->count == 0)
		{
			nextBlock->Remove();
			delete nextBlock;
		}
		else
		{
			OnBlockChanged( nextBlock );
			break;
		}
	}
}

	// split block into two blocks

bool ItemList_Base::Split( ItemBlock_Base *block, short index )
{
	ItemBlock_Base	*newBlk;
#if 0
	ItemMarker_Base	*marker,
					*next;
#endif
	short			copyCount;

	copyCount = block->count - index;
	if (copyCount <= 0) return false;
	
	newBlk = (ItemBlock_Base *)NewBlock();	// alloc block to hold spit
	newBlk->count = 0;
	block->InsertAfter( newBlk );
	
	MoveItems(	ItemBlock_Metric::address( newBlk, 0, itemSize ),
				ItemBlock_Metric::address( block, index, itemSize ),
				copyCount );

		// Adjust item counts for both blocks
	block->count -= copyCount;
	newBlk->count = copyCount;

	OnBlockChanged( block );

#if 0
		// Update all marker positions in this next block
	for (	marker = block->FirstMarker(); marker != NULL; marker = next)
	{
		next = (ItemMarker_Base *)marker->Next();

			// If marker points to data that was copied
		if (marker->index > index)
//			|| 	(marker->index == index
//				&& marker->trackType == ItemMarker_Base::Track_Next))
		{
				// Move marker to next block, and set index to copied data
			marker->SetBlock( newBlk );
			marker->SetIndex( marker->index - block->count );
		}
	}
#endif
	return true;
}

long ItemList_Base::Replace(
	ItemMarker_Base	*where,
	void				*list,
	long				inItemCount,
	ItemListUndoAction_Base *inSaveUndo )	
{
	ItemBlock_Base	*blk;					// pointer to block item is in
	UndoItem			*un;
	char				*unData = NULL,
					*srcData = (char *)list;
	long				actual = 0,
					start = where->index;
				
	if (inSaveUndo)
	{
		un = new UndoItem( *inSaveUndo, inItemCount, itemSize );
		unData = (char *)un->undoData;
		un->actionType = UndoItem::Action_Change;
		un->index = where->AbsIndex();
	}

	for (blk = where->block; blk && actual < inItemCount; blk = blk->Next())
	{
			// Calculate the number of items in the rest of the block,
			// but limit it to the number of items we want.
		long		copyCount = MIN( blk->count - start, inItemCount - actual );
		long		copyBytes = copyCount * itemSize;
		
			// If there's an undo record, then copy the items that we're
			// going to copy over to it.
		if (unData)
		{
			ConstructItems(	unData,
							ItemBlock_Metric::address( blk, start, itemSize ),
							copyCount );

			unData += copyBytes;
		}

			// Copy the items from the block to the array.
		CopyItems(	ItemBlock_Metric::address( blk, start, itemSize ),
					srcData,
					copyCount );
		
		OnBlockChanged( blk );

			// Update the pointers and count of items copied
		actual += copyCount;				// increment the count of copied items
		srcData += copyBytes;				// increment the list pointer
		start = 0;							// start at beginning of next block
	}
	return actual;
}

	// Swap the contents of a buffer and a range of data in the list. This is
	// used for undoing a change operation. No new undo record is made.

	// Again, the special "copy" functions are not used, since the number
	// of items is preserved.
long ItemList_Base::Swap(
	ItemMarker_Base	*where,
	void				*list,
	long				inItemCount )
{
	ItemBlock_Base	*blk;					// pointer to block item is in
	long				actual = 0,
					start = where->index;
	char				*srcData = (char *)list;
				
	for (blk = where->block; blk && actual < inItemCount; blk = blk->Next())
	{
			// Calculate the number of items in the rest of the block,
			// but limit it to the number of items we want.
		long		copyCount = MIN( blk->count - start, inItemCount - actual ),
					copyBytes = copyCount * itemSize;
		char *dstData = ItemBlock_Metric::address( blk, start, itemSize );

		for (int i = 0; i < copyBytes; i++)
		{
			char	t = *dstData;
			*dstData++ = *srcData;
			*srcData++ = t;
		}
	
		OnBlockChanged( blk );
		actual += copyCount;				// increment the count of copied items
		start = 0;							// start at beginning of next block
	}
	return actual;
}

	// Removes items from the main list and places them in the undo list.
	// The special copy functions are not used, since the items are merely
	// moved, not created or destroyed.

void ItemList_Base::Remove(
	ItemMarker_Base	*where,
	long				inItemCount,
	ItemListUndoAction_Base *inSaveUndo )
{
	ItemBlock_Base	*blk,					// pointer to block item is in
					*nextBlk,
					*cBlk;
	UndoItem			*un;
	ItemMarker_Base	*marker,
					*next;
	char				*unData = NULL;
	long				actual = 0,
					start = where->index;

	if (where->block == NULL) return;

		// Build an undo record for this action
	if (inSaveUndo)
	{
		un = new UndoItem( *inSaveUndo, inItemCount, itemSize );
		unData = (char *)un->undoData;
		un->actionType = UndoItem::Action_Delete;
		un->index = where->AbsIndex();
	}

	cBlk = where->block;
	for (blk = where->block; blk; blk = nextBlk)
	{
		nextBlk = blk->Next();

			// Calculate the number of items in the rest of the block,
			// but limit it to the number of items we want.
		long		copyCount = MIN( blk->count - start, inItemCount - actual );

			// If there's an undo record, then copy the items that we're
			// going to copy over to it.
		if (unData)
		{
			MoveItems(	unData,
						ItemBlock_Metric::address( blk, start, itemSize ),
						copyCount );
			unData += copyCount * itemSize;
		}
		else
		{
			DestroyItems( ItemBlock_Metric::address( blk, start, itemSize ), copyCount );
		}
		
			// If there's data in this block after the deleted material,
			// then move it down
		if (start + copyCount < blk->count)
		{
			MoveItems(	ItemBlock_Metric::address( blk, start, itemSize ),
						ItemBlock_Metric::address( blk, start + copyCount, itemSize ),
						blk->count - start - copyCount );
		}

		blk->count -= (short)copyCount;		// subtract from length of block
		count -= copyCount;
		OnBlockChanged( blk );

			// Adjust all markers in this block
		for (marker = blk->FirstMarker(); marker != NULL; marker = next)
		{
			next = (ItemMarker_Base *)marker->Next();

				// If all the data was removed from the block
			if (blk->count <= 0)
			{
				if (blk->Next())
				{
						// attach to next block
					marker->SetBlock( blk->Next() );
					marker->SetIndex( 0 );
				}
				else if (blk->Prev())
				{
						// attach to previous block
					marker->SetBlock( blk->Prev() );
					marker->SetIndex( blk->Prev()->count );
				}
				else
				{
						// If there's no items left, eliminate marker
					marker->SetBlock( NULL );
				}
			}
			else if (marker->index >= start)
			{
					// Adjust the marker pos
				if (marker->index < start + copyCount)
					marker->SetIndex( (short)start );
				else marker->SetIndex( marker->index - (short)copyCount );
				
					// If the marker is at the end of the block, then
					// try moving it to the next block.
			
				if (marker->index >= blk->count && blk->Next())
				{
					marker->SetBlock( blk->Next() );
					marker->SetIndex( marker->index - blk->count );
				}
			}
		}
		
		if (blk->count == 0)
		{
			if (cBlk == blk) cBlk = nextBlk;
			blk->Remove();
			delete blk;
		}
		
		actual += copyCount;				// increment the count of copied items
		start = 0;							// start at beginning of next block
		if (actual >= inItemCount) break;
	}

	if (cBlk) Compact( cBlk );
}

	// Insert data into the list. This is the most complicated one.
bool ItemList_Base::Insert(
	ItemMarker_Base	*where,
	void				*items,
	long				inItemCount,
	ItemListUndoAction_Base *inSaveUndo )
{
	ItemBlock_Base	*blk,					// pointer to block item is in
					*prevBlock,				// block before insertion
					*baseBlock,				// first block of all
					*nextBlock;			// block after insertion.
	ItemMarker_Base	*marker,				// marker pointer
					*next;
	UndoItem			*un = NULL;				// undo record
	char				*unData = NULL;			// undo data
	long				actual = 0;				// actual length transferred
	long				insertIndex;
	DList			newBlocks;				// list new new blocks
	char				*srcData = (char *)items;
					
	if (inItemCount <= 0) return true;

		// If the item list is currently empty, then allocate a new block
		// for it.
	if (blocks.Empty())
	{
			// build a new block
		blk = (ItemBlock_Base *)NewBlock();
		blk->count = 0;
		blocks.AddTail( blk );

			// set the marker to the beginning of the list.
		where->First();
	}
	
		// If the marker is pointing to nothing, or there's no data, then quit.
	if (	where->block == NULL) return false;

		// Build an undo record for this action
	if (inSaveUndo)
	{
		un = new UndoItem( *inSaveUndo, inItemCount, itemSize );
		unData = (char *)un->undoData;
		un->actionType = UndoItem::Action_Insert;
		un->index = where->AbsIndex();
	}
	
		// If the entire insertion will fit in the current block, then
		// we do that.
	blk = where->block;
	if (inItemCount + blk->count <= itemsPerBlock)
	{
			// Open a hole for the new items.
		MoveItems(	ItemBlock_Metric::address( blk, where->index + inItemCount, itemSize ),
					ItemBlock_Metric::address( blk, where->index, itemSize ),
					blk->count - where->index );

			// Copy in the new items
		ConstructItems(	ItemBlock_Metric::address( blk, where->index, itemSize ),
						srcData,
						inItemCount );
		blk->count += (short)inItemCount;
		count += inItemCount;
		OnBlockChanged( blk );

			// Keep the undo data
		if (unData) ConstructItems( unData, items, inItemCount );
					
			// Adjust all markers in this block

		for (	marker = blk->FirstMarker();
				marker != NULL;
				marker = (ItemMarker_Base *)marker->Next() )
		{
			if (	marker->index > where->index
				||	(marker->index == where->index
						&& marker->trackType == ItemMarker_Base::Track_Next))
			{
				marker->SetIndex( marker->index + (short)inItemCount );
			}
		}
		
			// One more test. There's a faint possibility of a marker
			// at the end of the last block.
			
		if (where->index == 0 && blk->Prev())
		{
			ItemBlock_Base	*prevBlk;
			ItemMarker_Base	*next;
	
			prevBlk = blk->Prev();

				// Adjust marker at end of previous block, if any

			for (marker = prevBlk->FirstMarker(); marker != NULL; marker = next)
			{
				next = (ItemMarker_Base *)marker->Next();

				if (marker->index >= prevBlk->count)
				{
					marker->SetBlock( blk );
					marker->SetIndex(
						(short)(marker->trackType == ItemMarker_Base::Track_Next
							? inItemCount : 0) );
				}
			}
		}
		return true;
	}
	
		// To avoid problems with allocating memory, I'm going to do a
		// strange thing, which is to allocate all of the blocks for
		// the insertion before actually inserting anything.

		// First, let's split the block we're working on right at the point
		// where we want to do the insertion.
		
	baseBlock = prevBlock = where->block;
	insertIndex = where->index;
	long		ct = inItemCount;

		// Split the block for insertion. Note that if the insertion point is at the
		// end of the block, then there is in fact no split. The reason for this is
		// because we're going to add a new block after this one anyway.
		// (Avoids an earlier bug which causes occasional creation of
		// size-0 blocks.)
	if (where->index < itemsPerBlock)
	{
		Split( baseBlock, where->index );

			// Assume that some of the new items will be placed in the first block
			// after the split point. Note that items could also be placed in the next
			// block, before the split. This is enough to take care of 99% of all
			// insertions.
		ct -= (itemsPerBlock - where->index);
	}

		// Now allocate enough new blocks for the amount of data we're
		// inserting.
	while (ct > 0)
	{
			// build a new block
		blk = (ItemBlock_Base *)NewBlock();
		
			// Initialize new block
		blk->count = 0;
		newBlocks.AddTail( blk );

			// Subtract from total items inserted
		ct -= itemsPerBlock;
	}
	
		// Address of the block after the base block, regardless of whether it's the split block or not.
	nextBlock = baseBlock->Next();

		// Now, copy data into the initial block after the split
	if (where->index < itemsPerBlock)
	{
		long		copyCount = MIN( inItemCount, itemsPerBlock - where->index );
		
			// Copy in the data, and set the block size
		ConstructItems(	ItemBlock_Metric::address( baseBlock, where->index, itemSize ),
						srcData,
						copyCount );
		baseBlock->count += (short)copyCount;
		count += copyCount;
		
			// Also, move a copy to the undo area.
		if (unData)
		{
			ConstructItems( unData, srcData, copyCount );
			unData += copyCount * itemSize;
		}
		
		srcData += copyCount * itemSize;
		actual += copyCount;
	}

		// Now, copy the data into the new blocks.
	while (actual < inItemCount)
	{
		long		copyCount = MIN( inItemCount - actual, itemsPerBlock );
		
			// Get the head of the block list.
		blk = (ItemBlock_Base *)newBlocks.RemHead();

			// Insert the block after the previous block
		prevBlock->InsertAfter( blk );
		
			// Copy in the data, and set the block size
		ConstructItems(	ItemBlock_Metric::address( blk, 0, itemSize ),
						srcData,
						copyCount );
		blk->count = (short)copyCount;
		count += copyCount;
		
			// Also, move a copy to the undo area.
		if (unData)
		{
			ConstructItems( unData, srcData, copyCount );
			unData += copyCount * itemSize;
		}
		
		srcData += copyCount * itemSize;
		prevBlock = blk;
		actual += copyCount;
	}

	if (where->index >= itemsPerBlock)
	{	
			// Since we didn't insert anything into the first block, it means that
			// the start of the insertion is going to be at the start of the first new block.
		baseBlock = baseBlock->Next();
		insertIndex = 0;
	}
	
	int32 index = where->index;
	
		// Adjust all markers. Markers will not have been affected at this point (they
		// used to be affected by the split block operation, but no more...
	for (marker = where->block->FirstMarker(); marker != NULL; marker = next)
	{
		next = (ItemMarker_Base *)marker->Next();

			// If marker points to data that was copied
		if (marker->index > index
			|| 	(marker->index == index
				&& marker->trackType == ItemMarker_Base::Track_Next))
		{
				// if there was a split, then we need to move all of these items into the
				// split block; otherwise, if there was NOT a split, then they need to go
				// into the "next" block; otherwise, if there is no next block then leave them here...
			if (nextBlock)
			{
					// Move marker to next block, and set index to copied data
				marker->SetBlock( nextBlock );
				marker->SetIndex( marker->index - index );
			}
			else
			{
					// Move marker to last block in insertion
				marker->SetBlock( blk );
				marker->SetIndex( blk->count );
			}
		}
		else if (marker->index == index
				&& marker->trackType == ItemMarker_Base::Track_Previous)
		{
			marker->SetBlock( baseBlock );
			marker->SetIndex( insertIndex );
		}
	}
	
#if 0
		// Adjust all markers. Most markers will already have been
		// adjusted by the split. However if a marker is tracking
		// previous events, then we need to move it to the start
		// of the first new data block.
		
	for (marker = where->block->FirstMarker(); marker != NULL; marker = next)
	{
		next = (ItemMarker_Base *)marker->Next();
		
			// If there was a split, then what we are iterating through is
			// The block that comes after the insertion, i.e. the split block.
			
			// What we're doing is moving markers from the beginning of
			// the split block to the beginning of the insertion.
		if (	marker->index == 0
			&&	marker->trackType == ItemMarker_Base::Track_Previous)
		{
			marker->SetBlock( baseBlock );
			marker->SetIndex( insertIndex );
		}
	}
#endif
	return true;
}

#if DEBUG
void ItemList_Base::Validate()
{
	for (ItemBlock_Base *b = FirstBlock(); b; b = b->Next() )
	{
		ItemMarker_Base	*marker;
		
		if (b != FirstBlock()) VERIFY( b->count > 0 );
		
		for (	marker = b->FirstMarker();
				marker;
				marker = (ItemMarker_Base *)marker->Next() )
		{
			VERIFY( marker->blockList == this );
			VERIFY( marker->block == b );
			VERIFY( marker->index >= 0 );
			VERIFY( marker->index <= b->count );
		}
	}
}
#endif

bool ItemList_Base::IsEmpty() const
{
	for (ItemBlock_Base *b = FirstBlock(); b; b = b->Next() )
	{
		if (b->count > 0) return false;
	}
	return true;
}

/* ===================================================================== *
   ItemMarker_Base member functions.
 * ===================================================================== */

	// REM: We might want to have the constructor default to pointing
	// to the first item in the list (if there is one).

	// constructor
ItemMarker_Base::ItemMarker_Base( ItemList_Base &l )
	: blockList( &l )
{
	item = NULL;
	block = NULL;
	index = 0;
	trackType = Track_Next;
}

	// copy constructor
ItemMarker_Base::ItemMarker_Base( const ItemMarker_Base &r )
{
		// Link into the marker list of that block.
	blockList = r.blockList;
	block = NULL;
	SetBlock( r.block );

	item = r.item;
	index = r.index;
	trackType = r.trackType;
}

	// NULL constructor
ItemMarker_Base::ItemMarker_Base()
{
		// zero out fields
	blockList = NULL;
	item = NULL;
	block = NULL;
	index = 0;
	trackType = Track_Next;
}

	// destructor
ItemMarker_Base::~ItemMarker_Base()
{
		// Unlink from block list
	SetBlock( NULL );
}

void ItemMarker_Base::SetBlock( ItemBlock_Base *bk )
{
	if (block != bk)
	{
		if (block)	DNode::Remove();
		if (bk)		bk->markers.AddTail( this );
		block = bk;
	}
}

	// change the index and ptr
void ItemMarker_Base::SetIndex( short n )
{
	index = n;
	item = (blockList == NULL || block == NULL || n >= block->count) ?
		NULL : (void *)(ItemBlock_Metric::address( block, n, blockList->itemSize ));
}

	// return the absolute item number in the list
long ItemMarker_Base::AbsIndex() const
{
	ItemBlock_Base	*b;
	long			pos = 0;
	
	if (block == NULL || blockList == NULL) return 0;
	
	for (b = blockList->FirstBlock(); b && b != block; b = b->Next())
	{
		pos += b->count;
	}
	return pos + index;
}

	// peek forward or backwards some items
void *ItemMarker_Base::Peek( signed long offset ) const
{
	ItemBlock_Base	*b = block;
	signed long		pos = index + offset;
	
	if (block == NULL || blockList == NULL) return NULL;

	if (pos >= b->count)
	{
		while (pos >= b->count)
		{
			pos -= b->count;
			b = b->Next();
			if (b == NULL) return NULL;
		}
	}
	else if (pos < 0)
	{
		while (pos < 0)
		{
			b = b->Prev();
			if (b == NULL) return NULL;
			pos += b->count;
		}
	}
	
	return (void *)( ItemBlock_Metric::address( block, pos, blockList->itemSize ) );
}

	// seek forward or backwards in the list
void *ItemMarker_Base::Seek( signed long offset )
{
	ItemBlock_Base	*b = block;
	signed long		pos = index + offset;
	
	if (pos >= b->count)
	{
		while (pos >= b->count)
		{
			if (b->Next() == NULL)
			{
				SetBlock( b );
				SetIndex( b->count );
				return NULL;
			}
			pos -= b->count;
			b = b->Next();
		}
		SetBlock( b );
	}
	else if (pos <= 0)
	{
		while (pos <= 0)
		{
			if (b->Prev() == NULL)
			{
				SetBlock( b );
				SetIndex( 0 );
				return NULL;
			}
			b = b->Prev();
			pos += b->count;
		}
		SetBlock( b );
	}
	
	SetIndex( (short)pos );
	return item;
}

	// Set to the first item in list
void *ItemMarker_Base::First()
{
	if (blockList == NULL)
	{
		item = NULL;
		return NULL;
	}

	SetBlock( blockList->FirstBlock() );
	if (block)
	{
		SetIndex( 0 );
		return item;
	}
	else
	{
		item = NULL;
		return NULL;
	}
}

	// Set to after the last item in list
void *ItemMarker_Base::Last()
{
	if (blockList == NULL)
	{
		item = NULL;
		return NULL;
	}

	SetBlock( blockList->LastBlock() );
	if (block) SetIndex( block->count );
	else item = NULL;

	return NULL;
}

	// point the position marker to a specific item
	// REM: This can be optimized a bit...
void *ItemMarker_Base::Set( long index )
{
	First();
	if (index == 0 && block == NULL) return NULL;
	return Seek( index );
}

	// true if at start of list
bool ItemMarker_Base::IsAtStart()
{
	return (blockList && block == blockList->FirstBlock() && (block == NULL || index <= 0));
}

	// true if at end of list or if pointing to "no object"
bool ItemMarker_Base::IsAtEnd()
{
	return (block == NULL
			|| blockList == NULL
			|| (block == blockList->LastBlock() && index >= block->count) );
}

	// Get subset of items
long ItemMarker_Base::Get( void *list, long inItemCount ) const
{
	ItemBlock_Base	*blk;					// pointer to block item is in
	long			actual = 0,
					start = index;
					
	if (blockList == NULL || block == NULL) return 0;

	for (blk = block; blk && actual < inItemCount; blk = blk->Next())
	{
			// Calculate the number of items in the rest of the block,
			// but limit it to the number of items we want.
		long		copyCount = MIN( blk->count - start, inItemCount - actual );
		
			// Copy the items from the block to the array.
		blockList->MoveItems(	list,
								ItemBlock_Metric::address( block, start, blockList->itemSize ),
								copyCount );
		
		actual += copyCount;				// increment the count of copied items
		list = (char *)list + copyCount * blockList->itemSize;	// increment the list pointer
		start = 0;							// start at beginning of next block
	}
	return actual;
}

/* ===================================================================== *
   UndoItem member functions.
 * ===================================================================== */

UndoItem::UndoItem( ItemListUndoAction_Base &inAction, long itemCount, size_t itemSize )
{
	undoData = (void *)(new char [ itemCount * itemSize ]);

	numItems = itemCount;
	inAction.editList.AddTail( this );
	inAction.size += sizeof *this + itemCount * itemSize;
}

UndoItem::~UndoItem()
{
	Remove();
	if (undoData) delete undoData;
}

	// undo last action
void ItemListUndoAction_Base::Undo()
{
	ItemMarker_Base	iPos( list );
	
	for (DNode *d = editList.Last(); d != NULL; d = d->Prev())
	{
		UndoItem		*uItem = (UndoItem *)d;

		iPos.Set( uItem->index );

		switch (uItem->actionType) {
	    case UndoItem::Action_Delete:
			list.Insert( &iPos, uItem->undoData, uItem->numItems, NULL );
			break;

	    case UndoItem::Action_Insert:
			list.Remove( &iPos, uItem->numItems, NULL );
			break;

	    case UndoItem::Action_Change:
			list.Swap( &iPos, uItem->undoData, uItem->numItems );
			break;
		}
	}
}

	// redo last action
void ItemListUndoAction_Base::Redo()
{
	ItemMarker_Base	iPos( list );

	for (DNode *d = editList.First(); d != NULL; d = d->Next())
	{
		UndoItem		*uItem = (UndoItem *)d;
		
		iPos.Set( uItem->index );
		switch (uItem->actionType) {
	    case UndoItem::Action_Insert:
			list.Insert( &iPos, uItem->undoData, uItem->numItems, 0);
	        break;

	    case UndoItem::Action_Delete:
			list.Remove( &iPos, uItem->numItems, 0);
	        break;

	    case UndoItem::Action_Change:
			list.Swap( &iPos, uItem->undoData, uItem->numItems );
	        break;
		}
	}
}

void *ItemListUndoAction_Base::CUndoIterator::Next()
{
	if (item == NULL) return NULL;
	if (++index >= item->numItems)
	{
		DNode		*d = item->Next();
	
		index = 0;
		if (d == NULL)
		{
			item = NULL;
			return NULL;
		}
		item = (UndoItem *)d;
	}
	return (uint8 *)item->undoData + index * itemSize;
}
		
void *ItemListUndoAction_Base::CUndoIterator::Item()
{
	if (item == NULL) return NULL;
	return (uint8 *)item->undoData + index * itemSize;
}
