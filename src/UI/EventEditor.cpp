/* ===================================================================== *
 * EventEditor.cpp (MeV/UI)
 * ===================================================================== */

#include "EventEditor.h"

#include "CursorCache.h"
#include "DataSnap.h"
#include "EventOp.h"
#include "EventTrack.h"
#include "EventRenderer.h"
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
#define D_ALLOC(x) //PRINT(x)		// Constructor/Destructor
#define D_HOOK(x) //PRINT(x)		// CStripView Implementation
#define D_INTERNAL(x) //PRINT(x)	// Internal Operations

// ---------------------------------------------------------------------------
// Constructor/Destructor

CEventEditor::CEventEditor(
	CStripFrameView	&frame,
	BRect rect,
	const char *name,
	bool makeScroller,
	bool makeMagButtons)
	:	CStripView(frame, rect, name, makeScroller, makeMagButtons),
		CObserver(frame.Track()),
		m_track((CEventTrack *)frame.Track()),
		m_frame(frame),
		m_lasso(NULL),
		m_dragType(DragType_None),
		m_pbCount(0),
		m_dragOp(NULL),
		m_dragging(false)
{
	D_ALLOC(("CEventEditor::CEventEditor(%s)\n",
			 name));

	m_renderers[EvtType_End] = new CEndEventRenderer(this);
	m_nullEventRenderer = new CNullEventRenderer(this);
	for (event_type i = 1; i < EvtType_Count; i++)
		m_renderers[i] = m_nullEventRenderer;

	// start observing every destination in use by this part
	CDestination *dest = NULL;
	int32 index = 0;
	CReadLock lock(m_track);
	while ((dest = m_track->GetNextUsedDestination(&index)) != NULL)
	{
		D_ALLOC((" -> start observing used destination '%s'\n",
				 dest->Name()));
		dest->AddObserver(this);
	}
}
					
CEventEditor::CEventEditor(
	CStripFrameView &frame,
	BRect rect,
	CEventTrack *track,
	const char *name,
	bool makeScroller,
	bool makeMagButtons)
	:	CStripView(frame, rect, name, makeScroller, makeMagButtons),
		CObserver(track),
		m_track(track),
		m_frame(frame),
		m_lasso(NULL),
		m_dragType(DragType_None),
		m_pbCount(0),
		m_dragOp(NULL),
		m_dragging(false)
{
	D_ALLOC(("CEventEditor::CEventEditor(%s, %s)\n",
			 name, track->Name()));

	m_renderers[EvtType_End] = new CEndEventRenderer(this);
	m_nullEventRenderer = new CNullEventRenderer(this);
	for (event_type i = 1; i < EvtType_Count; i++)
		m_renderers[i] = m_nullEventRenderer;

	// start observing every destination in use by this part
	CDestination *dest = NULL;
	int32 index = 0;
	CReadLock lock(m_track);
	while ((dest = m_track->GetNextUsedDestination(&index)) != NULL)
	{
		D_ALLOC((" -> start observing used destination '%s'\n",
				 dest->Name()));
		dest->AddObserver(this);
	}
}

CEventEditor::~CEventEditor()
{
	delete m_lasso;

	for (event_type i = 0; i < EvtType_Count; i++)
		if (m_renderers[i] != m_nullEventRenderer)
			delete m_renderers[i];
	delete m_nullEventRenderer;

	if (m_track != NULL)
	{
		m_track->RemoveObserver(this);

		// stop observing the destinations in use by this part
		CDestination *dest = NULL;
		int32 index = 0;
		CReadLock lock(m_track);
		while ((dest = m_track->GetNextUsedDestination(&index)) != NULL)
		{
			D_ALLOC((" -> stop observing used destination '%s'\n",
					 dest->Name()));
			dest->RemoveObserver(this);
		}
	}
}

// ---------------------------------------------------------------------------
// Hook Functions

const BCursor *
CEventEditor::CursorFor(
	int32 editMode) const
{
	switch (editMode)
	{
		case TOOL_CREATE:
			return CCursorCache::GetCursor(CCursorCache::PENCIL);
		case TOOL_ERASE:
			return CCursorCache::GetCursor(CCursorCache::ERASER);
		default:
			return CCursorCache::GetCursor(CCursorCache::DEFAULT);
	}
}

bool
CEventEditor::DoDrag(
	BPoint point,
	ulong buttons)
{
	int32 editMode = TrackWindow()->CurrentTool();

	switch (m_dragType)
	{
		case DragType_Events:
		case DragType_CopyEvents:
		case DragType_Create:
		{
			StSubjectLock trackLock(*Track(), Lock_Exclusive);
			long newTimeDelta, newValueDelta;
			EventOp	*newValueOp, *newTimeOp, *newDragOp;
			const CEvent	*dragEvent;
			
			if (m_dragType == DragType_Create)
				dragEvent = &m_newEv;
			else
				dragEvent = Track()->CurrentEvent();
	
			const CEventRenderer *renderer(RendererFor(*dragEvent));
			be_app->SetCursor(renderer->Cursor(m_clickPart, editMode, true));

			// Compute the difference between the original
			// time and the new time we're dragging the events to.
			newTimeDelta = renderer->QuantizeDragTime(*dragEvent, m_clickPart,
													  m_clickPos, point);
	
			// Compute the difference between the original value
			// and the new value we're dragging the events to.
			newValueDelta = renderer->QuantizeDragValue(*dragEvent, m_clickPart,
														m_clickPos, point);
	
			if ((newTimeDelta  == m_timeDelta)
			 && (newValueDelta == m_valueDelta))
				return true;
	
			newValueOp = renderer->CreateDragOp(*dragEvent, m_clickPart,
												newTimeDelta, newValueDelta);
			newTimeOp = renderer->CreateTimeOp(*dragEvent, m_clickPart,
											   newTimeDelta, newValueDelta);

			if (newTimeOp == NULL)
				newDragOp = newValueOp;
			else if (newValueOp == NULL)
				newDragOp = newTimeOp;
			else
			{
				// If we have two operations, then pair them, and release them
				// (because pair keeps references to the objects)
				newDragOp = new PairOp(newValueOp, newTimeOp);
				CRefCountObject::Release(newValueOp);
				CRefCountObject::Release(newTimeOp);
			}
	
			// Do audio feedback for this drag, but only if value changed,
			// and only if we're dragging something that can be fed back.
			if ((newValueOp != NULL) && (newValueDelta != m_valueDelta))
			{
				CEvent feedbackEvent(*dragEvent);
	
				// REM: EvAttr_Pitch should not be hard coded.
				// This should be in fact computed from the renderer.
				// Or from the valueOp.
				(*newValueOp)(feedbackEvent, Track()->ClockType());
				DoEventFeedback(feedbackEvent);
			}
			
			if (m_dragType == DragType_Create)
			{
				CEvent evCopy(m_newEv);
	
				if (m_dragOp)
					(*m_dragOp)(evCopy, Track()->ClockType());
				RendererFor(evCopy)->Invalidate(evCopy);
	
				evCopy = m_newEv;
				if (newDragOp)
					(*newDragOp)(evCopy, Track()->ClockType());
				RendererFor(evCopy)->Invalidate(evCopy);
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
			const CEvent *event = PickEvent(marker, point, partCode);
			if (event)
			{
				Track()->DeleteEvent(event);
			}
			break;
		}
		case DragType_Select:
		{
			DoRectangleTracking(point);
			TrackWindow()->SetHorizontalPositionInfo(Track(),
													 ViewCoordsToTime(point.x));
			break;
		}
		case DragType_Lasso:
		{
			DoLassoTracking(point);
			TrackWindow()->SetHorizontalPositionInfo(Track(),
													 ViewCoordsToTime(point.x));
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
	const CEvent *ev;
	TClockType clockType = Track()->ClockType();

	// For each event that overlaps the current view, draw it.
	for (ev = marker.FirstItemInRange(Track()->MinSelectTime(), Track()->MaxSelectTime());
		 ev;
		 ev = marker.NextItemInRange(Track()->MinSelectTime(), Track()->MaxSelectTime()))
	{
		if (ev->IsSelected())
		{
			CEvent evCopy(*ev);
			(*echoOp)(evCopy, clockType);
			if ((evCopy.Start() <= stopTime) && (evCopy.Stop() >= startTime))
				RendererFor(evCopy)->Draw(evCopy, true);
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

	// draw section markers
	SetHighColor(0, 0, 0, 255);
	SetLowColor(B_TRANSPARENT_COLOR);
	x = FrameView().TimeToViewCoords(Track()->SectionStart(), clockType);
	StrokeLine(BPoint(x, updateRect.top), BPoint(x, updateRect.bottom),
			   B_MIXED_COLORS);
	x = FrameView().TimeToViewCoords(Track()->SectionEnd(), clockType);
	StrokeLine(BPoint(x, updateRect.top), BPoint(x, updateRect.bottom),
			   B_MIXED_COLORS);
}

void
CEventEditor::FinishDrag(
	BPoint point,
	ulong buttons,
	bool commit)
{
	// Initialize an event marker for this Track().
	StSubjectLock trackLock(*Track(), Lock_Exclusive);

	TrackWindow()->Document()->SetActiveMaster(Track());

	int32 editMode = TrackWindow()->CurrentTool();
	EventMarker marker(Track()->Events());
	short partCode;
	const CEvent	*ev;
	if ((ev = PickEvent(marker, point, partCode)) != NULL)
		be_app->SetCursor(RendererFor(*ev)->Cursor(partCode, editMode));
	else
		be_app->SetCursor(CursorFor(editMode));

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
					RendererFor(m_newEv)->Invalidate(m_newEv);
					Track()->CreateEvent(this, m_newEv, "Create Event");
				}
				else if (m_dragType == DragType_CopyEvents)
				{
					Track()->CopySelectedEvents(this, *m_dragOp, "Drag");
				}
				else
				{
					Track()->ModifySelectedEvents(this, *m_dragOp, "Drag");

					// Update document default attributes
					const CEvent *selectedEvent = Track()->CurrentEvent();
					if (selectedEvent && selectedEvent->HasProperty(CEvent::Prop_Duration))
						TrackWindow()->Document()->SetDefaultAttribute(EvAttr_Duration,
																	   selectedEvent->Duration());
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
		EndRectangleTracking();
	}
	else if (m_dragType == DragType_Lasso)
	{	
		EndLassoTracking();
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
	const CEvent *ev;

	// For each event that overlaps the current view, draw it.
	for (ev = marker.FirstItemInRange(Track()->MinSelectTime(), Track()->MaxSelectTime());
		 ev;
		 ev = marker.NextItemInRange(Track()->MinSelectTime(), Track()->MaxSelectTime()))
	{
		if (ev->IsSelected())
			RendererFor(*ev)->Invalidate(*ev);
	}
}

void
CEventEditor::InvalidateSelection(
	EventOp &inOp)
{
	const CEvent	*ev;
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
			CEvent evCopy(*ev);
			inOp(evCopy, clockType);
			RendererFor(*ev)->Invalidate(evCopy);
		}
	}
}

void
CEventEditor::StartDrag(
	BPoint point,
	ulong buttons)
{
	CWriteLock lock(Track());

	ulong modifierKeys = modifiers();
	int32 toolState = TrackWindow()->CurrentTool();

	const CEvent	*ev;
	EventMarker marker(Track()->Events());
	short partCode;
	if ((ev = PickEvent(marker, point, partCode)) != NULL)
	{
		bool wasSelected = false;
		TrackWindow()->Document()->SetActiveMaster(Track());
		be_app->SetCursor(RendererFor(*ev)->Cursor(partCode, toolState, true));

		if (toolState == TOOL_ERASE)
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
					const_cast<CEvent *>(ev)->SetSelected(false);
					RendererFor(*ev)->Invalidate(*ev);
				
					// This could be faster.
					Track()->SummarizeSelection();
	
					// Let the world know the selection has changed
					CEventSelectionUpdateHint hint(*Track(), true);
					Track()->PostUpdate(&hint, this);
					return;
				}
			}
			else
			{
				if (!(modifierKeys & B_SHIFT_KEY))
					Track()->DeselectAll(this);

				const_cast<CEvent *>(ev)->SetSelected(true);
				RendererFor(*ev)->Invalidate(*ev);

				// This could be faster.
				Track()->SummarizeSelection();

				// Update document default attributes
				if (ev->HasProperty(CEvent::Prop_Duration))
					TrackWindow()->Document()->SetDefaultAttribute(EvAttr_Duration,
																   ev->Duration());

				// Let the world know the selection has changed
				CEventSelectionUpdateHint hint(*Track(), true);
				Track()->PostUpdate(&hint, this);
			}

			Track()->SetCurrentEvent(marker);
			if ((modifierKeys & B_CONTROL_KEY) && (partCode == 0))
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
	else
	{
		if (!(modifierKeys & B_SHIFT_KEY))
			Track()->DeselectAll(this);

		switch (toolState)
		{
			case TOOL_CREATE:
			{
				if (!ConstructEvent(point))
				{
					beep();
					return;
				}
	
				m_clickPart = 0;
				m_dragType = DragType_Create;
				RendererFor(m_newEv)->Invalidate(m_newEv);
				DoEventFeedback(m_newEv);
	
				// Poke selection times so that drag-limits will work properly
				// (Drag limits are implemented in Handler.QuantizeDragTime
				// and use the Track() selection times).
				// REM: This is somewhat of a kludge and may interfere
				// with plug-in development.
				Track()->SetSelectTime(m_newEv.Start(), m_newEv.Stop());
	
				m_timeDelta = 0;
				m_valueDelta = 0;
				break;
			}
			case TOOL_SELECT:
			{
				switch (TrackWindow()->SelectionMode())
				{
					case RECTANGLE_SELECTION:
					{
						BeginRectangleTracking(point);
						break;
					}
					case LASSO_SELECTION:
					{
						BeginLassoTracking(point);
						break;
					}
				}
				long time = ViewCoordsToTime(point.x);
				TrackWindow()->SetHorizontalPositionInfo(Track(), time);
				break;
			}
			break;
		}
	}
}

bool
CEventEditor::SubjectReleased(
	CObservable *subject)
{
	if (subject == m_track)
	{
		m_track->RemoveObserver(this);
		m_track = NULL;
		return true;
	}

	return false;
}

void
CEventEditor::SubjectUpdated(
	BMessage *message)
{
	BRect r(Bounds());

	bool selChange = false;
	if (message->FindBool("SelChange", 0, &selChange) == B_OK)
	{
		if (!IsSelectionVisible())
			return;
	}

	int32 destAttrs = 0;
	if (message->FindInt32("DestAttrs", &destAttrs) == B_OK)
	{
		_destinationUpdated(message);
		return;
	}

	int32 trackHint = 0;
	if (message->FindInt32("TrackAttrs", 0, &trackHint) == B_OK)
	{
		if (trackHint & CTrack::Update_AddDest)
		{
			_destinationAdded(message);
			return;
		}
		if (trackHint & CTrack::Update_DelDest)
		{
			_destinationRemoved(message);
			return;
		}
		if (!(trackHint & (CTrack::Update_Duration | CTrack::Update_SigMap |
						   CTrack::Update_TempoMap | CTrack::Update_Section)))
			return;
	}

	int32 minTime;
	if (message->FindInt32("MinTime", 0, &minTime) != B_OK)
		minTime = ViewCoordsToTime(Bounds().left);
	r.left = TimeToViewCoords(minTime) - 3.0;

	int32 maxTime;
	if (message->FindInt32("MaxTime", 0, &maxTime) != B_OK)
		maxTime = ViewCoordsToTime(Bounds().right);
	r.right = TimeToViewCoords(maxTime) + 4.0;

	if (trackHint & CTrack::Update_Duration)
		RecalcScrollRangeH();

	uint8 channel;
	if (trackHint & (CTrack::Update_SigMap | CTrack::Update_TempoMap))
	{
		// Invalidate everything if signature map changed
		Invalidate();
	}
	else if (message->FindInt8("channel", 0, (int8 *)&channel) == B_OK)
	{
		StSubjectLock trackLock(*Track(), Lock_Shared);
		EventMarker	marker(Track()->Events());

		// For each event that overlaps the current view, draw it.
		for (const CEvent *ev = marker.FirstItemInRange(minTime, maxTime);
			 ev;
			 ev = marker.NextItemInRange(minTime, maxTime))
		{
			if ((ev->HasProperty(CEvent::Prop_Channel))
			 && (ev->GetVChannel() == channel))
			{
				RendererFor(*ev)->Invalidate(*ev);
			}
		}
	}
	else if (selChange)
	{
		StSubjectLock trackLock(*Track(), Lock_Shared);
		EventMarker marker(Track()->Events());

		// For each event that overlaps the current view, draw it.
		for (const CEvent *ev = marker.FirstItemInRange(minTime, maxTime);
			 ev;
			 ev = marker.NextItemInRange(minTime, maxTime))
		{
			RendererFor(*ev)->Invalidate(*ev);
		}
	}
	else
	{
		Invalidate(r);
	}
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
CEventEditor::BeginLassoTracking(
	BPoint point)
{
	// start a lasso drag.
	m_cursorPos = point;
	m_dragType = DragType_Lasso;
	AddLassoPoint(point);
	DrawLasso();
}

void
CEventEditor::DoLassoTracking(
	BPoint point)
{
	if (point != m_cursorPos)
	{
		AddLassoPoint(point);
		m_cursorPos = point;

		BRect r = m_lasso->Frame();
		if (r.Width()  == 0.0)
			r.right += 1.0;
		if (r.Height() == 0.0)
			r.bottom += 1.0;
	
		// Calculate start and end time
		long minTime = ViewCoordsToTime(r.left);
		long maxTime = ViewCoordsToTime(r.right);
	
		// Now, select all events in the rectangle...
		// For each event that overlaps the current view, draw it.
		EventMarker	marker(Track()->Events());
		const CEvent *ev;
		bool selectionChanged = false;
		for (ev = marker.FirstItemInRange(minTime, maxTime);
			 ev;
			 ev = marker.NextItemInRange(minTime, maxTime))
		{
			const CEventRenderer *renderer(RendererFor(*ev));
			if (renderer == m_nullEventRenderer)
				continue;

			BRect extent(renderer->Extent(*ev));

			if (!ev->IsSelected() && r.Intersects(extent)
			 && IsRectInLasso(extent, gPrefs.inclusiveSelection))
			{
				const_cast<CEvent *>(ev)->SetSelected(true);
				selectionChanged = true;
				renderer->Invalidate(*ev);
			}
			else if (ev->IsSelected()
			 && !IsRectInLasso(extent, gPrefs.inclusiveSelection))
			{
				const_cast<CEvent *>(ev)->SetSelected(false);
				selectionChanged = true;
				renderer->Invalidate(*ev);
			}
		}
	
		if (selectionChanged)
		{
			Track()->SummarizeSelection();
		
			// Let the world know the selection has changed
			CEventSelectionUpdateHint hint(*Track(), true);
			Track()->PostUpdate(&hint, this);
		}
	}
}

void
CEventEditor::EndLassoTracking()
{
	DrawLasso();
	delete m_lasso;
	m_lasso = NULL;
}

void
CEventEditor::BeginRectangleTracking(
	BPoint point)
{
	// Do a selection rectangle drag...
	m_cursorPos = m_anchorPos = point;
	m_dragType = DragType_Select;
	DrawSelectRect();
}

void
CEventEditor::DoRectangleTracking(
	BPoint point)
{
	if (point != m_cursorPos)
	{
		DrawSelectRect();
		BPoint oldCursorPos = m_cursorPos;
		m_cursorPos = point;
		DrawSelectRect();

		BRect r;
		r.left = MIN(m_cursorPos.x, m_anchorPos.x);
		r.right = MAX(m_cursorPos.x, m_anchorPos.x);
		r.top = MIN(m_cursorPos.y, m_anchorPos.y);
		r.bottom = MAX(m_cursorPos.y, m_anchorPos.y);
	
		if (r.Width() == 0.0)
			r.right += 1.0;
		if (r.Height() == 0.0)
			r.bottom += 1.0;

		long minTime = ViewCoordsToTime(MIN(r.left, oldCursorPos.x));
		long maxTime = ViewCoordsToTime(MAX(r.right, oldCursorPos.x));
	
		// Now, select all events in the rectangle...
		// For each event that overlaps the current view, draw it.
		EventMarker	marker(Track()->Events());
		const CEvent	*ev;
		bool selectionChanged = false;
		for (ev = marker.FirstItemInRange(minTime, maxTime);
			 ev;
			 ev = marker.NextItemInRange(minTime, maxTime))
		{
			const CEventRenderer *renderer(RendererFor(*ev));
			if (renderer == m_nullEventRenderer)
				continue;

			BRect extent(renderer->Extent(*ev));

			if (!ev->IsSelected()
			 && (gPrefs.inclusiveSelection ? r.Intersects(extent)
			 							   : r.Contains(extent)))
			{
				const_cast<CEvent *>(ev)->SetSelected(true);
				selectionChanged = true;
				renderer->Invalidate(*ev);
			}
			else if (ev->IsSelected()
			 && (gPrefs.inclusiveSelection ? !r.Intersects(extent)
			 							   : !r.Contains(extent)))
			{
				const_cast<CEvent *>(ev)->SetSelected(false);
				selectionChanged = true;
				renderer->Invalidate(*ev);
			}
		}

		if (selectionChanged)
		{
			Track()->SummarizeSelection();
		
			// Let the world know the selection has changed
			CEventSelectionUpdateHint hint(*Track(), true);
			Track()->PostUpdate(&hint, this);
		}
	}
}

void
CEventEditor::EndRectangleTracking()
{
	DrawSelectRect();
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

const CEvent *
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
	const CEvent *ev;

	// For each event that overlaps the current view, draw it.
	for (ev = marker.FirstItemInRange(startTime, stopTime);
		 ev != NULL;
		 ev = marker.NextItemInRange(startTime, stopTime))
	{
		long dist = RendererFor(*ev)->Pick(*ev, pickPt, partCode);
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
	const CEvent &ev,
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

	PushState();

	SetDrawingMode(B_OP_INVERT);
	StrokeRect(r, B_MIXED_COLORS);

	PopState();
}

void
CEventEditor::UpdatePBMarkers()
{
	int32 newPBMarkers[8], count;

	// Process playback markers
	count = CPlayerControl::GetPlaybackMarkerTimes(Track(),
												   newPBMarkers,
												   MAX_PLAYBACK_MARKERS);
	if (m_pbMarkers[0]<0)
	{
		m_pbMarkers[0]=0;
	}
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
CEventEditor::DrawCreateEcho(
	int32 startTime,
	int32 stopTime)
{
	EventOp	*echoOp = PendingOperation();
	if (echoOp == NULL)
		echoOp = m_dragOp;

	CEvent evCopy(m_newEv);

	if (echoOp)
		(*echoOp)(evCopy, Track()->ClockType());
	
	if ((evCopy.Start() <= stopTime) && (evCopy.Stop()  >= startTime))
	{
		PushState();
		RendererFor(evCopy)->Draw(evCopy, true);
		PopState();
	}
}

// ---------------------------------------------------------------------------
// CStripView Implementation

void
CEventEditor::KeyDown(
	const char *bytes,
	int32 numBytes)
{
	if ((bytes[0] == B_LEFT_ARROW) || (bytes[0] == B_RIGHT_ARROW))
	{
		if (Track()->SelectionType() != CTrack::Select_None)
		{
			int32 delta = Track()->GridSnapEnabled() ? Track()->TimeGridSize() - 1
													 : 1;
			EventOp *op;
			if (modifiers() & B_SHIFT_KEY)
			{
				op = new DurationOffsetOp((bytes[0] == B_RIGHT_ARROW) ? delta
																	  : -delta);
				Track()->ModifySelectedEvents(NULL, *op, "Change Duration",
											  EvAttr_Duration);
			}
			else
			{
				if (bytes[0] == B_LEFT_ARROW)
				{
					if (delta > Track()->MinSelectTime())
						return;
					delta = -delta;
				}
				op = new TimeOffsetOp(delta);
				Track()->ModifySelectedEvents(NULL, *op, "Move", EvAttr_None);
			}
			CRefCountObject::Release(op);
		}
	}
	else if ((bytes[0] == B_UP_ARROW) || (bytes[0] == B_DOWN_ARROW))
	{
		if (Track()->CurrentEvent() != NULL)
		{
			enum E_EventAttribute attr = EvAttr_None;
			int32 delta;
			
			if (bytes[0] == B_UP_ARROW)
				delta = -1;
			else
				delta = 1;
					
			switch (Track()->CurrentEvent()->Command())
			{
				case EvtType_Note:
				{
					attr = EvAttr_Pitch;
					delta = -delta;
					if (modifiers() & B_SHIFT_KEY)
						delta *= 12;
					break;
				}
				case EvtType_Repeat:
				case EvtType_Sequence:
				case EvtType_TimeSig:
				{
					attr = EvAttr_VPos;
					break;
				}
			}
			
			if (attr != EvAttr_None)
			{
				EventOp *op = CreateOffsetOp(attr, delta, 0);
				if (op)
				{
					Track()->ModifySelectedEvents(NULL, *op, "Modify Events",
												  attr);
					CRefCountObject::Release(op);

					if (gPrefs.FeedbackEnabled(attr, false)
					 &&	(Track()->SelectionCount() == 1))
					{
						CPlayerControl::DoAudioFeedback(TrackWindow()->Document(),
														attr,
														Track()->CurrentEvent()->GetAttribute(attr),
														Track()->CurrentEvent());
					}
				}
			}
		}
	}
	else if ((bytes[0] == B_DELETE) || (bytes[0] == B_BACKSPACE))
	{
		if (Track()->SelectionType() != CTrack::Select_None)
		{
			Track()->DeleteSelection();
// 			TrackWindow()->Document()->SetModified();
		}		
	}
}

void
CEventEditor::MessageReceived(
	BMessage *message)
{
	switch (message->what)
	{
		case CObservable::UPDATED:
		{
			SubjectUpdated(message);
			break;
		}
		default:
		{
			CStripView::MessageReceived(message);
		}
	}
}

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
	
	MakeFocus();
}

void
CEventEditor::MouseMoved(
	BPoint point,
	uint32 transit,
	const BMessage *message)
{
	D_HOOK(("CEventEditor::MouseMoved(%.2f, %.2f)\n", point.x, point.y));

	int32 editMode = TrackWindow()->CurrentTool();
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
		BRect r(Bounds());
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
	else if (Window()->IsActive() && (message == NULL))
	{
		if ((transit == B_ENTERED_VIEW) || (transit == B_INSIDE_VIEW))
		{
			EventMarker marker(Track()->Events());
			short partCode;
			const CEvent	*ev;
			if ((ev = PickEvent(marker, point, partCode)) != NULL)
				be_app->SetCursor(RendererFor(*ev)->Cursor(partCode, editMode));
			else
				be_app->SetCursor(CursorFor(editMode));
			TrackWindow()->SetHorizontalPositionInfo(Track(), ViewCoordsToTime(point.x));
		}
		else
		{
			be_app->SetCursor(CCursorCache::GetCursor(CCursorCache::DEFAULT));
			TrackWindow()->SetHorizontalPositionInfo(NULL, 0);
		}
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
	else
	{
		EventMarker marker(Track()->Events());
		const CEvent	*ev;
		int32 editMode = TrackWindow()->CurrentTool();
		short partCode;
		if ((ev = PickEvent(marker, point, partCode)) != NULL)
			be_app->SetCursor(RendererFor(*ev)->Cursor(partCode, editMode));
		else
			be_app->SetCursor(CCursorCache::GetCursor(CCursorCache::DEFAULT));
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
// CObserver Implementation

bool
CEventEditor::Released(
	CObservable *subject)
{
	bool released = false;

	if (LockLooper())
	{
		released = SubjectReleased(subject);
		UnlockLooper();
	}

	return released;
}

// ---------------------------------------------------------------------------
// Internal Operations

void
CEventEditor::_destinationAdded(
	BMessage *message)
{
	D_INTERNAL(("CEventEditor::_destinationAdded()\n"));

	// part has started to use this destination
	int32 destID;
	if (message->FindInt32("DestID", &destID) != B_OK)
		return;
	CDestination *dest = TrackWindow()->Document()->FindDestination(destID);
	if (dest)
	{
		D_INTERNAL((" -> start observing destination %s\n",
					dest->Name()));
		dest->AddObserver(this);
	}
}

void
CEventEditor::_destinationRemoved(
	BMessage *message)
{
	D_INTERNAL(("CEventEditor::_destinationAdded()\n"));

	// part has started to use this destination
	int32 destID;
	if (message->FindInt32("DestID", &destID) != B_OK)
		return;
	CDestination *dest = TrackWindow()->Document()->FindDestination(destID);
	if (dest)
	{
		D_INTERNAL((" -> stop observing destination %s\n",
					dest->Name()));
		dest->RemoveObserver(this);
	}
}

void
CEventEditor::_destinationUpdated(
	BMessage *message)
{
	int32 destAttrs = 0;
	if (message->FindInt32("DestAttrs", &destAttrs) == B_OK)
	{
		// something changed about a destination in use by the part
		if (destAttrs & (CDestination::Update_Color |
						 CDestination::Update_Flags))
		{
			int32 destID = -1;
			if (message->FindInt32("DestID", &destID) != B_OK)
				return;

			// calculate time range of the current visible rect
			int32 minTime = ViewCoordsToTime(Bounds().left);
			int32 maxTime = ViewCoordsToTime(Bounds().right);

			// destination look has changed -> invalidate every event using 
			// that destination
			CReadLock lock(Track());
			EventMarker	marker(Track()->Events());
			for (const CEvent *ev = marker.FirstItemInRange(minTime, maxTime);
				 ev;
				 ev = marker.NextItemInRange(minTime, maxTime))
			{
				if ((ev->HasProperty(CEvent::Prop_Channel))
				 && (ev->GetVChannel() == destID))
				{
					RendererFor(*ev)->Invalidate(*ev);
				}
			}
		}
		return;
	}
}

// END - EventEditor.cpp
