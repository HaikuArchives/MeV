/* ===================================================================== *
 * StatusBar.h (MeV/UI)
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
 * History:
 *	11/08/2000	cell
 *		Original implementation
 * ===================================================================== */

#ifndef __C_StatusBar_H__
#define __C_StatusBar_H__

// Interface Kit
#include <View.h>

class BBitmap;

class CStatusBar :
	public BView
{

public:							// Constructor/Destructor

								CStatusBar(
									BRect frame,
									BScrollBar *scrollBar = NULL,
									bool dimOnDeactivate = true,
									uint32 resizingMode = B_FOLLOW_LEFT | B_FOLLOW_BOTTOM);

	virtual						~CStatusBar();

public:							// Hook Functions

	virtual void				DrawInto(
									BView *view,
									BRect updateRect);

public:							// Operations

	void						SetMinimumWidth(
									float width);

	void						Update();

public:							// BStringView Implementation

	virtual void				AttachedToWindow();

	virtual void				Draw(
									BRect updateRect);

	virtual void				DrawAfterChildren(
									BRect updateRect);

	virtual void				FrameResized(
									float width,
									float height);

	virtual void				MouseDown(
									BPoint point);

	virtual void				MouseMoved(
									BPoint point,
									uint32 transit,
									const BMessage *message);

	virtual void				MouseUp(
									BPoint point);

	virtual void				WindowActivated(
									bool active);

private:						// Internal Operations

	void						AllocBackBitmap(
									float width,
									float height);

	void						FreeBackBitmap();

private:						// Instance Data

	/**	The sibling scrollbar which should be resized by the 
	 *	status view.
	 */
	BScrollBar *				m_scrollBar;

	/** Is being resized currently. */
	bool						m_dragging;
	
	/** Offscreen buffer. */
	BBitmap *					m_backBitmap;
	BView *						m_backView;
	bool						m_dirty;

	float						m_minWidth;
	bool						m_dimOnDeactivate;
};

#endif /* __C_StatusBar_H__ */
