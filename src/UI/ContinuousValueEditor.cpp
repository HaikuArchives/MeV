/* ===================================================================== *
 * ContinuousValueEditor.cpp (MeV/User Interface)
 * ===================================================================== */

#include "ContinuousValueEditor.h"

#include "EventTrack.h"
#include "VChannel.h"
#include "MeVApp.h"
#include "MeVDoc.h"
#include "PlayerControl.h"
#include "ResourceUtils.h"

// ---------------------------------------------------------------------------
// Constructor

CContinuousValueEditor::CContinuousValueEditor(
	BLooper			&inLooper,
	CTrackEditFrame	&inFrame,
	BRect			rect,
	const char		*name )
	:	CEventEditor(	inLooper, inFrame, rect, name, true, true )
{
}

// ---------------------------------------------------------------------------
// General drawing routine

void CContinuousValueEditor::Draw( BRect updateRect )
{
	long				startTime = ViewCoordsToTime( updateRect.left - 1.0 ),
					stopTime  = ViewCoordsToTime( updateRect.right + 1.0 );
//	int				yPos; //,
//					lineCt;
	SetHighColor( 255, 255, 255 );
	FillRect( updateRect );

	SetHighColor( 220, 220, 220 );

	FillRect(	BRect(	updateRect.left, stripLogicalHeight,
					updateRect.right, stripLogicalHeight ) );

#if 0
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
#endif

	DrawGridLines( updateRect );

		// Initialize an event marker for this track.
	StSubjectLock		trackLock( *Track(), Lock_Shared );
	EventMarker		marker( Track()->Events() );

	bounds = Bounds();

		// For each event that overlaps the current view, draw it. (locked channels first)
	for (	const Event *ev = marker.FirstItemInRange( startTime, stopTime );
			ev;
			ev = marker.NextItemInRange( startTime, stopTime ) )
	{

		if (ev->HasProperty( Event::Prop_Channel ) && Track()->IsChannelLocked( ev->GetVChannel() ))
			Handler( *ev ).Draw( *this, *ev, false );
	}
	
		// For each event that overlaps the current view, draw it. (unlocked channels overdraw!)
	for (	const Event *ev = marker.FirstItemInRange( startTime, stopTime );
			ev;
			ev = marker.NextItemInRange( startTime, stopTime ) )
	{
		if (!Track()->IsChannelLocked( ev->GetVChannel() || !ev->HasProperty( Event::Prop_Channel ) ))
			Handler( *ev ).Draw( *this, *ev, false );
	}
	
	EventOp	*echoOp = PendingOperation();
	if (echoOp == NULL)
		echoOp = DragOperation();

	if (IsSelectionVisible())
	{
		if (m_dragType == DragType_Create)
			DrawCreateEcho( startTime, stopTime );
		else if (echoOp != NULL)
			DrawEchoEvents( startTime, stopTime );
		else if (m_dragType == DragType_Select)
			DrawSelectRect();
		else if (m_dragType == DragType_Lasso)
			DrawLasso();
	}

	DrawPlaybackMarkers(m_pbMarkers, m_pbCount, updateRect, false);
}

// ---------------------------------------------------------------------------
// Periodic update time tick -- used to update playback markers

void CContinuousValueEditor::Pulse()
{
	UpdatePBMarkers();
		// REM: Add code to process newly recorded events
		// REM: Add code to edit events via MIDI.
}

// ---------------------------------------------------------------------------
// Update message from another observer

void CContinuousValueEditor::OnUpdate( BMessage *inMsg )
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

// ---------------------------------------------------------------------------
// Calculate factors relating to zoom

void CContinuousValueEditor::CalcZoom()
{
	stripLogicalHeight = (long)(16.0 * pow( 1.4, verticalZoom ));
	pixelsPerValue = (double)stripLogicalHeight / (maxValue - minValue);
	stripLogicalHeight += eventAscent + eventDescent;
}

// ---------------------------------------------------------------------------
// When attached to window, set scroll range based on strip's logical height

void CContinuousValueEditor::AttachedToWindow()
{
	BRect		r( Frame() );

	SetViewColor( B_TRANSPARENT_32_BIT );
	SetScrollRange(	scrollRange.x, scrollValue.x,
					stripLogicalHeight,
					(stripLogicalHeight - r.Height()/2) / 2 );
}

// ---------------------------------------------------------------------------
// Continuous value editor mouse movement handler

void CContinuousValueEditor::MouseMoved(
	BPoint		point,
	ulong		transit,
	const BMessage	* )
{
//	const Event		*ev;
//	short			partCode;
//	const uint8		*newCursor;

	if (transit == B_EXITED_VIEW)
	{
		TrackWindow()->DisplayMouseTime( NULL, 0 );
//		SetViewCursor(B_CURSOR_SYSTEM_DEFAULT);
		return;
	}
	
	TrackWindow()->DisplayMouseTime( Track(), ViewCoordsToTime( point.x ) );

	StSubjectLock		trackLock( *Track(), Lock_Shared );
	EventMarker		marker( Track()->Events() );

	bounds = Bounds();

//	if ((ev = PickEvent( marker, point, partCode )) != NULL)
//	{
//		newCursor = Handler( *ev ).CursorImage( partCode );
//	}
//	else newCursor = NULL;
//	
//	if (newCursor == NULL)
//	{
//		if (crossCursor == NULL)
//		{
//			crossCursor = ResourceUtils::LoadCursor(1);
//		}
//
//		newCursor = crossCursor;
//	}
//	
//	SetViewCursor( newCursor );
}
