/* ===================================================================== *
 * EventTrack.cpp (MeV/Engine)
 * ===================================================================== */

#include "EventTrack.h"
#include "DocApp.h"
#include "MeVDoc.h"
#include "EventOp.h"
#include "EventEditor.h"
#include "EventRenderer.h"
#include "MeVFileID.h"

// Support Kit
#include <Debug.h>

#define D_INTERNAL(x) //PRINT(x)	// Internal Operations

// ---------------------------------------------------------------------------
// Constructor/Destructor

CEventTrack::CEventTrack(
	CMeVDoc &inDoc,
	TClockType cType,
	int32 inID,
	const char *inName)
	: CTrack(inDoc, cType, inID, inName)
{
	StSubjectLock		lock( *this, Lock_Exclusive );

	// Set the initial grid size and enable gridsnap
	timeGridSize = Ticks_Per_QtrNote / 2;
	gridSnapEnabled = true;

	// Initialize the count of selected events (initially 0)
	selectionCount = 0;

	// Associate the current event marker with the event list.
	currentEvent.SetList(events);

	// Initialize the list of filters
	filterCount = 0;

	// Set the signature map to invalid so that it will be regenerated
	validSigMap = false;

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
// Accessors

CDestination *
CEventTrack::GetNextUsedDestination(
	long *index) const
{
	ASSERT(IsReadLocked());

	used_destinations_map::const_iterator dest = m_destinations.begin();
	for (long i = 0; i < *index; i++)
		dest++;
	(*index)++;
	if (dest != m_destinations.end())
		return dest->first;
	else
		return NULL;
}

// ---------------------------------------------------------------------------
// Compute summary information for currently selected events

void CEventTrack::SummarizeSelection()
{
	StSubjectLock		lock( *this, Lock_Exclusive );
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
	const CEvent		*ev;

	lastEventTime = logicalLength = 0;

		// Only metered sequences can have time signature events, so there's
		// no point in even checking.
	if (sigMap.clockType != ClockType_Metered) validSigMap = true;

		// Every track starts at 4/4 (the default), but can be immediately changed
		// by the very first event.
	prevSigStart = 0;
	prevMinorUnitDur = Ticks_Per_QtrNote;
	prevMajorUnitDur = Ticks_Per_QtrNote * 4;

		// Loop through every event in the sequence and compile summary information
	for (	ev = marker.First();
			ev;
			ev = marker.Seek( 1 ))
	{
		long		first = ev->Start(),
				last;
				
			// last <== the stop time of the event
		if (ev->HasProperty( CEvent::Prop_Duration ))
			last = ev->Stop();
		else last = first;

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
				// Add the event to the selection range
			minSelectTime = MIN( minSelectTime, first );
			maxSelectTime = MAX( maxSelectTime, last );

				// Set the marker which points to the first selected event
				// -- but only if it does not already point to a selected event.
			if (selectionCount == 0)
			{
				const CEvent	*cEv = currentEvent;
			
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
		for (const CEvent *ev = marker.First(); ev; ev = marker.Seek( 1 ) )
		{
			if (!ev->IsSelected())
			{
				const_cast<CEvent *>(ev)->SetSelected( true );
				(inEditor->RendererFor(*ev))->Invalidate(*ev);
			}
		}
	}
	else
	{
		for (const CEvent *ev = marker.First(); ev; ev = marker.Seek( 1 ) )
		{
			const_cast<CEvent *>(ev)->SetSelected( true );
		}
	}

	SummarizeSelection();

	CEventSelectionUpdateHint hint(*this);
	PostUpdate(&hint, inEditor);
}

// ---------------------------------------------------------------------------
// Select all events.

void CEventTrack::DeselectAll( CEventEditor *inEditor, bool inDoUpdate )
{
	StSubjectLock					lock( *this, Lock_Exclusive );
	EventMarker					marker( events );
	CEventSelectionUpdateHint		hint( *this, true );
	const CEvent					*ev;

	if (inEditor != NULL)
	{
			// For each event, select it.
		for (	ev = marker.FirstItemInRange( MinSelectTime(), MaxSelectTime() );
				ev;
				ev = marker.NextItemInRange( MinSelectTime(), MaxSelectTime() ) )
		{
			if (ev->IsSelected())
			{
				const_cast<CEvent *>(ev)->SetSelected( false );
				(inEditor->RendererFor(*ev))->Invalidate(*ev);
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
			const_cast<CEvent *>(ev)->SetSelected( false );
		}
	}

	SummarizeSelection();
	if (inDoUpdate)
		PostUpdate(&hint, inEditor);
}

// ---------------------------------------------------------------------------
// Delete all selected events.

void
CEventTrack::DeleteEvent(
	const CEvent *which)
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
	for (const CEvent *ev = marker.First();
		 ev != NULL;
		 ev = marker.Seek(1))
	{
		if (ev == which)
		{
			// make a copy of the event
			CEvent evCopy(*ev);

			// Delete from original list
			marker.Remove(1, undoAction);

			_eventRemoved(&evCopy);

			// If the delete event was a time signature event, then
			// go head and invalidate the track's signature map.
			if (evCopy.Command() == EvtType_TimeSig)
				InvalidateSigMap();
		
			// If the deleted event was a tempo change event, then
			// go head and invalidate the document's tempo map.
			else if (evCopy.Command() == EvtType_Tempo)
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
	for (const CEvent *ev = marker.FirstItemInRange(MinSelectTime(), MaxSelectTime());
		 ev != NULL;
		 )
	{
		if (ev->IsSelected())
		{
			// Remember what event types we changed
			eventsModified[ev->Command()] = true;
			CEvent evCopy(*ev);
			// Delete from original list
			marker.Remove(1, undoAction);
			_eventRemoved(&evCopy);
			// Increment to next event
			ev = (const CEvent *)marker;
		}
		else
		{
			ev = (const CEvent *)marker.Seek(1);
		}
	}

	// If the delete event was a time signature event, then
	// go head and invalidate the track's signature map.
	if (eventsModified[EvtType_TimeSig])
		InvalidateSigMap();

	// If the deleted event was a tempo change event, then
	// go head and invalidate the document's tempo map.
	if (eventsModified[EvtType_Tempo])
		InvalidateTempoMap();

	// REM: We should be smarter about this update...
	SummarizeSelection();
	PostUpdate(&hint);

	// Add the undo action to the undo list for this observable
	if (undoAction)
		AddUndoAction(undoAction);
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
		for (CEvent *ev = const_cast<CEvent *>(marker.First());
			 ev;
			 ev = const_cast<CEvent *>(marker.Seek(1)))
		{
			if (!ev->IsSelected())
				continue;

			// Remember what event types we changed
			eventsModified[ev->Command()] = true;

			// Temporarily copy the event, and apply the vertical
			// component of the dragging to it.
			CEvent newEv(*ev);
			op( newEv, clockType );

			// Invalidate the new and old positions of the event
			if (inEditor != NULL)
			{
				const CEventRenderer *renderer = inEditor->RendererFor(*ev);
				renderer->Invalidate(*ev);
				renderer->Invalidate(newEv);
			}

			// replace the event in the sequence.
			marker.Replace(&newEv, 1, undoAction);
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
				const CEventRenderer *renderer = inEditor->RendererFor(*ev);
				renderer->Invalidate(*ev);
				op(*ev, clockType);
				renderer->Invalidate(*ev);
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

	PostUpdate(&hint, inEditor);
}

	// Create a new event
void CEventTrack::CreateEvent(
	CEventEditor			*inEditor,
	CEvent				&newEv,
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

		// Creating a new event..
		// dan: I don't like events starting as selected when using mouse. but currently
		//they will not be drawn properly unless selected.
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

		PostUpdate(&hint, inEditor);
	}

	_eventAdded(&newEv);
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
			(inEditor->RendererFor(*ev))->Invalidate(*ev);
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

	PostUpdate(&hint, inEditor);
}

	// Merge-in an event stream.
void CEventTrack::MergeEvents(
	CEvent				*inEvents,
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

	PostUpdate(&hint, NULL);
}

	// Filter an event through all of the filters assigned to this track.
void CEventTrack::FilterEvent( CEvent &ioEv )
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

	/**	Overrides AddUndoAction from CObservable to deal with
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

void
CEventTrack::_eventAdded(
	const CEvent *ev)
{
	D_INTERNAL(("CEventTrack::_eventAdded()\n"));

	// update list of used destinations if necessary
	if (ev->HasProperty(CEvent::Prop_Channel))
	{
		CDestination *dest = Document().FindDestination(ev->GetVChannel());
		
		if (m_destinations.find(dest) == m_destinations.end())
		{
			D_INTERNAL((" -> destination %s now in use by track %s\n",
				   		dest->Name(), Name()));

			m_destinations[dest] = 1;

			CUpdateHint hint;
			hint.AddInt32("TrackAttrs", Update_AddDest);
			hint.AddInt32("DestID", dest->ID());
			PostUpdate(&hint);
		}
		else
		{
			m_destinations[dest]++;
		}
	}
}

void
CEventTrack::_eventRemoved(
	const CEvent *ev)
{
	D_INTERNAL(("CEventTrack::_eventRemoved()\n"));

	// update list of used destinations if necessary
	if (ev->HasProperty(CEvent::Prop_Channel))
	{
		CDestination *dest = Document().FindDestination(ev->GetVChannel());
		m_destinations[dest]--;
		if (m_destinations[dest] == 0)
		{
			D_INTERNAL((" -> destination %s no longer in use by track %s\n",
				   		dest->Name(), Name()));

			m_destinations.erase(dest);

			CUpdateHint hint;
			hint.AddInt32("TrackAttrs", Update_DelDest);
			hint.AddInt32("DestID", dest->ID());
			PostUpdate(&hint);
		}
	}
}

void
CEventTrack::_initUsedDestinations()
{
	D_INTERNAL(("CEventTrack::_initUsedDestinations()\n"));
	ASSERT((IsReadLocked()));

	// Check the destination of each event
	EventMarker	marker(events);
	for (CEvent *ev = const_cast<CEvent *>(marker.First());
		 ev != NULL;
		 ev = const_cast<CEvent *>(marker.Seek(1)))
	{
		if (ev->HasProperty(CEvent::Prop_Channel))
		{
			CDestination *dest = Document().FindDestination(ev->GetVChannel());
			if (m_destinations.find(dest) == m_destinations.end())
				m_destinations[dest] = 0;
			else
				m_destinations[dest]++;
		}
	}
}

// ---------------------------------------------------------------------------
// CSerializable Implementation

void
CEventTrack::ReadChunk(
	CIFFReader &reader)
{
	switch (reader.ChunkID())
	{
		case TRACK_GRID_CHUNK:
		{
			CWriteLock lock(this);
			reader >> timeGridSize;
			reader >> gridSnapEnabled;
			break;
		}
		case Body_ID:
		{
			CWriteLock lock(this);
			ReadEventList(reader, events);
			SummarizeSelection();
			_initUsedDestinations();
			break;
		}
		default:
		{
			CTrack::ReadChunk(reader);
			break;
		}
	}
}

void
CEventTrack::Serialize(
	CIFFWriter &writer)
{
	CTrack::Serialize(writer);

	CReadLock lock(this);

	writer.Push(TRACK_GRID_CHUNK);
	writer << timeGridSize;
	writer << gridSnapEnabled;
	writer.Pop();
	
	if (events.TotalItems() > 0)
	{
		writer.Push(Body_ID);
		WriteEventList(writer, events);
		writer.Pop();
	}

	// +++ we need to write the operators to the track. We need IDs and keys...
}

// ---------------------------------------------------------------------------
// CEventSelectionUpdateHint Implementation

CEventSelectionUpdateHint::CEventSelectionUpdateHint(
	const CEventTrack &track,
	bool selChangeOnly)
{
	if (track.CountEvents())
	{
		AddInt32("MinTime", track.MinSelectTime());
		AddInt32("MaxTime", track.MaxSelectTime());
	}
	if (selChangeOnly)
	{
		AddBool("SelChange", true);
	}
}

// ---------------------------------------------------------------------------
// CEventUpdateHint Implementation

CEventUpdateHint::CEventUpdateHint(
	const CEventTrack &track,
	const CEvent &event)
{
	AddInt32("MinTime", event.Start());
	AddInt32("MaxTime", event.Stop());
}

// END - EventTrack.cpp
