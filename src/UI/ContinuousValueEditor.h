/* ===================================================================== *
 * ContinuousValueEditor.h (MeV/User Interface)
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
 *  classes relating to the pich bend editor strip
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

#ifndef __C_ContinuousValueEditor_H__
#define __C_ContinuousValueEditor_H__

#include "EventEditor.h"

// ---------------------------------------------------------------------------
// Continuous Value editor strip view

class CContinuousValueEditor : public CEventEditor {
protected:

	friend class	CPitchBendEventHandler;

	float			pixelsPerValue;		// Pixels per control value
	short			verticalZoom,			// Zoom amount in vertical direction
					stripLogicalHeight;		// logical height of strip.
	long				minValue,				// minimum value of event value
					maxValue,			// maxuimum event value
					eventAscent,			// max pixel ascent of events
					eventDescent;			// min pixel ascent of events

	void SetScrollValue( float inScrollValue, orientation inOrient )
	{
		CStripView::SetScrollValue( inScrollValue, inOrient );
		labelView->ScrollTo( 0.0, scrollValue.y );
	}

	void MouseMoved(
		BPoint		point,
		ulong		transit,
		const BMessage	* );

	void AttachedToWindow();
	void Pulse();

	void CalcZoom();
	
public:
		// ---------- Constructor

	CContinuousValueEditor(	BLooper			&inLooper,
						CTrackEditFrame 	&frame,
						BRect			rect,
						const char		*name );

		// ---------- Hooks

	void Draw( BRect updateRect );

		// Update message from another observer
	void OnUpdate( BMessage *inMsg );

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

#endif /* __C_ContinuousValueEditor_H__ */
