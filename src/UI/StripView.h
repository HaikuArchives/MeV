/* ===================================================================== *
 * StripView.h (MeV/UI)
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
 * History:
 *	1997		Talin
 *	Original implementation
 *	04/08/2000	cell
 *	General cleanup in preparation for initial SourceForge checkin
 *	10/08/2000	cell
 *	Added serialization caps.
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __C_StripView_H__
#define __C_StripView_H__

#include "Scroller.h"
#include "StripLabelView.h"

// Interface Kit
#include <Control.h>

class CIFFReader;
class CIFFWriter;
class CStripFrameView;

 /**
 *		Special strips for editors w/ scrollbars and spacers.
 *		StripView is a class which can function as a scrolling
 *		strip within the strip frame.
 *		@author	Talin, Christoper Lenz.   
 */
 
class CStripView :
	public CScrollerTarget
{
	friend class			CStripFrameView;

public:							// Constants

	enum messages
	{
								ADD_STRIP = 'stvA',

								REMOVE_STRIP,

								PROPORTIONS_CHANGED,

								REARRANGE_STRIPS
	};

public:							// Constructor/Destructor

								CStripView(
									CStripFrameView &frame,
									BRect rect,
									const char *name,
									bool makeScroller = false,
									bool makeMagButtons = false);

public:							// Hook Functions

	/**	Called when the window activates to tell this view
			to make the selection visible.	*/
	virtual void				OnGainSelection()
								{ Invalidate(); }
	
	/**	Called when some other window activates to tell this view
			to hide the selection.	*/
	virtual void				OnLoseSelection()
								{ Invalidate(); }

	virtual float				MinimumHeight() const;

	/**	Called when the vertical zoom value has been changed,
			either programatically, or by the user.	*/
	virtual void				ZoomChanged(
									int32 diff)
								{ }

public:							// Accessors

	CStripFrameView *			FrameView() const
								{ return &frame; }

	CStripLabelView *			LabelView() const
								{ return m_labelView; }
	void						SetLabelView(
									CStripLabelView *labelView);

	/**	Individual strips can have rulers as well.	*/
	CScrollerTarget *			RulerView() const
								{ return m_rulerView; }
	void						SetRulerView(
									CScrollerTarget *rulerView);

	CScrollerTarget *			TopView()
								{ return m_container; }

	/**	Returns true if this view should display the selection highlight.	*/
	bool						IsSelectionVisible()
								{ return m_selectionVisible; }

	bool						IsRemovable() const
								{ return m_removable; }
	void						SetRemovable(
									bool removable = true)
								{ m_removable = removable; }

public:							// Operations

	void						SetScrollValue(
									float value,
									orientation posture);

	/**	Called by framework when selection is gained or lost.	*/
	void						SetSelectionVisible(
									bool visible);

	/**	Increments the vertical zoom value.	*/
	void						ZoomBy(
									int32 diff);

	/**	Returns the vertical zoom setting:
			positive -> zoomed in by x steps
			0		 -> original zoom
			negative -> zoomed out by x steps.	 */
	int32						ZoomValue() const
								{ return m_verticalZoom; }

public:							// Serialization

	virtual void				ExportSettings(
									BMessage *settings) const;

	virtual void				ImportSettings(
									const BMessage *settings);

	static void					ReadState(
									CIFFReader &reader,
									BMessage *settings);

	static void					WriteState(
									CIFFWriter &writer,
									const BMessage *settings);

public:							// CScrollerTarget Implementation

	virtual void				AllAttached();

	virtual void				AttachedToWindow();

	virtual void				FrameResized(
									float width,
									float height);

	virtual void				MessageReceived(
									BMessage *message);

private:
	
	CStripFrameView	&			frame;

	CScrollerTarget *			m_container;

	CStripLabelView *			m_labelView;

	CScrollerTarget *			m_rulerView;
	
	CScroller *					rightScroller;

	BView *						rightSpacer;

	BControl *					magIncButton;

	BControl *					magDecButton;

	/**	True if selection should be shown.	*/
	bool						m_selectionVisible;

	/**	Defaults to true; false will disable the 'Hide' option.	*/
	bool						m_removable;

	int32						m_verticalZoom;
};

#endif /* __C_StripView_H__ */
