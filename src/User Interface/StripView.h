/* ===================================================================== *
 * StripView.h (MeV/User Interface)
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
 *  Special strips for editors w/ scrollbars and spacers
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

#ifndef __C_StripView_H__
#define __C_StripView_H__

#include "Scroller.h"
#include "StripFrameView.h"

// Interface Kit
#include <Control.h>

	// StripView is a class which can function as a scrolling
	// strip within the strip frame.

class CStripView : protected CScrollerTarget {
	friend class			CStripFrameView;

	CStripFrameView		&frame;
	CScrollerTarget		*container;
	CScroller			*rightScroller;
	BView				*rightSpacer;
	BControl				*magIncButton,
						*magDecButton;
	bool					selectionVisible;	// true if selection should be shown

	void AttachedToWindow()
	{
		bounds = Bounds();
		CScrollerTarget::AttachedToWindow();
		AdjustScrollers();
	}

protected:
	BRect				bounds;			// Cached bounds.

public:
		// ---------- Constructor

	CStripView(	CStripFrameView	&frame,
				BRect			rect,
				const char		*name,
				bool			makeScroller = false,
				bool			makeMagButtons = false );
	
		// ---------- Hooks

	void Draw( BRect updateRect );
	void FrameResized( float width, float height );
	
		// ---------- Getters

	CScrollerTarget *TopView() { return container; }

		// ---------- Setters

	void SetScrollValue( float inScrollValue, orientation inOrient )
	{
		CScrollerTarget::SetScrollValue( inScrollValue, inOrient );
		ScrollTo( scrollValue.x, scrollValue.y );
	}

		/**	Called when the window activates to tell this view
			to make the selection visible.
		*/
	virtual void OnGainSelection() { Invalidate(); }
	
		/**	Called when some other window activates to tell this view
			to hide the selection.
		*/
	virtual void OnLoseSelection() { Invalidate(); }
	
		/**	Returns true if this view should display the selection highlight. */
	bool IsSelectionVisible() { return selectionVisible; }
	
		/**	Called by framework when selection is gained or lost. */
	void SetSelectionVisible( bool inVisible );

		/**	Set which handler the zoom buttons are targeting. */
	void SetZoomTarget( BHandler *inHandler )
	{
		if (magIncButton) magIncButton->SetTarget( inHandler );
		if (magDecButton) magDecButton->SetTarget( inHandler );
	}
	
		/**	Return cached bounds of view (doesn't require app-server call.) */
	const BRect &ViewBounds() { return bounds; }
};

#endif /* __C_StripView_H__ */
