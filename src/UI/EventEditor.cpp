/* ===================================================================== *
 * EventEditor.cpp (MeV/User Interface)
 * ===================================================================== */

#include "EventEditor.h"

#include "DataSnap.h"
#include "EventOp.h"
#include "EventTrack.h"
#include "MeVApp.h"
#include "MeVDoc.h"
#include "PlayerControl.h"
#include "Polygon.h"
#include "StdEventOps.h"
#include "StripFrameView.h"
#include "StripLabelView.h"

// Support Kit
#include <Beep.h>
#include <Debug.h>

// Debugging Macros
#define D_HOOK(x) //PRINT(x)		// CStripView Implementation

CNullEventHandler		gNullEventHandler;
CEndEventHandler		gEndEventHandler;

// ---------------------------------------------------------------------------
// Constructor/Destructor

CEventEditor::CEventEditor(
	BLooper	&looper,
	CStripFrameView	&frame,
	BRect rect,
	const char *name,
	bool makeScroller,
	bool makeMagButtons)
	:	CStripView(frame, rect, name, makeScroller, makeMagButtons),
		CObserver(looper, frame.Track()),
		m_track((CEventTrack *)frame.Track()),
		m_frame(frame),
		m_lasso(NULL),
		m_dragType(DragType_None),
		m_pbCount(0),
		m_dragOp(NULL),
		m_dragging(false)
{
}
					
CEventEditor::CEventEditor(
	BLooper &looper,
	CStripFrameView &frame,
	BRect rect,
	CEventTrack *track,
	const char *name,
	bool makeScroller,
	bool makeMagButtons)
	:	CStripView(frame, rect, name, makeScroller, makeMagButtons),
		CObserver(looper, track),
		m_track(track),
		m_frame(frame),
		m_lasso(NULL),
		m_dragType(DragType_None),
		m_pbCount(0),
		m_dragOp(NULL),
		m_dragging(false)
{
}
	
CEventEditor::~CEventEditor()
{
	delete m_lasso;
}
	
// ---------------------------------------------------------------------------
// Hook Functions

bool
CEventEditor::DoDrag(
	BPoint point,
	ulong buttons)
{
	bounds = Bounds();

	switch (m_dragType)
	{
		case DragType_Events:
		case DragType_CopyEvents:
		case DragType_Create:
		{
			StSubjectLock trackLock(*Track(), Lock_Exclusive);
			long newTimeDelta, newValueDelta;
			EventOp	*newValueOp, *newTimeOp, *newDragOp;
			const Event	*dragEvent;
			
			if (m_dragType == DragType_Create)
				dragEvent = &m_newEv;
			else
				dragEvent = Track()->CurrentEvent();
	
			const CAbstractEventHandler &handler(*HandlerFor(*dragEvent));
	
			// Compute the difference between the original
			// time and the new time we're dragging the events to.
			newTimeDelta = handler.QuantizeDragTime(*this, *dragEvent,
													m_clickPart, m_clickPos,
													point);
	
			// Compute the difference between the original value
			// and the new value we're dragging the events to.
			newValueDelta = handler.QuantizeDragValue(*this, *dragEvent,
													  m_clickPart, m_clickPos,
													  point);
	
			if ((newTimeDelta  == m_timeDelta) && (newValueDelta == m_valueDelta))
				return true;
	
			newValueOp = handler.CreateDragOp(*this, *dragEvent, m_clickPart,
											  newTimeDelta, newValueDelta);
			newTimeOp = handler.CreateTimeOp(*this, *dragEvent, m_clickPart,
											 newTimeDelta, newValueDelta);
	
			if (newTimeOp == NULL)
				newDragOp = newValueOp;
			else if (newValueOp == NULL)
				newDragOp = newTimeOp;
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
			if ((newValueOp != NULL) && (newValueDelta != m_valueDelta))
			{
				Event feedbackEvent(*(Event *)dragEvent);
	
				// REM: EvAttr_Pitch should not be hard coded.
				// This should be in fact computed from the handler.
				// Or from the valueOp.
				(*newValueOp)(feedbackEvent, Track()->ClockType());
				DoEventFeedback(feedbackEvent);
			}
			
			if (m_dragType == DragType_Create)
			{
				Event evCopy(m_newEv);
	
				if (m_dragOp)
					(*m_dragOp)(evCopy, Track()->ClockType());
				HandlerFor(evCopy)->Invalidate(*this, evCopy);
	
				evCopy = m_newEv;
				if (newDragOp)
					(*newDragOp)(evCopy, Track()->ClockType());
				HandlerFor(evCopy)->Invalidate(*this, evCopy);
			}
			else
			{
				if (m_dragOp)
					InvalidateSelection(*m_dragOp);
				if (newDragOp)
					InvalidateSelection(*newDragOp);
			}
	
			CRefCountObject::Release(m_dragOp);
			m_dragOp = newDragOp;
	
			m_timeDelta = newTimeDelta;
			m_valueDelta = newValueDelta;
	
			TrackWindow()->SetHorizontalPositionInfo(Track(),
													 dragEvent->Start()
													 + m_timeDelta);
			break;
		}
		case DragType_Erase:
		{
			StSubjectLock trackLock(*Track(), Lock_Exclusive);
			EventMarker marker(Track()->Events());
			short partCode;
			const Event *event = PickEvent(marker, point, partCode);
			if (event)
			{
				Track()->DeleteEvent(event);
			}
			break;
		}
		case DragType_Select:
		{
			if (m_cursorPos != point)
			{
				DrawSelectRect();
				m_cursorPos = point;
				DrawSelectRect();
				TrackWindow()->SetHorizontalPositionInfo(Track(),
														 ViewCoordsToTime(point.x));
			}
			break;
		}
		case DragType_Lasso:
		{
			if (m_cursorPos != point)
			{
				AddLassoPoint(point);
				m_cursorPos = point;
				TrackWindow()->SetHorizontalPositionInfo(Track(),
														 ViewCoordsToTime(point.x));
			}
			break;
		}
	}

	return true;
}

void
CEventEditor::DrawEchoEvents(
	int32 startTime,
	int32 stopTime)
{
	EventOp	*echoOp = PendingOperation();
	if (echoOp == NULL)
		echoOp = m_dragOp;

	// Initialize an event marker for this Track().
	EventMarker	marker(Track()->Events());
	const Event *ev;
	TClockType clockType = Track()->ClockType();

	// For each event that overlaps the current view, draw it.
	for (ev = marker.FirstItemInRange(Track()->MinSelectTime(), Track()->MaxSelectTime());
		 ev;
		 ev = marker.NextItemInRange(Track()->MinSelectTime(), Track()->MaxSelectTime()))
	{
		if (ev->IsSelected() && !Track()->IsChannelLocked(*ev))
		{
			Event evCopy(*(Event *)ev);
			(*echoOp)(evCopy, clockType);
			if ((evCopy.Start() <= stopTime) && (evCopy.Stop() >= startTime))
				HandlerFor(evCopy)->Draw(*this, evCopy, true);
		}
	}
}

void
CEventEditor::DrawGridLines(
	BRect updateRect)
{
	TClockType clockType = Track()->ClockType();
	long startTime = m_frame.ViewCoordsToTime(updateRect.left - 2.0,
											  clockType );
	CSignatureMap::Iterator timeIter(Track()->SigMap(), startTime);
	long time;
	bool major;
	int32 majorTime = Track()->SigMap().entries->sigMajorUnitDur;
	double majorXStep = m_frame.TimeToViewCoords(majorTime, clockType);
	int32 steps = 1;
	
	while (majorXStep < 24)
	{
		if (steps == 2)
			steps = 5;
		else
			steps *= 2;
		majorXStep = m_frame.TimeToViewCoords(majorTime * steps, clockType);
	}

	double x;
	for (time = timeIter.First(major); ; time = timeIter.Next(major))
	{
		x = m_frame.TimeToViewCoords(time, clockType);
		if (x > updateRect.right)
			break;

		if (major)
		{
			if (timeIter.MajorCount() % steps)
				continue;
			SetHighColor(160, 160, 160, 255);
		}
		else if (steps > 1)
			continue;
		else
			SetHighColor(220, 220, 220, 255);

		if (x > 0.0)
		{
			StrokeLine(BPoint(x, updateRect.top),
					   BPoint(x, updateRect.bottom));
		}
	}
}

void
CEventEditor::FinishDrag(
	BPoint point,
	ulong buttons,
	bool commit)
{
	// Initialize an event marker for this Track().
	StSubjectLock trackLock(*Track(), Lock_Exclusive);

	bounds = Bounds();

	TrackWindow()->Document()->SetActiveMaster(Track());

	if ((m_dragType == DragType_Events)
	 || (m_dragType == DragType_CopyEvents)
	 || (m_dragType == DragType_Create))
	{
		if ((m_timeDelta == 0) && (m_valueDelta == 0)
		 && (m_dragType != DragType_Create))
			commit = false;
		
		// Remove highlight from piano keyboard.
		KillEventFeedback();
		
		if (m_dragOp || m_dragType == DragType_Create)
		{
			if (commit)
			{
				long prevTrackDuration = Track()->LastEventTime();
		
				if (m_dragType == DragType_Create)
				{
					// Creating a new event
					if (m_dragOp)
						(*m_dragOp)(m_newEv, Track()->ClockType());
					HandlerFor(m_newEv)->Invalidate(*this, m_newEv);
					Track()->CreateEvent(this, m_newEv, "Create Event");
				}
				else if (m_dragType == DragType_CopyEvents)
				{
					Track()->CopySelectedEvents(this, *m_dragOp, "Drag");
				}
				else
				{
					Track()->ModifySelectedEvents(this, *m_dragOp, "Drag");
				}
	
				if (prevTrackDuration != Track()->LastEventTime())
					RecalcScrollRangeH();
			}
			else
				InvalidateSelection(*m_dragOp);
		}
	}
	else if (m_dragType == DragType_Select)
	{
		DoRectangleSelection();
	}
	else if (m_dragType == DragType_Lasso)
	{	
		DoLassoSelection();
	}

	CRefCountObject::Release(m_dragOp);
	m_dragOp = NULL;
	m_dragType = DragType_None;
	TrackWindow()->SetHorizontalPositionInfo(NULL, 0);
}

void
CEventEditor::InvalidateSelection()
{
	StSubjectLock trackLock(*Track(), Lock_Shared);
	EventMarker marker(Track()->Events());
	const Event *ev;

	// For each event that overlaps the current view, draw it.
	for (ev = marker.FirstItemInRange(Track()->MinSelectTime(), Track()->MaxSelectTime());
		 ev;
		 ev = marker.NextItemInRange(Track()->MinSelectTime(), Track()->MaxSelectTime()))
	{
		if (ev->IsSelected())
		{
			HandlerFor(*ev)->Invalidate(*this, *ev);
		}
	}
}

void
CEventEditor::InvalidateSelection(
	EventOp &inOp)
{
	const Event	*ev;
	StSubjectLock trackLock( *Track(), Lock_Shared );
	EventMarker	marker( Track()->Events() );
	TClockType clockType = Track()->ClockType();

	// For each event that overlaps the current view, draw it.
	for (ev = marker.FirstItemInRange(Track()->MinSelectTime(), Track()->MaxSelectTime());
		 ev;
		 ev = marker.NextItemInRange(Track()->MinSelectTime(), Track()->MaxSelectTime()))
	{
		// REM: This can be made more efficient, possibly
		// by not invalidating notes outside the time range
		if (ev->IsSelected())
		{
			Event evCopy(*(Event *)ev);
			inOp(evCopy, clockType);
			HandlerFor(*ev)->Invalidate(*this, evCopy);
		}
	}
}

void
CEventEditor::StartDrag(
	BPoint point,
	ulong buttons)
{
	const Event	*ev;
	StSubjectLock trackLock(*Track(), Lock_Exclusive);
	EventMarker marker(Track()->Events());
	short partCode;
	ulong modifierKeys = modifiers();
	bounds = Bounds();
	int32 toolState = TrackWindow()->CurrentTool();

	if ((ev = PickEvent(marker, point, partCode)) != NULL)
	{
		bool wasSelected = false;
		TrackWindow()->Document()->SetActiveMaster(Track());

		if (toolState == CTrackWindow::TOOL_ERASE)
		{
			Track()->DeleteEvent(ev);
			m_dragType = DragType_Erase;
		}
		else
		{
			if (ev->IsSelected())
			{
				wasSelected = true;
				if (modifierKeys & B_SHIFT_KEY)
				{
					((Event *)ev)->SetSelected( false );
					HandlerFor(*ev)->Invalidate(*this, *ev);
				
					// This could be faster.
					Track()->SummarizeSelection();
	
					// Let the world know the selection has changed
					CEventSelectionUpdateHint hint(*Track(), true);
					PostUpdate(&hint, true);
					return;
				}
			}
			else
			{
				if (!(modifierKeys & B_SHIFT_KEY))
					Track()->DeselectAll(this);

				((Event *)ev)->SetSelected(true);
				HandlerFor(*ev)->Invalidate(*this, *ev);
	
				// This could be faster.
				Track()->SummarizeSelection();
	
				// Let the world know the selection has changed
				CEventSelectionUpdateHint hint(*Track(), true);
				PostUpdate(&hint, true);
			}

			Track()->SetCurrentEvent(marker);
			if (modifierKeys & B_CONTROL_KEY && partCode == 0)
				m_dragType = DragType_CopyEvents;
			else
				m_dragType = DragType_Events;
			m_clickPart = partCode;

			TrackWindow()->SetHorizontalPositionInfo(Track(), ev->Start());
	
			if (partCode == 0 || wasSelected == false)
				DoEventFeedback(*ev);
		}

		m_timeDelta = 0;
		m_valueDelta = 0;
	}
	else if (buttons & B_SECONDARY_MOUSE_BUTTON)
	{
		if (!(modifierKeys & B_SHIFT_KEY))
			Track()->DeselectAll(this);

		// If clicking with right button, then also do a lasso drag.
		m_cursorPos = point;
		m_dragType = DragType_Lasso;
		AddLassoPoint(point);
	}
	else
	{
		if (!(modifierKeys & B_SHIFT_KEY))
			Track()->DeselectAll(this);

		if (toolState == CTrackWindow::TOOL_CREATE)
		{
			if (!ConstructEvent(point))
			{
				beep();
				return;
			}

			m_clickPart = 0;
			m_dragType = DragType_Create;
			HandlerFor(m_newEv)->Invalidate(*this, m_newEv);
			DoEventFeedback(m_newEv);

			// Poke selection times so that drag-limits will work properly
			// (Drag limits are implemented in Handler.QuantizeDragTime
			// and use the Track() selection times).
			// REM: This is somewhat of a kludge and may interfere
			// with plug-in development.
			Track()->SetSelectTime(m_newEv.Start(), m_newEv.Stop());

			m_timeDelta = 0;
			m_valueDelta = 0;
		}
		else
		{
			// Do a selection rectangle drag...
			m_cursorPos = m_anchorPos = point;
			m_dragType = DragType_Select;
			DrawSelectRect();
			TrackWindow()->SetHorizontalPositionInfo(Track(),
													 ViewCoordsToTime(point.x));
		}
	}
}

// ---------------------------------------------------------------------------
// Accessors

CAbstractEventHandler *
CEventEditor::HandlerFor(
	const Event &event) const
{
	handler_map::const_iterator i;
	if ((i = m_handlers.find((TEventType)event.Command())) != m_handlers.end()) {
		return (*i).second;
	}
	return &gNullEventHandler;
}

// ---------------------------------------------------------------------------
// Operations

void
CEventEditor::AddLassoPoint(
	BPoint &point)
{
	if (m_lasso == NULL)
	{
		m_lasso = new CPolygon(&point, 1);
	}
	else
	{
		DrawLasso();
		m_lasso->AddPoints(&point, 1);
		DrawLasso();
	}
}

void
CEventEditor::DoLassoSelection()
{
	BRect		r = m_lasso->Frame();
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
	for (ev = marker.FirstItemInRange(minTime, maxTime);
		 ev;
		 ev = marker.NextItemInRange(minTime, maxTime))
	{
		if (!ev->IsSelected())		// No point in selecting if already
		{
			const CAbstractEventHandler	&handler(*HandlerFor(*ev));
			
			if (&handler == &gNullEventHandler)
				continue;
			if (Track()->IsChannelLocked(*ev))
				continue;

			BRect extent(handler.Extent(*this, *ev));

			if (r.Intersects(extent)
			 && IsRectInLasso(extent, gPrefs.inclusiveSelection))
			{
				((Event *)ev)->SetSelected(true);
				handler.Invalidate(*this, *ev);
			}
		}
	}
	
	// Delete the lasso points.
	FinishLasso();
	Track()->SummarizeSelection();

	// Let the world know the selection has changed
	CEventSelectionUpdateHint hint(*Track(), true);
	PostUpdate(&hint, true);
}

void
CEventEditor::DrawLasso()
{
	if (m_lasso == NULL)
		return;

	PushState();

	SetDrawingMode(B_OP_INVERT);
	StrokePolygon(m_lasso->Points(), m_lasso->CountPoints(),
				  m_lasso->Frame(), true, B_MIXED_COLORS);

	PopState();
}

void
CEventEditor::FinishLasso()
{
	DrawLasso();
	delete m_lasso;
	m_lasso = NULL;
}

bool
CEventEditor::IsRectInLasso(
	const BRect &extent,
	bool inclusive) const
{
	if (m_lasso == NULL)
		return false;
	else
		return m_lasso->Contains(extent, inclusive);
}
	
BRect
CEventEditor::LassoFrame() const
{
	return m_lasso->Frame();
}

void
CEventEditor::RecalcScrollRangeH()
{
	m_frame.RecalcScrollRange();
}

double
CEventEditor::TimeToViewCoords(
	long timeVal) const
{
	return m_frame.TimeToViewCoords(timeVal, m_track->ClockType());
}

long
CEventEditor::ViewCoordsToTime(
	double relPos) const
{
	return m_frame.ViewCoordsToTime(relPos, m_track->ClockType());
}

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

	startTime = m_frame.ViewCoordsToTime(pickPt.x - mouseLeftSlopSize,
										 m_track->ClockType());
	stopTime  = m_frame.ViewCoordsToTime(pickPt.x + mouseRightSlopSize,
										 m_track->ClockType());

	// Initialize an event marker for this Track().
	EventMarker marker(Track()->Events());
	const Event *ev;

	// For each event that overlaps the current view, draw it.
	for (ev = marker.FirstItemInRange(startTime, stopTime);
		 ev;
		 ev = marker.NextItemInRange(startTime, stopTime))
	{
		// Don't allow picking of events on locked channels...
		if (Track()->IsChannelLocked(*ev))
			continue;
		
		long dist = HandlerFor(*ev)->Pick(*this, *ev, pickPt, partCode);

		if ((dist < bestPick) && (dist >= 0))
		{
			bestPick = dist;
			resultPartCode = partCode;
			resultMarker = marker;
		}
	}

	if (bestPick >= LONG_MAX)
		return NULL;

	return resultMarker;
}

long
CEventEditor::PickDurationEvent( 
	const Event &ev,
	int yTop,
	int yBottom,
	BPoint pickPt,
	short &partCode)
{
	long left  = static_cast<long>(TimeToViewCoords(ev.Start()));
	long right = static_cast<long>(TimeToViewCoords(ev.Stop()));
	long width = right - left;
	long offset, durX;

	if ((pickPt.y < yTop) || (pickPt.y > yBottom))
	{
		return LONG_MAX;
	}

	partCode = 0;

	// If the event is very short, then expand the size of the
	// hit zone slightly to help in picking on the event.
	if (width < 8)
		offset = 4 - width / 2;
	else
		offset = 0;

	if (pickPt.x < left)
	{
		// Mouse clicked to left of event
		if (pickPt.x < left - offset) return LONG_MAX;
		return 1000 + static_cast<long>(left - pickPt.x) * 8;
	}
	else if (pickPt.x > right)
	{
		// Mouse clicked to right of event.
		if (pickPt.x > right + offset) return LONG_MAX;
		partCode = 1;
		return 1000 + static_cast<long>(pickPt.x - right) * 8;
	}
	else								
	{
		// Mouse clicked between start & stop
		// If the mouse is within 8 pixels of the end of the note
		// (or 1/2 of the length of the note, if note is smaller than
		// 16 pixels) then
		durX  = right - MIN( 8, width / 2 );
	
		// Events which have duration have the ability to drag the
		// right edge of the event in order to drag the duration
		// of the event.
		if (pickPt.x > durX)
			partCode = 1;
		return 0;
	}
}

// ---------------------------------------------------------------------------
// Given a time value, return the time of the nearest grid-snap position.

int32
CEventEditor::SnapToGrid(
	int32 time,
	bool initial)
{
	int32 majorUnit;
	int32 extraTime;
	int32 quantizedExtra;

	Track()->SigMap().DecomposeTime(time, majorUnit, extraTime);
	
	if (initial)
		quantizedExtra = DataSnapLower(extraTime, 0, Track()->TimeGridSize());
	else
		quantizedExtra = DataSnapNearest(extraTime, 0, Track()->TimeGridSize());
	
	return time + (quantizedExtra - extraTime);
}

// ---------------------------------------------------------------------------
// Draw selection rectangle

void
CEventEditor::DrawPlaybackMarkers(
	int32 *markers,
	int32 count,
	BRect updateRect,
	bool erase)
{
	if (count < 1)
		return;

	PushState();

	long startTime = ViewCoordsToTime(updateRect.left - 1.0);
	long stopTime  = ViewCoordsToTime(updateRect.right + 1.0);
	BPoint p1(0.0, updateRect.top);
	BPoint p2(0.0, updateRect.bottom);
	rgb_color black = {0, 0, 0, 255};

	SetDrawingMode(B_OP_INVERT);
	BeginLineArray(count);
	for (int i = 0; i < count; i++)
	{
		if ((markers[i] >= startTime) && (markers[i] <= stopTime))
		{
			p1.x = p2.x = TimeToViewCoords(markers[i]);
			AddLine(p1, p2, black);
		}
	}
	EndLineArray();

	PopState();
}

void
CEventEditor::DrawSelectRect()
{
	BRect r;
	r.left = MIN(m_cursorPos.x, m_anchorPos.x);
	r.right = MAX(m_cursorPos.x, m_anchorPos.x);
	r.top = MIN(m_cursorPos.y, m_anchorPos.y);
	r.bottom = MAX(m_cursorPos.y, m_anchorPos.y);

	if (r.Width()  == 0.0)
		r.right += 1.0;
	if (r.Height() == 0.0)
		r.bottom += 1.0;

	SetDrawingMode(B_OP_INVERT);
	StrokeRect(r, B_MIXED_COLORS);
	SetDrawingMode(B_OP_COPY);
}

void
CEventEditor::UpdatePBMarkers()
{
	int32 newPBMarkers[8], count;

	// Process playback markers
	count = CPlayerControl::GetPlaybackMarkerTimes(Track(),
												   newPBMarkers,
												   MAX_PLAYBACK_MARKERS);
	if (count != m_pbCount
	 || memcmp(m_pbMarkers, newPBMarkers, count * sizeof(m_pbMarkers[0])) != 0)
	{
		DrawPlaybackMarkers(m_pbMarkers, m_pbCount, Bounds(), true);

		memcpy(m_pbMarkers, newPBMarkers, count * sizeof(m_pbMarkers[0]));
		m_pbCount = count;

		DrawPlaybackMarkers(m_pbMarkers, m_pbCount, Bounds(), false);
	}
}

void
CEventEditor::DoRectangleSelection()
{
	BRect		r;
	DrawSelectRect();
	long			minTime,
				maxTime;
	EventMarker	marker( Track()->Events() );
	const Event	*ev;
	
	r.left			= MIN(m_cursorPos.x, m_anchorPos.x );
	r.right			= MAX(m_cursorPos.x, m_anchorPos.x );
	r.top			= MIN(m_cursorPos.y, m_anchorPos.y );
	r.bottom		= MAX(m_cursorPos.y, m_anchorPos.y );

	if (r.Width()  == 0.0)
		r.right += 1.0;
	if (r.Height() == 0.0)
		r.bottom += 1.0;

	minTime = ViewCoordsToTime(r.left);
	maxTime = ViewCoordsToTime(r.right);

	// Now, select all events in the rectangle...
	// For each event that overlaps the current view, draw it.
	for (ev = marker.FirstItemInRange(minTime, maxTime);
		 ev;
		 ev = marker.NextItemInRange(minTime, maxTime))
	{
		// Don't allow picking of events on locked channels...
		if (Track()->IsChannelLocked(*ev)) continue;

		if (!ev->IsSelected())
		{
			const CAbstractEventHandler	&handler(*HandlerFor(*ev));
			if (&handler == &gNullEventHandler)
				continue;

			if (r.Contains(handler.Extent(*this, *ev))
				|| (gPrefs.inclusiveSelection
					&& r.Intersects(handler.Extent(*this, *ev))))
			{
				((Event *)ev)->SetSelected(true);
				handler.Invalidate(*this, *ev);
			}
		}
	}

	Track()->SummarizeSelection();

	// Let the world know the selection has changed
	CEventSelectionUpdateHint hint(*Track(), true);
	PostUpdate(&hint, true);
}

void
CEventEditor::DrawCreateEcho(
	int32 startTime,
	int32 stopTime)
{
	EventOp	*echoOp = PendingOperation();
	if (echoOp == NULL)
		echoOp = m_dragOp;

	Event evCopy(m_newEv);

	if (echoOp)
		(*echoOp)(evCopy, Track()->ClockType());
	
	if ((evCopy.Start() <= stopTime) && (evCopy.Stop()  >= startTime))
	{
		PushState();
		HandlerFor(evCopy)->Draw(*this, evCopy, true);
		PopState();
	}
}

// ---------------------------------------------------------------------------
// CStripView Implementation

void
CEventEditor::MouseDown(
	BPoint point)
{
	D_HOOK(("CEventEditor::MouseDown(%.2f, %.2f)\n", point.x, point.y));

	int32 buttons = B_PRIMARY_MOUSE_BUTTON;
	Window()->CurrentMessage()->FindInt32("buttons", &buttons);

	m_clickPos = point;

	SetMouseEventMask(B_POINTER_EVENTS,
					  B_NO_POINTER_HISTORY | B_LOCK_WINDOW_FOCUS);
	StartDrag(point, buttons);
	m_dragging = true;
}

void
CEventEditor::MouseMoved(
	BPoint point,
	uint32 transit,
	const BMessage *message)
{
	D_HOOK(("CEventEditor::MouseMoved(%.2f, %.2f)\n", point.x, point.y));

	if (m_dragging)
	{
		int32 buttons = B_PRIMARY_MOUSE_BUTTON;
		Window()->CurrentMessage()->FindInt32("buttons", &buttons);

		// Implement dragging		
		if (DoDrag(point, buttons) == false)
		{
			FinishDrag(point, buttons, false);
			return;
		}

		// Implement auto-scrolling (horizontal for frame)
		BRect r(ViewBounds());
		if (point.x > r.right)
			m_frame.ScrollBy(MIN((point.x - r.right) / 4, 10.0), B_HORIZONTAL);
		else if (point.x < r.left)
			m_frame.ScrollBy(MAX((point.x - r.left) / 4, -10.0), B_HORIZONTAL);
		
		// Implement auto-scrolling (vertical for editor)
		if (point.y > r.bottom)
			ScrollBy(MIN((point.y - r.bottom) / 4, 10.0), B_VERTICAL);
		else if (point.y < r.top)
			ScrollBy(MAX((point.y - r.top) / 4, - 10.0), B_VERTICAL);
	}
}

void
CEventEditor::MouseUp(
	BPoint point)
{
	D_HOOK(("CEventEditor::MouseUp(%.2f, %.2f)\n", point.x, point.y));

	if (m_dragging)
	{
		int32 buttons = B_PRIMARY_MOUSE_BUTTON;
		Window()->CurrentMessage()->FindInt32("buttons", &buttons);

		FinishDrag(point, buttons, true);
		m_dragging = false;
	}
}

void
CEventEditor::SetScrollValue(
	float inScrollValue,
	orientation inOrient)
{
	CStripView::SetScrollValue(inScrollValue, inOrient);
	if (RulerView())
		RulerView()->ScrollTo(scrollValue.x, 0.0);
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
