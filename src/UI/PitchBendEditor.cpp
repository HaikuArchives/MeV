/* ===================================================================== *
 * PitchBendEditor.cpp (MeV/User Interface)
 * ===================================================================== */

#include "PitchBendEditor.h"

// Application
#include "Idents.h"
#include "MeVApp.h"
#include "MeVDoc.h"
// Engine
#include "EventTrack.h"
#include "PlayerControl.h"
#include "StdEventOps.h"
#include "VChannel.h"
// StripView
#include "StripLabelView.h"
// Support
#include "MathUtils.h"
#include "ResourceUtils.h"

// Gnu C Library
#include <float.h>
// Support Kit
#include <Beep.h>
#include <Debug.h>

enum E_PitchBendParts {
	Part_Whole = 0,				// the whole event, when start and stop are superimposed
	Part_Start,					// the beginning part only
	Part_End,						// the ending part only
};

/*	How many drag styles does a pitch bend event have? How many parts:

	Parts we can click on:
		Combined whole
			vertical = drag start and end values (tie together)
			horiztonal = drag start only
		connecting line
			vertical = drag start and end values (seperate delta)
			horizontal = drag start only
			
			(That requires a new op...)

		Start point
			vertical = drag start value
			horizontal = drag start only

			BendOffsetOp

		Combined whole + duration marker
			vertical = drag stop value
			horizontal = drag duration
		Stop point
			vertical = drag stop value
			hotizontal = drag 
			
			IBendOffsetOp
			
	Why are the sliders acting so funky?
*/

// ---------------------------------------------------------------------------
// An operator which modifies both pitch bends by an equal offset
// and also forces them to the same value if the duration is 0.

class DualBendOffsetOp : public EventOp {
	void operator()( Event &, TClockType );

public:
	int32			delta;

	DualBendOffsetOp( int16 dp ) { delta = dp; }
	const char *UndoDescription() const { return "Change Pitch Bend"; }
};

void DualBendOffsetOp::operator()( Event &ev, TClockType )
{
	if (ev.Command() == EvtType_PitchBend)
	{
		if (ev.Duration() > 0)
		{
			ev.pitchBend.startBend =
				(uint16)CLAMP( 0, ((int16)ev.pitchBend.startBend) + delta, 0x3fff );
			ev.pitchBend.targetBend =
				(uint16)CLAMP( 0, ((int16)ev.pitchBend.targetBend) + delta, 0x3fff );
		}
		else
		{
			ev.pitchBend.startBend = ev.pitchBend.targetBend =
				(uint16)CLAMP( 0, ((int16)ev.pitchBend.targetBend) + delta, 0x3fff );
		}
	}
}

// ---------------------------------------------------------------------------
// pitch bend handler class for pitch bend editor

class CPitchBendEventHandler : public CAbstractEventHandler {
		// No constructor

		// Invalidate the event
	void Invalidate(	CEventEditor	&editor,
						const Event		&ev ) const ;

		// Draw the event (or an echo)
	void Draw(			CEventEditor	&editor,
						const Event		&ev,
						bool 			shadowed ) const;

		// Invalidate the event
	BRect Extent(		CEventEditor	&editor,
						const Event		&ev ) const;

		// Pick a single event and returns the distance.
	long Pick(			CEventEditor	&editor,
						const Event		&ev,
						BPoint			pickPt,
						short			&partCode ) const;

		// For a part code returned earlier, return a cursor
		// image...
	const uint8 *CursorImage( short partCode ) const;

	long QuantizeDragTime(
		CEventEditor	&editor,
		const Event		&inClickEvent,
		short			partCode,			// Part of event clicked
		BPoint			inClickPos,
		BPoint			inDragPos,
		bool				inInitial = false ) const;

		// Quantize the vertical position of the mouse based
		// on the event type and return a value delta.
	long QuantizeDragValue(
		CEventEditor	&editor,
		const Event		&inClickEvent,
		short			partCode,			// Part of event clicked
		BPoint			inClickPos,
		BPoint			inDragPos ) const;

		// Make a drag op for dragging notes...
	EventOp *CreateDragOp(
		CEventEditor	&editor,
		const Event		&ev,
		short			partCode,
		long			timeDelta,			// The horizontal drag delta
		long			valueDelta ) const;

	EventOp *CreateTimeOp(
		CEventEditor	&editor,			// The editor
		const Event		&ev,				// The clicked event
		short			partCode,			// Part of event clicked
		long			timeDelta,			// The horizontal drag delta
		long			valueDelta ) const;
};

	// Invalidate the event
void CPitchBendEventHandler::Invalidate(
	CEventEditor	&editor,
	const Event		&ev ) const
{
	((CPitchBendEditor &)editor).Invalidate( Extent( editor, ev ) );
}

	// Draw the event (or an echo)
void CPitchBendEventHandler::Draw(
	CEventEditor	&editor,
	const Event		&ev,
	bool 			shadowed ) const
{
	CPitchBendEditor	&lEditor = (CPitchBendEditor &)editor;
	bool				locked = 	editor.Track()->IsChannelLocked( ev.GetVChannel() );
	VChannelEntry *vce = lEditor.TrackWindow()->Document()->GetVChannel(ev.GetVChannel());
	BRect			r;

	if (shadowed)
	{
		if (locked) return;
		lEditor.SetDrawingMode( B_OP_BLEND );
	}
	
	if (locked)	lEditor.SetHighColor( 192, 192, 192 );
	else			lEditor.SetHighColor( vce->fillColor );
		
	if (ev.Duration() > 0 && ev.pitchBend.updatePeriod > 0)
	{
		long				v;
			
		for (int i = ev.pitchBend.updatePeriod; i < ev.Duration(); i += ev.pitchBend.updatePeriod)
		{
			v 	= ev.pitchBend.startBend - 0x2000
				+ ((ev.pitchBend.targetBend - ev.pitchBend.startBend) * i / ev.Duration());
			
			r.left = lEditor.TimeToViewCoords( ev.Start() + i ) - 1.0;
			r.top = lEditor.ValueToViewCoords( v ) - 1.0;			
			r.right = r.left + 1.0;
			r.bottom = r.top + 1.0;
			
			lEditor.FillRect( r );
		}

		r.left = lEditor.TimeToViewCoords( ev.Stop() ) - 2.0;
		r.top = lEditor.ValueToViewCoords( ev.pitchBend.targetBend - 0x2000 ) - 2.0;
		r.right = r.left + 4.0;
		r.bottom = r.top + 4.0;
	
		lEditor.FillRect( r );

		r.InsetBy( -1.0, -1.0 );
		if (locked)
			lEditor.SetHighColor( 128, 128, 128 );
		else if (ev.IsSelected() && !shadowed && editor.IsSelectionVisible())
			lEditor.SetHighColor( 0, 0, 255 );
		else	
			lEditor.SetHighColor( 0, 0, 0 );
		lEditor.StrokeRect( r );
	}

	r.left = lEditor.TimeToViewCoords( ev.Start() ) - 2.0;
	r.top = lEditor.ValueToViewCoords( ev.pitchBend.startBend - 0x2000 ) - 2.0;
	r.right = r.left + 4.0;
	r.bottom = r.top + 4.0;
	
	if (locked)	lEditor.SetHighColor( 192, 192, 192 );
	else			lEditor.SetHighColor( vce->fillColor );
	lEditor.FillRect( r );
	
	if (locked)
		lEditor.SetHighColor( 128, 128, 128 );
	else if (ev.IsSelected() && !shadowed && editor.IsSelectionVisible())
		lEditor.SetHighColor( 0, 0, 255 );
	else lEditor.SetHighColor( 0, 0, 0 );
	
	r.InsetBy( -1.0, -1.0 );
	lEditor.StrokeRect( r );
	lEditor.SetDrawingMode( B_OP_COPY );
}

	// Compute the extent of the event.
BRect CPitchBendEventHandler::Extent(
	CEventEditor		&editor,
	const Event		&ev ) const
{
	CPitchBendEditor	&lEditor = (CPitchBendEditor &)editor;
	BRect			r;
	int32			yStart, yStop;
	
	yStart	= lEditor.ValueToViewCoords( ev.pitchBend.startBend - 0x2000 );
	yStop	= lEditor.ValueToViewCoords( ev.pitchBend.targetBend - 0x2000 );

	r.left		= lEditor.TimeToViewCoords( ev.Start() ) - 5.0;
	r.right	= lEditor.TimeToViewCoords( ev.Stop()  ) + 5.0;
	r.bottom	= MAX( yStart, yStop ) + 5.0;
	r.top		= MIN( yStart, yStop ) - 5.0;

	return r;
}

	//	Returns the distance between a point and a line
static float DistFromPointToLine( BPoint &lineStart, BPoint &lineEnd, BPoint &testPt )
{
	double			x1 = testPt.x  - lineStart.x,
					y1 = testPt.y  - lineStart.y;

	double			x2 = lineEnd.x - lineStart.x,
					y2 = lineEnd.y - lineStart.y;

	double			dist;

	if (x2 < 0) { x2 = -x2; x1 = -x1; }
	if (y2 < 0) { y2 = -y2; y1 = -y1; }

		//	If line is just a single point, then return maxint
	if (x2 == 0 && y2 == 0) return DBL_MAX;

		//	First, get the distance along the line
	dist = x1 * x2 + y1 * y2;

		//	If closest point on line is not between the two points, then
		//	return maxint.
	if (dist < 0 || dist > (x2 * x2 + y2 * y2)) return DBL_MAX;

	if (x2 > y2)	dist = y1 - y2 * x1 / x2;
	else			dist = x1 - x2 * y1 / y2;

	return fabs(dist);
}

	// Pick a single event and return the part code
	// (or -1 if event not picked)
long CPitchBendEventHandler::Pick(
	CEventEditor	&editor,
	const Event		&ev,
	BPoint			pickPt,
	short			&partCode ) const
{
	CPitchBendEditor	&lEditor = (CPitchBendEditor &)editor;
	if (ev.Duration() > 0)
	{
		BPoint			start,
						stop;
		int32			dist,
						best;
					
		start.x = lEditor.TimeToViewCoords( ev.Start() );
		start.y = lEditor.ValueToViewCoords( ev.pitchBend.startBend - 0x2000 );
	
		stop.x = lEditor.TimeToViewCoords( ev.Stop() );
		stop.y = lEditor.ValueToViewCoords( ev.pitchBend.targetBend - 0x2000 );
	
		best	= MAX(fabs( pickPt.x - stop.x ), fabs( pickPt.y - stop.y ) );
		dist	= MAX(fabs( pickPt.x - start.x ), fabs( pickPt.y - start.y ) );
		
		partCode = Part_End;
		if (dist < best)
		{
			best = dist;
			partCode = Part_Start;
		}
		
		if (best > 3)
		{
			dist = DistFromPointToLine( start, stop, pickPt );
			if (dist < best)
			{
				best = dist + 2;
				partCode = Part_Whole;
			}
		}
		
		if (best > 8) return LONG_MAX;
		return best;
	}
	else
	{
		BPoint			diff;
		
		diff.x = pickPt.x - lEditor.TimeToViewCoords( ev.Start() );
		diff.y = pickPt.y - lEditor.ValueToViewCoords( ev.pitchBend.startBend - 0x2000 );

		if (diff.y > 8 || diff.y < -8 || diff.x > 10 || diff.x < -8) return LONG_MAX;
		
		if (diff.x >= 3) partCode = Part_End;
		else partCode = Part_Whole;

		return MAX(fabs( diff.x ), fabs( diff.y ) );
	}
}

const uint8 *CPitchBendEventHandler::CursorImage( short partCode ) const
{
	switch (partCode) {
	case Part_Whole:
	case Part_Start:
		return B_HAND_CURSOR;			// Return the normal hand cursor

//	case Part_End:						// Return resizing cursor
//		if (resizeCursor == NULL)
//		{
//			resizeCursor = ResourceUtils::LoadCursor(2);
//		}
//		return resizeCursor;
	}

	return NULL;
}

	// Quantize the vertical position of the mouse based
	// on the event type and return a value delta.
long CPitchBendEventHandler::QuantizeDragValue(
	CEventEditor		&editor,
	const Event		&inClickEvent,
	short			partCode,			// Part of event clicked
	BPoint			inClickPos,
	BPoint			inDragPos ) const
{
	CPitchBendEditor	&lEditor = (CPitchBendEditor &)editor;

		// Simply scale the dragged value by the pixel step value
	return (long)((double)(inClickPos.y - inDragPos.y) / lEditor.pixelsPerValue);
}

	// Pitchbends have NO time quantization
long CPitchBendEventHandler::QuantizeDragTime(
	CEventEditor	&editor,
	const Event		&inClickEvent,
	short			partCode,			// Part of event clicked
	BPoint			inClickPos,
	BPoint			inDragPos,
	bool				inInitial ) const
{
	return editor.ViewCoordsToTime( inDragPos.x - inClickPos.x );
}

EventOp *CPitchBendEventHandler::CreateDragOp(
	CEventEditor	&editor,
	const Event		&ev,
	short			partCode,
	long			timeDelta,			// The horizontal drag delta
	long			valueDelta ) const
{
	switch (partCode) {
	case Part_Whole:			return new DualBendOffsetOp( valueDelta );
	case Part_Start:			return new IBendOffsetOp( valueDelta );
	case Part_End:			return new BendOffsetOp( valueDelta );
	};
	return NULL;
}

EventOp *CPitchBendEventHandler::CreateTimeOp(
	CEventEditor		&editor,
	const Event		&ev,
	short			partCode,
	long				timeDelta,			// The horizontal drag delta
	long				valueDelta ) const
{
	if (partCode == Part_End) return new DurationOffsetOp( timeDelta );
	else
	{
		timeDelta = MAX( timeDelta, -editor.Track()->MinSelectTime() );
		return new TimeOffsetOp( timeDelta );
	}
}

// ---------------------------------------------------------------------------
// Dispatch table for linear editor

CPitchBendEventHandler		pitchBendHandler;

// ---------------------------------------------------------------------------
// Linear Editor class

	// ---------- Constructor

CPitchBendEditor::CPitchBendEditor(
	BLooper			&inLooper,
	CTrackEditFrame	&inFrame,
	BRect			rect )
	:	CContinuousValueEditor(	inLooper, inFrame, rect, "Pitch Bend Editor" )
{
	SetHandlerFor(EvtType_PitchBend, &pitchBendHandler);

	minValue		= 0 - 0x2000;
	maxValue 	= 0x3fff - 0x2000;
	eventAscent	= 3.0;
	eventDescent	= 3.0;
	verticalZoom	= 6;

	CalcZoom();
	SetZoomTarget( (CObserver *)this );

	// Make the label view on the left-hand side
	SetLabelView(new CStripLabelView(BRect(-1.0, -1.0, 20.0, rect.Height() + 1),
									 "Pitch Bend", B_FOLLOW_TOP_BOTTOM,
									 B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE));

	SetFlags(Flags() | B_PULSE_NEEDED);
}

// ---------------------------------------------------------------------------
// Convert an event value to a y-coordinate

long CPitchBendEditor::ValueToViewCoords( int value )
{
	return stripLogicalHeight - ((value - minValue) * pixelsPerValue) - eventDescent;
}

// ---------------------------------------------------------------------------
// Convert a y-coordinate to an event value

long CPitchBendEditor::ViewCoordsToValue( int yPos, bool limit )
{
	long	value =  (stripLogicalHeight - yPos - eventDescent) / pixelsPerValue + minValue;

	if (limit)
	{
		if (value < minValue) return minValue;
		if (value > maxValue) return maxValue;
	}
	return value;
}

void CPitchBendEditor::MessageReceived( BMessage *msg )
{
	switch (msg->what) {
	case ZoomOut_ID:

		if (verticalZoom < 12)
		{
			BRect		r( Frame() );
			float		sValue = (ScrollValue( B_VERTICAL ) + (r.bottom - r.top) / 2) / pixelsPerValue;

			verticalZoom++;
			CalcZoom();
			Hide();
			SetScrollRange(	scrollRange.x, scrollValue.x,
							stripLogicalHeight,
							(sValue * pixelsPerValue) - (r.bottom - r.top) / 2 );
			Show();
		}
		break;

	case ZoomIn_ID:

		if (verticalZoom > 4) 
		{
			BRect		r( Frame() );
			float		sValue = (ScrollValue( B_VERTICAL ) + (r.bottom - r.top) / 2) / pixelsPerValue;

			verticalZoom--;
			CalcZoom();
			Hide();
			SetScrollRange(	scrollRange.x, scrollValue.x,
							stripLogicalHeight,
							(sValue * pixelsPerValue) - (r.bottom - r.top) / 2 );
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
// Update message from another observer

void CPitchBendEditor::OnUpdate( BMessage *inMsg )
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
		r.left = TimeToViewCoords( minTime ) - 5.0;
	}
	else minTime = 0;

	if (inMsg->FindInt32( "MaxTime", 0, &maxTime ) == B_OK)
	{
		r.right = TimeToViewCoords( maxTime ) + 5.0;
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
			if (ev->Command() == EvtType_PitchBend && ev->GetVChannel() == channel)
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
			if (ev->Command() == EvtType_PitchBend)
				Handler( *ev ).Invalidate( *this, *ev );
		}
	}
	else
	{
		Invalidate( r );
	}
}

// ---------------------------------------------------------------------------
// Linear editor mouse movement handler

bool CPitchBendEditor::ConstructEvent( BPoint point )
{
	// check if destination is set
	int32 destination = TrackWindow()->Document()->GetDefaultAttribute(EvAttr_Channel);
	if (TrackWindow()->Document()->GetVChannel(destination) == NULL)
		return false;

	// Initialize a new event
	m_newEv.SetCommand(TrackWindow()->NewEventType(EvtType_PitchBend));

	// Compute the difference between the original
	// time and the new time we're dragging the events to.
	int32 time;
	time = Handler(m_newEv).QuantizeDragTime(*this, m_newEv, 0,
											 BPoint(0.0, 0.0), point,
											 true);
	TrackWindow()->DisplayMouseTime(Track(), time);

	m_newEv.SetStart(time);
	m_newEv.SetDuration(0);
	m_newEv.SetVChannel(destination);

	switch (m_newEv.Command())
	{
		case EvtType_PitchBend:
		{
			m_newEv.pitchBend.targetBend = m_newEv.pitchBend.startBend = ViewCoordsToValue(point.y, true) + 0x2000;
			m_newEv.pitchBend.updatePeriod =  TrackWindow()->Document()->GetDefaultAttribute(EvAttr_UpdatePeriod);
			break;
		}
		default:
		{
			beep();
			return false;
		}
	}

	return true;
}

// END - PitchBendEditor.cpp
