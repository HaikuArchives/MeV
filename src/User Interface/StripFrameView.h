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

class CStripFrameView : public CScrollerTarget {
	struct ChildInfo {
		int32		y, h;
		int32		proportion;		// Ideal proportion
		bool			fixedSize;		// TRUE if size of item is fixed
	};

protected:
	BList			childViews;
	BList			childInfoList;
	CScrollerTarget	*ruler;			// Optional horizontal ruler frame
	
	static char		*cursor;

	void ArrangeViews();

protected:
	void AttachedToWindow()
	{
		AdjustScrollers();
		CScrollerTarget::AttachedToWindow();
	}
	
public:

		// ---------- Constructor

	CStripFrameView(	BRect	frame,
						char	*name,
						ulong	resizingMode = B_FOLLOW_LEFT | B_FOLLOW_TOP );
	~CStripFrameView();

		// ---------- Child view management

	bool AddChildView(	BView	*inView,
						int		inHeight,
						int		inIndex = -1,
						bool		inFixed = false );

	void RemoveChildView( BView *view );

		// ---------- Hooks

	void Draw( BRect updateRect );

	void MouseDown( BPoint point );
	void MouseMoved( BPoint point, ulong transit, const BMessage *message );
	
	void FrameResized( float width, float height );
	
		// ---------- Setters

	void SetScrollValue( float inScrollValue, orientation inOrient );
	void SetRuler( CScrollerTarget *inRuler ) { ruler = inRuler; }

		// ---------- Getters

	virtual ulong MinimumViewSize( BView *inChild ) { return 0; }
	CScrollerTarget *Ruler() { return ruler; }

		// REM: This is kludged, there should be a parameter.
	BPoint FrameSize()
	{
		return BPoint( Frame().Width() - 14.0 - 20.0, Frame().Height() );
	}
	
	int32 CountStrips() { return childViews.CountItems(); }
	BView *StripAt( int32 inIndex ) { return (BView *)childViews.ItemAt( inIndex ); }
};

#endif /* __C_StripFrameView_H__ */
