/* ===================================================================== *
 * TrackListView.h (MeV/User Interface)
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
 *  
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

#ifndef __C_TrackListView_H__
#define __C_TrackListView_H__

#include "Observer.h"

// Interface Kit
#include <ListView.h>

class CMeVDoc;

class BPopUpMenu;

class CTrackListView
	:	public BListView
{
	friend class CTrackListWindow;

public:							// Constructor/Destructor

								CTrackListView(
									BRect frame,
									CMeVDoc *doc);

	virtual						~CTrackListView();

public:							// BListView Implementation

	virtual void				AttachedToWindow();

	virtual void				Draw(
									BRect updateRect);

	virtual void				GetPreferredSize(
									float *width,
									float *height);

	virtual bool				InitiateDrag(
									BPoint point,
									int32 index,
									bool wasSelected);

	virtual void				KeyDown(
									const char *bytes,
									int32 numBytes);

	virtual void				MessageReceived(
									BMessage *message);

	virtual void				MouseDown(
									BPoint point);

	virtual void				MouseMoved(
									BPoint point,
									uint32 transit,
									const BMessage *message);

	virtual void				MouseUp(
									BPoint point);

protected:						// Operations

	void						BuildTrackList();

	void						SetDocument(
									CMeVDoc *doc);

	void						ShowContextMenu(
									BPoint point);

private:						// Instance Data

	CMeVDoc *					m_doc;

	BPopUpMenu *				m_contextMenu;

	int32						m_lastClickButton;

	BPoint						m_lastClickPoint;

	bool						m_reordering;

	BRect						m_currentReorderMark;

	int32						m_lastReorderIndex;

	BRect						m_lastReorderMark;
};

#endif /* __C_TrackListView_H__ */
