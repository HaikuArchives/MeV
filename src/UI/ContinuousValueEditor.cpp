/* ===================================================================== *
 * ContinuousValueEditor.cpp (MeV/UI)
 * ===================================================================== */

#include "ContinuousValueEditor.h"

#include "EventRenderer.h"
#include "EventTrack.h"
#include "Destination.h"
#include "MeVApp.h"
#include "MeVDoc.h"
#include "PlayerControl.h"
#include "ResourceUtils.h"

// ---------------------------------------------------------------------------
// Constructor

CContinuousValueEditor::CContinuousValueEditor(
	CStripFrameView	&frameView,
	BRect rect,
	const char *name)
	:	CEventEditor(frameView, rect, name, true, true)
{
}

// ---------------------------------------------------------------------------
// Operations

void
CContinuousValueEditor::CalcZoom()
{
	stripLogicalHeight = (long)(16.0 * pow( 1.4, verticalZoom ));
	pixelsPerValue = (double)stripLogicalHeight / (maxValue - minValue);
	stripLogicalHeight += eventAscent + eventDescent;
}

// ---------------------------------------------------------------------------
// CEventEditor Implementation

void
CContinuousValueEditor::AttachedToWindow()
{
	BRect r(Frame());

	SetViewColor(B_TRANSPARENT_32_BIT);
	SetScrollRange(scrollRange.x, scrollValue.x,
				   stripLogicalHeight, 
				   (stripLogicalHeight - r.Height() / 2) / 2 );
}

void
CContinuousValueEditor::Draw(
	BRect updateRect)
{
	long startTime = ViewCoordsToTime(updateRect.left - 1.0);
	long stopTime  = ViewCoordsToTime(updateRect.right + 1.0);
	SetHighColor(255, 255, 255);
	FillRect(updateRect);

	if (updateRect.bottom >= stripLogicalHeight)
	{
		SetHighColor(220, 220, 220);
		FillRect(BRect(updateRect.left, stripLogicalHeight,
					   updateRect.right, stripLogicalHeight),
				 B_MIXED_COLORS);
	}

	updateRect.bottom = stripLogicalHeight;
	DrawGridLines(updateRect);
	DrawHorizontalGrid(updateRect);

	// Initialize an event marker for this track.
	StSubjectLock trackLock(*Track(), Lock_Shared);
	EventMarker marker(Track()->Events());

	// For each event that overlaps the current view, draw it.
	for (const Event *ev = marker.FirstItemInRange(startTime, stopTime);
		 ev != NULL;
		 ev = marker.NextItemInRange(startTime, stopTime))
	{
		if (ev->Command() == EvtType_PitchBend)
			RendererFor(*ev)->Draw(*ev, false);
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

void
CContinuousValueEditor::Pulse()
{
	UpdatePBMarkers();
	// REM: Add code to process newly recorded events
	// REM: Add code to edit events via MIDI.
}

void
CContinuousValueEditor::SetScrollValue(
	float value,
	orientation posture)
{
	CStripView::SetScrollValue(value, posture);

	LabelView()->ScrollTo(0.0, scrollValue.y);
}

// END - ContinuousValueEditor.cpp
