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

#include "Scroller.h"

// Support Kit
#include <List.h>
#include <String.h>

class CStripSplitter;
class CStripView;

class CStripFrameView
	:	public CScrollerTarget
{

public:							// Constructor/Destructor

								CStripFrameView(
									BRect frame,
									char *name,
									uint32 resizingMode = B_FOLLOW_LEFT | B_FOLLOW_TOP);

								~CStripFrameView();

public:							// Accessors

	CScrollerTarget *			Ruler() const
								{ return m_ruler; }
	void						SetRuler(
									CScrollerTarget *ruler)
								{ m_ruler = ruler; }

	// REM: This is kludged, there should be a parameter.
	BPoint						FrameSize()
								{ return BPoint(Frame().Width() - 14.0 - 20.0,
												Frame().Height()); }

public:							// Operations

	int32						CountTypes() const
								{ return m_types.CountItems(); }
	BString						TypeAt(
									int32 index) const;
	void						AddType(
									BString name);

	bool						AddStrip(
									CStripView *view,
									float proportion = 0.0,
									int32 index = -1,
									bool fixedSize = false);									
	int32						CountStrips() const
								{ return m_strips.CountItems(); }
	int32						IndexOf(
									CStripView *view) const;
	void						PackStrips();
	bool						RemoveStrip(
									CStripView *view);
	CStripView *				StripAt(
									int32 index) const;
	void						SwapStrips(
									CStripView *strip1,
									CStripView *strip2);

public:							// CScrollerTarget Implementation

	virtual void				AttachedToWindow();

	virtual void				FrameResized(
									float width,
									float height);

	virtual void				MessageReceived(
									BMessage *message);

	virtual void				SetScrollValue(
									float position,
									orientation posture);

protected:						// Internal Operations

	void						ArrangeViews();

	void						UpdateProportions();

	void						UpdateSplitters();

protected:						// Instance Data

	// contains strip_info objects
	BList						m_strips;

	// a list of available strips
	BList						m_types;

	// Optional horizontal ruler frame
	CScrollerTarget *			m_ruler;

private:						// Internal Types

	struct strip_info {
		CStripView *		strip;
		BView *				container;
		CStripSplitter *	splitter;		// splitter above the strip
		float				vertical_offset;
		float				height;
		float				proportion;		// Ideal proportion
		bool				fixed_size;		// true if size of item is fixed
	};

	struct strip_type {
		BString				name;
		BBitmap *			icon;
	};
};

#endif /* __C_StripFrameView_H__ */
