/* ===================================================================== *
 * EventEditor.cpp (MeV/User Interface)
 * ===================================================================== */

#include "EventEditor.h"
#include "EventOp.h"
#include "StdEventOps.h"
#include "DataSnap.h"
#include "PlayerControl.h"
#include "MeVApp.h"
#include "MeVDoc.h"

// Support Kit
#include <Beep.h>

CNullEventHandler		gNullEventHandler;
CEndEventHandler		gEndEventHandler;

// ---------------------------------------------------------------------------
// Constructor/Destructor

CEventEditor::CEventEditor(
	BLooper	&inLooper,
	CTrackEditFrame	&inFrame,
	BRect rect,
	const char *name,
	bool makeScroller = false,
	bool makeMagButtons = false)
	:	CStripView(inFrame, rect, name, makeScroller, makeMagButtons),
		CObserver(inLooper, inFrame.Track()),
		track((CEventTrack *)inFrame.Track()),
		frame(inFrame)
{
	Init();
}
					
CEventEditor::CEventEditor(
	BLooper &inLooper,
	CTrackEditFrame &inFrame,
	BRect rect,
	CEventTrack *inTrack,
	const char *name,
	bool makeScroller = false,
	bool makeMagButtons = false)
	:	CStripView(inFrame, rect, name, makeScroller, makeMagButtons),
		CObserver(inLooper, inTrack),
		track(inTrack),
		frame(inFrame)
{
	ruler = NULL;
	Init();
}
	
CEventEditor::~CEventEditor()
{
	delete lassoPoints;
}
	
// ---------------------------------------------------------------------------
// Init function for event editor.

void
CEventEditor::Init()
{
	dragOp = NULL;
	pbCount = 0;
	dragType = DragType_None;
	lassoPoints = NULL;

	// Set the handler array to the "null" handler.
	for (int i = 0; i < EvtType_Count; i++)
	{
		handlers[ i ] = &gNullEventHandler;
	}
}

// ---------------------------------------------------------------------------
// All event editors handle picking in the same way

const Event *
CEventEditor::PickEvent(
	EventMarker	&resultMarker,
	const BPoint &pickPt,
	short &resultPartCode)
{
	long startTime, stopTime;
	long bestPick = LONG_MAX;
	const int mouseLeftSlopSize = 128, mouseRightSlopSize = 8;
	short partCode;

	startTime = frame.ViewCoordsToTime(pickPt.x - mouseLeftSlopSize, track->ClockType());
	stopTime  = frame.ViewCoordsToTime(pickPt.x + mouseRightSlopSize, track->ClockType());

		// Initialize an event marker for this track.
	EventMarker		marker( Track()->Events() );
	const Event		*ev;

		// For each event that overlaps the current view, draw it.
	for (	ev = marker.FirstItemInRange( startTime, stopTime );
			ev;
			ev = marker.NextItemInRange( startTime, stopTime ) )
	{
		long dist;
		
			// Don't allow picking of events on locked channels...
		if (Track()->IsChannelLocked( *ev )) continue;
		
		dist = Handler( *ev ).Pick( *this, *ev, pickPt, partCode );

		if (dist < bestPick && dist >= 0)
		{
			bestPick = dist;
			resultPartCode = partCode;
			resultMarker = marker;
		}
	}

	if (bestPick >= LONG_MAX) return NULL;

	return resultMarker;
}

	// General mouse handling

void CEventEditor::MouseDown( BPoint point )
{
	ulong		buttons;

	clickPos = point;
	GetMouse( &point, &buttons, TRUE );
	StartDrag( point, buttons );

	do
	{
		snooze(20 * 1000);
		GetMouse( &point, &buttons, TRUE );

			// Implement dragging		
		if (DoDrag( point, buttons ) == false)
		{
			FinishDrag( point, buttons, false );
			return;
		}

			// Implement auto-scrolling (horizontal for frame)
		BRect	r( ViewBounds() );
		if (point.x > r.right)
		{
			frame.ScrollBy(	MIN( (point.x - r.right)/4, 10.0 ),
							B_HORIZONTAL );
		}
		else if (point.x < r.left)
		{
			frame.ScrollBy(	MAX( (point.x - r.left)/4, -10.0 ),
							B_HORIZONTAL );
		}
		
			// Implement auto-scrolling (vertical for editor)
		if (point.y > r.bottom)
		{
			ScrollBy(	MIN( (point.y - r.bottom)/4, 10.0 ),
						B_VERTICAL );
		}
		else if (point.y < r.top)
		{
			ScrollBy(	MAX( (point.y - r.top)/4, -10.0 ),
						B_VERTICAL );
		}
		
		Window()->UpdateIfNeeded();
	}
	while (buttons) ;

	FinishDrag( point, buttons, true );
}

// ---------------------------------------------------------------------------
// Invalidate all selected events.

void CEventEditor::InvalidateSelection()
{
	StSubjectLock	trackLock( *Track(), Lock_Shared );
	EventMarker		marker( Track()->Events() );
	const Event		*ev;

		// For each event that overlaps the current view, draw it.
	for (	ev = marker.FirstItemInRange( Track()->MinSelectTime(), Track()->MaxSelectTime() );
			ev;
			ev = marker.NextItemInRange( Track()->MinSelectTime(), Track()->MaxSelectTime() ) )
	{
		if (ev->IsSelected())
		{
			Handler( *ev ).Invalidate( *this, *ev );
		}
	}
}

// ---------------------------------------------------------------------------
// Invalidate all selected events, with a displacement

void CEventEditor::InvalidateSelection( EventOp &inOp )
{
	const Event		*ev;
	StSubjectLock	trackLock( *Track(), Lock_Shared );
	EventMarker		marker( Track()->Events() );
	TClockType		clockType = Track()->ClockType();

		// For each event that overlaps the current view, draw it.
	for (	ev = marker.FirstItemInRange( Track()->MinSelectTime(), Track()->MaxSelectTime() );
			ev;
			ev = marker.NextItemInRange( Track()->MinSelectTime(), Track()->MaxSelectTime() ) )
	{
			// REM: This can be made more efficient, possibly
			// by not invalidating notes outside the time range
	
		if (ev->IsSelected())
		{
			Event		evCopy( *(Event *)ev );
			
			inOp( evCopy, clockType );
			Handler( *ev ).Invalidate( *this, evCopy );
		}
	}
}

// ---------------------------------------------------------------------------
// Draw vertical grid lines representing time.

void CEventEditor::DrawGridLines( BRect updateRect )
{
	TClockType	clockType	= track->ClockType();
	long			startTime = frame.ViewCoordsToTime( updateRect.left - 2.0, clockType );
	CSignatureMap::Iterator		timeIter( Track()->SigMap(), startTime );
	long			time;
	bool			major;
// int32		majorTime = timeIter.MajorUnitTime();
	int32		majorTime = Track()->SigMap().entries->sigMajorUnitDur;
	double		majorXStep = frame.TimeToViewCoords( majorTime, clockType );
	int32		steps = 1;
	
	while (majorXStep < 24)
	{
		if (steps == 2) steps = 5;
		else steps *= 2;
		majorXStep = frame.TimeToViewCoords( majorTime * steps, clockType );
	}

	for (time = timeIter.First( major ); ; time = timeIter.Next( major ))
	{
		double		x;

		x = frame.TimeToViewCoords( time, clockType );
		
		if (x > updateRect.right) break;

		if (major)
		{
			if (timeIter.MajorCount() % steps) continue;
			SetHighColor( 160, 160, 160 );
		}
		else if (steps > 1) continue;
		else SetHighColor( 220, 220, 220 );

		if (x > 0.0)
		{
			StrokeLine(	BPoint( x, updateRect.top ),
						BPoint( x, updateRect.bottom ) );
		}
	}
}

// ---------------------------------------------------------------------------
// Given a rectangular extent, do a pick operation on an event
// which fills that extent.

long CEventEditor::PickDurationEvent( 
	const Event		&ev,
	int				yTop,
	int				yBottom,
	BPoint			pickPt,
	short			&partCode )
{
	long			left  = static_cast<long>(TimeToViewCoords(ev.Start())),
					right = static_cast<long>(TimeToViewCoords(ev.Stop())),
					width = right - left,
					offset,
					durX;

	if (	pickPt.y < yTop
		||	pickPt.y > yBottom )
	{
		return LONG_MAX;
	}
	
	partCode = 0;

		// If the event is very short, then expand the size of the
		// hit zone slightly to help in picking on the event.
	if (width < 8)
	{
		offset = 4 - width / 2;
	}
	else offset = 0;

	if (pickPt.x < left)				// Mouse clicked to left of event
	{
		if (pickPt.x < left - offset) return LONG_MAX;
		return 1000 + static_cast<long>(left - pickPt.x) * 8;
	}
	else if (pickPt.x > right)			// Mouse clicked to right of event.
	{
		if (pickPt.x > right + offset) return LONG_MAX;
		partCode = 1;
		return 1000 + static_cast<long>(pickPt.x - right) * 8;
	}
	else								// Mouse clicked between start & stop
	{
			// If the mouse is within 8 pixels of the end of the note
			// (or 1/2 of the length of the note, if note is smaller than
			// 16 pixels) then
		durX  = right - MIN( 8, width / 2 );
	
			// Events which have duration have the ability to drag the
			// right edge of the event in order to drag the duration
			// of the event.
		if (pickPt.x > durX) partCode = 1;
		return 0;
	}
}

// ---------------------------------------------------------------------------
// Label view for editor labels

void CLabelView::Draw( BRect updateRect )
{
	BRect		r( Bounds() );
	float		w = StringWidth( Name() ),
				h,
				x,
				y;
	font_height	fi;
	
	GetFontHeight( &fi );
	h = fi.ascent + fi.descent;
	
	x = r.left + (r.Width() - h) / 2.0 + fi.descent;
	y = r.top + floor( (r.Height() - w) / 2.0 );

	CBorderView::Draw( updateRect );

	SetHighColor( 128, 128, 128 );
	SetLowColor( 220, 220, 220 );
	DrawString( Name(), BPoint( x, y ) );
}

// ---------------------------------------------------------------------------
// Given a time value, return the time of the nearest grid-snap position.

int32 CEventEditor::SnapToGrid( int32 inTime, bool inInitial )
{
	int32			majorUnit,
					extraTime,
					quantizedExtra;

	track->SigMap().DecomposeTime( inTime, majorUnit, extraTime );
	
	if (inInitial) quantizedExtra = DataSnapLower( extraTime, 0, Track()->TimeGridSize() );
	else quantizedExtra = DataSnapNearest( extraTime, 0, Track()->TimeGridSize() );
	
	return inTime + (quantizedExtra - extraTime );
}

// ---------------------------------------------------------------------------
// Add an extra ruler to the strip

void CEventEditor::SetRuler( CScrollerTarget *inRuler )
{
	BRect				b( Frame() );
	int32				h = static_cast<int32>(inRuler->Bounds().Height());
	BView				*pad;
	
	ruler = inRuler;

	MoveTo(	b.left, h+1 );
	ResizeBy( 0.0, -h-1 );

	if (labelView)
	{
		labelView->MoveTo( -1.0, h );
		labelView->ResizeBy( 0.0, -h );

		rgb_color	border,
					fill;
				
		border.red = border.blue = border.green = 220;
		border.alpha = 0;
		fill.red = fill.green = fill.blue = 128;
		fill.alpha = 128;

		pad = new CBorderView(
			BRect( -1.0, -1.0, 20.0, h ),
			"",
			B_FOLLOW_LEFT | B_FOLLOW_TOP,
			B_WILL_DRAW,
			&fill);
		
		TopView()->AddChild( pad );
	}
	
	inRuler->MoveTo( b.left, 0.0 );
	inRuler->ResizeTo( b.Width(), h );
	TopView()->AddChild( inRuler );
}

void CEventEditor::AddLassoPoint( BPoint &inPoint )
{
	if (lassoPoints == NULL)
	{
		lassoPoints = new CPolygon( &inPoint, 1 );
	}
	else
	{
		DrawLasso();
		lassoPoints->AddPoints( &inPoint, 1 );
		DrawLasso();
	}
}

void CEventEditor::DrawLasso()
{
	if (lassoPoints == NULL) return;

	drawing_mode dm = DrawingMode();

	SetDrawingMode( B_OP_INVERT );
	StrokePolygon(	lassoPoints->Points(),
					lassoPoints->CountPoints(),
					lassoPoints->Frame(),
					false );
	SetDrawingMode( dm );
}

// ---------------------------------------------------------------------------
// Draw selection rectangle

void CEventEditor::DrawSelectRect()
{
	BRect		r;
	
	r.left		= MIN( cursorPos.x, anchorPos.x );
	r.right	= MAX( cursorPos.x, anchorPos.x );
	r.top		= MIN( cursorPos.y, anchorPos.y );
	r.bottom	= MAX( cursorPos.y, anchorPos.y );

	if (r.Width()  == 0.0) r.right += 1.0;
	if (r.Height() == 0.0) r.bottom += 1.0;

	SetDrawingMode( B_OP_INVERT );
	StrokeRect( r, B_MIXED_COLORS );
	SetDrawingMode( B_OP_COPY );
}

// ---------------------------------------------------------------------------
// Draw playback markers

void CEventEditor::DrawPBMarkers( int32 *inArray, int32 inCount, BRect inUpdateRect, bool inErase )
{
	long			startTime = ViewCoordsToTime( inUpdateRect.left - 1.0 ),
				stopTime  = ViewCoordsToTime( inUpdateRect.right + 1.0 );

	SetDrawingMode( B_OP_INVERT );
	for (int i = 0; i < inCount; i++)
	{
		int32		t = inArray[ i ];

		if (t >= startTime && t <= stopTime)
		{
			double		x = TimeToViewCoords( t );

			FillRect( BRect( x, inUpdateRect.top, x, inUpdateRect.bottom ) );
		}
	}
	SetDrawingMode( B_OP_COPY );
}

// ---------------------------------------------------------------------------
// Update the playback marker positions

void CEventEditor::UpdatePBMarkers()
{
	int32			newPBMarkers[ 8 ],
					count;

		// Process playback markers
	count = CPlayerControl::GetPlaybackMarkerTimes(	Track(),
													newPBMarkers,
													Max_PB_Markers );
	if (		count != pbCount
		||	memcmp( pbMarkers, newPBMarkers, count * sizeof pbMarkers[ 0 ] ) != 0)
	{
		DrawPBMarkers( pbMarkers, pbCount, Bounds(), true );

		memcpy( pbMarkers, newPBMarkers, count * sizeof pbMarkers[ 0 ] );
		pbCount = count;

		DrawPBMarkers( pbMarkers, pbCount, Bounds(), false );
	}
}

// ---------------------------------------------------------------------------
// Select all events within the current selection rectangle

void CEventEditor::DoRectangleSelection()
{
	BRect		r;
	DrawSelectRect();
	long			minTime,
				maxTime;
	EventMarker	marker( Track()->Events() );
	const Event	*ev;
	
	r.left			= MIN( cursorPos.x, anchorPos.x );
	r.right		= MAX( cursorPos.x, anchorPos.x );
	r.top			= MIN( cursorPos.y, anchorPos.y );
	r.bottom		= MAX( cursorPos.y, anchorPos.y );

	if (r.Width()  == 0.0) r.right += 1.0;
	if (r.Height() == 0.0) r.bottom += 1.0;

	minTime = ViewCoordsToTime( r.left );
	maxTime = ViewCoordsToTime( r.right );

		// Now, select all events in the rectangle...
		// For each event that overlaps the current view, draw it.
	for (	ev = marker.FirstItemInRange( minTime, maxTime );
			ev;
			ev = marker.NextItemInRange( minTime, maxTime ) )
	{
			// Don't allow picking of events on locked channels...
		if (Track()->IsChannelLocked( *ev )) continue;

		if (!ev->IsSelected())		// No point in selecting if already
		{
			const CAbstractEventHandler	&handler( Handler( *ev ) );
			
			if (&handler == &gNullEventHandler) continue;

			if (r.Contains( handler.Extent( *this, *ev ) )
				|| (gPrefs.inclusiveSelection
					&& r.Intersects( handler.Extent( *this, *ev ) )))
			{
				((Event *)ev)->SetSelected( true );
				handler.Invalidate( *this, *ev );
			}
		}
	}

	Track()->SummarizeSelection();

		// Let the world know the selection has changed
	CEventSelectionUpdateHint		hint( *Track(), true );
	PostUpdate( &hint, true );
}

// ---------------------------------------------------------------------------
// Select all events within the current lasso region

void CEventEditor::DoLassoSelection()
{
	BRect		r = lassoPoints->Frame();
	long			minTime,
				maxTime;
	EventMarker	marker( Track()->Events() );
	const Event	*ev;

	if (r.Width()  == 0.0) r.right += 1.0;
	if (r.Height() == 0.0) r.bottom += 1.0;

	minTime = ViewCoordsToTime( r.left );
	maxTime = ViewCoordsToTime( r.right );

		// Now, select all events in the rectangle...
		// For each event that overlaps the current view, draw it.
	for (	ev = marker.FirstItemInRange( minTime, maxTime );
			ev;
			ev = marker.NextItemInRange( minTime, maxTime ) )
	{
		if (!ev->IsSelected())		// No point in selecting if already
		{
			const CAbstractEventHandler	&handler( Handler( *ev ) );
			
			if (&handler == &gNullEventHandler) continue;
			if (Track()->IsChannelLocked( *ev )) continue;
			
			BRect	extent( handler.Extent( *this, *ev ) );

			if (r.Intersects( extent ) && IsRectInLasso( extent, gPrefs.inclusiveSelection ))
			{
				((Event *)ev)->SetSelected( true );
				handler.Invalidate( *this, *ev );
			}
		}
	}
	
		// Delete the lasso points.
	FinishLasso();
	Track()->SummarizeSelection();

		// Let the world know the selection has changed
	CEventSelectionUpdateHint		hint( *Track(), true );
	PostUpdate( &hint, true );
}

// ---------------------------------------------------------------------------
// Draw the echo of a created event being dragged

void CEventEditor::DrawCreateEcho( int32 startTime, int32 stopTime )
{
	EventOp	*echoOp = PendingOperation();
	if (echoOp == NULL) echoOp = dragOp;

	Event		evCopy( newEv );
		
	if (echoOp) (*echoOp)( evCopy, Track()->ClockType() );
		
	if (	evCopy.Start() <= stopTime && evCopy.Stop()  >= startTime)
	{
		Handler( evCopy ).Draw( *this, evCopy, true );
	}
}

// ---------------------------------------------------------------------------
// Draw the echo of a bunch of events being dragged

void CEventEditor::DrawEchoEvents( int32 startTime, int32 stopTime )
{
	EventOp	*echoOp = PendingOperation();
	if (echoOp == NULL) echoOp = dragOp;

		// Initialize an event marker for this track.
	EventMarker		marker( Track()->Events() );
	const Event		*ev;
	TClockType		clockType = Track()->ClockType();

		// For each event that overlaps the current view, draw it.
	for (	ev = marker.FirstItemInRange( Track()->MinSelectTime(), Track()->MaxSelectTime() );
			ev;
			ev = marker.NextItemInRange( Track()->MinSelectTime(), Track()->MaxSelectTime() ) )
	{
		if (ev->IsSelected() && !Track()->IsChannelLocked( *ev ))
		{
			Event		evCopy( *(Event *)ev );
	
			(*echoOp)( evCopy, clockType );
		
			if (	evCopy.Start() <= stopTime
				&& 	evCopy.Stop()  >= startTime)
			{
				Handler( evCopy ).Draw( *this, evCopy, true );
			}
		}
	}
}

// ---------------------------------------------------------------------------
// Standard "pick" function for event editors

void CEventEditor::StartDrag( BPoint point, ulong buttons )
{
	const Event		*ev;
	StSubjectLock		trackLock( *Track(), Lock_Exclusive );
	EventMarker		marker( Track()->Events() );
	short			partCode;
	ulong			modifierKeys = modifiers();

	bounds = Bounds();
	
	if ((ev = PickEvent( marker, point, partCode )) != NULL)
	{
		bool			wasSelected = false;
	
		Document().SetActiveMaster( Track() );

		if (ev->IsSelected())
		{
			wasSelected = true;
		
			if (modifierKeys & B_SHIFT_KEY)
			{
				((Event *)ev)->SetSelected( false );
				Handler( *ev ).Invalidate( *this, *ev );
			
					// This could be faster.
				Track()->SummarizeSelection();

					// Let the world know the selection has changed
				CEventSelectionUpdateHint		hint( *Track(), true );
				PostUpdate( &hint, true );
				return;
			}
		}
		else
		{
			if (!(modifierKeys & B_SHIFT_KEY))
			{
				Track()->DeselectAll( this );
			}
			((Event *)ev)->SetSelected( true );
			Handler( *ev ).Invalidate( *this, *ev );

				// This could be faster.
			Track()->SummarizeSelection();

				// Let the world know the selection has changed
			CEventSelectionUpdateHint		hint( *Track(), true );
			PostUpdate( &hint, true );
		}
		
		Track()->SetCurrentEvent( marker );
		if (modifierKeys & B_CONTROL_KEY && partCode == 0)
			dragType = DragType_CopyEvents;
		else dragType = DragType_Events;
		clickPart = partCode;
		
		TrackWindow()->DisplayMouseTime( Track(), ev->Start() );
	
		if (partCode == 0 || wasSelected == false)
			DoEventFeedback( *ev );

		timeDelta = 0;
		valueDelta = 0;
	}
	else if (buttons & B_SECONDARY_MOUSE_BUTTON)
	{
		if (!(modifierKeys & B_SHIFT_KEY))
			Track()->DeselectAll( this );

			// If clicking with right button, then also do a lasso drag.
		cursorPos = point;
		dragType = DragType_Lasso;
		AddLassoPoint( point );
	}
	else
	{
		if (!(modifierKeys & B_SHIFT_KEY))
			Track()->DeselectAll( this );

		int32 toolState = TrackWindow()->CurrentTool();
		
		if (toolState == CTrackWindow::TOOL_CREATE)
		{
			if (!ConstructEvent( point ))
			{
				beep();
				return;
			}

			clickPart = 0;
			dragType = DragType_Create;
			Handler( newEv ).Invalidate( *this, newEv );
			
			DoEventFeedback( newEv );

				// Poke selection times so that drag-limits will work properly
				// (Drag limits are implemented in Handler.QuantizeDragTime
				// and use the track selection times).
				// REM: This is somewhat of a kludge and may interfere
				// with plug-in development.
			Track()->SetSelectTime( newEv.Start(), newEv.Stop() );

			timeDelta = 0;
			valueDelta = 0;
		}
		else
		{
				// Do a selection rectangle drag...
			cursorPos = anchorPos = point;
			dragType = DragType_Select;
			DrawSelectRect();
			TrackWindow()->DisplayMouseTime( Track(), ViewCoordsToTime( point.x ) );
		}
	}
}

// ---------------------------------------------------------------------------
// Standard "drag" function for event editors

bool CEventEditor::DoDrag( BPoint point, ulong buttons )
{
	bounds = Bounds();

	if (		dragType == DragType_Events
		||	dragType == DragType_CopyEvents
		||	dragType == DragType_Create)
	{
		StSubjectLock	trackLock( *Track(), Lock_Exclusive );
		long			newTimeDelta,
					newValueDelta;
		EventOp		*newValueOp,
					*newTimeOp,
					*newDragOp;
		const Event	*dragEvent;
		
		if (dragType == DragType_Create)	dragEvent = &newEv;
		else								dragEvent = Track()->CurrentEvent();

		const CAbstractEventHandler		&handler( Handler( *dragEvent ) );

			// Compute the difference between the original
			// time and the new time we're dragging the events to.
		newTimeDelta = handler.QuantizeDragTime(
			*this, *dragEvent, clickPart, clickPos, point );

			// Compute the difference between the original value
			// and the new value we're dragging the events to.
		newValueDelta = handler.QuantizeDragValue(
			*this, *dragEvent, clickPart, clickPos, point );
	
		if (	newTimeDelta  == timeDelta
			&&	newValueDelta == valueDelta)
					return true;

		newValueOp = handler.CreateDragOp(
			*this, *dragEvent, clickPart, newTimeDelta, newValueDelta );

		newTimeOp = handler.CreateTimeOp(
			*this, *dragEvent, clickPart, newTimeDelta, newValueDelta );
			
		if (newTimeOp == NULL)		newDragOp = newValueOp;
		else if (newValueOp == NULL)	newDragOp = newTimeOp;
		else
		{
				// If we have two operations, then pair them, and release them
				// (because pair keeps references to the objects)
			newDragOp = new PairOp( newValueOp, newTimeOp );
			CRefCountObject::Release( newValueOp );
			CRefCountObject::Release( newTimeOp );
		}

			// Do audio feedback for this drag, but only if value changed,
			// and only if we're dragging something that can be fed back.
		if (newValueOp != NULL && newValueDelta != valueDelta)
		{
			Event			feedbackEvent( *(Event *)dragEvent );

				// REM: EvAttr_Pitch should not be hard coded.
				// This should be in fact computed from the handler.
				// Or from the valueOp.
			(*newValueOp)( feedbackEvent, Track()->ClockType() );
			DoEventFeedback( feedbackEvent );
		}
		
		if (	dragType == DragType_Create)
		{
			Event		evCopy( newEv );
					
			if (dragOp) (*dragOp)( evCopy, Track()->ClockType() );
			Handler( evCopy ).Invalidate( *this, evCopy );

			evCopy = newEv;
			if (newDragOp) (*newDragOp)( evCopy, Track()->ClockType() );
			Handler( evCopy ).Invalidate( *this, evCopy );
		}
		else
		{
			if (dragOp)		InvalidateSelection( *dragOp );
			if (newDragOp)	InvalidateSelection( *newDragOp );
		}
		
		CRefCountObject::Release( dragOp );
		dragOp = newDragOp;

		timeDelta = newTimeDelta;
		valueDelta = newValueDelta;

		TrackWindow()->DisplayMouseTime( Track(), dragEvent->Start() + timeDelta );
	}
	else if (dragType == DragType_Select)
	{
		if (cursorPos != point)
		{
			DrawSelectRect();
			cursorPos = point;
			DrawSelectRect();
			TrackWindow()->DisplayMouseTime( Track(), ViewCoordsToTime( point.x ) );
		}
	}
	else if (dragType == DragType_Lasso)
	{
		if (cursorPos != point)
		{
			AddLassoPoint( point );
			cursorPos = point;
			TrackWindow()->DisplayMouseTime( Track(), ViewCoordsToTime( point.x ) );
		}
	}
	return true;
}

void CEventEditor::FinishDrag(
	BPoint		point,
	ulong		buttons,
	bool			commit )
{
		// Initialize an event marker for this track.
	StSubjectLock		trackLock( *Track(), Lock_Exclusive );

	bounds = Bounds();

	Document().SetActiveMaster( Track() );

	if (dragType == DragType_Events || dragType == DragType_CopyEvents
		|| dragType == DragType_Create)
	{
		if (timeDelta == 0 && valueDelta == 0 && dragType != DragType_Create)
			commit = false;
		
			// Remove highlight from piano keyboard.
		KillEventFeedback();
		
		if (dragOp || dragType == DragType_Create)
		{
			if (commit)
			{
				long		prevTrackDuration = Track()->LastEventTime();
		
				if (	dragType == DragType_Create)
				{
						// Creating a new event
					if (dragOp) (*dragOp)( newEv, Track()->ClockType() );
					Handler( newEv ).Invalidate( *this, newEv );
					Track()->CreateEvent( this, newEv, "Create Event" );
				}
				else if (dragType == DragType_CopyEvents)
				{
					Track()->CopySelectedEvents( this, *dragOp, "Drag" );
				}
				else
				{
					Track()->ModifySelectedEvents( this, *dragOp, "Drag" );
				}
	
				if (prevTrackDuration != Track()->LastEventTime())
					RecalcScrollRangeH();
			}
			else InvalidateSelection( *dragOp );
		}
	}
	else if (dragType == DragType_Select)
	{
		DoRectangleSelection();
	}
	else if (dragType == DragType_Lasso)
	{	
		DoLassoSelection();
	}

	CRefCountObject::Release( dragOp ); dragOp = NULL;
	dragType = DragType_None;
	TrackWindow()->DisplayMouseTime( NULL, 0 );
}

// ---------------------------------------------------------------------------
// Quantize the time of the dragging operation and
// return a time delta.

long CAbstractEventHandler::QuantizeDragTime(
	CEventEditor	&editor,
	const Event		&inClickEvent,
	short			partCode,			// Part of event clicked
	BPoint			inClickPos,
	BPoint			inDragPos,
	bool				inInitial ) const
{

	long				t1,
					t2,
					timeDelta;
	
	timeDelta = editor.ViewCoordsToTime( inDragPos.x - inClickPos.x );

		// If no grid snap, then return just timeDelta
	if (!editor.Track()->GridSnapEnabled()) return timeDelta;
	
	t1 = editor.SnapToGrid( inClickEvent.Start(), inInitial );
	t2 = editor.SnapToGrid( inClickEvent.Start() + timeDelta, inInitial );
	return t2 - t1;
}

EventOp *CAbstractEventHandler::CreateTimeOp(
	CEventEditor	&editor,				// The editor
	const Event	&ev,					// The clicked event
	short		partCode,			// Part of event clicked
	long			timeDelta,			// The horizontal drag delta
	long			valueDelta ) const
{
	long		limit;

	if (editor.Track()->GridSnapEnabled())
	{
		limit = -DataSnapLower(	editor.Track()->MinSelectTime(),
								0,
								editor.Track()->TimeGridSize() );
	}
	else limit = -editor.Track()->MinSelectTime();

	timeDelta = MAX( timeDelta, limit );
	return new TimeOffsetOp( timeDelta );
}

// ---------------------------------------------------------------------------
// Event handler for END events.

	// Invalidate the event
void CEndEventHandler::Invalidate(
	CEventEditor		&editor,
	const Event		&ev ) const
{
	editor.Invalidate( Extent( editor, ev ) );
}

static pattern endPt = { 0, 0, 0xff, 0xff, 0, 0, 0xff, 0xff };

	// Draw the event (or an echo)
void CEndEventHandler::Draw(
	CEventEditor	&editor,
	const Event		&ev,
	bool 			shadowed ) const
{
	BRect			r( editor.ViewBounds() );

	r.left	= editor.TimeToViewCoords( ev.Start() );
	r.right	= r.left + 1.0;
	
	editor.SetDrawingMode( B_OP_OVER );

	if (shadowed)
	{
		editor.SetHighColor( 0, 0, 0 );
		editor.SetDrawingMode( B_OP_BLEND );
	}
	else if (ev.IsSelected() && editor.IsSelectionVisible())
	{
		editor.SetHighColor( 128, 0, 128 );
	}
	else
	{
		editor.SetHighColor( 0, 0, 0 );
	}

	editor.SetLowColor( B_TRANSPARENT_32_BIT );
	editor.FillRect( r, endPt );
}

	// Compute the extent of the event.
BRect CEndEventHandler::Extent(
	CEventEditor		&editor,
	const Event		&ev ) const
{
	BRect			r( editor.ViewBounds() );

	r.left	= editor.TimeToViewCoords( ev.Start() );
	r.right	= r.left + 1.0;

	return r;
}

	// Pick a single event and return the part code
	// (or -1 if event not picked)
long CEndEventHandler::Pick(
	CEventEditor	&editor,
	const Event		&ev,
	BPoint			pickPt,
	short			&partCode ) const
{
	BRect			r( Extent( editor, ev ) );
	float			dist = fabs( r.left - pickPt.x );
	
	if (dist < 3.0)
	{
		partCode = 0;
		return static_cast<long>(dist);
	}

	return LONG_MAX;
}

const uint8 *CEndEventHandler::CursorImage( short partCode ) const
{
	return B_HAND_CURSOR;			// Return the normal hand cursor
}

	// Quantize the vertical position of the mouse based
	// on the event type and return a value delta.
long CEndEventHandler::QuantizeDragValue(
	CEventEditor		&editor,
	const Event		&inClickEvent,
	short			partCode,			// Part of event clicked
	BPoint			inClickPos,
	BPoint			inDragPos ) const
{
	return 0;
}

EventOp *CEndEventHandler::CreateDragOp(
	CEventEditor		&editor,
	const Event		&ev,
	short			partCode,
	long				timeDelta,			// The horizontal drag delta
	long				valueDelta ) const
{
	return NULL;
}
