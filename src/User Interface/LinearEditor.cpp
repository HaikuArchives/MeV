/* ===================================================================== *
 * LinearEditor.cpp (MeV/User Interface)
 * ===================================================================== */

#include "LinearEditor.h"
#include "VChannel.h"
#include "PlayerControl.h"
#include "MeVApp.h"
#include "MeVDoc.h"
#include "StdEventOps.h"
#include "ResourceUtils.h"

// Interface Kit
#include <Region.h>

const uint8			*resizeCursor,
					*crossCursor;

// ---------------------------------------------------------------------------
// Note handler class for linear editor

	// Invalidate the event
void CLinearNoteEventHandler::Invalidate(
	CEventEditor	&editor,
	const Event		&ev ) const
{
	CLinearEditor	&lEditor = (CLinearEditor &)editor;

	BRect	r;

	r.left	= lEditor.TimeToViewCoords( ev.Start() ) - 1.0;
	r.right	= lEditor.TimeToViewCoords( ev.Stop()  ) + 1.0;
	r.bottom	= lEditor.PitchToViewCoords( ev.note.pitch )   + 1.0;
	r.top	= r.bottom - lEditor.whiteKeyStep - 2.0;

	lEditor.Invalidate( r );
}

void DrawNoteShape(	
	BView		*view,
	BRect		inRect,
	rgb_color		outline,
	rgb_color		fill,
	rgb_color		highlight,
	bool			drawHighlight );

void DrawNoteShape(	
	BView		*view,
	BRect		inRect,
	rgb_color		outline,
	rgb_color		fill,
	rgb_color		highlight,
	bool			drawHighlight )
{
	view->SetHighColor( outline );
	
	if (inRect.Width() <= 2.0 || inRect.Height() <= 2.0)
	{
		view->FillRect( inRect );
	}
	else if (inRect.Width() <= 4.0 || inRect.Height() <= 2.0)
	{
		BRect		hLine;

		hLine.left = inRect.left + 1;
		hLine.right = inRect.right - 1;
		hLine.top = hLine.bottom = inRect.top;
		view->FillRect( hLine );
		hLine.top = hLine.bottom = inRect.bottom;
		view->FillRect( hLine );

		view->FillRect( BRect( inRect.left, inRect.top + 1, inRect.left, inRect.bottom - 1 ) );
		view->FillRect( BRect( inRect.right, inRect.top + 1, inRect.right, inRect.bottom - 1 ) );

		view->SetHighColor( fill );
		if (drawHighlight)
		{
			hLine.top = hLine.bottom = inRect.top + 1;
			view->FillRect( hLine );

			hLine.top = inRect.top + 3;
			hLine.bottom = inRect.bottom - 1;
			view->FillRect( hLine );

			view->SetHighColor( highlight );
			hLine.top = hLine.bottom = inRect.top + 2;
			hLine.left = inRect.left + 2;
			view->FillRect( hLine );

			view->SetHighColor( 255, 255, 255 );
			hLine.left = hLine.right = inRect.left + 1;
			view->FillRect( hLine );
		}
		else
		{
			hLine.top = inRect.top + 1;
			hLine.bottom = inRect.bottom - 1;
			view->FillRect( hLine );
		}
	}
	else
	{
		BRect	r1,
				r2,
				r3;
		
		r1.left = inRect.left + 2;
		r1.right = inRect.right - 2;
		
		r2.left = r2.right = inRect.left + 1;
		r3.left = r3.right = inRect.right - 1;

		r1.top = r1.bottom = inRect.top;		view->FillRect( r1 );
		r1.top = r1.bottom = inRect.bottom;	view->FillRect( r1 );

		r2.top = r2.bottom = r3.top = r3.bottom = inRect.top + 1;
		view->FillRect( r2 );
		view->FillRect( r3 );

		r2.top = r2.bottom = r3.top = r3.bottom = inRect.bottom - 1;
		view->FillRect( r2 );
		view->FillRect( r3 );

		view->FillRect( BRect( inRect.left, inRect.top + 2, inRect.left, inRect.bottom - 2 ) );
		view->FillRect( BRect( inRect.right, inRect.top + 2, inRect.right, inRect.bottom - 2 ) );

		view->SetHighColor( fill );
		
		r1.top = inRect.top + 2;
		r1.bottom = inRect.bottom - 2;

		r1.left = r1.right = inRect.left + 1;		view->FillRect( r1 );
		r1.left = r1.right = inRect.right - 1;	view->FillRect( r1 );

		if (drawHighlight)
		{
			r1.left = inRect.left + 2;
			r1.right = inRect.right - 2;
			r1.top = r1.bottom = inRect.top + 1;
			view->FillRect( r1 );
			r1.top = inRect.top + 3;
			r1.bottom = inRect.bottom - 1;
			view->FillRect( r1 );

			view->SetHighColor( highlight );
			r2.top = r2.bottom = inRect.top + 2;
			r2.left = inRect.left + 3;
			r2.right = inRect.right - 2;
			view->FillRect( r2 );

			view->SetHighColor( 255, 255, 255 );
			r2.left = r2.right = inRect.left + 2;
			view->FillRect( r2 );
		}
		else
		{
			view->FillRect( BRect( inRect.left + 2, inRect.top + 1, inRect.right - 2, inRect.bottom - 1 ) );
		}
	}
}

	// Draw the event (or an echo)
void CLinearNoteEventHandler::Draw(
	CEventEditor	&editor,
	const Event		&ev,
	bool 			shadowed ) const
{
	CLinearEditor		&lEditor = (CLinearEditor &)editor;
	CEventTrack		*track = editor.Track();
	static rgb_color	black = { 0, 0, 0 },
					medGrey = { 128, 128, 128 },
					ltGrey = { 192, 192, 192 },
					blue = { 0, 0, 255 };

	BRect			r;

	r.left		= lEditor.TimeToViewCoords( ev.Start() );
	r.right	= lEditor.TimeToViewCoords( ev.Stop()  );
	r.bottom	= lEditor.PitchToViewCoords( ev.note.pitch );
	r.top 	= r.bottom - lEditor.whiteKeyStep;

	if (track->IsChannelLocked( ev.GetVChannel() ))
	{
		if (!shadowed) DrawNoteShape( &lEditor, r, medGrey, ltGrey, black, false );
		return;
	}

	VChannelEntry		&vce = lEditor.Document().GetVChannel( ev.GetVChannel() );

	if (shadowed)
	{
		lEditor.SetDrawingMode( B_OP_BLEND );
		DrawNoteShape( &lEditor, r, black, vce.fillColor, vce.highlightColor, true );
		lEditor.SetDrawingMode( B_OP_COPY );
	}
	else if (ev.IsSelected() && editor.IsSelectionVisible())
	{
		DrawNoteShape( &lEditor, r, blue, vce.fillColor, vce.highlightColor, true );
	}
	else
	{
		DrawNoteShape( &lEditor, r, black, vce.fillColor, vce.highlightColor, true );
	}
}

	// Compute the extent of the event.
BRect CLinearNoteEventHandler::Extent(
	CEventEditor		&editor,
	const Event		&ev ) const
{
	CLinearEditor	&lEditor = (CLinearEditor &)editor;
	BRect			r;

	r.left		= lEditor.TimeToViewCoords( ev.Start() );
	r.right	= lEditor.TimeToViewCoords( ev.Stop()  );
	r.bottom	= lEditor.PitchToViewCoords( ev.note.pitch );
	r.top   	= r.bottom - lEditor.whiteKeyStep;

	return r;
}

	// Pick a single event and return the part code
	// (or -1 if event not picked)
long CLinearNoteEventHandler::Pick(
	CEventEditor	&editor,
	const Event		&ev,
	BPoint			pickPt,
	short			&partCode ) const
{
	CLinearEditor	&lEditor = (CLinearEditor &)editor;
	int				top,
					bottom;

	bottom	= lEditor.PitchToViewCoords( ev.note.pitch );
	top		= bottom - lEditor.whiteKeyStep;

	return lEditor.PickDurationEvent( ev, top, bottom, pickPt, partCode );
}

const uint8 *CLinearNoteEventHandler::CursorImage( short partCode ) const
{
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
	
	return NULL;
}

	// Quantize the vertical position of the mouse based
	// on the event type and return a value delta.
long CLinearNoteEventHandler::QuantizeDragValue(
	CEventEditor	&editor,
	const Event		&inClickEvent,
	short			partCode,			// Part of event clicked
	BPoint			inClickPos,
	BPoint			inDragPos ) const
{
	CLinearEditor	&lEditor = (CLinearEditor &)editor;

		// Get the pitch and y position of the old note.

	long			oldPitch	= inClickEvent.note.pitch;
	float			oldYPos		= lEditor.PitchToViewCoords( oldPitch );
	long			newPitch;

		// Add in the vertical drag delta to the note position,
		// and compute the new pitch.
		
	newPitch = lEditor.ViewCoordsToPitch(
		oldYPos + inDragPos.y - inClickPos.y - lEditor.whiteKeyStep / 2, false );
					
	return newPitch - oldPitch;
}

EventOp *CLinearNoteEventHandler::CreateDragOp(
	CEventEditor	&editor,
	const Event		&ev,
	short			partCode,
	long			timeDelta,			// The horizontal drag delta
	long			valueDelta ) const
{
	if (partCode == 0)
		return new PitchOffsetOp( valueDelta );
	else return NULL;
}

EventOp *CLinearNoteEventHandler::CreateTimeOp(
	CEventEditor	&editor,
	const Event		&ev,
	short			partCode,
	long			timeDelta,			// The horizontal drag delta
	long			valueDelta ) const
{
	if (partCode == 1)
		return new DurationOffsetOp( timeDelta );
	else return CAbstractEventHandler::CreateTimeOp( editor, ev, partCode, timeDelta, valueDelta );
}

// ---------------------------------------------------------------------------
// Dispatch table for linear editor ~~~EVENTLIST

CLinearNoteEventHandler		linearNoteHandler;

// ---------------------------------------------------------------------------
// Linear Editor class

	// ---------- Constructor

CLinearEditor::CLinearEditor(
	BLooper			&inLooper,
	CTrackEditFrame	&inFrame,
	BRect			rect )
	:	CEventEditor(	inLooper, inFrame, rect,
						"Linear Editor", true, true )
{
	handlers[ EvtType_Note ]	= &linearNoteHandler;
	handlers[ EvtType_End ]		= &gEndEventHandler;

	verticalZoom		= 1;
	whiteKeyStep		= 8;
	CalcZoom();
	SetZoomTarget( (CObserver *)this );

		// Make the label view on the left-hand side
	labelView = new CPianoKeyboardView(	BRect(	0.0,
												0.0,
												20.0,
												rect.Height() ),
										this,
										B_FOLLOW_TOP_BOTTOM,
										B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE );

	ResizeBy( -21.0, 0.0 );
	MoveBy( 21.0, 0.0 );
	TopView()->AddChild( labelView );

	SetFlags( Flags() | B_PULSE_NEEDED );
}

// ---------------------------------------------------------------------------
// Convert a pitch-value to a y-coordinate

long CLinearEditor::PitchToViewCoords( int pitch )
{
	static uint8	offset[ 12 ] = { 0, 1, 2, 3, 4, 6, 7, 8, 9, 10, 11, 12 };
	int				octave = pitch / 12,
					note = pitch % 12;

	return	stripLogicalHeight
			- octave * octaveStep
			- ((offset[ note ] * whiteKeyStep) >> 1);
}

// ---------------------------------------------------------------------------
// Convert a y-coordinate to a pitch value

long CLinearEditor::ViewCoordsToPitch( int yPos, bool limit )
{
	static uint8	offsetTable[ 28 ] =
	{	0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 4, 5, 5, 5,
		6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 11
	};
	
	int				octave = (stripLogicalHeight - yPos) / octaveStep,
					offset = (stripLogicalHeight - yPos) % octaveStep,
					key;
					
	if (offset < 0)
	{
		octave--;
		offset += octaveStep;
	}

	key = offsetTable[ (offset * 4) / whiteKeyStep ] + octave * 12;

	if (limit)
	{
		if (key < 0) return 0;
		if (key > 127) return 127;
	}
	return key;
}

void CLinearEditor::Draw( BRect updateRect )
{
	long				startTime = ViewCoordsToTime( updateRect.left - 1.0 ),
					stopTime  = ViewCoordsToTime( updateRect.right + 1.0 ),
					minPitch = ViewCoordsToPitch( updateRect.bottom + whiteKeyStep, true ),
					maxPitch = ViewCoordsToPitch( updateRect.top - whiteKeyStep, true );
	int				yPos,
					lineCt;
#if 0
	screen_info		sInfo;
	
	get_screen_info( &sInfo );

	BRect			bounds( updateRect );
	
	bounds.OffsetTo( B_ORIGIN );

	BBitmap			*bm = new BBitmap( bounds, sInfo.mode, true );
	BView			*bv = new BView( bounds, NULL, 0, 0 );
	
// bv->ScrollTo( updateRect.LeftTop() );
	bm->Lock();
	bm->AddChild( bv );
	
	bv->SetHighColor( 255, 255, 255 );
	bv->FillRect( updateRect );

	bv->SetHighColor( 220, 220, 220 );
	for (int i = 4; i < 200; i += 5)
	{
		bv->FillRect( BRect(	updateRect.left, 
								i,
								updateRect.right,
								i ) );
	}
	bv->Sync();
	bm->Unlock();

	DrawBitmap( bm, updateRect.LeftTop() );
	
	bm->Lock();
	delete bv;
	bm->Unlock();
	delete bm;

#else
	SetHighColor( 255, 255, 255 );
	FillRect( updateRect );

	SetHighColor( 220, 220, 220 );

		// Draw horizontal grid lines.
		// REM: This needs to be faster.
	for (	yPos = stripLogicalHeight, lineCt = 0;
			yPos >= updateRect.top;
			yPos -= whiteKeyStep, lineCt++ )
	{
		if (lineCt >= 7) lineCt = 0;

			// Fill solid rectangle with gridline
		if (yPos <= updateRect.bottom)
		{
			if (lineCt == 0)
				SetHighColor( 180, 180, 180 );
			else SetHighColor( 220, 220, 220 );
	
			FillRect(	BRect(	updateRect.left, yPos,
								updateRect.right, yPos ) );
		}
	}

	DrawGridLines( updateRect );

		// Initialize an event marker for this track.
	StSubjectLock		trackLock( *Track(), Lock_Shared );
	EventMarker		marker( Track()->Events() );

	bounds = Bounds();
	
		// REM: We should be able to figure out the maximum and minimum pitch of notes 
		// which are in the update rect.

		// For each event that overlaps the current view, draw it. (locked channels first)
	for (	const Event *ev = marker.FirstItemInRange( startTime, stopTime );
			ev;
			ev = marker.NextItemInRange( startTime, stopTime ) )
	{
		if (ev->Command() == EvtType_Note
			&& (ev->note.pitch < minPitch || ev->note.pitch >maxPitch)) continue;

		if (Track()->IsChannelLocked( *ev ))
			Handler( *ev ).Draw( *this, *ev, false );
	}
	
		// For each event that overlaps the current view, draw it. (unlocked channels overdraw!)
	for (	const Event *ev = marker.FirstItemInRange( startTime, stopTime );
			ev;
			ev = marker.NextItemInRange( startTime, stopTime ) )
	{
		if (ev->Command() == EvtType_Note
			&& (ev->note.pitch < minPitch || ev->note.pitch >maxPitch)) continue;

		if (!Track()->IsChannelLocked( *ev ))
			Handler( *ev ).Draw( *this, *ev, false );
	}
	
	EventOp	*echoOp = PendingOperation();
	if (echoOp == NULL) echoOp = dragOp;

	if (!IsSelectionVisible())
	{
		// Do nothing...
	}
	else if (dragType == DragType_Create)
	{
		DrawCreateEcho( startTime, stopTime );
#if 0
		Event		evCopy( newEv );
			
		if (echoOp) (*echoOp)( evCopy, Track()->ClockType() );
			
		if (	evCopy.Start() <= stopTime && evCopy.Stop()  >= startTime)
		{
			Handler( evCopy ).Draw( *this, evCopy, true );
		}
#endif
	}
	else if (echoOp != NULL)
	{
		DrawEchoEvents( startTime, stopTime );
#if 0	
			// Initialize an event marker for this track.
		EventMarker		marker( Track()->Events() );
		const Event		*ev;
		TClockType		clockType = Track()->ClockType();

			// For each event that overlaps the current view, draw it.
		for (	ev = marker.FirstItemInRange( Track()->MinSelectTime(), Track()->MaxSelectTime() );
				ev;
				ev = marker.NextItemInRange( Track()->MinSelectTime(), Track()->MaxSelectTime() ) )
		{
			if (ev->IsSelected())
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
#endif
	}
	else if (dragType == DragType_Select)
	{
		DrawSelectRect();
	}
	else if (dragType == DragType_Lasso)
	{
		DrawLasso();
	}

	DrawPBMarkers( pbMarkers, pbCount, updateRect, false );
	
#endif
}

void CLinearEditor::Pulse()
{
	UpdatePBMarkers();
	
		// REM: Add code to process newly recorded events
		// REM: Add code to edit events via MIDI.
}

// ---------------------------------------------------------------------------
// Update message from another observer

void CLinearEditor::OnUpdate( BMessage *inMsg )
{
	int32		minTime = 0,
				maxTime = LONG_MAX;
	int32		trackHint;
	bool			flag;
	bool			selChange = false;
	int8			channel = -1;
	BRect		r( Bounds() );

	bounds = r;

	if (inMsg->FindBool( "SelChange", 0, &flag ) == B_OK)
	{
		if (!IsSelectionVisible()) return;
		selChange = flag;
	}

	if (inMsg->FindInt32( "TrackAttrs", 0, &trackHint ) == B_OK)
	{
			// REM: what do we do if track changes name?
		if (!(trackHint &
			(CTrack::Update_Duration|CTrack::Update_SigMap|CTrack::Update_TempoMap)))
				return;
	}
	else trackHint = 0;

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

	if (trackHint & CTrack::Update_Duration) RecalcScrollRangeH();

	if (trackHint & (CTrack::Update_SigMap|CTrack::Update_TempoMap))
	{
		RecalcScrollRangeH();
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

void CLinearEditor::CalcZoom()
{
	octaveStep			= 7 * whiteKeyStep;
	stripLogicalHeight	= 75 * whiteKeyStep - 1;
}

void CLinearEditor::AttachedToWindow()
{
	BRect		r( Frame() );

	SetViewColor( B_TRANSPARENT_32_BIT );
	SetScrollRange(	scrollRange.x, scrollValue.x,
					stripLogicalHeight,
					(stripLogicalHeight - r.Height()/2) / 2 );
}

void CLinearEditor::MessageReceived( BMessage *msg )
{
	switch (msg->what) {
	case ZoomOut_ID:

		if (whiteKeyStep < 12)
		{
			BRect		r( Frame() );
			float		sValue = (ScrollValue( B_VERTICAL ) + (r.bottom - r.top) / 2) / whiteKeyStep;

			whiteKeyStep++;
			CalcZoom();
			Hide();
			SetScrollRange(	scrollRange.x, scrollValue.x,
							stripLogicalHeight,
							(sValue * whiteKeyStep) - (r.bottom - r.top) / 2 );
			labelView->Invalidate();
			Show();
		}
		break;

	case ZoomIn_ID:

		if (whiteKeyStep > 4) 
		{
			BRect		r( Frame() );
			float		sValue = (ScrollValue( B_VERTICAL ) + (r.bottom - r.top) / 2) / whiteKeyStep;

			whiteKeyStep--;
			CalcZoom();
			Hide();
			SetScrollRange(	scrollRange.x, scrollValue.x,
							stripLogicalHeight,
							(sValue * whiteKeyStep) - (r.bottom - r.top) / 2 );
			labelView->Invalidate();
			Show();
		}
		break;

	case Update_ID:
	case Delete_ID:
		CObserver::MessageReceived( msg );
		break;

	default:
		CStripView::MessageReceived( msg );
		break;
	}
}

// ---------------------------------------------------------------------------
// Linear editor mouse movement handler

void CLinearEditor::MouseMoved(
	BPoint		point,
	ulong		transit,
	const BMessage	* )
{
	const Event		*ev;
	short			partCode;
	const uint8		*newCursor;

	if (transit == B_EXITED_VIEW)
	{
		TrackWindow()->DisplayMouseTime( NULL, 0 );
		TrackWindow()->RestoreCursor();
		return;
	}
	
	TrackWindow()->DisplayMouseTime( Track(), ViewCoordsToTime( point.x ) );

	StSubjectLock		trackLock( *Track(), Lock_Shared );
	EventMarker		marker( Track()->Events() );

	bounds = Bounds();

	if ((ev = PickEvent( marker, point, partCode )) != NULL)
	{
		newCursor = Handler( *ev ).CursorImage( partCode );
	}
	else newCursor = NULL;
	
	if (newCursor == NULL)
	{
		if (crossCursor == NULL)
		{
			crossCursor = ResourceUtils::LoadCursor(1);
		}

		newCursor = crossCursor;
	}
	
	TrackWindow()->SetCursor( newCursor );
}

#if 0
void CLinearEditor::StartDrag( BPoint point, ulong buttons )
{
	const Event		*ev;
	StSubjectLock		trackLock( *Track(), Lock_Shared );
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

			// Start audio feedback
		if ((partCode == 0 || wasSelected == false) && gPrefs.FeedbackEnabled( EvAttr_Pitch ))
			CPlayerControl::DoAudioFeedback( &Track()->Document(), EvAttr_Pitch, ev->note.pitch, ev );
			
			// If it has a pitch, then show that pitch
		if (ev->HasAttribute( EvAttr_Pitch ))
			((CPianoKeyboardView *)labelView)->SetSelKey( ev->note.pitch );
		
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
		
		if (toolState == TOOL_CREATE)
		{
			int32		time;
			CMeVDoc		&doc = Document();
		
				// Initialize a new event.
			newEv.SetCommand( TrackWindow()->GetNewEventType() );
			
				// Compute the difference between the original
				// time and the new time we're dragging the events to.
			time = Handler( newEv ).QuantizeDragTime(
				*this,
				newEv,
				0,
				BPoint( 0.0, 0.0 ),
				point,
				true );
			TrackWindow()->DisplayMouseTime( Track(), time );

			newEv.SetStart( time );
			newEv.SetDuration( TrackWindow()->GetNewEventDuration() - 1 );
			newEv.SetVChannel( doc.GetDefaultAttribute( EvAttr_Channel ) );

			switch (newEv.Command()) {
			case EvtType_End:
				newEv.SetDuration( 0 );
				break;

			case EvtType_Note:
				newEv.note.pitch = ViewCoordsToPitch( point.y, true );
				newEv.note.attackVelocity = doc.GetDefaultAttribute( EvAttr_AttackVelocity );
				newEv.note.releaseVelocity = doc.GetDefaultAttribute( EvAttr_ReleaseVelocity );
				break;

			default:
				beep();
				return;
			};

			clickPart = 0;
			dragType = DragType_Create;
			Handler( newEv ).Invalidate( *this, newEv );

				// Start audio feedback
			if (gPrefs.FeedbackEnabled( EvAttr_Pitch ))
				CPlayerControl::DoAudioFeedback( &Track()->Document(), EvAttr_Pitch, newEv.note.pitch, &newEv );

				// If it has a pitch, then show that pitch
			if (newEv.HasAttribute( EvAttr_Pitch ))
				((CPianoKeyboardView *)labelView)->SetSelKey( newEv.note.pitch );

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

bool CLinearEditor::DoDrag( BPoint point, ulong buttons )
{
	bounds = Bounds();

	if (		dragType == DragType_Events
		||	dragType == DragType_CopyEvents
		||	dragType == DragType_Create)
	{
		StSubjectLock		trackLock( *Track(), Lock_Shared );
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
			if (gPrefs.FeedbackEnabled( EvAttr_Pitch ))
				CPlayerControl::DoAudioFeedback(
					&Track()->Document(), 
					EvAttr_Pitch,
					feedbackEvent.note.pitch,
					&feedbackEvent );

				// If it has a pitch, then show that pitch
			if (feedbackEvent.HasAttribute( EvAttr_Pitch ))
				((CPianoKeyboardView *)labelView)->SetSelKey( feedbackEvent.note.pitch );
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

void CLinearEditor::FinishDrag(
	BPoint		point,
	ulong		buttons,
	bool		commit )
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
				ApplyDragOp();
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
#endif

bool CLinearEditor::ConstructEvent( BPoint point )
{
	int32		time;
	CMeVDoc		&doc = Document();

		// Initialize a new event.
	newEv.SetCommand( TrackWindow()->GetNewEventType( EvtType_Note ) );
			
		// Compute the difference between the original
		// time and the new time we're dragging the events to.
	time = Handler( newEv ).QuantizeDragTime(
		*this,
		newEv,
		0,
		BPoint( 0.0, 0.0 ),
		point,
		true );
	TrackWindow()->DisplayMouseTime( Track(), time );

	newEv.SetStart( time );
	newEv.SetDuration( TrackWindow()->GetNewEventDuration() - 1 );
	newEv.SetVChannel( doc.GetDefaultAttribute( EvAttr_Channel ) );

	switch (newEv.Command()) {
	case EvtType_End:
		newEv.SetDuration( 0 );
		break;

	case EvtType_Note:
		newEv.note.pitch = ViewCoordsToPitch( point.y, true );
		newEv.note.attackVelocity = doc.GetDefaultAttribute( EvAttr_AttackVelocity );
		newEv.note.releaseVelocity = doc.GetDefaultAttribute( EvAttr_ReleaseVelocity );
		break;

	default:
		return false;
	};

	return true;
}

// ---------------------------------------------------------------------------
// Do additional audio feedback or selection for this event..

void CLinearEditor::DoEventFeedback( const Event &ev )
{
		// Start audio feedback
	if (gPrefs.FeedbackEnabled( EvAttr_Pitch ))
		CPlayerControl::DoAudioFeedback( &Track()->Document(), EvAttr_Pitch, ev.note.pitch, &ev );
			
		// If it has a pitch, then show that pitch
	if (ev.HasAttribute( EvAttr_Pitch ))
		((CPianoKeyboardView *)labelView)->SetSelKey( ev.note.pitch );
}

// ---------------------------------------------------------------------------
// Remove any feedback artifacts for this event.

void CLinearEditor::KillEventFeedback()
{
	((CPianoKeyboardView *)labelView)->SetSelKey( -1 );
}

// ---------------------------------------------------------------------------
// Draw piano keyboard

void CPianoKeyboardView::Draw( BRect updateRect )
{
	BRect		r( Bounds() );
	int32		yPos;
	int32		lineCt;
	int32		wh = editor->whiteKeyStep,
				bl = (wh + 1) / 4;
	int32		key;
	static int8	black[ 7 ] = { 1, 1, 0, 1, 1, 1, 0 };

	SetHighColor( 128, 128, 128 );

	FillRect( BRect( updateRect.right, r.top, updateRect.right, r.bottom ) );

	SetHighColor( 255, 255, 255 );
	FillRect( BRect( updateRect.left, editor->stripLogicalHeight, updateRect.right - 1, r.bottom ) );
	SetLowColor( 128, 128, 128 );

		// Draw horizontal grid lines.
		// REM: This needs to be faster.
	for (	yPos = editor->stripLogicalHeight, lineCt = 0, key = 0;
			yPos >= updateRect.top;
			yPos -= wh, lineCt++, key++ )
	{
		if (lineCt >= 7) lineCt = 0;

			// Fill solid rectangle with gridline
		if (yPos <= updateRect.bottom + wh + bl)
		{
			FillRect(	BRect(	updateRect.left, yPos,
								updateRect.right - 1, yPos ),
						B_SOLID_LOW );
		
			if (selKey == key)		SetHighColor( 148, 148, 255 );
			else						SetHighColor( 255, 255, 255 );

			FillRect(	BRect(	r.left, yPos - wh + 1,
								r.right - 1, yPos - 1 ) );

		}
		if (black[ lineCt ]) key++;
	}

	SetHighColor( 96, 96, 96 );
	SetLowColor( 148, 148, 255 );

		// Draw horizontal grid lines.
		// REM: This needs to be faster.
	for (	yPos = editor->stripLogicalHeight, lineCt = 0, key = 0;
			yPos >= updateRect.top;
			yPos -= wh, lineCt++, key++ )
	{
		if (lineCt >= 7) lineCt = 0;

		if (black[ lineCt ]) key++;

			// Fill solid rectangle with gridline
		if (yPos <= updateRect.bottom + wh + bl)
		{
			if (black[ lineCt ])
			{
				FillRect(	BRect(	r.left, yPos - wh - bl,
									r.left + 11, yPos - wh + bl ) );

				if (selKey == key)
				{
					FillRect(	BRect(	r.left, yPos - wh - bl + 1,
										r.left + 10, yPos - wh + bl - 1 ),
								B_SOLID_LOW );
				}
			}
		}
	}
}

void CPianoKeyboardView::SetSelKey( int32 newKey )
{
	BRect	r( Bounds() );

	if (selKey != newKey)
	{
		BRegion		region;
		BRect		update;
	
		r.bottom	= editor->PitchToViewCoords( selKey ) + 1.0;
		r.top	= r.bottom - editor->whiteKeyStep - 2.0;
		
		region.Include( r );
		update = r;

		selKey = newKey;
	
		r.bottom	= editor->PitchToViewCoords( selKey ) + 1.0;
		r.top	= r.bottom - editor->whiteKeyStep - 2.0;
		
		region.Include( r );
		update = update | r;

		Window()->Lock();
		ConstrainClippingRegion( &region );	
		Draw( update );
		ConstrainClippingRegion( NULL );	
		Window()->Unlock();
	}
}
