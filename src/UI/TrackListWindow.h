/* ===================================================================== *
 * TrackListWindow.h (MeV/UI)
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
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __C_TrackListWindow_H__
#define __C_TrackListWindow_H__

#include "AppWindow.h"

class CEventTrack;
class CMeVDoc;
class CTrackListView;

 /**
  *		Playback controls window.
  *	 	@author	Talin, Christoper Lenz.  
  */

class CTrackListWindow :
	public CAppWindow
{

public:							// Constants

	static const BRect			DEFAULT_DIMENSIONS;

public:							// Constructor/Destructor

								CTrackListWindow(
									BPoint position,
									CWindowState &state);

	virtual						~CTrackListWindow();
	
public:							// Operations

	void						WatchTrack(
									CEventTrack *track);

public:							// CAppWindow Implementation

	virtual void				FrameResized(
									float width,
									float height);

	virtual void				MenusBeginning();

	virtual void				MessageReceived(
									BMessage *message);

	virtual bool				SubjectReleased(
									CObservable *subject);

	virtual void				Zoom(
									BPoint origin,
									float width,
									float height);

protected:						// Internal Operations

	void						_addMenuBar();

private:						// Instance Data

	CMeVDoc *					m_doc;

	CTrackListView *			m_listView;

	bool						m_zoomed;

	bool						m_zooming;

	BRect						m_manualSize;
};

#endif /* __C_TrackListWindow_H__ */
