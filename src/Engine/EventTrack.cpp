/* ===================================================================== *
 * EventTrack.cpp (MeV/Engine)
 * ===================================================================== */

#include "EventTrack.h"
#include "DocApp.h"
#include "MeVDoc.h"
#include "EventOp.h"
#include "EventEditor.h"
#include "MeVFileID.h"
#include "IFFWriter.h"
#include "IFFReader.h"

// ---------------------------------------------------------------------------
// Event track constructor

CEventTrack::CEventTrack( CMeVDoc &inDoc, TClockType cType, int32 inID, char *inName )
	: CTrack( inDoc, cType, inID, inName )
{
	StSubjectLock		lock( *this, Lock_Exclusive );

		// Set the initial grid size and enable gridsnap
	timeGridSize = Ticks_Per_QtrNote / 2;
	gridSnapEnabled = true;

		// Initialize the count of selected events (initially 0)
	selectionCount = 0;

		// Associate the current event marker with the event list.
	currentEvent.SetList( events );
	
		// Initialize the list of filters
	filterCount = 0;

		// Set the signature map to invalid so that it will be regenerated
	validSigMap = false;

		// Initialize the section markers to the beginning of the track
	sectionStart = sectionEnd = 0;

		// Initialize the signature map.
	if (sigMap.clockType == ClockType_Metered)
	{
		sigMap.entries		= NULL;
		sigMap.numEntries	= 0;
	}
	
		// Sunmarize the current selection (which is NIL).
	SummarizeSelection();

		// Compile list of track operators
	CompileOperators();
}

// ---------------------------------------------------------------------------
// Compute summary information for currently selected events

void CEventTrack::SummarizeSelection()
{
//	StSubjectLock		lock( *this, Lock_Exclusive );
	int32			sigChangeCount = 1,		// Number of timesigs encountered
					tempoChangeCount = 0;		// Number of tempo changes encountered
	int32			minorUnitDur,				// Major unit of time signature
					majorUnitDur,				// Minor unit of time signature
					prevMajorUnitDur,
					prevMinorUnitDur,
					time,
					sigStart,
					prevSigStart;
	double			initialTempoPeriod = 0.0;

	prevAggregateAction = 0;
	
		// Initialize selection times to an "inside-out" selection.
	minSelectTime = LONG_MAX;
	maxSelectTime = LONG_MIN;
	selectionCount = 0;

	EventMarker		marker( events );
	const Event		*ev;

	lastEventTime = logicalLength = 0;

		// Only metered sequences can have time signature events, so there's
		// no point in even checking.
	if (sigMap.clockType != ClockType_Metered) validSigMap = true;

		// Every track starts at 4/4 (the default), but can be immediately changed
		// by the very first event.
	prevSigStart = 0;
	prevMinorUnitDur = Ticks_Per_QtrNote;
	prevMajorUnitDur = Ticks_Per_QtrNote * 4;

#if 1
		// Make sure track data is valid
	Validate();
#endif

		// Reset the bit-arrays of channels used and channels selected
	usedChannels.Clear();
	selectedChannels.Clear();

		// Loop through every event in the sequence and compile summary information
	for (	ev = marker.First();
			ev;
			ev = marker.Seek( 1 ))
	{
		long		first = ev->Start(),
				last;
				
			// last <== the stop time of the event
		if (ev->HasProperty( Event::Prop_Duration ))
			last = ev->Stop();
		else last = first;

			// If this event has a channel, then mark channel as used
		if (ev->HasProperty( Event::Prop_Channel ))
		{
			usedChannels.Set( ev->common.vChannel );
		}

		switch (ev->Command()) {
		case EvtType_End:
		
				// An 'end' event overrides the implicit track duration.
			if (logicalLength <= 0) logicalLength = last;
			break;
			
		case EvtType_TimeSig:

			if (validSigMap) break;

			time = ev->Start();
			if (time < 0) time = 0;
			minorUnitDur = (Ticks_Per_QtrNote * 4) >> ev->sigChange.denominator;
			majorUnitDur = minorUnitDur * ev->sigChange.numerator;
			
			if (majorUnitDur <= 0) majorUnitDur = Ticks_Per_QtrNote * 3;
			if (minorUnitDur <= 0) minorUnitDur = Ticks_Per_QtrNote * 1;

				// If it's effectively the same time signature...
			if (		prevMinorUnitDur != minorUnitDur
				||	prevMajorUnitDur != majorUnitDur)
			{
					// Justify this time to the beginning of the bar.
				sigStart = time - (time - prevSigStart) % prevMajorUnitDur;

					// Only if one or more bars have elapsed do we create a new entry.
				if (sigStart > prevSigStart)
				{
					sigChangeCount++;
					prevSigStart = sigStart;
				}
				prevMinorUnitDur = minorUnitDur;
				prevMajorUnitDur = majorUnitDur;
			}
			break;
			
		case EvtType_Tempo:
			tempoChangeCount++;
			if (initialTempoPeriod == 0.0)
				initialTempoPeriod = RateToPeriod( (double)ev->tempo.newTempo / 1000.0 );
			break;
		}

		if (ev->IsSelected())
		{
				// If this event has a channel, then include channel in selection
			if (ev->HasProperty( Event::Prop_Channel ))
				selectedChannels.Set( ev->common.vChannel );

				// Add the event to the selection range
			minSelectTime = MIN( minSelectTime, first );
			maxSelectTime = MAX( maxSelectTime, last );

				// Set the marker which points to the first selected event
				// -- but only if it does not already point to a selected event.
			if (selectionCount == 0)
			{
				const Event	*cEv = currentEvent;
			
				if (cEv == NULL || !cEv->IsSelected()) currentEvent = marker;
			}
			selectionCount++;
		}

			// Compute the time of the last event stop time in the track
		if (last > lastEventTime) lastEventTime = last;
	}
	
	if (validSigMap == false)	
	{
		CSignatureMap::SigChange	*sigList,
								*prevSig;
		int32					sigIndex = 0;
		
		sigList = new CSignatureMap::SigChange[ sigChangeCount ];
		prevSig = sigList;

			// Default is 4/4
		sigList[ 0 ].sigTime = 0;
		sigList[ 0 ].sigMinorUnitDur = Ticks_Per_QtrNote;
		sigList[ 0 ].sigMajorUnitDur = Ticks_Per_QtrNote * 4;
		
		for (	ev = marker.First();
				ev;
				ev = marker.Seek( 1 ))
		{
			if (ev->Command() != EvtType_TimeSig) continue;
			
			time = ev->Start();
			if (time < 0) time = 0;
			minorUnitDur = (Ticks_Per_QtrNote * 4) >> ev->sigChange.denominator;
			majorUnitDur = minorUnitDur * ev->sigChange.numerator;
			
			if (majorUnitDur <= 0) majorUnitDur = Ticks_Per_QtrNote * 3;
			if (minorUnitDur <= 0) minorUnitDur = Ticks_Per_QtrNote * 1;

				// If it's effectively the same time signature...
			if (		prevSig->sigMinorUnitDur == minorUnitDur
				&&	prevSig->sigMajorUnitDur == majorUnitDur)
			{
				continue;
			}

				// Justify this time to the beginning of the bar.
			sigStart = time - (time - prevSig->sigTime) % prevSig->sigMajorUnitDur;

				// Only if one or more bars have elapsed do we create a new entry.
			if (sigStart > prevSig->sigTime)
			{
				sigIndex++;
				prevSig = &sigList[ sigIndex ];
				prevSig->sigTime = sigStart;
			}
			prevSig->sigMinorUnitDur = minorUnitDur;
			prevSig->sigMajorUnitDur = majorUnitDur;
		}

		delete sigMap.entries;
		sigMap.entries = sigList;
		sigMap.numEntries = sigIndex + 1;
		validSigMap = true;

			// Tell all observers that our time signature changed.
		NotifyUpdate( CTrack::Update_SigMap | CTrack::Update_Summary, NULL );
	}
	else NotifyUpdate( CTrack::Update_Summary, NULL );
	
		// If this is the master real track (the only track in which tempo events
		// have any effect), and tempo events have been modified, then we need
		// to recompile the document's tempo map.
	if (GetID() == 1 && !Document().ValidTempoMap())
	{
		CTempoMapEntry		*newTempoMap,
							*nextEntry;
		
		if (initialTempoPeriod == 0) initialTempoPeriod = RateToPeriod( Document().InitialTempo() );
		
			// We know how many tempo events there are already (since we counted them
			// in the last section) so we can go ahead and allocate the new array of tempomap
			// entries.
		if (tempoChangeCount <= 1)
		{
			newTempoMap = new CTempoMapEntry[ 1 ];
			newTempoMap->SetInitialTempo( initialTempoPeriod );
		}
		else
		{
			newTempoMap = new CTempoMapEntry[ tempoChangeCount + 1 ];
			newTempoMap->SetInitialTempo( initialTempoPeriod );

			nextEntry = &newTempoMap[ 1 ];

			for (	ev = marker.First();
				ev;
				ev = marker.Seek( 1 ))
			{
				if (ev->Command() == EvtType_Tempo)
				{
					double		per;
		
						// Calculate tempo period
					per = RateToPeriod( (double)ev->tempo.newTempo / 1000.0 );
	
						// Set up a tempo entry based off the previous one.
					nextEntry->SetTempo(	nextEntry[ - 1 ],
										per,
										ev->Start(),
										ev->Duration(),
										ClockType() );
				}
			}
		}
		
			// Replace old tempo map with new
		Document().ReplaceTempoMap( newTempoMap, tempoChangeCount );
		NotifyUpdate( CTrack::Update_TempoMap, NULL );
		if (Sibling())
			Sibling()->NotifyUpdate( CTrack::Update_TempoMap, NULL );
		Document().NotifyUpdate( CMeVDoc::Update_TempoMap, NULL );
	}

		// If there was no END event, then set the logical length to the end of
		// the measure of the last event.
	if (logicalLength <= 0)
	{
		int32			majorUnitCount;
		CSignatureMap::SigChange	*sig;
		
		sig = sigMap.PreviousMajorUnit(	lastEventTime,
										majorUnitCount,
										logicalLength );
								
		if (lastEventTime > logicalLength || logicalLength == 0)
			logicalLength += sig->sigMajorUnitDur;

// 	logicalLength = lastEventTime;
	}
}

// ---------------------------------------------------------------------------
//	Indicates that a tempo event has been moved.
//	(only applies to master tracks)

void CEventTrack::InvalidateTempoMap()
{
	if (GetID() == 0 || GetID() == 1) Document().InvalidateTempoMap();
}

// ---------------------------------------------------------------------------
// Debugging code. Iterates through all events in the track and insures
// that event timestamps are sorted, otherwise it tosses and exception.

void CEventTrack::Validate()
{
#if 0
	StSubjectLock	lock( *this, Lock_Shared );
#if 1
	EventMarker		marker( events );
	long				prevTime = 0;

		// Start by determining if the events are in order

		// For each event that overlaps the current view, draw it.
	for (	const Event *ev = marker.First(); ev; ev = marker.Seek( 1 ) )
	{
		long		time = ev->Start();

		VERIFY( time >= prevTime );
		prevTime = time;
	}
#endif	
#if DEBUG
	events.Validate();
#endif
#endif
}

// ---------------------------------------------------------------------------
// Select all events.

void CEventTrack::SelectAll( CEventEditor *inEditor )
{
	StSubjectLock		lock( *this, Lock_Exclusive );
	EventMarker		marker( events );
	
		// If this is a master track, then make this the active one.
	Document().SetActiveMaster( this );

		// For each event, select it.
	if (inEditor != NULL)
	{
		for (const Event *ev = marker.First(); ev; ev = marker.Seek( 1 ) )
		{
			if (IsChannelLocked( *ev )) continue;
			if (!ev->IsSelected())
			{
				((Event *)ev)->SetSelected( true );
				(inEditor->Handler( *ev )).Invalidate( *inEditor, *ev );
			}
		}
	}
	else
	{
		for (const Event *ev = marker.First(); ev; ev = marker.Seek( 1 ) )
		{
			if (IsChannelLocked( *ev )) continue;
			((Event *)ev)->SetSelected( true );
		}
	}

	SummarizeSelection();

	CEventSelectionUpdateHint	 hint( *this );

	if (inEditor) inEditor->PostUpdate( &hint, true );
	else PostUpdate( &hint, NULL );
}

// ---------------------------------------------------------------------------
// Select all events.

void CEventTrack::DeselectAll( CEventEditor *inEditor, bool inDoUpdate )
{
	StSubjectLock					lock( *this, Lock_Exclusive );
	EventMarker					marker( events );
	CEventSelectionUpdateHint		hint( *this, true );
	const Event					*ev;

	if (inEditor != NULL)
	{
			// For each event, select it.
		for (	ev = marker.FirstItemInRange( MinSelectTime(), MaxSelectTime() );
				ev;
				ev = marker.NextItemInRange( MinSelectTime(), MaxSelectTime() ) )
		{
			if (ev->IsSelected())
			{
				((Event *)ev)->SetSelected( false );
				(inEditor->Handler( *ev )).Invalidate( *inEditor, *ev );
			}
		}
	}
	else
	{
			// For each event, select it.
		for (	ev = marker.FirstItemInRange( MinSelectTime(), MaxSelectTime() );
				ev;
				ev = marker.NextItemInRange( MinSelectTime(), MaxSelectTime() ) )
		{
			((Event *)ev)->SetSelected( false );
		}
	}

	SummarizeSelection();
	if (inEditor) inEditor->PostUpdate( &hint, true );
	else if (inDoUpdate) PostUpdate( &hint, NULL );
}

// ---------------------------------------------------------------------------
// Delete all selected events.

void
CEventTrack::DeleteEvent(
	const Event *which)
{
	StSubjectLock lock(*this, Lock_Exclusive);
	CEventUpdateHint hint(*this, *which);

	// Create a new undo action to record all of the changes
	// we are about to make to the event list. If we fail to
	// complete the edit, we can roll back the edit to this point.
	EventListUndoAction *undoAction = new EventListUndoAction(events,
															  *this,
															  "Delete Event");
	// Initialize an event marker for this track.
	EventMarker marker(events);

	marker.Track(EventMarker::Track_Next);

	// If the operations did not effect the ordering of
	// events, then things are much simpler.
	// For each selected event.
	for (const Event *ev = marker.First();
		 ev != NULL;
		 ev = marker.Seek(1))
	{
		if (ev == which)
		{
			// Remember what event types we changed
			uint8 eventType = ev->Command();

			// Delete from original list
			marker.Remove(1, undoAction);

			// If the delete event was a time signature event, then
			// go head and invalidate the track's signature map.
			if (eventType == EvtType_TimeSig)
				InvalidateSigMap();

			// If the deleted event was a tempo change event, then
			// go head and invalidate the document's tempo map.
			else if (eventType == EvtType_Tempo)
				InvalidateTempoMap();

			// REM: We should be smarter about this update...
			SummarizeSelection();
			PostUpdate(&hint, NULL);

			// Add the undo action to the undo list for this observable
			if (undoAction)
				AddUndoAction(undoAction);
			Document().SetModified();
			break;
		}
	}
}

void CEventTrack::DeleteSelection()
{
	StSubjectLock					lock( *this, Lock_Exclusive );
	EventListUndoAction			*undoAction = NULL;
	CEventSelectionUpdateHint		hint( *this );
	uint8						eventsModified[ 127 ];
	
	memset( eventsModified, 0, sizeof eventsModified );

	// Create a new undo action to record all of the changes
	// we are about to make to the event list. If we fail to
	// complete the edit, we can roll back the edit to this point.

	undoAction = new EventListUndoAction( events, *this, "Delete Events" );

		// Initialize an event marker for this track.
	EventMarker		marker( events );

	marker.Track( EventMarker::Track_Next );

		// If the operations did not effect the ordering of
		// events, then things are much simpler.
	
		// For each selected event.
	for (	const Event *ev = marker.FirstItemInRange( MinSelectTime(), MaxSelectTime() );
			ev; )
	{
		Event		newEv;
	
		if (ev->IsSelected())
		{
				// Remember what event types we changed
			eventsModified[ ev->Command() ] = true;

			marker.Remove( 1, undoAction );	// Delete from original list
			ev = (ConstEventPtr)marker;		// Increment to next event
			
		}
		else
		{
			ev = (Event *)marker.Seek( 1 );
		}
	}

		// If the delete event was a time signature event, then
		// go head and invalidate the track's signature map.
	if (eventsModified[ EvtType_TimeSig ]) InvalidateSigMap();

		// If the deleted event was a tempo change event, then
		// go head and invalidate the document's tempo map.
	if (eventsModified[ EvtType_Tempo ]) InvalidateTempoMap();

		// REM: We should be smarter about this update...
	SummarizeSelection();
	PostUpdate( &hint, NULL );

		// Add the undo action to the undo list for this observable
	if (undoAction) AddUndoAction( undoAction );
	Document().SetModified();
}

	// Change some attribute of all of the selected events.
void CEventTrack::ModifySelectedEvents(
	CEventEditor			*inEditor,
	EventOp				&op,
	const char			*inActionLabel,
	int32				inAggregateAction )
{
	StSubjectLock			trackLock( *this, Lock_Exclusive );
	EventListUndoAction	*undoAction = NULL;
	TClockType			clockType = ClockType();
	uint8				eventsModified[ 127 ];
	
		// Initialize an event marker for this track.
	long					prevTrackDuration = LastEventTime(),
						prevLogicalLength = LogicalLength();
	long					minTime = MinSelectTime(),
						maxTime = MaxSelectTime();

	if (SelectionType() == CEventTrack::Select_None) return;

		// We need a try block here, because this operation can potentially
		// run out of memory, especially if there is not enough memory to
		// create the undo records needed.

	memset( eventsModified, 0, sizeof eventsModified );

		/*	IF the most recent undo action is not NULL,
			and the most recent undo action was created by the code listed below,
			and the aggregate action code indicates that this action is a continuation
			of the previous action, then don't create a new undo action.
		*/
	if (		inAggregateAction != 0
		&&	inAggregateAction == prevAggregateAction
		&&	prevAggregateUndo != NULL
		&&	IsMostRecentUndoAction( prevAggregateUndo ))
	{
			// At this point, all of the selected events SHOULD have been
			// saved in a previous undo action. So, we want to avoid creating
			// another one.
		undoAction = NULL;
	}
	else
	{
			// Create the undo action normally...
		undoAction = new EventListUndoAction( events, *this, inActionLabel );
		prevAggregateUndo = undoAction;
	}

	if (!op.CanModifyOrder())
	{
			// Initialize an event marker for this track.
		EventMarker		marker( events );

		marker.Track( EventMarker::Track_Next );

			// Applying a modifier to the events which can
			// cause a change in event order
	
			// For each selected event.
		for (	Event *ev = (Event *)marker.First();
				ev;
				ev = (Event *)marker.Seek( 1 ) )
		{
			Event		newEv;

			if (!ev->IsSelected()) continue;

				// Remember what event types we changed
			eventsModified[ ev->Command() ] = true;

				// Temporarily copy the event, and apply the vertical
				// component of the dragging to it.
			newEv = *ev;
			op( newEv, clockType );

				// Invalidate the new and old positions of the event
			if (inEditor != NULL)
			{
				const CAbstractEventHandler		&handler = inEditor->Handler( *ev );
				handler.Invalidate( *inEditor, *ev );
				handler.Invalidate( *inEditor, newEv );
			}

				// replace the event in the sequence.
			marker.Replace( &newEv, 1, undoAction );
		}
	}
	else
	{
			// Applying a modfier to the events which can't
			// change event order.
	
		EventPtr	tempBuffer,
				ev;
		long		count, i;
		
			// If the edit was such that the event might change their positions
			// in the list, then extract the events, change them, and then
			// re-merge the changed events into the list.

		count = events.ExtractSelected( tempBuffer, undoAction );

		for (i = 0, ev = tempBuffer; i < count; i++, ev++)
		{
				// Remember what event types we changed
			eventsModified[ ev->Command() ] = true;

			if (inEditor != NULL)
			{
				const CAbstractEventHandler		&handler = inEditor->Handler( *ev );
				handler.Invalidate( *inEditor, *ev );
				op( *ev, clockType );
				handler.Invalidate( *inEditor, *ev );
			}
			else
			{
				op( *ev, clockType );
			}
		}

		events.Merge( tempBuffer, count, undoAction );
		delete [] tempBuffer;
	}

		// Add the undo action to the undo list for this observable
	if (undoAction) AddUndoAction( undoAction );

		// If the modified event was a time signature event, then
		// go head and invalidate the track's signature map.
	if (eventsModified[ EvtType_TimeSig ]) InvalidateSigMap();

		// If the modified event was a tempo change event, then
		// go head and invalidate the document's tempo map.
	if (eventsModified[ EvtType_Tempo ]) InvalidateTempoMap();

		// Compute the summary of the selection
	SummarizeSelection();
	
		// Save aggregate action stuff. Since SummarizeSelection wipes out this
		// information, this must be done after that call.
	prevAggregateAction = inAggregateAction;

		// Set the modified flag in the document
	Document().SetModified();

		// Compute an update hint which is the union
		// of the old and new selection ranges.

	CUpdateHint		hint;
	hint.AddInt32( "MinTime", MIN( minTime, MinSelectTime() ) );
	hint.AddInt32( "MaxTime", MAX( maxTime, MaxSelectTime() ) );
	if (		prevTrackDuration != LastEventTime()
		||	prevLogicalLength != LogicalLength())
	{
		CTrack::AddUpdateHintBits( hint, CTrack::Update_Duration );
	}

	if (inEditor) inEditor->PostUpdate( &hint, true );
	else PostUpdate( &hint, NULL );
}

	// Create a new event
void CEventTrack::CreateEvent(
	CEventEditor			*inEditor,
	Event				&newEv,
	const char			*inActionLabel )
{
	StSubjectLock			trackLock( *this, Lock_Exclusive );
	EventListUndoAction	*undoAction = NULL;
		// Initialize an event marker for this track.
	long					prevTrackDuration = LastEventTime(),
						prevLogicalLength = LogicalLength();
	long					minTime = LONG_MAX,
						maxTime = LONG_MIN;

	if (SelectionType() != CEventTrack::Select_None)
	{
		minTime = MinSelectTime(),
		maxTime = MaxSelectTime();
	}

		// We need a try block here, because this operation can potentially
		// run out of memory, especially if there is not enough memory to
		// create the undo records needed.

	undoAction = new EventListUndoAction( events, *this, inActionLabel );

		// Creating a new event
	newEv.SetSelected( true );
	events.Merge( &newEv, 1, undoAction );

		// If the new event was a time signature event, then
		// go head and invalidate the track's signature map.
	if (newEv.Command() == EvtType_TimeSig) InvalidateSigMap();

		// If the new event was a tempo change event, then
		// go head and invalidate the document's tempo map.
	if (newEv.Command() == EvtType_Tempo) InvalidateTempoMap();

		// Add the undo action to the undo list for this observable
	if (undoAction) AddUndoAction( undoAction );

		// Compute the summary of the selection
	SummarizeSelection();
	
		// Set the modified flag in the document
	Document().SetModified();

		// Compute an update hint which is the union
		// of the old and new selection ranges.
	if (SelectionType() != CEventTrack::Select_None)
	{
			// If this is a master track, then make this the active one.
		Document().SetActiveMaster( this );
		
		minTime = MIN( minTime, MinSelectTime() );
		maxTime = MAX( maxTime, MaxSelectTime() );
	}
	
	if (minTime <= maxTime)
	{
		CUpdateHint		hint;
		hint.AddInt32( "MinTime", minTime );
		hint.AddInt32( "MaxTime", maxTime );
		if (		prevTrackDuration != LastEventTime()
			||	prevLogicalLength != LogicalLength())
		{
			CTrack::AddUpdateHintBits( hint, CTrack::Update_Duration );
		}

		if (inEditor) inEditor->PostUpdate( &hint, true );
		else PostUpdate( &hint, NULL );
	}
}

	// Change some attribute of all of the selected events.
void CEventTrack::CopySelectedEvents(
	CEventEditor			*inEditor,
	EventOp				&op,
	const char			*inActionLabel )
{
	StSubjectLock			trackLock( *this, Lock_Exclusive );
	EventListUndoAction	*undoAction = NULL;
	TClockType			clockType = ClockType();
		// Initialize an event marker for this track.
	long					prevTrackDuration = LastEventTime(),
						prevLogicalLength = LogicalLength();
	long					minTime = MinSelectTime(),
						maxTime = MaxSelectTime();
	uint8				eventsModified[ 127 ];

	if (SelectionType() == CEventTrack::Select_None) return;

	memset( eventsModified, 0, sizeof eventsModified );

		// We need a try block here, because this operation can potentially
		// run out of memory, especially if there is not enough memory to
		// create the undo records needed.

	undoAction = new EventListUndoAction( events, *this, inActionLabel );

		// Making a copy of the dragged events
		
	EventPtr	tempBuffer,
			ev;
	long		i, count;
			
		// Make a copy of all selected events, modify the
		// copies according to the drag ops, and re-insert the
		// new events into the track. Oh, and deselect the old
		// events too.

	count = events.CopySelected( tempBuffer );

	DeselectAll( inEditor );

	for (i = 0, ev = tempBuffer; i < count; i++, ev++)
	{
			// Remember what event types we changed
		eventsModified[ ev->Command() ] = true;

		op( *ev, clockType );
		if (inEditor != NULL)
		{
			(inEditor->Handler( *ev )).Invalidate( *inEditor, *ev );
		}
	}

	events.Merge( tempBuffer, count, undoAction );
	delete [] tempBuffer;

		// Add the undo action to the undo list for this observable
	if (undoAction) AddUndoAction( undoAction );

		// If the new event was a time signature event, then
		// go head and invalidate the track's signature map.
	if (eventsModified[ EvtType_TimeSig ]) InvalidateSigMap();

		// If the new event was a tempo change event, then
		// go head and invalidate the document's tempo map.
	if (eventsModified[ EvtType_Tempo ]) InvalidateTempoMap();

		// Compute the summary of the selection
	SummarizeSelection();
	
		// Set the modified flag in the document
	Document().SetModified();

		// Compute an update hint which is the union
		// of the old and new selection ranges.
	CUpdateHint		hint;
	hint.AddInt32( "MinTime", MIN( minTime, MinSelectTime() ) );
	hint.AddInt32( "MaxTime", MAX( maxTime, MaxSelectTime() ) );
	if (		prevTrackDuration != LastEventTime()
		||	prevLogicalLength != LogicalLength())
	{
		CTrack::AddUpdateHintBits( hint, CTrack::Update_Duration );
	}

	if (inEditor) inEditor->PostUpdate( &hint, true );
	else PostUpdate( &hint, NULL );
}

	// Merge-in an event stream.
void CEventTrack::MergeEvents(
	Event				*inEvents,
	int32				eventCount,
	EventListUndoAction	*undoAction)
{
	if (eventCount <= 0) return;
	
		// Initialize an event marker for this track.
	long				prevTrackDuration = LastEventTime(),
					prevLogicalLength = LogicalLength();
	long				minTime = 0,
					maxTime = LONG_MAX;
	uint8				eventsModified[ 127 ];

	memset( eventsModified, 0, sizeof eventsModified );

		// We need a try block here, because this operation can potentially
		// run out of memory, especially if there is not enough memory to
		// create the undo records needed.

	EventPtr	ev;
	long		i;
	
	minTime = inEvents[0].Start();
	
	for (i = 0, ev = inEvents; i < eventCount; i++, ev++)
	{
			// Remember what event types we changed
		eventsModified[ ev->Command() ] = true;
		maxTime = MAX( maxTime, ev->Stop() );
	}

	events.Merge( inEvents, eventCount, undoAction );

		// If the new event was a time signature event, then
		// go head and invalidate the track's signature map.
	if (eventsModified[ EvtType_TimeSig ]) InvalidateSigMap();

		// If the new event was a tempo change event, then
		// go head and invalidate the document's tempo map.
	if (eventsModified[ EvtType_Tempo ]) InvalidateTempoMap();

		// Compute the summary of the selection
	SummarizeSelection();
	
		// Set the modified flag in the document
	Document().SetModified();

	CUpdateHint		hint;
	hint.AddInt32( "MinTime", minTime );
	hint.AddInt32( "MaxTime", maxTime );
	if (		prevTrackDuration != LastEventTime()
		||	prevLogicalLength != LogicalLength())
	{
		CTrack::AddUpdateHintBits( hint, CTrack::Update_Duration );
	}

	PostUpdate( &hint, NULL );
}

	// Filter an event through all of the filters assigned to this track.
void CEventTrack::FilterEvent( Event &ioEv )
{
	for (int i = 0; i < filterCount; i++)
	{
		(*filters[ i ])( ioEv, clockType );
	}
}

void CEventTrack::CompileOperators()
{
	int			i;

	StSubjectLock			trackLock( *this, Lock_Exclusive );
	
		// Release ref counts on all old filters.
	for (i = 0; i < filterCount; i++)
	{
		CRefCountObject::Release( filters[ i ] );
	}
	
	filterCount = 0;
	
		// Add active filters associated with document to this list.
	for (	i = 0;
			i < Document().CountActiveOperators() && filterCount < Max_Track_Filters;
			i++)
	{
		filters[ filterCount++ ] = Document().ActiveOperatorAt( i );
	}

		// Add active filters associated with this track to compiled list,
		// unless they were already added from document.
	for (	i = 0;
			i < CountOperators() && filterCount < Max_Track_Filters;
			i++)
	{
		EventOp		*op = OperatorAt( i );
		
		if (Document().ActiveOperatorIndex( op ) < 0)
		{
			filters[ filterCount++ ] = op;
		}
		else CRefCountObject::Release( filters[ i ] );
	}
}

	/**	Set an operator active / inactive. */
void CEventTrack::SetOperatorActive( EventOp *inOp, bool enabled )
{
	StSubjectLock			trackLock( *this, Lock_Exclusive );

	if (enabled)
	{
		if (!operators.HasItem( inOp ))
		{
				// Add to list of active operators
			inOp->Acquire();
			operators.AddItem( inOp );

				// Recompile operator list.
			CompileOperators();
		}
	}
	else
	{
		if (operators.HasItem( inOp ))
		{
				// Remove from list of active operators
			operators.RemoveItem( inOp );
			CRefCountObject::Release( inOp );

				// Recompile operator list.
			CompileOperators();
		}
	}
}

	// For operations on master tracks... 
CEventTrack *CEventTrack::Sibling()
{
	if (GetID() == 0) return (CEventTrack *)Document().FindTrack( 1L );
	if (GetID() == 1) return (CEventTrack *)Document().FindTrack( 0L );
	return NULL;
}

	/**	Overrides AddUndoAction from CObservableSubject to deal with
		master track issues. */
void CEventTrack::AddUndoAction( UndoAction *inAction )
{
		// If this is a master real track, then always add undo info to first master track.
	if (GetID() == 0)
	{
		CTrack *tk = Document().FindTrack( 1L );

		tk->AddUndoAction( inAction );
	}
	else
	{
		CTrack::AddUndoAction( inAction );
	}
}

CEventSelectionUpdateHint::CEventSelectionUpdateHint(
	const CEventTrack &track,
	bool selChangeOnly)
{
	if (track.CountItems())
	{
		AddInt32("MinTime", track.MinSelectTime());
		AddInt32("MaxTime", track.MaxSelectTime());
	}
	if (selChangeOnly)
	{
		AddBool("SelChange", true);
	}
}

CEventUpdateHint::CEventUpdateHint(
	const CEventTrack &track,
	const Event &event)
{
	AddInt32("MinTime", event.Start());
	AddInt32("MaxTime", event.Stop());
}

		/** Write the VCTable to a MeV file. */
void CEventTrack::WriteTrack( CIFFWriter &writer )
{
	uint8			gsnap = gridSnapEnabled ? 1 : 0;

	CTrack::WriteTrack( writer );

		// Write section markers if non-default.
	if (sectionStart != 0 || sectionEnd != 0)
	{
		writer.Push( Track_Section_ID );
		writer << sectionStart << sectionEnd;
		writer.Pop();
	}

	writer.Push( Track_Grid_ID );
	writer << timeGridSize << gsnap;
	writer.Pop();
	
	// REM: We need to write the operators to the track. We need IDs and keys...

	StSubjectLock		trackLock( *this, Lock_Shared );

	if (events.TotalItems() > 0)
	{
		writer.Push( Body_ID );
		WriteEventList( writer, events );
		writer.Pop();
	}
}

void CEventTrack::ReadTrackChunk( CIFFReader &reader )
{
	uint8			gsnap;

	switch (reader.ChunkID()) {
	case Track_Section_ID:
		reader >> sectionStart >> sectionEnd;
		break;

	case Track_Grid_ID:
		reader >> timeGridSize >> gsnap;
		gridSnapEnabled = gsnap ? 1 : 0;
		break;

	case Body_ID:
		{
			StSubjectLock		trackLock( *this, Lock_Exclusive );

			ReadEventList( reader, events );
		}
		break;

	default:
		CTrack::ReadTrackChunk( reader );
		break;
	}
}

void CEventTrack::LockChannel( int32 inChannel, bool inLocked )
{
	if (IsChannelLocked( inChannel ) != inLocked)
	{
		lockedChannels.Set( inChannel, inLocked );
		CUpdateHint		hint;
		
		if (inLocked == false)
		{
			StSubjectLock					lock( *this, Lock_Exclusive );
		
			EventMarker					marker( events );
			const Event					*ev;
	
				// For each event, select it.
			for (	ev = marker.FirstItemInRange( MinSelectTime(), MaxSelectTime() );
					ev;
					ev = marker.NextItemInRange( MinSelectTime(), MaxSelectTime() ) )
			{
				if (ev->GetVChannel() == inChannel)
					((Event *)ev)->SetSelected( false );
			}
	
			SummarizeSelection();
		}			
	
		hint.AddInt8( "channel", inChannel );
		PostUpdate( &hint, NULL );
	}
}

bool CEventTrack::IsChannelLocked( const Event &ev )
{
	return (ev.HasProperty( Event::Prop_Channel ) && IsChannelLocked( ev.GetVChannel() ));
}
