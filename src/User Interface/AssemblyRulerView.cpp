/* ===================================================================== *
 * TrackWindow.cpp (MeV/User Interface)
 * ===================================================================== */

#include "AssemblyRulerView.h"
#include "DataSnap.h"
#include "EventTrack.h"
#include "ResourceUtils.h"

// Support Kit
#include <Debug.h>

// Debugging Macros
#define D_ALLOC(x) //PRINT (x)		// Constructor/Destructor
#define D_HOOK(x) //PRINT (x)		// CRulerView/CObserver Implementation
#define D_OPERATION(x) //PRINT (x)	// Operations

// ---------------------------------------------------------------------------
// Constructor/Destructor

CAssemblyRulerView::CAssemblyRulerView(
	BLooper &looper,
	CTrackEditFrame &frameView,
	CEventTrack *track,
	BRect rect,
	const char *name,
	ulong resizingMode,
	ulong flags)
	:	CRulerView(frameView, rect, name, resizingMode, flags),
		CObserver(looper, track),
		m_showMarkers(true),
		m_markerBitmap(NULL)
{
	D_ALLOC(("CAssemblyRulerView::CAssemblyRulerView()\n"));

	m_markerBitmap = ResourceUtils::LoadImage("SectionMarker");
	SetViewColor(B_TRANSPARENT_32_BIT);
}
	
// ---------------------------------------------------------------------------
// CRulerView Implementation

void
CAssemblyRulerView::Draw(
	BRect updateRect)
{
	CEventTrack	*track = (CEventTrack *)subject;
	TClockType	clockType = track->ClockType();
	long startTime = frame.ViewCoordsToTime( updateRect.left - 48.0, clockType );
	if (startTime < 0)
	{
		startTime = 0;
	}

	CSignatureMap::Iterator timeIter(track->SigMap(), startTime);
	long time;
	BRect rect(Bounds());
	bool major;
	int32 majorTime = track->SigMap().entries->sigMajorUnitDur;
	double majorXStep = frame.TimeToViewCoords( majorTime, clockType );
	int32 steps = 1;

	while (majorXStep < 24)
	{
		if (steps == 2)
		{
			steps = 5;
		}
		else
		{
			steps *= 2;
		}
		majorXStep = frame.TimeToViewCoords(majorTime * steps, clockType);
	}

	SetHighColor(255, 255, 220, 255);
	FillRect(BRect(updateRect.left, rect.top, updateRect.right, rect.bottom - 1));
	SetHighColor(0, 0, 0, 255);
	FillRect(BRect(updateRect.left, rect.bottom, updateRect.right, rect.bottom));

	for (time = timeIter.First(major); ; time = timeIter.Next(major))
	{
		double x = frame.TimeToViewCoords( time, clockType );
		if (x > updateRect.right)
		{
			break;
		}

		if (major)
		{
			if (timeIter.MajorCount() % steps)
			{
				continue;
			}
			SetHighColor( 160, 160, 140 );
		}
		else if (steps > 1)
		{
			continue;
		}
		else
		{
			SetHighColor( 210, 210, 180 );
		}

		if (x > 0.0)
		{
			StrokeLine(BPoint(x, updateRect.top),
					   BPoint(x, updateRect.bottom));
		}
	}

	CSignatureMap::Iterator timeIter2(track->SigMap(), startTime);
	SetHighColor( 0, 0, 0 );

	for (time = timeIter2.First(major) ; ; time = timeIter2.Next(major))
	{
		double x = frame.TimeToViewCoords(time, clockType);
		if (x > updateRect.right)
		{
			break;
		}

		if (major && !(timeIter2.MajorCount() % steps))
		{
			char str[16];

			if (track->ClockType() == ClockType_Metered)
			{
				sprintf(str, "%02ld", timeIter2.MajorCount());
			}
			else
			{
				sprintf(str, "%02ld:00", timeIter2.MajorCount());
			}

			DrawString(str, BPoint(x + 4, 9));
		}
	}

	if (m_showMarkers)
	{
		// Now, draw the track section markers...
		double	x;
	
		SetDrawingMode(B_OP_OVER);
		x = frame.TimeToViewCoords(track->SectionStart(), clockType);
		DrawBitmapAsync(m_markerBitmap, BPoint(x - 4.0, 0.0));
		x = frame.TimeToViewCoords(track->SectionEnd(), clockType);
		DrawBitmap(m_markerBitmap, BPoint(x - 4.0, 0.0));
//		SetDrawingMode(B_OP_COPY);
	}
}

void
CAssemblyRulerView::MouseDown(
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
		CEventTrack	*track = (CEventTrack *)subject;
		int32 markers[] = {track->SectionStart(), track->SectionEnd()};
		for (int i = 0; i < 2; i++)
		{
			float x = frame.TimeToViewCoords(markers[i], track->ClockType());
			if ((point.x >= x - 4.0) && (point.x <= x + 4.0))
			{
				BMessage message(MARKER_MOVED);
				message.AddInt32("which", i);
				DragMessage(&message, BRect(0.0, 0.0, -1.0, -1.0), (CRulerView *)this);
				break;
			}
		}
	}
}
	
void
CAssemblyRulerView::MouseMoved(
	BPoint point,
	uint32 transit,
	const BMessage *message)
{
	D_HOOK(("CAssemblyView::MouseMoved()\n"));

	if (message)
	{
		switch (message->what)
		{
			case MARKER_MOVED:
			{
				D_HOOK((" -> MARKER_MOVED\n"));

				int32 which;
				if (message->FindInt32("which", &which) != B_OK)
				{
					return;
				}

				CEventTrack	*track = (CEventTrack *)subject;
				int32 time = frame.ViewCoordsToTime(point.x, track->ClockType());
				if (track->GridSnapEnabled())
				{
					int32 majorUnit, extraTime;
					track->SigMap().DecomposeTime(time, majorUnit, extraTime);
					time += DataSnapNearest( extraTime, 0, track->TimeGridSize() ) - extraTime;
				}
				
				if (time < 0)
				{
					time = 0;
				}

				int32 markers[] = {track->SectionStart(), track->SectionEnd()};
				if (time != markers[which])
				{
					float x = frame.TimeToViewCoords(markers[which], track->ClockType());
					Invalidate(BRect(x - 4.0, 0.0, x + 5.0, 10));
					markers[which] = time;
					track->SetSection(markers[0], markers[1]);
					x = frame.TimeToViewCoords(markers[which], track->ClockType());
					Invalidate(BRect(x - 4.0, 0.0, x + 5.0, 10));
				}

				// Implement auto-scrolling (horizontal for frame)
				BRect r(Bounds());
				if (point.x > r.right)
				{
					frame.ScrollBy(MIN((point.x - r.right) / 4, 10.0),
								   B_HORIZONTAL );
				}
				else if (point.x < r.left)
				{
					frame.ScrollBy(MAX( (point.x - r.left) / 4, -10.0),
								   B_HORIZONTAL);
				}
//				Window()->UpdateIfNeeded();
			}
		}
	}
}

void
CAssemblyRulerView::MouseUp(
	BPoint point)
{
	D_HOOK(("CAssemblyView::MouseUp()\n"));

	CEventTrack	*track = (CEventTrack *)subject;
	track->NotifyUpdate(CTrack::Update_Section, this);
}

// ---------------------------------------------------------------------------
// CObserver Implementation

void
CAssemblyRulerView::OnUpdate(
	BMessage *message)
{
	D_HOOK(("CAssemblyRulerView::OnUpdate()\n"));
	
	int32 trackHint;
	// Only ONE change we are interested in, and that's section markers...
	if (message->FindInt32("TrackAttrs", 0, &trackHint) == B_OK)
	{
		if (trackHint &
			(CTrack::Update_Section | CTrack::Update_SigMap | CTrack::Update_TempoMap))
		{
				Invalidate();
		}
	}
}

// ---------------------------------------------------------------------------
// Operations

void
CAssemblyRulerView::ShowMarkers(
	bool show)
{
	D_OPERATION(("CAssemblyRulerView::ShowMarkers()\n"));

	if (show != m_showMarkers)
	{
		m_showMarkers = show;
		Invalidate();
	}
}

// END - AssemblyRulerView.cpp
