/* ===================================================================== *
 * VelocityEditor.cpp (MeV/User Interface)
 * ===================================================================== */

#include "VelocityEditor.h"

#include "EventTrack.h"
#include "VChannel.h"
#include "MeVDoc.h"
#include "MathUtils.h"
#include "ResourceUtils.h"

#include "StripLabelView.h"

// ---------------------------------------------------------------------------
// Note handler class for linear editor

	// Invalidate the event
void CVelocityNoteEventHandler::Invalidate(
	CEventEditor	&editor,
	const Event		&ev ) const
{
	CVelocityEditor	&vEditor = (CVelocityEditor &)editor;

	BRect			r( vEditor.ViewBounds() );

	r.left	= vEditor.TimeToViewCoords( ev.Start() );
	r.right	= vEditor.TimeToViewCoords( ev.Stop()  );
	r.top   = r.bottom - (MAX( ev.note.attackVelocity, ev.note.releaseVelocity ) * r.Height()) / 128 - 1.0;

	vEditor.Invalidate( r );
}

	// Draw the event (or an echo)
void CVelocityNoteEventHandler::Draw(
	CEventEditor	&editor,
	const Event		&ev,
	bool 			shadowed ) const
{
	CVelocityEditor	&vEditor = (CVelocityEditor &)editor;

	VChannelEntry	*vce = vEditor.TrackWindow()->Document()->GetVChannel( ev.GetVChannel() );

	BRect			r( vEditor.ViewBounds() );
	BPoint			points[ 4 ] = { BPoint( 0.0, 0.0 ), BPoint( 0.0, 0.0 ), BPoint( 0.0, 0.0 ), BPoint( 0.0, 0.0 ) };
	
	r.left	= vEditor.TimeToViewCoords( ev.Start() );
	r.right	= vEditor.TimeToViewCoords( ev.Stop()  );

	points[ 0 ].x = points[ 1 ].x = r.left;
	points[ 2 ].x = points[ 3 ].x = r.right;
	
	points[ 0 ].y = points[ 3 ].y = r.bottom + 1.0;
	points[ 1 ].y = r.bottom - (int32)(ev.note.attackVelocity * r.Height()) / 128;
	points[ 2 ].y = r.bottom - (int32)(ev.note.releaseVelocity * r.Height()) / 128;
	r.top = MIN( points[ 1 ].y, points[ 2 ].y );

	if (points[ 1 ].y == points[ 2 ].y)
	{
			// Rects fill faster, so let's use that if we can.
		if (ev.IsSelected() && editor.IsSelectionVisible())
			vEditor.SetHighColor( 0, 0, 255 );
		else vEditor.SetHighColor( 0, 0, 0 );
		
		vEditor.StrokeRect( r );
		r.InsetBy( 1.0, 1.0 );
		vEditor.SetHighColor( vce->fillColor );
		vEditor.SetDrawingMode( B_OP_BLEND );
		vEditor.FillRect( r );
	}
	else
	{
		vEditor.SetHighColor( vce->fillColor );
		vEditor.SetDrawingMode( B_OP_BLEND );
		vEditor.FillPolygon( points, 4, r );

		vEditor.SetDrawingMode( B_OP_COPY );

		if (ev.IsSelected() && editor.IsSelectionVisible())
			vEditor.SetHighColor( 0, 0, 255 );
		else vEditor.SetHighColor( 0, 0, 0 );
		
		vEditor.StrokePolygon( points, 4, r, false );
	}
	vEditor.SetDrawingMode( B_OP_COPY );
}

	// Compute the extent of the event.
BRect CVelocityNoteEventHandler::Extent(
	CEventEditor		&editor,
	const Event		&ev ) const
{
	CVelocityEditor	&vEditor = (CVelocityEditor &)editor;

	BRect			r( vEditor.ViewBounds() );

	r.left	= vEditor.TimeToViewCoords( ev.Start() );
	r.right	= vEditor.TimeToViewCoords( ev.Stop()  );
	r.top   = r.bottom - (MAX( ev.note.attackVelocity, ev.note.releaseVelocity ) * r.Height()) / 128;

	return r;
}

	// Pick a single event and return the part code
	// (or -1 if event not picked)
long CVelocityNoteEventHandler::Pick(
	CEventEditor	&editor,
	const Event		&ev,
	BPoint			pickPt,
	short			&partCode ) const
{
#if 0
	CLinearEditor	&lEditor = (CLinearEditor &)editor;
	int				top,
					bottom;

	bottom= lEditor.PitchToViewCoords( ev.note.pitch );
	top   = bottom - lEditor.whiteKeyStep;

	return lEditor.PickDurationEvent( ev, top, bottom, pickPt, partCode );
#endif
	return 0;
}

const uint8 *CVelocityNoteEventHandler::CursorImage( short partCode ) const
{
#if 0
	size_t		size;

	switch (partCode) {
	case 0:
		return B_HAND_CURSOR;			// Return the normal hand cursor

	case 1:								// Return resizing cursor
		if (resizeCursor == NULL)
		{
			resizeCursor = ResourceUtils::LoadCursor(2);
		}
		return resizeCursor;
	}
#endif	
	return NULL;
}

	// Quantize the vertical position of the mouse based
	// on the event type and return a value delta.
long CVelocityNoteEventHandler::QuantizeDragValue(
	CEventEditor	&editor,
	const Event		&inClickEvent,
	short			partCode,			// Part of event clicked
	BPoint			inClickPos,
	BPoint			inDragPos ) const
{
#if 0
	CLinearEditor	&lEditor = (CLinearEditor &)editor;

		// Get the pitch and y position of the old note.

	long			oldPitch	= inClickEvent.note.pitch;
	float			oldYPos		= lEditor.PitchToViewCoords( oldPitch );
	long			newPitch;

		// Add in the vertical drag delta to the note position,
		// and compute the new pitch.
		
		// REM: This does not account for feedback on the 
		// Piano roll vertical strip. That's a problem!
	
	newPitch = lEditor.ViewCoordsToPitch(
		oldYPos + inDragPos.y - inClickPos.y - lEditor.whiteKeyStep / 2, false );
					
	return newPitch - oldPitch;
#endif
	return 0;
}

EventOp *CVelocityNoteEventHandler::CreateDragOp(
	CEventEditor	&editor,
	const Event		&ev,
	short			partCode,
	long			timeDelta,			// The horizontal drag delta
	long			valueDelta ) const
{
#if 0
	if (partCode == 0)
		return new PitchOffsetOp( valueDelta );
	else return NULL;
#endif
	return NULL;
}

EventOp *CVelocityNoteEventHandler::CreateTimeOp(
	CEventEditor	&editor,
	const Event		&ev,
	short			partCode,
	long			timeDelta,			// The horizontal drag delta
	long			valueDelta ) const
{
	return NULL;
}

// ---------------------------------------------------------------------------
// Dispatch table for linear editor

CVelocityNoteEventHandler		velocityNoteHandler;

// ---------------------------------------------------------------------------
// Velocity Editor class

	// ---------- Constructor

CVelocityEditor::CVelocityEditor(
	BLooper			&inLooper,
	CTrackEditFrame &inFrame,
	BRect			rect )
	:	CEventEditor(	inLooper, inFrame, rect,
						"Velocity Strip", false, false )
{

	SetHandlerFor(EvtType_Note, &velocityNoteHandler);
	SetFlags(Flags() | B_FULL_UPDATE_ON_RESIZE);

	// Make the label view on the left-hand side
	SetLabelView(new CStripLabelView(BRect(-1.0, -1.0, 20.0, rect.Height() + 1),
									 "Velocity", B_FOLLOW_TOP_BOTTOM,
									 B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE));
}

void CVelocityEditor::Draw( BRect updateRect )
{
	long				startTime = ViewCoordsToTime( updateRect.left - 1.0 ),
					stopTime  = ViewCoordsToTime( updateRect.right + 1.0 );
	long				drawCount = 0;

	SetHighColor( 255, 255, 255 );
	FillRect( updateRect );

	SetHighColor( 220, 220, 220 );

	DrawGridLines( updateRect );

		// Initialize an event marker for this track.
	StSubjectLock		trackLock( *Track(), Lock_Shared );
	EventMarker		marker( Track()->Events() );
	
	bounds = Bounds();

		// For each event that overlaps the current view, draw it.
	for (	const Event *ev = marker.FirstItemInRange( startTime, stopTime );
			ev;
			ev = marker.NextItemInRange( startTime, stopTime ) )
	{
			// Only draw events for non-locked channels...
		if (ev->Command() != EvtType_Note) continue;
		if (Track()->IsChannelLocked( *ev )) continue;
		
		Handler( *ev ).Draw( *this, *ev, false );
		
		drawCount++;
		if (drawCount >= 40)
		{
				// Let some other threads have a turn, OK?
				// (This wouldn't be needed with shared locks you know...)
			trackLock.Release();
			trackLock.Acquire();
			drawCount = 0;
		}
	}
}

		// Update message from another observer
void CVelocityEditor::OnUpdate( BMessage *inMsg )
{
	int32		minTime = 0,
				maxTime = LONG_MAX;
	int32		trackHint;
	bool			flag;
	bool			selChange = false;
	int8			channel = -1;
	BRect		r( Bounds() );

	bounds = r;

	if (inMsg->FindInt32( "TrackAttrs", 0, &trackHint ) == B_OK)
	{
			// REM: what do we do if track changes name?
	
		if (!(trackHint &
			(CTrack::Update_Duration|CTrack::Update_SigMap|CTrack::Update_TempoMap)))
				return;
	}
	else trackHint = 0;

	if (inMsg->FindBool( "SelChange", 0, &flag ) == B_OK)
	{
		if (!IsSelectionVisible()) return;
		selChange = flag;
	}

	if (inMsg->FindInt32( "MinTime", 0, &minTime ) == B_OK)
	{
		r.left = TimeToViewCoords( minTime ) - 1.0;
	}
	else minTime = 0;

	if (inMsg->FindInt32( "MaxTime", 0, &maxTime ) == B_OK)
	{
		r.right = TimeToViewCoords( maxTime ) + 1.0;
	}
	else maxTime = LONG_MAX;
	
	if (inMsg->FindInt8( "channel", 0, &channel ) != B_OK) channel = -1;

	if (trackHint & CTrack::Update_Duration)
		TrackEditFrame().RecalcScrollRange();

	if (trackHint & (CTrack::Update_SigMap|CTrack::Update_TempoMap))
	{
// 	TrackWindow()->InvalidateRuler();
		Invalidate();			// Invalidate everything if signature map changed
	}
	else if (channel >= 0)
	{
		StSubjectLock		trackLock( *Track(), Lock_Shared );
		EventMarker		marker( Track()->Events() );
		
			// For each event that overlaps the current view, draw it.
		for (	const Event *ev = marker.FirstItemInRange( minTime, maxTime );
				ev;
				ev = marker.NextItemInRange( minTime, maxTime ) )
		{
			if (ev->HasProperty( Event::Prop_Channel ) && ev->GetVChannel() == channel)
				Handler( *ev ).Invalidate( *this, *ev );
		}
	}
	else if (selChange)
	{
		StSubjectLock		trackLock( *Track(), Lock_Shared );
		EventMarker		marker( Track()->Events() );

			// For each event that overlaps the current view, draw it.
		for (	const Event *ev = marker.FirstItemInRange( minTime, maxTime );
				ev;
				ev = marker.NextItemInRange( minTime, maxTime ) )
		{
			Handler( *ev ).Invalidate( *this, *ev );
		}
	}
	else
	{
		Invalidate( r );
	}
}

void CVelocityEditor::MouseMoved(
	BPoint		point,
	ulong		transit,
	const BMessage	*message)
{
	CEventEditor::MouseMoved(point, transit, message);

	if (transit == B_EXITED_VIEW)
	{
		TrackWindow()->DisplayMouseTime( NULL, 0 );
//		TrackWindow()->RestoreCursor();
		return;
	}

	TrackWindow()->DisplayMouseTime( Track(), ViewCoordsToTime( point.x ) );

//	SetViewCursor(UIDefs::CROSS_HAIR_CURSOR);
}

void CVelocityEditor::StartDrag( BPoint point, ulong buttons )
{
	BRect			r( Bounds() );

	dragTime		= ViewCoordsToTime( point.x );
	dragVelocity	= 127 * (r.bottom - point.y) / r.Height();
	m_dragType		= DragType_Sculpt;
	dragAction	= NULL;
	smallestTime = LONG_MAX;
	largestTime	= LONG_MIN;
	TrackWindow()->DisplayMouseTime( Track(), dragTime );
}

bool CVelocityEditor::DoDrag( BPoint point, ulong buttons )
{
	bounds = Bounds();

	if (m_dragType == DragType_Sculpt )
	{
		StSubjectLock		trackLock( *Track(), Lock_Exclusive );
		BRect			r( Bounds() );
		int32			time,
						time1,
						time2,
						dt,
						vel,
						vel1,
						vel2,
						dv;

			// If no undo record has been created yet, or someone did another
			// undoable action in-between calls to DoDrag, then start a new
			// undo action.
		if (		dragAction == NULL
			||	!Track()->IsMostRecentUndoAction( dragAction ))
		{
				// If there was another action since the last call to
				// DoDrag, we'll need to start the undo information fresh,
				// so post our update message and initialize time range.
			if (dragAction != NULL)
			{
					// If there were in fact ANY events saved, then
					// go ahead and do the post.
				if (largestTime >= smallestTime)
				{
					CUpdateHint		hint;
					hint.AddInt32( "MinTime", smallestTime );
					hint.AddInt32( "MaxTime", largestTime );
					PostUpdate( &hint, true );
				}
				smallestTime	= LONG_MAX;
				largestTime	= LONG_MIN;
			}

			dragAction = new EventListUndoAction( Track()->Events(), *Track(), "Edit Velocity" );
			Track()->AddUndoAction( dragAction );
		}

			// Compute the time and velocity coords of the mouse.
		time		= ViewCoordsToTime( point.x );
		vel		= 127.0 * (r.bottom - point.y) / r.Height();

		EventMarker		marker( Track()->Events() );

			// Sort the times. Velocitys are associated with the times.
		if (time < dragTime)
		{
			time1 = time; time2 = dragTime;
			vel1 = vel; vel2 = dragVelocity;
		}
		else
		{
			time2 = time; time1 = dragTime;
			vel2 = vel; vel1 = dragVelocity;
		}
		
			// Compute the delta values for slope
		dt = time2 - time1;
		dv = vel2 - vel1;

			// For each event that overlaps the current view, draw it.
		for (	const Event *ev = marker.FirstItemInRange( time1, time2 );
				ev;
				ev = marker.NextItemInRange( time1, time2 ) )
		{
			if (Track()->IsChannelLocked( *ev )) continue;

			if (ev->Command() == EvtType_Note)
			{
				Event	evCopy( *ev );
				bool		eventSaved = false;

					// Modify the note based on the button states.	

				if (buttons & B_PRIMARY_MOUSE_BUTTON)
				{
					int32		t = ev->Start();
			
					if (	t >= time1 && t < time2)
					{
						int32	v = (t - time1) * dv / dt + vel1;
					
						evCopy.note.attackVelocity = CLAMP( 1L, v, 127L );
						printf("ve %d\n",evCopy.note.attackVelocity);
					}
				}

				if (buttons & B_SECONDARY_MOUSE_BUTTON)
				{
					int32		t = ev->Stop();
			
					if (	t >= time1 && t < time2)
					{
						int32	v = (t - time1) * dv / dt + vel1;
					
						evCopy.note.releaseVelocity = CLAMP( 0L, v, 127L );
						
					}
				}
				
					// If the event overlaps the range of time between
					// smallest <--> largest then it has already been
					// stored in the undo repository.
				if (ev->Start() <= largestTime && ev->Stop() >= smallestTime)
				{
					eventSaved = true;
				}
				
				if (		ev->note.attackVelocity != evCopy.note.attackVelocity
					||	ev->note.releaseVelocity != evCopy.note.releaseVelocity)
				{
					Handler( *ev ).Invalidate( *this, *ev );

						// If the event has not yet been saved, then
						// go ahead and add it into the undo store
						// and replace the event.
					if (!eventSaved)
					{
						marker.Replace( &evCopy, 1, dragAction );
					}
					else
					{
							// If the event has already been added into the undo
							// store (which we can assume because we add ALL events
							// within the time range into the store, regardless
							// of wether they are modified or not), then we don't
							// want to take up memory by adding a redundant copy,
							// so just poke the event directly.
						((Event *)ev)->note.attackVelocity  = evCopy.note.attackVelocity;
						((Event *)ev)->note.releaseVelocity = evCopy.note.releaseVelocity;
					}
					Handler( *ev ).Invalidate( *this, *ev );
				}
				else
				{
						// If the event hasn't changed, go ahead and replace it
						// anyway, since this will cause the event to be entered
						// into the undo list. Basically, we want to make SURE
						// that all events within the range smallest - largest
						// are saved, so that any new events we modify only
						// need be saved if they are outside that range.
					if (!eventSaved)
					{
						marker.Replace( &evCopy, 1, dragAction );
					}
				}
			}
		}

		dragTime = time;
		dragVelocity = vel;

		smallestTime = MIN( smallestTime, dragTime );
		largestTime = MAX( largestTime, dragTime );

		TrackWindow()->DisplayMouseTime( Track(), dragTime );
	}
	return true;
}

void CVelocityEditor::FinishDrag(
	BPoint		point,
	ulong		buttons,
	bool		commit )
{
	Track()->SummarizeSelection();

	if (largestTime >= smallestTime)
	{
		CUpdateHint		hint;
		hint.AddInt32( "MinTime", smallestTime );
		hint.AddInt32( "MaxTime", largestTime );
		PostUpdate( &hint, true );
	}
	TrackWindow()->DisplayMouseTime( NULL, 0 );
}
