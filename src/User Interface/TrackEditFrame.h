/* ===================================================================== *
 * TrackEditFrame.h (MeV/User Interface)
 * ---------------------------------------------------------------------
 * License:
 *  The contents of this file are subject to the Mozilla Public
 *  License Version 1.1 (the "License"); you may not use this file
 *  except in compliance with the License. You may obtain a copy of
 *  the License at http://www.mozilla.org/MPL/
 *
 *  Software distributed under the License is distributed on an "AS
 *  IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 *  implied. See the License for the specific language governing
 *  rights and limitations under the License.
 *
 *  The Original Code is MeV (Musical Environment) code.
 *
 *  The Initial Developer of the Original Code is Sylvan Technical 
 *  Arts. Portions created by Sylvan are Copyright (C) 1997 Sylvan 
 *  Technical Arts. All Rights Reserved.
 *
 *  Contributor(s): 
 *		Christopher Lenz (cell)
 *
 * ---------------------------------------------------------------------
 * Purpose:
 *  Override's strip frame to handle scaling/tracks
 * ---------------------------------------------------------------------
 * History:
 *	1997		Talin
 *		Original implementation
 *	04/08/2000	cell
 *		General cleanup in preparation for initial SourceForge checkin
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __C_TrackEditFrame_H__
#define __C_TrackEditFrame_H__

#include "StripFrameView.h"
#include "Track.h"

class CTrackEditFrame : public CStripFrameView {

	enum {
		Max_Zoom		= 24,
		Min_Zoom		= 8
	};

	CTrack				*track;
	short				hZoomStep;
	double				pixelsPerTimeUnit;		// pixels per unit of time.
	TClockType			frameClockType;			// Clock type of frame grid
	
	void SetScaling();
	
	void FrameResized( float width, float height )
	{
		RecalcScrollRange();
		CStripFrameView::FrameResized( width, height );
	}

public:

		/**	Constructor. */
	CTrackEditFrame(	BRect	frame,
						char	*name,
						CTrack	*track,
						ulong	resizingMode = B_FOLLOW_LEFT | B_FOLLOW_TOP );

		// ---------- Getters

		/**	Return the address of the track. */
	CTrack *Track() { return track; }
	short HZoomStep() { return hZoomStep; }

		// ---------- Zoom functions

	void ZoomIn();
	void ZoomOut();

		// ---------- Conversion functions

		/**	Convert time interval into x-coordinate. */
	double TimeToViewCoords( long timeVal, TClockType clockType );

		/**	Convert pixel x-coordinate into time interval. */
	long ViewCoordsToTime( double relPos, TClockType clockType );
	
		// Adjust the scroll range of this frame to match track.
	void RecalcScrollRange();
	
		/** Set the clock type being viewed. */
	void SetFrameClockType( TClockType inType )
	{
		Hide();
		frameClockType = inType;
		Show();
		ruler->Invalidate();
	}

		/** Get the clock type being viewed. */
	TClockType FrameClockType() { return frameClockType; }
};

#endif /* __C_TrackEditFrame_H__ */
