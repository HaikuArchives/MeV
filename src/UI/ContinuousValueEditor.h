/* ===================================================================== *
 * ContinuousValueEditor.h (MeV/UI)
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

class CContinuousValueEditor
	:	public CEventEditor
{
	// ++++++ this class shouldn't need to know about a descendant-related
	//		  class !
	friend class CPitchBendEventHandler;

public:							// Constructor/Destructor

								CContinuousValueEditor(
									CStripFrameView &frameView,
									BRect rect,
									const char *name);

public:							// Hook Functions

	/**	Derived classes implement this to optionally draw 
		horizontal grid lines.
	*/
	virtual void				DrawHorizontalGrid(
									BRect updateRect)
								{ }

public:							// Operations

	/** Calculate factors relating to zoom. */
	void						CalcZoom();

	void						DeselectAll();

public:							// CEventEditor Implementation

	/** When attached to window, set scroll range based on strip's 
		logical height. */
	virtual void				AttachedToWindow();

	virtual void				Draw(
									BRect updateRect);

	virtual bool				SupportsShadowing()
								{ return true; }

	/**	Called when the window activates to tell this view
		to make the selection visible.
	*/
	virtual void				OnGainSelection()
								{ InvalidateSelection(); }

	/**	Called when some other window activates to tell this view
		to hide the selection.
	*/
	virtual void				OnLoseSelection()
								{ InvalidateSelection(); }

	/** Periodic update time tick -- used to update playback markers. */
	virtual void				Pulse();

	virtual void				SetScrollValue(
									float value,
									orientation posture);

protected:						// Instance Data

	/** Pixels per control value. */
	float						pixelsPerValue;

	/** Zoom amount in vertical direction. */
	short						verticalZoom;

	/** Logical height of strip. */
	short						stripLogicalHeight;

	/** Mminimum event value. */
	long						minValue;

	/** Maximum event value. */
	long						maxValue;

	/** Max pixel ascent of events. */
	long						eventAscent;

	/** Min pixel ascent of events. */
	long						eventDescent;
};

#endif /* __C_ContinuousValueEditor_H__ */
