/* ===================================================================== *
 * ContinuousValueEditor.cpp (MeV/UI)
 * ===================================================================== */

#include "ContinuousValueEditor.h"

#include "EventTrack.h"
#include "Destination.h"
#include "MeVApp.h"
#include "MeVDoc.h"
#include "PlayerControl.h"
#include "ResourceUtils.h"

// ---------------------------------------------------------------------------
// Constructor

CContinuousValueEditor::CContinuousValueEditor(
	BLooper &looper,
	CStripFrameView	&frameView,
	BRect rect,
	const char *name)
	:	CEventEditor(looper, frameView, rect, name, true, true)
{
}

// ---------------------------------------------------------------------------
// General drawing routine

void
CContinuousValueEditor::Draw(
	BRect updateRect)
{
	long startTime = ViewCoordsToTime(updateRect.left - 1.0);
	long stopTime  = ViewCoordsToTime(updateRect.right + 1.0);
	SetHighColor(255, 255, 255);
	FillRect(updateRect);

	SetHighColor(220, 220, 220);

	FillRect(BRect(updateRect.left, stripLogicalHeight,
				   updateRect.right, stripLogicalHeight));

	DrawGridLines(updateRect);

	// Initialize an event marker for this track.
	StSubjectLock trackLock(*Track(), Lock_Shared);
	EventMarker marker(Track()->Events());

	bounds = Bounds();

	// For each event that overlaps the current view, draw it. (locked channels first)
	for (const Event *ev = marker.FirstItemInRange(startTime, stopTime);
		 ev;
		 ev = marker.NextItemInRange(startTime, stopTime))
	{
		if (ev->HasProperty(Event::Prop_Channel)
		 && Track()->IsChannelLocked(ev->GetVChannel()))
			HandlerFor(*ev)->Draw(*ev, false);
	}

	// For each event that overlaps the current view, draw it. (unlocked channels overdraw!)
	for (const Event *ev = marker.FirstItemInRange(startTime, stopTime);
		 ev;
		 ev = marker.NextItemInRange(startTime, stopTime))
	{
		if (!Track()->IsChannelLocked(ev->GetVChannel()
		 || !ev->HasProperty(Event::Prop_Channel)))
			HandlerFor(*ev)->Draw(*ev, false);
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

void
CContinuousValueEditor::Pulse()
{
	UpdatePBMarkers();
	// REM: Add code to process newly recorded events
	// REM: Add code to edit events via MIDI.
}

// ---------------------------------------------------------------------------
// Update message from another observer

void
CContinuousValueEditor::OnUpdate(
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

// ---------------------------------------------------------------------------
// Calculate factors relating to zoom

void
CContinuousValueEditor::CalcZoom()
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

void
CContinuousValueEditor::MouseMoved(
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
	bounds = Bounds();
}

// END - ContinuousValueEditor.cpp
