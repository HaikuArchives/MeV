/* ===================================================================== *
 * TrackEditFrame.cpp (MeV/User Interface)
 * ===================================================================== */

#include "TrackEditFrame.h"
#include "TempoMap.h"
#include "MeVDoc.h"

CTrackEditFrame::CTrackEditFrame(
	BRect	frame,
	char	*name,
	CTrack	*inTrack,
	ulong	resizingMode )
		: CStripFrameView( frame, name, resizingMode ),
		  track( inTrack )
{
	hZoomStep = 16;
	pixelsPerTimeUnit = 1.0;
	SetScaling();
	frameClockType = track->ClockType();
//	frameClockType = ClockType_Metered;
//	frameClockType = ClockType_Real;
}

void CTrackEditFrame::SetScaling()
{
	int32			t = ViewCoordsToTime( scrollValue.x, frameClockType );

	pixelsPerTimeUnit = (double)(1 << hZoomStep) / (256.0 * 256.0 * 16.0);
	Hide();
	SetScrollRange(	TimeToViewCoords( track->LastEventTime(), frameClockType ) + FrameSize().x,
					TimeToViewCoords( t, frameClockType ),
					0.0, 0.0 );
	Show();
}

void CTrackEditFrame::RecalcScrollRange()
{
	SetScrollRange(	TimeToViewCoords( track->LastEventTime(), track->ClockType() ) + FrameSize().x,
					scrollValue.x,
					0.0, 0.0 );
}

void CTrackEditFrame::ZoomIn()
{
	if (hZoomStep < Max_Zoom)
	{
		hZoomStep++;
		SetScaling();
		Ruler()->Invalidate();
	}
}

void CTrackEditFrame::ZoomOut()
{
	if (hZoomStep > Min_Zoom)
	{
		hZoomStep--;
		SetScaling();
		Ruler()->Invalidate();
	}
}

	/**	Convert time interval into x-coordinate. */
double CTrackEditFrame::TimeToViewCoords( long timeVal, TClockType clockType )
{
	if (clockType != frameClockType)
	{
		const CTempoMap &tMap( track->Document().TempoMap() );

		if (clockType == ClockType_Metered)
			timeVal = tMap.ConvertMeteredToReal( timeVal );
		else timeVal = tMap.ConvertRealToMetered( timeVal );
	}
	return floor( pixelsPerTimeUnit * (float)timeVal );
}

	/**	Convert pixel x-coordinate into time interval. */
long CTrackEditFrame::ViewCoordsToTime( double relPos, TClockType clockType )
{
	long			timeVal;

	timeVal = (long)( relPos / pixelsPerTimeUnit );
	if (clockType != frameClockType)
	{
		const CTempoMap &tMap( track->Document().TempoMap() );

		if (clockType == ClockType_Metered)
			timeVal = tMap.ConvertRealToMetered( timeVal );
		else timeVal = tMap.ConvertMeteredToReal( timeVal );
	}
	return timeVal;
}
