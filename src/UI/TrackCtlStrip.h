/* ===================================================================== *
 * TrackCtlStrip.h (MeV/User Interface)
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
 *  classes relating to the track control editor
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

#ifndef __C_TrackCtlStrip_H__
#define __C_TrackCtlStrip_H__

#include "TrackEditFrame.h"
#include "EventEditor.h"
#include "DocWindow.h"

// ---------------------------------------------------------------------------
// Linear editor strip view

class CTrackCtlStrip : public CEventEditor {
protected:

	friend class	CTrackEventHandler;
	friend class	CRepeatEventHandler;
	friend class	CSequenceEventHandler;
	friend class	CTimeSigEventHandler;
	friend class	CProgramChangeEventHandler;
	friend class	CTempoEventHandler;

	int32			barHeight;
	short			stripLogicalHeight;		// logical height of strip.
	font_height		fontSpec;

	void MouseMoved(
		BPoint		point,
		ulong		transit,
		const BMessage	* );

	void AttachedToWindow();
	void MessageReceived( BMessage *msg );
	void Pulse();
	void CalcZoom();

public:
		// ---------- Constructor

	CTrackCtlStrip(	BLooper			&inLooper,
					CTrackEditFrame &frame,
					BRect			rect,
					CEventTrack		*track,
					char				*inStripName = "Track" );
					
		// ---------- Hooks

	void Draw( BRect updateRect );

/*	void StartDrag( BPoint point, ulong buttons );
	bool DoDrag( BPoint point, ulong buttons );
	void FinishDrag( BPoint point, ulong buttons, bool commit ); */

		// Construct a new event of the current selected type
	bool ConstructEvent( BPoint point );

		// Construct a new event of the given type
	bool ConstructEvent( BPoint point, TEventType inType );

		// Update message from another observer
	void OnUpdate( BMessage *inMsg );

		// ---------- Conversion funcs

		// returns y-pos for pitch
	long VPosToViewCoords( int pos );

		// returns pitch for y-pos and optionally clamp to legal values
	long ViewCoordsToVPos( int yPos, bool limit = true );

		// ---------- Getters

	bool SupportsShadowing() { return true; }

		// ---------- General operations

	void DeselectAll();
	
		/**	Called when the window activates to tell this view
			to make the selection visible.
		*/
	virtual void OnGainSelection() { InvalidateSelection(); }
	
		/**	Called when some other window activates to tell this view
			to hide the selection.
		*/
	virtual void OnLoseSelection() { InvalidateSelection(); }
};

#endif /* __C_TrackCtlStrip_H__ */
