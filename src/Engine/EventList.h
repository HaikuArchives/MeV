/* ===================================================================== *
 * EventList.h (MeV/Engine)
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
 *  Defines basic event container with undo history
 * ---------------------------------------------------------------------
 * History:
 *	1997		Joe Pearce
 *		Original implementation
 *	04/08/2000	cell
 *		General cleanup in preparation for initial SourceForge checkin
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __C_EventList_H__
#define __C_EventList_H__

#include "Event.h"
#include "ItemList.h"
#include "Observer.h"

class CAbstractReader;
class CAbstractWriter;
class EventListUndoAction;

/* ============================================================================ *
	EventLists
 * ============================================================================ */

	// A block holding a list of events

class EventBlock : public ItemBlock<Event,128>
{
	friend class		EventList;
	friend class		EventMarker;
	friend class		ItemList<EventBlock,Event>;

private:
		// This field is set to the stop time of the event which has the last
		// stop time in the block. It can be used to quickly determine if the
		// block has any events which might overlap the current view.
	uint32				maxStopTime;

	uint32				lastTSigTime;			// time of last time signature
	TimeSig				lastTSigValue;			// value of last time signature

		// The summary data listed above is evaluated in a lazy fashion. Each
		// time we modify the data in the block, validSummaryData is set to
		// false. Whenever we attempt to access the summary data, it will recalculate
		// it if validSummaryData is set to false, and then set it to true.

	bool				validSummaryData;

// Operations
	EventBlock *Next( void ) const { return (EventBlock *)ItemBlock_Base::Next(); }
	EventBlock *Prev( void ) const { return (EventBlock *)ItemBlock_Base::Prev(); }

	void Summarize( void );
	
		// Private constructor
	EventBlock()
	{
		validSummaryData = false;
	}

public:

		// Returns the minimum time of any event in the block
	long MinTime( void ) const
	{
		return (count > 0) ? ItemAddress( 0 )->Start() : 0;
	}

		// Returns the maximum time of any event in the block.
	long MaxTime( void )
	{
		if (!validSummaryData) Summarize();
		return maxStopTime;
	}
};

class EventList : public ItemList<EventBlock,Event> {
	friend class EventMarker;

		// Override functions in itemList so that we can properly
		// copy the "extra" data in the event.

		// Copy items to new uninitialized memory
	virtual void ConstructItems( void *outDst, void *inSrc, long inItemCount );

		// Copy items over old items
	virtual void CopyItems( void *outDst, void *inSrc, long inItemCount );

		// Copy items to new uninitalized memory, destructing source items
	virtual void MoveItems( void *outDst, void *inSrc, long inItemCount );

		// Destroy items
	virtual void DestroyItems( void *outDst, long inItemCount );


	EventBlock *FirstBlock( void ) const
		{ return (EventBlock *)ItemList<EventBlock,Event>::FirstBlock(); }

	EventBlock *LastBlock( void ) const
		{ return (EventBlock *)ItemList<EventBlock,Event>::LastBlock(); }

		// Notifies subclasses that a block has changed. This can be used in case
		// of extra information associated with a block that needs to be changed.
	void OnBlockChanged( ItemBlock_Base *inChangedBlock )
	{
		((EventBlock *)inChangedBlock)->validSummaryData = false;
	}

public:
		// The time of the latest event in the sequence
	long MaxTime( void );

		// Merge a list of sorted events into the EventList
	bool Merge( Event *inEventArray, long inEventCount, EventListUndoAction *inAction );

		// Extract all selected events from a sequence and place into linear buffer.
		// Linear buffer is allocated by this function, you must delete it.
	long ExtractSelected( EventPtr &result, EventListUndoAction *inAction );

		// Copy all selected events from a sequence and place into linear buffer.
		// Linear buffer is allocated by this function, you must delete it.
	long CopySelected( EventPtr &result );

		/** Insert a block of time */
	void InsertTime( long startTime, long offset, EventListUndoAction *inAction );
	
		/** Delete a block of time. */
	void DeleteTime( long startTime, long offset, EventListUndoAction *inAction );
	
		/** Append an event to a sequence, matching with earlier events if needed...
			The list must be sorted....
		*/
	bool AppendRawEvents(	Event *inEventArray,
						long inEventCount,
						EventListUndoAction *ioAction,
						EventMarker &ioFirstUnmatchedEvent );

		// Summarize entire sequence
// void SummarizeAll( void );

#if DEBUG
	void Validate();
#endif
};

class EventMarker : public ItemMarker<EventBlock,Event>
{
		// Skip any item which is not in the range
	const Event *SkipItemsNotInRange( long minTime, long maxTime );

public:
	// Member functions for??
	// Selecting?
	// Deselecting?
	
		/**	Default constructor */
	EventMarker() : ItemMarker<EventBlock,Event>() {}

		/**	Construct and associate with list. */
	EventMarker( EventList &l ) : ItemMarker<EventBlock,Event>( l ) {}

		/**	COpy constructor */
	EventMarker( const EventMarker &r ) : ItemMarker<EventBlock,Event>( r ) {}

		/**	Sets the marker at a given time. */
	const Event *SeekForwardToTime( long time, bool fromStart = 1 );

		/**	Skip this block, and seek to the start of the next one.
			Used mainly for operating on summary data. */
	Event *NextBlock( void );

		/**	Iterators to iterate only the items with a range of times. */
	const Event *FirstItemInRange( long minTime, long maxTime );

		/**	Iterators to iterate only the items with a range of times. */
	const Event *NextItemInRange( long minTime, long maxTime );

		/**	Assignment operator. */
	EventMarker &operator=( EventMarker &em )
	{
		SetBlock( (EventBlock *)em.block );
		SetIndex( em.index );
		return em;
	}

	void SetList( EventList &l )
	{
		ItemMarker<EventBlock,Event>::SetList( &l );
	}

		/**	Replace the event with new data, and re-sort if needed. */
	void Modify( Event &newEvent, EventListUndoAction *inUndoAction );
	
		/**	Return const pointer to event. */
	operator ConstEventPtr()	{ return Peek( 0 ); }
};

class EventListUndoAction : public ItemListUndoAction<Event> {
	const char			*description;
	CObservableSubject	&subject;
	
	const char *Description() const { return description; }
	void Undo();
	void Redo();

public:
	EventListUndoAction( EventList &inList, CObservableSubject &inSubject, const char *inDescription )
		: ItemListUndoAction<Event>( inList ),
		  subject( inSubject )
	{
		description = inDescription;
	}
};

unsigned long ReadDeltaValue( CAbstractReader &reader );
unsigned long ReadFixed( CAbstractReader &reader, unsigned long maxVal );
void ReadEventList( CAbstractReader &reader, EventList &outEvents );

void WriteDeltaValue( CAbstractWriter &writer, unsigned long value );
void WriteFixed( CAbstractWriter &writer, unsigned long value, unsigned long maxVal );
void WriteEventList( CAbstractWriter &writer, EventList &inEvents );

#endif /* __C_EventList_H__ */