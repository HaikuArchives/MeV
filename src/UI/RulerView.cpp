/* ===================================================================== *
 * RulerView.cpp (MeV/UI)
 * ===================================================================== */

#include "RulerView.h"

#include "CursorCache.h"
#include "DataSnap.h"
#include "EventTrack.h"
#include "ResourceUtils.h"
#include "StripFrameView.h"

// Interface Kit
#include <Bitmap.h>
// Support Kit
#include <Debug.h>

// Debugging Macros
#define D_ALLOC(x) //PRINT(x)		// Constructor/Destructor
#define D_HOOK(x) //PRINT(x)		// CScrollerTarget/CObserver Implementation
#define D_OPERATION(x) //PRINT(x)	// Operations
#define D_INTERNAL(x) //PRINT(x)	// Internal Methods

// ---------------------------------------------------------------------------
// Constructor/Destructor

CRulerView::CRulerView(
	BRect frame,
	const char *name,
	BLooper &looper,
	CStripFrameView *frameView,
	CEventTrack *track,
	ulong resizingMode,
	ulong flags)
	:	CScrollerTarget(frame, name, resizingMode, flags),
		CObserver(track),
		m_track(track),
		m_frameView(frameView),
		m_showMarkers(true)
{
	D_ALLOC(("CRulerView::CRulerView()\n"));

	m_frameView->SetRuler(this);
	m_leftMarker = ResourceUtils::LoadImage("LeftSectionMarker");
	m_rightMarker = ResourceUtils::LoadImage("RightSectionMarker");
	m_markerWidth = m_leftMarker->Bounds().Width();
	SetViewColor(B_TRANSPARENT_COLOR);
}

CRulerView::~CRulerView()
{
	D_ALLOC(("CRulerView::~CRulerView()\n"));

	if (m_track != NULL)
		m_track->RemoveObserver(this);

	delete m_leftMarker;
	delete m_rightMarker;
}

// ---------------------------------------------------------------------------
// Operations

void
CRulerView::ShowMarkers(
	bool show)
{
	D_OPERATION(("CRulerView::ShowMarkers()\n"));

	if (show != m_showMarkers)
	{
		m_showMarkers = show;
		Invalidate();
	}
}

// ---------------------------------------------------------------------------
// CScrollerTarget Implementation

void
CRulerView::Draw(
	BRect updateRect)
{
	TClockType clockType = m_track->ClockType();
	long startTime = m_frameView->ViewCoordsToTime(updateRect.left - 48.0,
												   clockType );
	if (startTime < 0)
		startTime = 0;

	BPoint leftMarkerOffset(0.0, 0.0);
	BPoint rightMarkerOffset(0.0, 0.0);
	if (m_showMarkers)
	{
		leftMarkerOffset.x = m_frameView->TimeToViewCoords(m_track->SectionStart(),
												 		   clockType);
		rightMarkerOffset.x = m_frameView->TimeToViewCoords(m_track->SectionEnd(),
												 			clockType);
	}

	CSignatureMap::Iterator timeIter(m_track->SigMap(), startTime);
	long time;
	BRect rect(Bounds());
	bool major;
	int32 majorTime = m_track->SigMap().entries->sigMajorUnitDur;
	double majorXStep = m_frameView->TimeToViewCoords(majorTime, clockType);
	int32 steps = 1;

	while (majorXStep < 24)
	{
		if (steps == 2)
			steps = 5;
		else
			steps *= 2;
		majorXStep = m_frameView->TimeToViewCoords(majorTime * steps,
												   clockType);
	}

	SetDrawingMode(B_OP_COPY);
	SetLowColor(255, 255, 220, 255);
	if (m_showMarkers)
	{
		if (updateRect.left < leftMarkerOffset.x)
		{
			FillRect(BRect(updateRect.left, rect.top, leftMarkerOffset.x,
						   rect.bottom - 1.0), B_SOLID_LOW);
		}
		if (updateRect.right > rightMarkerOffset.x)
		{
			FillRect(BRect(rightMarkerOffset.x, rect.top, updateRect.right,
						   rect.bottom - 1.0), B_SOLID_LOW);
		}
		SetLowColor(tint_color(LowColor(), B_DARKEN_1_TINT));
		FillRect(BRect(leftMarkerOffset.x, rect.top, rightMarkerOffset.x,
					   rect.bottom - 1.0), B_SOLID_LOW);
	}
	else
	{
		FillRect(BRect(updateRect.left, rect.top, updateRect.right,
					   rect.bottom - 1.0), B_SOLID_LOW);
	}

	for (time = timeIter.First(major); ; time = timeIter.Next(major))
	{
		double x = m_frameView->TimeToViewCoords( time, clockType );
		if (x > updateRect.right)
			break;

		if (major)
		{
			if (timeIter.MajorCount() % steps)
				continue;
			SetHighColor(160, 160, 140, 255);
		}
		else if (steps > 1)
		{
			continue;
		}
		else
		{
			SetHighColor(210, 210, 180, 255);
		}

		if (x > 0.0)
		{
			if ((x > leftMarkerOffset.x) && (x < rightMarkerOffset.x))
				SetHighColor(tint_color(HighColor(), B_DARKEN_1_TINT));
			StrokeLine(BPoint(x, updateRect.top),
					   BPoint(x, updateRect.bottom));
		}
	}

	CSignatureMap::Iterator timeIter2(m_track->SigMap(), startTime);
	SetHighColor(0, 0, 0, 255);
	SetDrawingMode(B_OP_OVER);

	for (time = timeIter2.First(major); ; time = timeIter2.Next(major))
	{
		double x = m_frameView->TimeToViewCoords(time, clockType);
		if (x > updateRect.right)
			break;

		if (major && !(timeIter2.MajorCount() % steps))
		{
			char str[16];
			if (m_track->ClockType() == ClockType_Metered)
				sprintf(str, "%02ld", timeIter2.MajorCount() + 1);
			else
				sprintf(str, "%02ld:00", timeIter2.MajorCount());

			DrawString(str, BPoint(x + 4, 9));
		}
	}

	SetHighColor(0, 0, 0, 255);
	StrokeLine(BPoint(updateRect.left, rect.bottom),
			   BPoint(updateRect.right, rect.bottom),
			   B_SOLID_HIGH);

	if (m_showMarkers)
	{
		// Now, draw the track section markers...
		SetDrawingMode(B_OP_OVER);
		DrawBitmapAsync(m_leftMarker, leftMarkerOffset);
		DrawBitmapAsync(m_rightMarker,
						rightMarkerOffset - BPoint(m_markerWidth, 0.0));
	}
}

void
CRulerView::MessageReceived(
	BMessage *message)
{
	switch (message->what)
	{
		case CObservable::UPDATED:
		{
			int32 trackHint;
			// Only ONE change we are interested in, and that's section markers...
			if (message->FindInt32("TrackAttrs", 0, &trackHint) == B_OK)
			{
				if (trackHint & CTrack::Update_Section)
				{
					BRect r(Bounds());
					int32 minTime;
					if (message->FindInt32("MinTime", 0, &minTime) != B_OK)
						minTime = m_frameView->ViewCoordsToTime(Bounds().left,
																m_track->ClockType());
					r.left = m_frameView->TimeToViewCoords(minTime,
														   m_track->ClockType());
					int32 maxTime;
					if (message->FindInt32("MaxTime", 0, &maxTime) != B_OK)
						maxTime = m_frameView->ViewCoordsToTime(Bounds().right,
																m_track->ClockType());
					r.right = m_frameView->TimeToViewCoords(maxTime,
															m_track->ClockType());
					r.InsetBy(-m_markerWidth, 0.0);
					Invalidate(r);
				}
				else if (trackHint & (CTrack::Update_SigMap | CTrack::Update_TempoMap))
				{
					Invalidate();
				}
			}
		}
	}
}

void
CRulerView::MouseDown(
	BPoint point)
{
	if (!m_showMarkers)
	{
		// won't manipulate markers that aren't visible
		return;
	}

	int32 buttons;
	Window()->CurrentMessage()->FindInt32("buttons", &buttons);
	if (buttons == B_PRIMARY_MOUSE_BUTTON)
	{
		// move markers with primary (left) mouse button
		SetMouseEventMask(B_POINTER_EVENTS, B_NO_POINTER_HISTORY | B_LOCK_WINDOW_FOCUS);

		// Check if we hit a marker...
		int32 marker = MarkerAt(point);
		if (marker >= 0)
		{
			BMessage message(MARKER_MOVED);
			message.AddInt32("which", marker);
			DragMessage(&message, BRect(0.0, 0.0, -1.0, -1.0), this);
		}
	}
}
	
void
CRulerView::MouseMoved(
	BPoint point,
	uint32 transit,
	const BMessage *message)
{
	D_HOOK(("CRulerView::MouseMoved()\n"));

	if (message)
	{
		switch (message->what)
		{
			case MARKER_MOVED:
			{
				D_HOOK((" -> MARKER_MOVED\n"));

				int32 which;
				if (message->FindInt32("which", &which) != B_OK)
					return;

				StSubjectLock lock(*m_track, Lock_Exclusive);
				int32 time = m_frameView->ViewCoordsToTime(point.x, m_track->ClockType());
				if (m_track->GridSnapEnabled())
				{
					int32 majorUnit, extraTime;
					m_track->SigMap().DecomposeTime(time, majorUnit, extraTime);
					time += DataSnapNearest(extraTime, 0, m_track->TimeGridSize()) - extraTime;
				}
				
				if (time < 0)
					time = 0;

				int32 markers[] = { m_track->SectionStart(), m_track->SectionEnd() };
				if (time != markers[which])
				{
					markers[which] = time;
					m_track->SetSection(markers[0], markers[1]);
				}

				// Implement auto-scrolling (horizontal for frame)
				BRect r(Bounds());
				if (point.x > r.right)
				{
					m_frameView->ScrollBy(MIN((point.x - r.right) / 4, 10.0),
								   B_HORIZONTAL );
				}
				else if (point.x < r.left)
				{
					m_frameView->ScrollBy(MAX( (point.x - r.left) / 4, -10.0),
								   B_HORIZONTAL);
				}
			}
		}
	}
	else
	{
		int32 marker = MarkerAt(point);
		if (marker >= 0)
			be_app->SetCursor(CCursorCache::GetCursor(CCursorCache::HORIZONTAL_RESIZE));
		else
			be_app->SetCursor(CCursorCache::GetCursor(CCursorCache::DEFAULT));
	}
}

void
CRulerView::MouseUp(
	BPoint point)
{
	D_HOOK(("CRulerView::MouseUp()\n"));

	int32 marker = MarkerAt(point);
	if (marker >= 0)
		be_app->SetCursor(CCursorCache::GetCursor(CCursorCache::HORIZONTAL_RESIZE));
	else
		be_app->SetCursor(CCursorCache::GetCursor(CCursorCache::DEFAULT));
}

void
CRulerView::SetScrollValue(
	float value,
	orientation which)
{
	CScrollerTarget::SetScrollValue(value, which);

	ScrollTo(scrollValue.x, scrollValue.y);
}

// ---------------------------------------------------------------------------
// CObserver Implementation

bool
CRulerView::Released(
	CObservable *subject)
{
	D_HOOK(("CRulerView::Released()\n"));

	bool released = false;

	if (LockLooper())
	{
		m_track->RemoveObserver(this);
		m_track = NULL;
		released = true;
		UnlockLooper();
	}

	return released;
}

void
CRulerView::Updated(
	BMessage *message)
{
	D_HOOK(("CRulerView::Updated()\n"));

	if (Window())
		Window()->PostMessage(message, this);
}

// ---------------------------------------------------------------------------
// Internal Methods

int32
CRulerView::MarkerAt(
	BPoint point)
{
	D_INTERNAL(("CRulerView::Updated()\n"));

	// won't manipulate markers that aren't visible
	if (!m_showMarkers)
		return -1;

	if ((point.y < Bounds().top) || (point.y > Bounds().bottom))
		return -1;

	int32 markers[] = { m_track->SectionStart(), m_track->SectionEnd() };
	for (int i = 0; i < 2; i++)
	{
		float x = m_frameView->TimeToViewCoords(markers[i],
												m_track->ClockType());
		if ((i == SECTION_START)
		 && (point.x >= x) && (point.x <= x + m_markerWidth))
		{
			return SECTION_START;
		}
		if ((i == SECTION_END)
		 && (point.x >= x - m_markerWidth) && (point.x <= x))
		{
			return SECTION_END;			
		}
	}

	return -1;
}

// END - RulerView.cpp
