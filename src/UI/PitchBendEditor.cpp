/* ===================================================================== *
 * PitchBendEditor.cpp (MeV/UI)
 * ===================================================================== */

#include "PitchBendEditor.h"

#include "CursorCache.h"
#include "Destination.h"
#include "EventRenderer.h"
#include "EventTrack.h"
#include "Idents.h"
#include "MathUtils.h"
#include "MeVApp.h"
#include "MeVDoc.h"
#include "PlayerControl.h"
#include "StdEventOps.h"
#include "StripLabelView.h"
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

class DualBendOffsetOp
	:	public EventOp
{

public:							// Constructor

								DualBendOffsetOp(
									int16 dp)
								{ delta = dp; }

public:							// Operators

	void						operator()(
									Event &ev,
									TClockType clockType);

public:							// EventOp Implementation

	const char *				UndoDescription() const
								{ return "Change Pitch Bend"; }

public:							// Instance Data

	int32						delta;
};

void
DualBendOffsetOp::operator()(
	Event &ev,
	TClockType clockType)
{
	if (ev.Command() == EvtType_PitchBend)
	{
		if (ev.Duration() > 0)
		{
			ev.pitchBend.startBend =
				(uint16)CLAMP(0, ((int16)ev.pitchBend.startBend) + delta,
							  0x3fff );
			ev.pitchBend.targetBend =
				(uint16)CLAMP(0, ((int16)ev.pitchBend.targetBend) + delta,
							  0x3fff);
		}
		else
		{
			ev.pitchBend.startBend = ev.pitchBend.targetBend = 
				(uint16)CLAMP(0, ((int16)ev.pitchBend.targetBend) + delta,
							  0x3fff );
		}
	}
}

// ---------------------------------------------------------------------------
// pitch bend renderer class for pitch bend editor

class CPitchBendEventRenderer
	:	public CEventRenderer
{

public:							// No constructor

								CPitchBendEventRenderer(
									CEventEditor * const editor)
									:	CEventRenderer(editor)
								{ }

public:							// CEventRenderer Implementation

	// Invalidate the event
	void						Invalidate(
									const Event	&ev) const ;

	// Draw the event (or an echo)
	void						Draw(
									const Event &ev,
									bool shadowed) const;

	// Invalidate the event
	BRect						Extent(
									const Event &ev) const;

		// Pick a single event and returns the distance.
	long						Pick(
									const Event &ev,
									BPoint pickPt,
									short &partCode) const;

	// For a part code returned earlier, return a cursor
	// image...
	const BCursor *				Cursor(
									short partCode,
									int32 editMode = CEventEditor::TOOL_SELECT,
									bool dragging = false) const;

	long						QuantizeDragTime(
									const Event &ev,
									short partCode,
									BPoint clickPos,
									BPoint dragPos,
									bool initial = false) const;

	// Quantize the vertical position of the mouse based
	// on the event type and return a value delta.
	long						QuantizeDragValue(
									const Event &ev,
									short partCode,
									BPoint clickPos,
									BPoint dragPos ) const;

	// Make a drag op for dragging notes...
	EventOp *					CreateDragOp(
									const Event &ev,
									short partCode,
									long timeDelta,
									long valueDelta) const;

	EventOp *					CreateTimeOp(
									const Event &ev,
									short partCode,
									long timeDelta,
									long valueDelta) const;

protected:						// Accessors

	CPitchBendEditor * const	Editor() const
								{ return (CPitchBendEditor *)CEventRenderer::Editor(); }
};

void
CPitchBendEventRenderer::Invalidate(
	const Event &ev ) const
{
	Editor()->Invalidate(Extent(ev));
}

void
CPitchBendEventRenderer::Draw(
	const Event &ev,
	bool shadowed) const
{
	CDestination *dest = Editor()->TrackWindow()->Document()->FindDestination(ev.GetVChannel());
	BRect r;

	if (shadowed)
		Editor()->SetDrawingMode( B_OP_BLEND );
	
	Editor()->SetHighColor(dest->Color());
		
	if ((ev.Duration() > 0) && (ev.pitchBend.updatePeriod > 0))
	{
		Editor()->BeginLineArray(ev.Duration() / ev.pitchBend.updatePeriod);
		for (int i = ev.pitchBend.updatePeriod; i < ev.Duration(); i += ev.pitchBend.updatePeriod)
		{
			long v = ev.pitchBend.startBend - 0x2000
					 + ((ev.pitchBend.targetBend - ev.pitchBend.startBend)
					    * i / ev.Duration());
			BPoint point(Editor()->TimeToViewCoords(ev.Start() + i),
						 Editor()->ValueToViewCoords(v));
			Editor()->AddLine(point, point, Editor()->HighColor());
		}
		Editor()->EndLineArray();

		r.left = Editor()->TimeToViewCoords(ev.Stop() ) - 2.0;
		r.top = Editor()->ValueToViewCoords(ev.pitchBend.targetBend - 0x2000) - 2.0;
		r.right = r.left + 4.0;
		r.bottom = r.top + 4.0;
	
		Editor()->FillEllipse(r);

		r.InsetBy(-1.0, -1.0);
		if (ev.IsSelected() && !shadowed && Editor()->IsSelectionVisible())
			Editor()->SetHighColor(0, 0, 255);
		else
			Editor()->SetHighColor(0, 0, 0);
		Editor()->StrokeEllipse(r);
	}

	r.left = Editor()->TimeToViewCoords(ev.Start()) - 2.0;
	r.top = Editor()->ValueToViewCoords(ev.pitchBend.startBend - 0x2000) - 2.0;
	r.right = r.left + 4.0;
	r.bottom = r.top + 4.0;
	
	Editor()->SetHighColor(dest->Color());
	Editor()->FillEllipse(r);
	
	if (ev.IsSelected() && !shadowed && Editor()->IsSelectionVisible())
		Editor()->SetHighColor(0, 0, 255);
	else
		Editor()->SetHighColor(0, 0, 0);
	
	r.InsetBy(-1.0, -1.0);
	Editor()->StrokeEllipse(r);
}

BRect
CPitchBendEventRenderer::Extent(
	const Event &ev) const
{
	float yStart = Editor()->ValueToViewCoords(ev.pitchBend.startBend - 0x2000);
	float yStop = Editor()->ValueToViewCoords(ev.pitchBend.targetBend - 0x2000);

	BRect r;
	r.left = Editor()->TimeToViewCoords(ev.Start()) - 5.0;
	r.right	= Editor()->TimeToViewCoords(ev.Stop()) + 5.0;
	r.bottom = MAX(yStart, yStop) + 5.0;
	r.top = MIN(yStart, yStop) - 5.0;

	return r;
}

long
CPitchBendEventRenderer::Pick(
	const Event &ev,
	BPoint pickPt,
	short &partCode) const
{
	if (ev.Duration() > 0)
	{
		BPoint start(Editor()->TimeToViewCoords(ev.Start()),
					 Editor()->ValueToViewCoords(ev.pitchBend.startBend - 0x2000));
		BPoint stop(Editor()->TimeToViewCoords(ev.Stop()),
					Editor()->ValueToViewCoords(ev.pitchBend.targetBend - 0x2000));
	
		float best = MAX(fabs(pickPt.x - stop.x), fabs(pickPt.y - stop.y));
		float dist = MAX(fabs(pickPt.x - start.x), fabs(pickPt.y - start.y));

		partCode = Part_End;
		if (dist < best)
		{
			best = dist;
			partCode = Part_Start;
		}
		
		if (best > 3.0)
		{
			dist = MathUtils::DistanceFromPointToLine(pickPt, start, stop);
			if (dist < best)
			{
				best = dist + 2.0;
				partCode = Part_Whole;
			}
		}
		
		if (best > 8.0)
			return LONG_MAX;
		return static_cast<long>(best);
	}
	else
	{
		BPoint diff(pickPt.x - Editor()->TimeToViewCoords(ev.Start()),
					pickPt.y - Editor()->ValueToViewCoords(ev.pitchBend.startBend - 0x2000));

		if ((diff.y > 8) || (diff.y < -8) || (diff.x > 10) || (diff.x < -8))
			return LONG_MAX;
		
		if (diff.x >= 3.0)
			partCode = Part_End;
		else
			partCode = Part_Whole;

		return static_cast<long>(MAX(fabs(diff.x), fabs(diff.y)));
	}
}

const BCursor *
CPitchBendEventRenderer::Cursor(
	short partCode,
	int32 editMode,
	bool dragging) const
{
	switch (partCode)
	{
		case Part_Whole:
		{
			if (dragging)
				return CCursorCache::GetCursor(CCursorCache::DRAGGING);
			return CCursorCache::GetCursor(CCursorCache::DRAGGABLE);
		}
		case Part_Start:
		case Part_End:
		{
			return CCursorCache::GetCursor(CCursorCache::CROSS_HAIR);
		}
	}

	return NULL;
}

long
CPitchBendEventRenderer::QuantizeDragValue(
	const Event &ev,
	short partCode,
	BPoint clickPos,
	BPoint dragPos) const
{
	// Simply scale the dragged value by the pixel step value
	double val = (clickPos.y - dragPos.y) / Editor()->pixelsPerValue;
	return static_cast<long>(val);
}

long
CPitchBendEventRenderer::QuantizeDragTime(
	const Event &ev,
	short partCode,
	BPoint clickPos,
	BPoint dragPos,
	bool initial) const
{
	return Editor()->ViewCoordsToTime(dragPos.x - clickPos.x);
}

EventOp *
CPitchBendEventRenderer::CreateDragOp(
	const Event &ev,
	short partCode,
	long timeDelta,
	long valueDelta) const
{
	switch (partCode)
	{
		case Part_Whole:
			return new DualBendOffsetOp(valueDelta);
		case Part_Start:
			return new IBendOffsetOp(valueDelta);
		case Part_End:
			return new BendOffsetOp(valueDelta);
	};

	return NULL;
}

EventOp *
CPitchBendEventRenderer::CreateTimeOp(
	const Event &ev,
	short partCode,
	long timeDelta,
	long valueDelta) const
{
	if (partCode == Part_End)
	{
		return new DurationOffsetOp(timeDelta);
	}
	else
	{
		timeDelta = MAX(timeDelta, - Editor()->Track()->MinSelectTime());
		return new TimeOffsetOp(timeDelta);
	}
}

// ---------------------------------------------------------------------------
// Constructor/Destructor

CPitchBendEditor::CPitchBendEditor(
	CStripFrameView	&frame,
	BRect rect)
	:	CContinuousValueEditor(frame, rect, "Pitch Bend")
{
	SetRendererFor(EvtType_PitchBend, new CPitchBendEventRenderer(this));

	minValue = 0 - 0x2000;
	maxValue = 0x3fff - 0x2000;
	eventAscent = 3;
	eventDescent = 3;
	verticalZoom = 6;

	CalcZoom();

	// Make the label view on the left-hand side
	SetLabelView(new CStripLabelView(BRect(-1.0, -1.0, 20.0, rect.Height() + 1),
									 "Pitch Bend", B_FOLLOW_TOP_BOTTOM,
									 B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE));
}

// ---------------------------------------------------------------------------
// Operations

float
CPitchBendEditor::ValueToViewCoords(
	int value)
{
	float val = floor((value - minValue) * pixelsPerValue);
	return stripLogicalHeight - val - eventDescent;
}

long
CPitchBendEditor::ViewCoordsToValue(
	float yPos,
	bool limit)
{
	long value = static_cast<long>((stripLogicalHeight - yPos - eventDescent)
								   / pixelsPerValue + minValue);
	if (limit)
	{
		if (value < minValue)
			return minValue;
		if (value > maxValue)
			return maxValue;
	}
	return value;
}

// ---------------------------------------------------------------------------
// CEventEditor Implementation

const BCursor *
CPitchBendEditor::CursorFor(
	int32 editMode) const
{
	// cannot create or delete events in this strip, only modify
	return CEventEditor::CursorFor(editMode);
}

void
CPitchBendEditor::DrawHorizontalGrid(
	BRect updateRect)
{
	BRect rect(updateRect);
	rect.top = rect.bottom = ValueToViewCoords(0);
	if (rect.Intersects(updateRect))
	{
		SetHighColor(160, 160, 160, 255);
		StrokeLine(rect.LeftTop(), rect.RightTop());
	}
}

void
CPitchBendEditor::MouseMoved(
	BPoint point,
	uint32 transit,
	const BMessage *message)
{
	CContinuousValueEditor::MouseMoved(point, transit, message);

	if (Window()->IsActive())
	{
		if ((transit == B_EXITED_VIEW) || (transit == B_OUTSIDE_VIEW))
		{
			TrackWindow()->SetVerticalPositionInfo("");
		}
		else
		{
			int32 value = ViewCoordsToValue(point.y);
			BString text;
			text << value;
			TrackWindow()->SetVerticalPositionInfo(text);
		}
	}
}

bool
CPitchBendEditor::ConstructEvent(
	BPoint point)
{
	// check if destination is set
	int32 destination = TrackWindow()->Document()->GetDefaultAttribute(EvAttr_Channel);
	if (TrackWindow()->Document()->FindDestination(destination) == NULL)
		return false;

	// Initialize a new event
	m_newEv.SetCommand(TrackWindow()->NewEventType(EvtType_PitchBend));

	// Compute the difference between the original
	// time and the new time we're dragging the events to.
	int32 time;
	time = RendererFor(m_newEv)->QuantizeDragTime(m_newEv, 0,
												 BPoint(0.0, 0.0), point,
												 true);
	TrackWindow()->SetHorizontalPositionInfo(Track(), time);

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

void
CPitchBendEditor::ZoomChanged(
	int32 diff)
{
	BRect r(Frame());
	float scroll = (ScrollValue(B_VERTICAL) + (r.bottom - r.top) / 2) / pixelsPerValue;

	verticalZoom += diff;
	if (verticalZoom > 12)
		verticalZoom = 12;
	else if (verticalZoom < 4)
		verticalZoom = 4;

	Hide();
	CalcZoom();
	SetScrollRange(scrollRange.x, scrollValue.x, stripLogicalHeight,
				   (scroll * pixelsPerValue) - (r.bottom - r.top) / 2);
	Show();
}

// END - PitchBendEditor.cpp
