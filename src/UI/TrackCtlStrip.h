/* ===================================================================== *
 * TrackCtlStrip.h (MeV/UI)
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

#include "DocWindow.h"
#include "EventEditor.h"
#include "StripFrameView.h"

// ---------------------------------------------------------------------------
// Linear editor strip view

class CTrackCtlStrip
	:	public CEventEditor
{
	friend class	CTrackEventHandler;
	friend class	CRepeatEventHandler;
	friend class	CSequenceEventHandler;
	friend class	CTimeSigEventHandler;
	friend class	CProgramChangeEventHandler;
	friend class	CTempoEventHandler;

public:							// Constructor/Destructor

								CTrackCtlStrip(
									BLooper &looper,
									CStripFrameView &frame,
									BRect rect,
									CEventTrack *track,
									char *name = "Track");

public:							// Accessors

	float						BarHeight() const
								{ return m_barHeight; }

public:							// Operations

	void						DeselectAll();

	/** returns y-pos for pitch */
	float						VPosToViewCoords(
									int32 pos) const;

	/** returns pitch for y-pos and optionally clamp to legal values */
	int32						ViewCoordsToVPos(
									float y,
									bool limit = true) const;

public:							// CEventEditor Implementation

	virtual void				AttachedToWindow();

	/** Construct a new event of the current selected type */
	virtual bool				ConstructEvent(
									BPoint point);

	virtual void				Draw(
									BRect updateRect);

	virtual void				MessageReceived(
									BMessage *message);

	virtual void				MouseMoved(
									BPoint point,
									uint32 transit,
									const BMessage *message);

	/**	Called when the window activates to tell this view
		to make the selection visible. */
	virtual void				OnGainSelection()
								{ InvalidateSelection(); }
	
	/**	Called when some other window activates to tell this view
		to hide the selection. */
	virtual void				OnLoseSelection()
								{ InvalidateSelection(); }

	/** Update message from another observer */
	virtual void				OnUpdate(
									BMessage *message);

	virtual void				Pulse();

	virtual bool				SupportsShadowing()
								{ return true; }

	virtual void				ZoomChanged(
									int32 diff);

public:							// Internal Operations

	/** Construct a new event of the given type */
	bool						ConstructEvent(
									BPoint point,
									event_type type);

	void						CalcZoom();

private:

	int32						m_barHeight;

	// logical height of strip.
	short						m_stripLogicalHeight;
};

#endif /* __C_TrackCtlStrip_H__ */
