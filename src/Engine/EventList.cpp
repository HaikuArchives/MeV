/* ===================================================================== *
 * EventList.cpp (MeV/Engine)
 * ===================================================================== */

#include "EventList.h"
#include "Writer.h"
#include "Reader.h"

// Gnu C Library
#include <stdio.h>

// ---------------------------------------------------------------------------
// Compute latest stop time of all events in block

void EventBlock::Summarize( void )
{
	const Event		*ev;
	long			maxTime = 0;
	int				i;

	for (	i = 0, ev = ItemAddress( 0 );
			i < count;
			ev++, i++ )
	{
		if (ev->HasProperty( Event::Prop_Duration ))
			maxTime = maxTime > ev->Stop() ? maxTime : ev->Stop();
		else maxTime = maxTime > ev->Start() ? maxTime : ev->Start();
	}

	maxStopTime = maxTime;
	validSummaryData = true;
}

// ---------------------------------------------------------------------------
// Set a marker at a given time

const Event *EventMarker::SeekForwardToTime( long time, bool fromStart )
{
	EventBlock		*b, *goodBlock = NULL;

	if (fromStart) b = (EventBlock *)FirstBlock();
	else b = (EventBlock *)block;

		// For each block, look for a block who's last event has a time greater than
		// or equal to the time we are searching for.
	for (; b; b = (EventBlock *)b->Next() )
	{
		const Event	*ev;

			// If block has no entries, then skip
		if (b->count > 0)
		{
				// Get address of last item in the block
			ev = b->ItemAddress( b->count - 1 );

				// Store the address of the last "good" block that we found.
			goodBlock = b;

				// If the start time of the last item in the block is less than
				// the time we are searching for, then continue; Else if the
				// start timer is greater or equal, then we've found the block
				// we're looking for.
			if (ev->Start() >= time) break;
		}
		fromStart = 1;
	}

	if (goodBlock == NULL) Last();
	else
	{
		const Event	*ev;
		int16		i = fromStart ? 0 : index;

			// We found the correct block. Now, linearly search for the first
			// item which has a time greater than or equal to the given time.
			// (REM: Later we could speed this up by making a BSearch)
		for (ev = goodBlock->ItemAddress( i ); i < goodBlock->count; i++, ev++)
		{
			if (ev->Start() >= time) break;
		}

			// Set the block and index within the block to point to
			// the event we found.
		SetBlock( goodBlock );
		SetIndex( i );
	}

	return (Event *)item;
}

// ---------------------------------------------------------------------------
// Skip this block, and seek to the start of the next one.

Event *EventMarker::NextBlock()
{
	EventBlock		*b = (EventBlock *)block;

		// Look for the next block which actually has items in it.
	for (	b = (EventBlock *)b->Next();
			b;
			b = (EventBlock *)b->Next() )
	{
		if (b->count == 0) continue;

		SetBlock( b );
		SetIndex( index );
		return (Event *)item;
	}

	SetBlock( NULL );
	return NULL;
}

// ---------------------------------------------------------------------------
// Skip any item which is not in the range of time between minTime and maxTime.
// Note that this properly handles long-duration events which may begin before
// the range of time.

const Event *EventMarker::SkipItemsNotInRange( long minTime, long maxTime )
{
	EventBlock		*b = (EventBlock *)block;

	for (;;)
	{
			// If we are looking at a new block, then check to see if
			// that block is in our range, in other words if that block's
			// start and end times (aggregated from all of the events in the
			// block) overlap the search range.
		if (index == 0)
		{
			for (;;)
			{
					// Since blocks are sorted by minTime, we need search
					// no further if we find a block later than the range
					// specified.
				if (	(b == NULL)
					||	(b->MinTime() > maxTime) ) return NULL;

					// If we find a block where all events in the block
					// terminate earlier than the minTime, then we need
					// not look at it.
				if (b->MaxTime() < minTime)
				{
					b = (EventBlock *)b->Next();
					if (b == NULL) return NULL;
				}
				else break;
			}
		}

			// Scan through all the items in the block, looking for ones
			// in our range.
		for (int i = index; i < b->count; i++)
		{
			const Event *ev = b->ItemAddress( i );
	
			if (ev->HasProperty( Event::Prop_Duration ))
			{
					// If the event has a stop time, and the event stops before
					// the range, then skip it.
				if (ev->Stop() < minTime) continue;
			}
			else
			{
					// If the event has no duration, and the event occurs before
					// the range, then skip it.
				if (ev->Start() < minTime) continue;
			}

				// If the event starts after the range, then quit
			if (ev->Start() > maxTime) return NULL;

				// Remember the last event we searched, and then quit.
			SetBlock( b );
			SetIndex( i );
			return (Event *)item;
		}

			// Skip to the next block.
		b = (EventBlock *)b->Next();
		if (b == NULL) return NULL;					// no items are visible
		index = 0;
	}
}

// ---------------------------------------------------------------------------
// Return the first item in the specified range

const Event *EventMarker::FirstItemInRange( long minTime, long maxTime )
{
	First();
	return SkipItemsNotInRange( minTime, maxTime );
}

// ---------------------------------------------------------------------------
// Return the next item in the specified range

const Event *EventMarker::NextItemInRange( long minTime, long maxTime )
{
	Seek( 1 );
	return SkipItemsNotInRange( minTime, maxTime );
}

// ---------------------------------------------------------------------------
// Replace a musical event, and if the time changed, re-insert in correct position.
// Note that summary data is automatically invaliated by this operation.

void EventMarker::Modify( Event &newEvent, EventListUndoAction *inUndoAction )
{
	const Event		*ev = Peek( 0 );		// address of event

		// If they are the same, don't bother.
	if (memcmp( ev, &newEvent, sizeof (Event) ) == 0) return;

	if (newEvent.common.start != ev->common.start)
	{
		const Event	*nextEv = Peek( 1 ),
					*prevEv = Peek( -1 );

			// If the start time of this event is between the start times of
			// the two adjacent events, then we need not move this event.
		if (   (nextEv == NULL || nextEv->common.start >= newEvent.common.start)
			&& (prevEv == NULL || prevEv->common.start <= newEvent.common.start))
		{
			Replace( &newEvent, 1, inUndoAction );
		}
		else
		{
			EventMarker	temp( *this );

			Remove( 1, inUndoAction );			// delete this event.

				// Insert at the new time. (Seek from start)
			temp.SeekForwardToTime( newEvent.common.start, true );
			temp.Insert( &newEvent, 1, inUndoAction );
		}
	}
	else
	{
		Replace( &newEvent, 1, inUndoAction );
	}
}

// ---------------------------------------------------------------------------
// Merge a list of sorted events into the EventList, preserving the
// order of items in the list.
//
// Modified to optimze event insertion by inserting events in clumps where
// possible.

bool EventList::Merge( Event *inEvents, long count, EventListUndoAction *inAction )
{
	bool			status = true;
	EventMarker		marker( *this );
	int				i;

	marker.First();
	marker.Track( EventMarker::Track_Next );

		// Insert each event in it's proper order in time.
	while (status != false && count > 0)
	{
		marker.SeekForwardToTime( inEvents->Start(), false );
		if (marker.IsAtEnd())
		{
				// If we're at the end of the list, insert all events
#if DEBUG
			status = marker.Insert( inEvents, count, inAction );
	
			Validate();
			return status;
#else
			return marker.Insert( inEvents, count, inAction );
#endif
		}
		
			// See how many events can be inserted before the next
			// event in the list.
		for (i = 1; i < count; i++)
		{
			if (inEvents[ i ].Start() > ((ConstEventPtr)marker)->Start())
				break;
		}
		//i removed this debug statment dan
		//printf( "marker.Insert partial\n" );
		status = marker.Insert( inEvents, i, inAction );
		inEvents += i;
		count -= i;
	}

#if DEBUG
	Validate();
#endif
	return status;
}

// ---------------------------------------------------------------------------
// Extract all selected events into an allocated list.

long EventList::ExtractSelected( EventPtr &result, EventListUndoAction *inAction )
{
	EventMarker		marker( *this ),
					firstSelected( *this );
	const Event		*ev;
	Event			*list,
					*put;
	long				count = 0,
					index;

		// First, count how many selected events there are.
		// REM: A firstSelected hint could speed this up...
	for ( ev = marker.First(); ev; ev = marker.Seek( 1 ) )
	{
		if (ev->IsSelected())
		{
			if (count == 0) firstSelected = marker;
			count++;
		}
	}

		// Make a list of the number of selected events
	list = new Event[ count ];
	put = list;
	marker = firstSelected;

		// Iterate through only that part of the sequence that contained
		// selected events.
	for ( ev = (ConstEventPtr)marker, index = 0; index < count; )
	{
			// For each selected event
		if (ev->IsSelected())
		{
			*put++ = *(Event *)ev;		// Copy to temporary list
			marker.Remove( 1, inAction );	// Delete from original list
			ev = (ConstEventPtr)marker;	// Increment to next event
			index++;
		}
		else ev = marker.Seek( 1 );
	}

	result = list;
	return count;
}

// ---------------------------------------------------------------------------
// Append an event to a sequence, matching with earlier events if needed...
// This function is useful in importing from a MIDI file, and in recording.

bool EventList::AppendRawEvents(
	Event				*inEvents,
	long					inEventCount,
	EventListUndoAction	*ioAction,
	EventMarker			&ioFirstUnmatchedEvent )
{
	EventMarker			marker( *this );
	const Event			*ev;
	bool					status = true;
	
	if (inEventCount < 1) return true;

	marker.Track( EventMarker::Track_Next );
	ioFirstUnmatchedEvent.Track( EventMarker::Track_Previous );

		// Move the marker to the end of the sequence.
		// If there's a last event, and the last event is after the event being inserted,
		// then seek to the correct insertion point.
		// (We do it this way because of the assumption that most appends are going
		// to be at the end of the sequence.)
	marker.Last();
	if ((ev = marker.Seek( -1 )) != NULL)
	{
		if (ev->Start() > inEvents->Start())
		{
			marker.SeekForwardToTime( inEvents->Start(), true );
		}
	}
	
	while (status != false && inEventCount > 0)
	{
			// If it's a note off, or a note on with zero velocity...
		if (		inEvents->Command() == EvtType_NoteOff
			||	(inEvents->Command() == EvtType_Note && inEvents->note.attackVelocity == 0))
		{
			if ((const Event *)ioFirstUnmatchedEvent == NULL)
				ioFirstUnmatchedEvent.First();

			bool			pastFirst = false;
			EventMarker	matchPos( ioFirstUnmatchedEvent );
		
				// Seek forward to first note-on that has no matching note off.
			for (ev = (const Event *)matchPos; ev; ev = matchPos.Seek( 1 ) )
			{
					// If this is a note that has no matching note off...
				if (		ev->Command() == EvtType_Note
					&&	ev->note.releaseVelocity == MIDIValueUnset)
				{

						// And it matches the note off...
					if (		ev->GetVChannel() == inEvents->GetVChannel()
						&&	ev->note.pitch == inEvents->note.pitch)
					{
							// Change the release velocity
						if (inEvents->Command() == EvtType_NoteOff)
							((Event *)ev)->note.releaseVelocity = inEvents->note.attackVelocity;
						else ((Event *)ev)->note.releaseVelocity = MIDIReleaseSpecial;
						
							// Change the duration
						((Event *)ev)->SetDuration( inEvents->Start() - ev->Start() );
						
							// And we're done...
						break;
					}

						// If we've encountered an unmatched event, and it didn't
						// match, and this is the first such event, then remember this
						// position to search from next time.
					if (!pastFirst) ioFirstUnmatchedEvent = matchPos;
					pastFirst = true;
				}
			}

			if (!pastFirst) ioFirstUnmatchedEvent = matchPos;
		}
		else
		{
				// If we're not already at the end, then seek to the appropriate time...
			if (!marker.IsAtEnd()) marker.SeekForwardToTime( inEvents->Start(), false );

				// Insert one event.
			status = marker.Insert( inEvents, 1, ioAction );
		}
		
		inEvents++;
		inEventCount--;
	}

#if DEBUG
	Validate();
#endif

	return status;
}

// ---------------------------------------------------------------------------
// Copy all selected events into an allocated list.

long EventList::CopySelected( EventPtr &result )
{
	EventMarker		marker( *this ),
					firstSelected( *this );
	const Event		*ev;
	Event			*list,
					*put;
	long				count = 0,
					index;

		// First, count how many selected events there are.
		// REM: A firstSelected hint could speed this up...
	for ( ev = marker.First(); ev; ev = marker.Seek( 1 ) )
	{
		if (ev->IsSelected())
		{
			if (count == 0) firstSelected = marker;
			count++;
		}
	}

		// Make a list of the number of selected events
	list = new Event[ count ];
	put = list;
	marker = firstSelected;

		// Iterate through only that part of the sequence that contained
		// selected events.
	for ( ev = (ConstEventPtr)marker, index = 0; index < count; )
	{
			// For each selected event
		if (ev->IsSelected())
		{
			*put++ = *(Event *)ev;		// Copy to temporary list
			index++;
		}
		ev = marker.Seek( 1 );
	}

	result = list;
	return count;
}

// ---------------------------------------------------------------------------
// Insert block of time. This involves modifying the start times of all events
// after the insertion point.

void EventList::InsertTime( long startTime, long offset, EventListUndoAction *inAction )
{
	EventMarker		marker( *this );
	const Event		*ev;

		// Seek to the time we want to modify.
	ev = marker.SeekForwardToTime( startTime, false );

		// Get and replace all events after the time insertion.
		// REM: Should we deal with time overflow? Or is that dealt with at 
		// a higher level?
	for ( ; ev; ev = marker.Seek( 1 ))
	{
		Event		modifiedEvent;

		marker.Get( &modifiedEvent );
		modifiedEvent.common.start += offset;
		marker.Replace( &modifiedEvent, 1, inAction );
	}
}

// ---------------------------------------------------------------------------
// Delete block of time. This involves modifying the start times of all events
// which come after the deleted block.

void EventList::DeleteTime( long startTime, long offset, EventListUndoAction *inAction )
{
	EventMarker		marker( *this );
	const Event		*ev;

	ev = marker.SeekForwardToTime( startTime, false );

	for ( ; ev; ev = marker.Seek( 1 ))
	{
		Event		modifiedEvent;

		marker.Get( &modifiedEvent );
		if (modifiedEvent.common.start < startTime + offset)
			modifiedEvent.common.start = startTime;
		else modifiedEvent.common.start -= offset;
		marker.Replace( &modifiedEvent, 1, inAction );
	}
}

// ---------------------------------------------------------------------------
// Copy items to new uninitialized memory

void EventList::ConstructItems( void *outDst, void *inSrc, long inItemCount )
{
	Event		*srcEv = (Event *)inSrc,
				*dstEv = (Event *)outDst;

		// For each event
	while (inItemCount--)
	{
		new ( dstEv ) Event( *srcEv );
		
		dstEv++;
		srcEv++;
	}
}

// ---------------------------------------------------------------------------
// Copy items over old items

void EventList::CopyItems( void *outDst, void *inSrc, long inItemCount )
{
	Event		*srcEv = (Event *)inSrc,
				*dstEv = (Event *)outDst;

		// For each event
	while (inItemCount--)
	{
		*dstEv++ = *srcEv++;
	}
}

// ---------------------------------------------------------------------------
// Copy items to new uninitalized memory, destructing source items

void EventList::MoveItems( void *outDst, void *inSrc, long inItemCount )
{
#if 0
	Event		*srcEv = (Event *)inSrc,
				*dstEv = (Event *)outDst;
				
	if (dstEv > srcEv)
	{
		srcEv += inItemCount;
		dstEv += inItemCount;
		
			// For each event, copy and destruct the old item.
		while (inItemCount--)
		{
			*--dstEv = *--srcEv;
			srcEv->~Event();
		}
	}
	else
	{
			// For each event, copy and destruct the old item.
		while (inItemCount--)
		{
			*dstEv++ = *srcEv;
			srcEv->~Event();
			srcEv++;
		}
	}
#endif

		// I _think_ that memmove should work -- because no events that I'm aware of
		// use any sort of circular lists or back-pointers to the event structure. As long
		// as the reference count is properly maintained (in this case it is, since the number
		// of valid items remains constant) everything should be OK.
	memmove( outDst, inSrc, inItemCount * sizeof (Event) );
}

// ---------------------------------------------------------------------------
// Destroy items

void EventList::DestroyItems( void *outDst, long inItemCount )
{
	Event		*dstEv = (Event *)outDst;

		// For each event
	while (inItemCount--)
	{
		dstEv->~Event();
		dstEv++;
	}
}

// ---------------------------------------------------------------------------
// Calculate the time of the latest stop time in the sequence. This is used
// for setting the horizontal scroll bar.

long EventList::MaxTime( void )
{
	long			time = 0;

	for (EventBlock *b = FirstBlock(); b; b = b->Next() )
	{
		time = time > b->MaxTime() ? time : b->MaxTime();
	}

	return time;
}

// ---------------------------------------------------------------------------
// EventList Undo function

		/**	Apply this undo action. */
void EventListUndoAction::Undo()
{
	int32			maxTime = LONG_MIN,
					minTime = LONG_MAX;
	Event			*ev;
	CUndoIterator	iter( sizeof *ev );
					
	for (ev = (Event *)iter.First( *this ); ev != NULL; ev = (Event *)iter.Next() )
	{
		minTime = minTime < ev->Start() ? minTime : ev->Start();
		maxTime = maxTime > ev->Stop() ? maxTime : ev->Stop();
	}

	ItemListUndoAction<Event>::Undo();
	
	for (ev = (Event *)iter.First( *this ); ev != NULL; ev = (Event *)iter.Next() )
	{
		minTime = minTime < ev->Start() ? minTime : ev->Start();
		maxTime = maxTime > ev->Stop() ? maxTime : ev->Stop();
	}

	CUpdateHint		hint;
	if (maxTime >= minTime)
	{
		hint.AddInt32( "MinTime", minTime );
		hint.AddInt32( "MaxTime", maxTime );
	}
	subject.PostUpdate( &hint );
}
	
// ---------------------------------------------------------------------------
// EventList Redo function

		/**	Apply this redo action. */
void EventListUndoAction::Redo()
{
	int32			maxTime = LONG_MIN,
					minTime = LONG_MAX;
	Event			*ev;
	CUndoIterator	iter( sizeof *ev );
					
	for (ev = (Event *)iter.First( *this ); ev != NULL; ev = (Event *)iter.Next() )
	{
		minTime = minTime < ev->Start() ? minTime : ev->Start();
		maxTime = maxTime > ev->Stop() ? maxTime : ev->Stop();
	}

	ItemListUndoAction<Event>::Redo();
	
	for (ev = (Event *)iter.First( *this ); ev != NULL; ev = (Event *)iter.Next() )
	{
		minTime = minTime < ev->Start() ? minTime : ev->Start();
		maxTime = maxTime > ev->Stop() ? maxTime : ev->Stop();
	}

	CUpdateHint		hint;
	if (maxTime >= minTime)
	{
		hint.AddInt32( "MinTime", minTime );
		hint.AddInt32( "MaxTime", maxTime );
	}
	subject.PostUpdate( &hint );
}
	

#if DEBUG
void EventList::Validate()
{
	EventMarker		marker( *this );
	long				prevTime = 0;
	long				index = 0;

		// Start by determining if the events are in order

		// For each event that overlaps the current view, draw it.
	for (const Event *ev = marker.First(); ev; ev = marker.Seek( 1 ), index++ )
	{
		long		time = ev->Start();

		if (time < prevTime)
		{
//			ASSERT( time >= prevTime );
		}
		prevTime = time;
	}
	
	ItemList<EventBlock,Event>::Validate();
}
#endif

void WriteDeltaValue( CAbstractWriter &writer, unsigned long value )
{
	uint8	b[ 8 ];
	int32	i = 0;

	while (value != 0 || i == 0)
	{
		b[ i++ ] = value | 0x80;
		value >>= 7;
	}
	
	b[ 0 ] &= 0x7f;
	
	while (i > 0)
	{
		writer << b[ --i ];
	}
}

unsigned long ReadDeltaValue( CAbstractReader &reader )
{
	unsigned long		v = 0;
	uint8				b = 0;
	
	do
	{
		reader >> b;
		v = (v << 7) | (b & 0x7f);
	} while (b & 0x80);
	
	return v;
}

void WriteFixed( CAbstractWriter &writer, unsigned long value, unsigned long maxVal )
{
	uint8	b[ 8 ];
	int32	i = 0;

	while (maxVal != 0)
	{
		b[ i++ ] = value & 0x7f;
		value >>= 7;
		maxVal >>= 7;
	}
	
	while (i > 0)
	{
		writer << b[ --i ];
	}
}

unsigned long ReadFixed( CAbstractReader &reader, unsigned long maxVal )
{
	unsigned long		v = 0;
	uint8				b = 0;
	
	while (maxVal != 0)
	{
		reader >> b;
		v = (v << 7) | b;
		maxVal >>= 7;
	}
	
	return v;
}

void WriteEventList( CAbstractWriter &writer, EventList &inEvents )
{
	int32			prevTime = 0;
	EventMarker		marker( inEvents );
	const Event		*ev;

	for (	ev = marker.First(); ev; ev = marker.Seek( 1 ))
	{
		long		time = ev->Start();
		
		WriteDeltaValue( writer, time - prevTime );
		prevTime = time;
		
			// write command byte (incl. selection bit)
		writer << ev->common.command;
		if (ev->HasProperty( Event::Prop_Duration ))
		{
				// Write the event's duration
			WriteDeltaValue( writer, ev->Duration() );
		}
		else if (ev->HasProperty( Event::Prop_ExtraData ))
		{
				// Write the event's extended data
			WriteDeltaValue( writer, ev->ExtendedDataSize() );
			writer.MustWrite( ev->ExtendedData(), ev->ExtendedDataSize() );
		}
		else WriteDeltaValue( writer, 0 );						// write fake duration

		if (ev->HasProperty( Event::Prop_Channel ))
			writer << ev->common.vChannel;
		
		switch (ev->Command()) {
		case EvtType_Note:								// note event
			writer << ev->note.pitch << ev->note.attackVelocity << ev->note.releaseVelocity;
			break;

		case EvtType_ChannelATouch:						// channel aftertouch
			writer << ev->aTouch.value;
			WriteFixed( writer, ev->aTouch.updatePeriod, 0x3fff );
			break;

		case EvtType_PolyATouch:							// polyphonic aftertouch
			writer << ev->aTouch.pitch << ev->aTouch.value;
			break;

		case EvtType_Controller:							// controller change
			writer	<< ev->controlChange.controller
					<< ev->controlChange.MSB
					<< ev->controlChange.LSB;
			WriteFixed( writer, ev->controlChange.updatePeriod, 0x3fff );
			break;

		case EvtType_ProgramChange:						// program change
			writer	<< ev->programChange.program
					<< ev->programChange.bankMSB
					<< ev->programChange.bankLSB
					<< ev->programChange.vPos;
			break;

		case EvtType_PitchBend:							// pitch bend
			WriteFixed( writer, ev->pitchBend.targetBend, 0x3fff );
			WriteFixed( writer, ev->pitchBend.startBend, 0x3fff );
			WriteFixed( writer, ev->pitchBend.updatePeriod, 0x3fff );
			break;
			
		case EvtType_SysEx:								// system exclusive
// REM: Eliminate high bit use
			writer << ev->sysEx.vPos;
//			WriteDeltaValue( writer, ev->ExtendedDataSize() );
//			writer.MustWrite( ev->ExtendedData(), ev->ExtendedDataSize() );
			break;
			
		case EvtType_Text:								// text message
// REM: Eliminate high bit use
			writer << ev->text.vPos << ev->text.textType;
//			WriteDeltaValue( writer, ev->ExtendedDataSize() );
//			writer.MustWrite( ev->ExtendedData(), ev->ExtendedDataSize() );
			break;
			
		case EvtType_Tempo:								// change tempo event
			WriteFixed( writer, ev->tempo.newTempo, ULONG_MAX);
			break;

		case EvtType_TimeSig:								// change time signature event
			writer << ev->sigChange.vPos << ev->sigChange.numerator << ev->sigChange.denominator;
			break;

		case EvtType_Repeat:								// repeat a section
			writer << ev->repeat.vPos;
			WriteFixed( writer, ev->repeat.repeatCount, USHRT_MAX);
			break;

		case EvtType_Sequence:							// play another track
			writer	<< ev->sequence.vPos
					<< ev->sequence.transposition
					<< ev->sequence.flags;
			WriteFixed( writer, ev->sequence.sequence, USHRT_MAX );
			break;

//		case EvtType_Locate:								// locate to "duration" time
//		case EvtType_Cue:									// trigger a cue point
//		case EvtType_MTCCue:								// trigger an MTC cue point
		case EvtType_MuteTrack:							// mute a track
//		case EvtType_Splice:								// a "splice" event for overdub
//		case EvtType_SpliceOut:							// a "splice" event for overdub
		case EvtType_UserEvent:							// has type and data fields
		case EvtType_Branch:								// conditional branch to track
//		case EvtType_Erase:								// erase notes on channel
//		case EvtType_Punch:								// punch in over track
		case EvtType_ChannelTranspose:						// transposition for vChannel
		case EvtType_ChannelMute:							// mute a vChannel
		case EvtType_ChannelVolume:						// velocity contour event
			break;

		case EvtType_End:									// end of track
//		case EvtType_Stop:								// stop the sequencer
//		case EvtType_Go:									// start the sequencer
			break;

		default:
			break;
		}
	}
}

void ReadEventList( CAbstractReader &reader, EventList &outEvents )
{
	Event		ev;
	int32		prevTime = 0;
	int32		dataSize = 0;
	EventMarker	marker( outEvents );
	uint8		byteRead;
	bool			skip = false;
	
	marker.First();
	marker.Track( ItemMarker_Base::Track_Next );
	
	while (reader.BytesAvailable() > 0)
	{
		unsigned long		v = 0;

		ev.SetCommand( EvtType_End );		// clear out any extended data
		
			//	Special version of read-delta-value to deal with push-back
		do
		{
			if (skip) skip = false;
			else reader >> byteRead;
			v = (v << 7) | (byteRead & 0x7f);
		} while (byteRead & 0x80);
		
		prevTime += v;
		
//		prevTime += ReadDeltaValue( reader );
		ev.common.start = prevTime;
		reader >> ev.common.command;
		
		if (ev.HasProperty( Event::Prop_Duration ))
		{
				// Read the event's duration
			ev.common.duration = ReadDeltaValue( reader );
		}
		else if (ev.HasProperty( Event::Prop_ExtraData ))
		{
				// Read the event's extended data
			ev.common.duration = 0;
			dataSize = ReadDeltaValue( reader );
			reader.MustRead( ev.ExtendedData(), ev.ExtendedDataSize() );
		}
		else ReadDeltaValue( reader );						// discard fake duration

		if (ev.HasProperty( Event::Prop_Channel ))
			reader >> ev.common.vChannel;
					
		switch (ev.Command()) {
		case EvtType_Note:								// note event
			reader >> ev.note.pitch >> ev.note.attackVelocity >> ev.note.releaseVelocity;
			break;

		case EvtType_ChannelATouch:						// channel aftertouch
			reader >> ev.aTouch.value;
			ev.aTouch.updatePeriod = ReadFixed( reader, 0x3fff );
			break;

		case EvtType_PolyATouch:							// polyphonic aftertouch
			reader >> ev.aTouch.pitch >> ev.aTouch.value;
			break;

		case EvtType_Controller:							// controller change
			reader	>> ev.controlChange.controller
					>> ev.controlChange.MSB
					>> ev.controlChange.LSB;
			ev.controlChange.updatePeriod = ReadFixed( reader, 0x3fff );
			break;

		case EvtType_ProgramChange:						// program change
			reader	>> ev.programChange.program
					>> ev.programChange.bankMSB
					>> ev.programChange.bankLSB
					>> ev.programChange.vPos;
			break;

		case EvtType_PitchBend:							// pitch bend
			ev.pitchBend.targetBend = ReadFixed( reader, 0x3fff );
			ev.pitchBend.startBend = ReadFixed( reader, 0x3fff );
			ev.pitchBend.updatePeriod = ReadFixed( reader, 0x3fff );
			break;
			
		case EvtType_SysEx:								// system exclusive
			reader >> ev.sysEx.vPos;
			ev.SetExtendedDataSize( dataSize );
			break;
			
		case EvtType_Text:								// text message
			reader >> ev.text.vPos >> ev.text.textType;
			ev.SetExtendedDataSize( dataSize );
			break;
			
		case EvtType_Tempo:								// change tempo event
			ev.tempo.newTempo = ReadFixed( reader, ULONG_MAX);
			break;

		case EvtType_TimeSig:								// change time signature event
			reader >> ev.sigChange.vPos >> ev.sigChange.numerator >> ev.sigChange.denominator;
			break;

		case EvtType_Repeat:								// repeat a section
			reader >> ev.repeat.vPos;
			ev.repeat.repeatCount = ReadFixed( reader, 0xffff );
			break;

		case EvtType_Sequence:							// play another track
			reader	>> ev.sequence.vPos
					>> ev.sequence.transposition
					>> ev.sequence.flags;
			ev.sequence.sequence = ReadFixed( reader, 0xffff );
			break;

#if 0
		case EvtType_Locate:								// locate to "duration" time
		case EvtType_Cue:									// trigger a cue point
		case EvtType_MTCCue:								// trigger an MTC cue point
		case EvtType_MuteVChannel:							// mute a vChannel
		case EvtType_MuteTrack:							// mute a track
		case EvtType_SpliceIn:								// a "splice" event for overdub
		case EvtType_SpliceOut:							// a "splice" event for overdub
		case EvtType_UserEvent:							// has type and data fields
		case EvtType_Branch:								// conditional branch to track
		case EvtType_Erase:								// erase notes on channel
		case EvtType_Punch:								// punch in over track
		case EvtType_VelocityContour:						// velocity contour event
		case EvtType_Transpose:							// transposition for vChannel
			break;

		case EvtType_End:									// end of track
		case EvtType_Stop:								// stop the sequencer
		case EvtType_Go:									// start the sequencer
			break;
#endif

		default:
				// Skip over any bytes with high bit clear.
				// Set the skip flag so that next byte will be read
			do { reader >> byteRead; } while (!(byteRead & 0x80));
			skip = true;
			break;
		}
		
		marker.Insert( &ev, 1, NULL );
	}
}
