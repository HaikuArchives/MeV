/* ===================================================================== *
 * VelocityEditor.cpp (MeV/UI)
 * ===================================================================== */

#include "VelocityEditor.h"

#include "CursorCache.h"
#include "EventTrack.h"
#include "Destination.h"
#include "MeVDoc.h"
#include "MathUtils.h"
#include "ResourceUtils.h"

#include "StripLabelView.h"

// Interface Kit
#include <MenuItem.h>
#include <PopUpMenu.h>
// Gnu C Library
#include <stdio.h>
// Support Kit
#include <Debug.h>

// ---------------------------------------------------------------------------
// Note handler class for linear editor

class CVelocityNoteEventHandler
	:	public CEventHandler
{

public:							// No constructor

								CVelocityNoteEventHandler(
									CEventEditor * const editor)
									:	CEventHandler(editor)
								{ }

public:							// CEventHandler Implementation

	// Invalidate the event
	void						Invalidate(
									const Event &ev) const ;

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
									short &partCode) const
								{ return 0; }

	// For a part code returned earlier, return a cursor
	// image...
	const BCursor *				Cursor(
									short partCode,
									int32 editMode = CEventEditor::TOOL_SELECT,
									bool dragging = false) const;

	// Quantize the vertical position of the mouse based
	// on the event type and return a value delta.
	long						QuantizeDragValue(
									const Event &ev,
									short partCode,
									BPoint clickPos,
									BPoint dragPos) const
								{ return 0; }

	// Make a drag op for dragging notes...
	EventOp *					CreateDragOp(
									const Event &ev,
									short partCode,
									long timeDelta,
									long valueDelta) const
								{ return NULL; }

	EventOp *					CreateTimeOp(
									const Event &ev,
									short partCode,
									long timeDelta,
									long valueDelta) const
								{ return NULL; }

protected:						// Accessors

	CVelocityEditor * const		Editor() const
								{ return (CVelocityEditor *)CEventHandler::Editor(); }
};

// ---------------------------------------------------------------------------
// Label View for the Velocity Strip

class CVelocityStripLabelView
	:	public CStripLabelView
{

public:							// No constructor

								CVelocityStripLabelView(
									BRect rect);

public:							// CStripLabelView Implementation

	virtual void				InitContextMenu();

	virtual void				ShowContextMenu(
									BPoint point);
};

// ---------------------------------------------------------------------------
// Constructor/Destructor

CVelocityEditor::CVelocityEditor(
	BLooper &looper,
	CStripFrameView &frameView,
	BRect rect)
	:	CEventEditor(looper, frameView, rect,
					 "Velocity", false, false),
		m_coupleAttackRelease(true)
{
	SetHandlerFor(EvtType_Note, new CVelocityNoteEventHandler(this));
	SetFlags(Flags() | B_FULL_UPDATE_ON_RESIZE);

	// Make the label view on the left-hand side
	SetLabelView(new CVelocityStripLabelView(BRect(-1.0, -1.0, 20.0,
												   rect.Height() + 1)));
}

// ---------------------------------------------------------------------------
// CEventEditor Implementation

const BCursor *
CVelocityEditor::CursorFor(
	int32 editMode) const
{
	// cannot create or delete events in this strip, only modify
	return CCursorCache::GetCursor(CCursorCache::DEFAULT);
}

void
CVelocityEditor::Draw(
	BRect updateRect)
{
	bounds = Bounds();

	SetHighColor(255, 255, 255);
	FillRect(updateRect);

	SetHighColor(220, 220, 220);
	DrawGridLines(updateRect);

	// Initialize an event marker for this track.
	StSubjectLock trackLock(*Track(), Lock_Shared);
	EventMarker marker(Track()->Events());

	// For each event that overlaps the current view, draw it.
	long startTime = ViewCoordsToTime(updateRect.left - 1.0);
	long stopTime = ViewCoordsToTime(updateRect.right + 1.0);
	for (const Event *ev = marker.FirstItemInRange(startTime, stopTime);
		 ev;
		 ev = marker.NextItemInRange(startTime, stopTime))
	{
		// Only draw events for non-locked channels...
		if ((ev->Command() != EvtType_Note)
		 || (Track()->IsChannelLocked(*ev)))
			continue;

		HandlerFor(*ev)->Draw(*ev, false);
	}
}

void
CVelocityEditor::OnUpdate(
	BMessage *message)
{
	BRect r(Bounds());
	bounds = r;

	bool selChange = false;
	if (message->FindBool("SelChange", 0, &selChange) == B_OK)
	{
		if (!IsSelectionVisible())
			return;
	}

	int32 trackHint = 0;
	if (message->FindInt32("TrackAttrs", 0, &trackHint) == B_OK)
	{
		if (!(trackHint & (CTrack::Update_Duration | CTrack::Update_SigMap |
						   CTrack::Update_TempoMap)))
			return;
	}

	int32 minTime;
	if (message->FindInt32("MinTime", 0, &minTime) != B_OK)
		minTime = ViewCoordsToTime(Bounds().left);
	r.left = TimeToViewCoords(minTime) - 1.0;

	int32 maxTime;
	if (message->FindInt32("MaxTime", 0, &maxTime) != B_OK)
		maxTime = ViewCoordsToTime(Bounds().right);
	r.right = TimeToViewCoords(maxTime) + 1.0;

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
		for (const Event *ev = marker.FirstItemInRange(minTime, maxTime);
			 ev;
			 ev = marker.NextItemInRange(minTime, maxTime))
		{
			if ((ev->HasProperty(Event::Prop_Channel))
			 && (ev->GetVChannel() == channel))
			{
				HandlerFor(*ev)->Invalidate(*ev);
			}
		}
	}
	else if (selChange)
	{
		StSubjectLock trackLock(*Track(), Lock_Shared);
		EventMarker marker(Track()->Events());

		// For each event that overlaps the current view, draw it.
		for (const Event *ev = marker.FirstItemInRange(minTime, maxTime);
			 ev;
			 ev = marker.NextItemInRange(minTime, maxTime))
		{
			HandlerFor(*ev)->Invalidate(*ev);
		}
	}
	else
	{
		Invalidate(r);
	}
}

void
CVelocityEditor::MessageReceived(
	BMessage *message)
{
	switch (message->what)
	{
		case COUPLE_ATTACK_RELEASE:
		{	
			m_coupleAttackRelease = !m_coupleAttackRelease;
			break;
		}
		case Update_ID:
		case Delete_ID:
		{
			CObserver::MessageReceived(message);
			break;
		}
		default:
		{
			CStripView::MessageReceived(message);
		}
	}
}

void
CVelocityEditor::MouseMoved(
	BPoint point,
	uint32 transit,
	const BMessage *message)
{
	CEventEditor::MouseMoved(point, transit, message);

	if (transit == B_EXITED_VIEW)
	{
		TrackWindow()->SetHorizontalPositionInfo(NULL, 0);
		TrackWindow()->SetVerticalPositionInfo("");
		return;
	}

	TrackWindow()->SetHorizontalPositionInfo(Track(),
											 ViewCoordsToTime(point.x));
	int8 velocity = static_cast<int8>(127 * ((Bounds().bottom - point.y)
										     / Bounds().Height()));
	BString text;
	text << velocity;
	TrackWindow()->SetVerticalPositionInfo(text);
}

void
CVelocityEditor::StartDrag(
	BPoint point,
	ulong buttons)
{
	BRect r(Bounds());

	m_dragTime = ViewCoordsToTime(point.x);
	m_dragVelocity = 127 * static_cast<int32>((r.bottom - point.y)
											  / r.Height());
	m_dragType = DragType_Sculpt;
	m_dragAction = NULL;
	m_smallestTime = LONG_MAX;
	m_largestTime = LONG_MIN;
	TrackWindow()->SetHorizontalPositionInfo(Track(), m_dragTime);
}

bool
CVelocityEditor::DoDrag(
	BPoint point,
	ulong buttons)
{
	bounds = Bounds();

	if (m_dragType == DragType_Sculpt)
	{
		StSubjectLock trackLock(*Track(), Lock_Exclusive);
		BRect r(Bounds());

		// If no undo record has been created yet, or someone did another
		// undoable action in-between calls to DoDrag, then start a new
		// undo action.
		if ((m_dragAction == NULL)
		 ||	!Track()->IsMostRecentUndoAction(m_dragAction))
		{
			// If there was another action since the last call to
			// DoDrag, we'll need to start the undo information fresh,
			// so post our update message and initialize time range.
			if (m_dragAction != NULL)
			{
				// If there were in fact ANY events saved, then
				// go ahead and do the post.
				if (m_largestTime >= m_smallestTime)
				{
					CUpdateHint hint;
					hint.AddInt32("MinTime", m_smallestTime);
					hint.AddInt32("MaxTime", m_largestTime);
					PostUpdate(&hint, true);
				}
				m_smallestTime = LONG_MAX;
				m_largestTime = LONG_MIN;
			}

			m_dragAction = new EventListUndoAction(Track()->Events(), *Track(),
												   "Edit Velocity");
			Track()->AddUndoAction(m_dragAction);
		}

		// Compute the time and velocity coords of the mouse.
		short vel = static_cast<short>(127 * (r.bottom - point.y) / r.Height());
		short vel1, vel2;
		long time = ViewCoordsToTime(point.x);
		long time1, time2;

		// Sort the times. Velocitys are associated with the times.
		if (time < m_dragTime)
		{
			time1 = time;
			time2 = m_dragTime;
			vel1 = vel;
			vel2 = m_dragVelocity;
		}
		else
		{
			time1 = m_dragTime;
			time2 = time;
			vel1 = m_dragVelocity;
			vel2 = vel;
		}
		
		// Compute the delta values for slope
		long timeDelta = time2 - time1;
		short velocityDelta = vel2 - vel1;

		// For each event that overlaps the current view, draw it.
		EventMarker marker(Track()->Events());
		for (const Event *ev = marker.FirstItemInRange(time1, time2);
			 ev;
			 ev = marker.NextItemInRange(time1, time2))
		{
			if (Track()->IsChannelLocked(*ev))
				continue;

			if (ev->Command() == EvtType_Note)
			{
				Event evCopy(*ev);
				bool eventSaved = false;

				if (m_coupleAttackRelease)
				{
					evCopy.note.attackVelocity = CLAMP(1L, vel, 127L);
					evCopy.note.releaseVelocity = CLAMP(1L, vel, 127L);
				}
				else
				{
					// Modify the note based on the button states.	
					if (buttons & B_PRIMARY_MOUSE_BUTTON)
					{
						long startTime = ev->Start();
						if ((startTime >= time1) && startTime < time2)
						{
							short newVelocity = (startTime - time1)
												* velocityDelta / timeDelta
												+ vel1;
							evCopy.note.attackVelocity = CLAMP(1L, newVelocity,
															   127L);
						}
					}
	
					if (buttons & B_SECONDARY_MOUSE_BUTTON)
					{
						long stopTime = ev->Stop();
						if ((stopTime >= time1) && (stopTime < time2))
						{
							int32 newVelocity = (stopTime - time1)
												* velocityDelta / timeDelta
												+ vel1;
							evCopy.note.releaseVelocity = CLAMP(0L, newVelocity,
																127L);
							
						}
					}
				}				

				// If the event overlaps the range of time between
				// smallest <--> largest then it has already been
				// stored in the undo repository.
				if ((ev->Start() <= m_largestTime)
				 && (ev->Stop() >= m_smallestTime))
					eventSaved = true;
				
				if ((ev->note.attackVelocity != evCopy.note.attackVelocity)
				 ||	(ev->note.releaseVelocity != evCopy.note.releaseVelocity))
				{
					HandlerFor(*ev)->Invalidate(*ev);

					// If the event has not yet been saved, then
					// go ahead and add it into the undo store
					// and replace the event.
					if (!eventSaved)
					{
						marker.Replace(&evCopy, 1, m_dragAction);
					}
					else
					{
							// If the event has already been added into the undo
							// store (which we can assume because we add ALL events
							// within the time range into the store, regardless
							// of wether they are modified or not), then we don't
							// want to take up memory by adding a redundant copy,
							// so just poke the event directly.
						const_cast<Event *>(ev)->note.attackVelocity = evCopy.note.attackVelocity;
						const_cast<Event *>(ev)->note.releaseVelocity = evCopy.note.releaseVelocity;
					}

					HandlerFor(*ev)->Invalidate(*ev);
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
						marker.Replace(&evCopy, 1, m_dragAction);
				}
			}
		}

		m_dragTime = time;
		m_dragVelocity = vel;

		m_smallestTime = MIN(m_smallestTime, m_dragTime);
		m_largestTime = MAX(m_largestTime, m_dragTime);

		TrackWindow()->SetHorizontalPositionInfo(Track(), m_dragTime);
	}

	return true;
}

void
CVelocityEditor::FinishDrag(
	BPoint point,
	ulong buttons,
	bool commit )
{
	Track()->SummarizeSelection();

	if (m_largestTime >= m_smallestTime)
	{
		CUpdateHint hint;
		hint.AddInt32("MinTime", m_smallestTime);
		hint.AddInt32("MaxTime", m_largestTime);
		PostUpdate(&hint, true);
	}

	TrackWindow()->SetHorizontalPositionInfo(NULL, 0);
}

// ---------------------------------------------------------------------------
// CVelocityNoteEventHandler: CEventHandler Implementation

void
CVelocityNoteEventHandler::Invalidate(
	const Event &ev) const
{
	Editor()->Invalidate(Extent(ev));
}

void
CVelocityNoteEventHandler::Draw(
	const Event &ev,
	bool shadowed) const
{
	Destination	*dest = Editor()->TrackWindow()->Document()->GetVChannel(ev.GetVChannel());

	BRect rect(Editor()->Bounds());
	rect.left = Editor()->TimeToViewCoords(ev.Start());
	rect.right = Editor()->TimeToViewCoords(ev.Stop());

	BPoint points[4] = { BPoint(0.0, 0.0),
						 BPoint(0.0, 0.0),
						 BPoint(0.0, 0.0),
						 BPoint(0.0, 0.0) };
	points[0].x = points[1].x = rect.left;
	points[2].x = points[3].x = rect.right;
	points[0].y = points[3].y = rect.bottom + 1.0;
	points[1].y = rect.bottom - (long)(ev.note.attackVelocity * rect.Height())
								/ 128;
	points[2].y = rect.bottom - (long)(ev.note.releaseVelocity * rect.Height())
								/ 128;
	rect.top = MIN(points[1].y, points[2].y);

	if (points[1].y == points[2].y)
	{
		// Rects fill faster, so let's use that if we can.
		if (ev.IsSelected() && Editor()->IsSelectionVisible())
			Editor()->SetHighColor(0, 0, 255);
		else
			Editor()->SetHighColor(0, 0, 0);
		
		Editor()->StrokeRect(rect);
		rect.InsetBy(1.0, 1.0);
		Editor()->SetHighColor(dest->fillColor);
		Editor()->SetDrawingMode(B_OP_BLEND);
		Editor()->FillRect(rect);
	}
	else
	{
		Editor()->SetHighColor(dest->fillColor);
		Editor()->SetDrawingMode(B_OP_BLEND);
		Editor()->FillPolygon(points, 4, rect);

		Editor()->SetDrawingMode(B_OP_COPY);
		if (ev.IsSelected() && Editor()->IsSelectionVisible())
			Editor()->SetHighColor(0, 0, 255);
		else
			Editor()->SetHighColor(0, 0, 0);
		Editor()->StrokePolygon(points, 4, rect, false);
	}
	Editor()->SetDrawingMode(B_OP_COPY);
}

BRect
CVelocityNoteEventHandler::Extent(
	const Event &ev) const
{
	BRect rect(Editor()->Bounds());
	rect.left = Editor()->TimeToViewCoords(ev.Start());
	rect.right = Editor()->TimeToViewCoords(ev.Stop());
	rect.top = rect.bottom - (MAX(ev.note.attackVelocity,
								  ev.note.releaseVelocity) * rect.Height()) / 128;

	return rect;
}

const BCursor *
CVelocityNoteEventHandler::Cursor(
	short partCode,
	int32 editMode,
	bool dragging) const
{
	return CCursorCache::GetCursor(CCursorCache::PENCIL);
}

// ---------------------------------------------------------------------------
// CVelocityStripLabelView: Constructor/Destructor

CVelocityStripLabelView::CVelocityStripLabelView(
	BRect rect)
	:	CStripLabelView(rect, "Velocity", B_FOLLOW_TOP_BOTTOM,
						B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE)
{
}

// ---------------------------------------------------------------------------
// CVelocityStripLabelView: CStripLabelView Implementation

void
CVelocityStripLabelView::InitContextMenu()
{
	CStripLabelView::InitContextMenu();

	ContextMenu()->AddSeparatorItem();
	ContextMenu()->AddItem(new BMenuItem("Couple Attack / Release",
										 new BMessage(CVelocityEditor::COUPLE_ATTACK_RELEASE)));
}

void
CVelocityStripLabelView::ShowContextMenu(
	BPoint point)
{
	BMenuItem *item = ContextMenu()->FindItem(CVelocityEditor::COUPLE_ATTACK_RELEASE);
	if (item)
		item->SetMarked(((CVelocityEditor *)StripView())->m_coupleAttackRelease);

	CStripLabelView::ShowContextMenu(point);
}

// END - VelocityEditor.cpp
