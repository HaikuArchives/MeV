/* ===================================================================== *
 * VelocityEditor.cpp (MeV/UI)
 * ===================================================================== */

#include "VelocityEditor.h"

#include "CursorCache.h"
#include "EventRenderer.h"
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
// Note renderer class for linear editor

class CVelocityNoteEventRenderer
	:	public CEventRenderer
{

public:							// No constructor

								CVelocityNoteEventRenderer(
									CEventEditor * const editor)
									:	CEventRenderer(editor)
								{ }

public:							// CEventRenderer Implementation

	// Invalidate the event
	void						Invalidate(
									const CEvent &ev) const ;

	// Draw the event (or an echo)
	void						Draw(
									const CEvent &ev,
									bool shadowed) const;

	// Invalidate the event
	BRect						Extent(
									const CEvent &ev) const;

	// Pick a single event and returns the distance.
	long						Pick(
									const CEvent &ev,
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
									const CEvent &ev,
									short partCode,
									BPoint clickPos,
									BPoint dragPos) const
								{ return 0; }

	// Make a drag op for dragging notes...
	EventOp *					CreateDragOp(
									const CEvent &ev,
									short partCode,
									long timeDelta,
									long valueDelta) const
								{ return NULL; }

	EventOp *					CreateTimeOp(
									const CEvent &ev,
									short partCode,
									long timeDelta,
									long valueDelta) const
								{ return NULL; }

protected:						// Accessors

	CVelocityEditor * const		Editor() const
								{ return (CVelocityEditor *)CEventRenderer::Editor(); }
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
	CStripFrameView &frameView,
	BRect rect)
	:	CEventEditor(frameView, rect, "Velocity", false, false),
		m_coupleAttackRelease(true)
{
	SetRendererFor(EvtType_Note, new CVelocityNoteEventRenderer(this));
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
	SetHighColor(255, 255, 255);
	FillRect(updateRect);

	SetHighColor(220, 220, 220);
	DrawGridLines(updateRect);

	// Initialize an event marker for this track.
	CReadLock lock(Track());
	EventMarker marker(Track()->Events());

	// For each event that overlaps the current view, draw it.
	long startTime = ViewCoordsToTime(updateRect.left - 1.0);
	long stopTime = ViewCoordsToTime(updateRect.right + 1.0);
	for (const CEvent *ev = marker.FirstItemInRange(startTime, stopTime);
		 ev;
		 ev = marker.NextItemInRange(startTime, stopTime))
	{
		// Only draw events for non-locked channels...
		if (ev->Command() != EvtType_Note)
			continue;

		RendererFor(*ev)->Draw(*ev, false);
	}

	DrawPlaybackMarkers(m_pbMarkers, m_pbCount, updateRect, false);
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
		default:
		{
			CEventEditor::MessageReceived(message);
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

	if (Window()->IsActive())
	{
		if (transit == B_EXITED_VIEW)
		{
			TrackWindow()->SetVerticalPositionInfo("");
		}
		else
		{
			int8 velocity = static_cast<int8>(127 * ((Bounds().bottom - point.y)
												     / Bounds().Height()));
			BString text;
			text << velocity;
			TrackWindow()->SetVerticalPositionInfo(text);
		}
	}
}

void
CVelocityEditor::Pulse()
{
	UpdatePBMarkers();
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
	if (m_dragType == DragType_Sculpt)
	{
		CWriteLock lock(Track());
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
					Track()->PostUpdate(&hint, this);
				}
				m_smallestTime = LONG_MAX;
				m_largestTime = LONG_MIN;
			}

			m_dragAction = new EventListUndoAction(Track()->Events(), *Track(),
												   "Change Velocity");
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
		for (const CEvent *ev = marker.FirstItemInRange(time1, time2);
			 ev;
			 ev = marker.NextItemInRange(time1, time2))
		{
			if (ev->Command() == EvtType_Note)
			{
				CEvent evCopy(*ev);
				bool eventSaved = false;

				if (m_coupleAttackRelease)
				{
					long startTime = ev->Start();
					if ((startTime >= time1) && (startTime < time2))
					{
						short newVelocity = CLAMP(1L, vel, 127L);
						evCopy.note.attackVelocity = newVelocity;
						evCopy.note.releaseVelocity = newVelocity;
					}
				}
				else
				{
					// Modify the note based on the button states.	
					if (buttons & B_PRIMARY_MOUSE_BUTTON)
					{
						long startTime = ev->Start();
						if ((startTime >= time1) && (startTime < time2))
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
					RendererFor(*ev)->Invalidate(*ev);

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
						const_cast<CEvent *>(ev)->note.attackVelocity = evCopy.note.attackVelocity;
						const_cast<CEvent *>(ev)->note.releaseVelocity = evCopy.note.releaseVelocity;
					}

					RendererFor(*ev)->Invalidate(*ev);
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
		Track()->PostUpdate(&hint, this);
	}

	TrackWindow()->SetHorizontalPositionInfo(NULL, 0);
}

// ---------------------------------------------------------------------------
// CVelocityNoteEventRenderer: CEventRenderer Implementation

void
CVelocityNoteEventRenderer::Invalidate(
	const CEvent &ev) const
{
	Editor()->Invalidate(Extent(ev));
}

void
CVelocityNoteEventRenderer::Draw(
	const CEvent &ev,
	bool shadowed) const
{
	CDestination	*dest = Editor()->TrackWindow()->Document()->FindDestination(ev.GetVChannel());

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
		Editor()->SetHighColor(dest->Color());
		Editor()->SetDrawingMode(B_OP_BLEND);
		Editor()->FillRect(rect);
	}
	else
	{
		Editor()->SetHighColor(dest->Color());
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
CVelocityNoteEventRenderer::Extent(
	const CEvent &ev) const
{
	BRect rect(Editor()->Bounds());
	rect.left = Editor()->TimeToViewCoords(ev.Start());
	rect.right = Editor()->TimeToViewCoords(ev.Stop());
	rect.top = rect.bottom - (MAX(ev.note.attackVelocity,
								  ev.note.releaseVelocity) * rect.Height()) / 128;

	return rect;
}

const BCursor *
CVelocityNoteEventRenderer::Cursor(
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
