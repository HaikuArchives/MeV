/* ===================================================================== *
 * StripLabelView.h (MeV/StripView)
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
 *  Label views used by editor strips
 * ---------------------------------------------------------------------
 * History:
 *	1997		Talin
 *		Original implementation
 *	04/08/2000	cell
 *		General cleanup in preparation for initial SourceForge checkin
 *	07/15/2000	cell
 *		Separated from EventEditor.h/cpp
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __C_StripLabelView_H__
#define __C_StripLabelView_H__

// Interface Kit
#include <View.h>

class CStripFrameView;
class CStripView;

class BPopUpMenu;

class CStripLabelView
	:	public BView
{
	friend class CStripView;

public:							// Constructor/Destructor

								CStripLabelView(
									BRect rect,
									const char *name,
									uint32 resizeFlags,
									uint32 flags);

								~CStripLabelView();

public:							// Hook Functions

	virtual void				DrawInto(
									BView *view,
									BRect updateRect);

public:							// Accessors

	CStripFrameView *			FrameView() const;

	CStripView *				StripView() const
								{ return m_stripView; }

public:							// BView Implementation

	virtual void				AttachedToWindow();

	virtual void				Draw(
									BRect updateRect);

	virtual void				MouseDown(
									BPoint point);

	virtual void				MouseMoved(
									BPoint point,
									uint32 transit,
									const BMessage *message);

	virtual void				MouseUp(
									BPoint point);

protected:						// Internal Methods

	BPopUpMenu *				ContextMenu() const
								{ return m_contextMenu; }

	virtual void				InitContextMenu();

	virtual void				ShowContextMenu(
									BPoint point);

private:						// Internal Operations

	// called by CStripView when attached to the strip
	void						attach(
									CStripView *stripView);

	BBitmap *					make_drag_bitmap();

private:						// Instance Data

	CStripView *				m_stripView;

	BBitmap *					m_bitmap;

	BPopUpMenu *				m_contextMenu;

	bool						m_dragging;
};

#endif /* __C_StripLabelView_H__ */
