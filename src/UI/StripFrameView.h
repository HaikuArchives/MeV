/* ===================================================================== *
 * StripFrameView.h (MeV/User Interface)
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
 *  Frame which contains scrolling strips
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

#ifndef __C_StripFrameView_H__
#define __C_StripFrameView_H__

#include <AppKit.h>
#include <View.h>
#include "Scroller.h"

class CStripFrameView :
	public CScrollerTarget
{

public:								// Constructor/Destructor

									CStripFrameView(
										BRect frame,
										char *name,
										ulong resizingMode = B_FOLLOW_LEFT | B_FOLLOW_TOP);

									~CStripFrameView();

public:								// CScrollerTarget Implementation

	virtual void					AttachedToWindow();

	virtual void					Draw(
										BRect updateRect);

	virtual void					FrameResized(
										float width,
										float height);

	virtual void					MouseDown(
										BPoint point);
	virtual void					MouseMoved(
										BPoint point,
										ulong transit,
										const BMessage *message);
	
public:							// Operations

	bool						AddChildView(
									BView *inView,
									int inHeight,
									int inIndex = -1,
									bool inFixed = false);

	void						RemoveChildView(
									BView *view );

	void						SetScrollValue(
									float inScrollValue,
									orientation inOrient);

	void						SetRuler(
									CScrollerTarget *inRuler)
								{
									m_ruler = inRuler;
								}

public:							// Accessors

	virtual ulong				MinimumViewSize(
									BView *inChild)
								{
									return 0;
								}

	CScrollerTarget *			Ruler() const
								{
									return m_ruler;
								}

	// REM: This is kludged, there should be a parameter.
	BPoint						FrameSize()
								{
									return BPoint( Frame().Width() - 14.0 - 20.0, Frame().Height() );
								}
	
	int32						CountStrips() const
								{
									return m_childViews.CountItems();
								}

	BView *						StripAt(
									int32 inIndex) const
								{
									return (BView *)m_childViews.ItemAt( inIndex );
								}
								
protected:						// Instance Data

	BList						m_childViews;

	BList						m_childInfoList;

	// Optional horizontal ruler frame
	CScrollerTarget *			m_ruler;
	
	void						ArrangeViews();

private:						// Internal Types

	struct ChildInfo {
		int32		y, h;
		int32		proportion;		// Ideal proportion
		bool		fixedSize;		// true if size of item is fixed
	};
};

#endif /* __C_StripFrameView_H__ */
