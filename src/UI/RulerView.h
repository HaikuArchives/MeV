/* ===================================================================== *
 * RulerView.h (MeV/UI)
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
 * Contributor(s): 
 *		Christopher Lenz (cell)
 *
 * History:
 *	1997		Talin
 *		Original implementation
 *	04/08/2000	cell
 *		General cleanup in preparation for initial SourceForge checkin
 *	04/21/2000	cell
 *		Separated from TradkEditFrame.h
 *	09/29/2000	cell
 *		Merged with subclass CAssemblyRulerView
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __C_RulerView_H__
#define __C_RulerView_H__

#include "Observer.h"
#include "Scroller.h"

class CEventTrack;
class CStripFrameView;

/**
 *	Ruler View, associated with track edit frames
 *	@author		Talin, Christopher Lenz
 *	@package	UI
 */
class CRulerView :
	public CScrollerTarget,
	public CObserver
{

public:							// Constants

	enum messages
	{
								MARKER_MOVED = 'mARm'
	};

	enum markers
	{
								SECTION_START,

								SECTION_END
	};

public:							// Constructor/Destructor

								CRulerView(
									BRect frame,
									const char *name,
									BLooper &looper,
									CStripFrameView *frameView,
									CEventTrack	*track,
									ulong resizingModeMask,
									ulong flags);

	virtual						~CRulerView();

protected:						// Accessors

	/**	Returns either SECTION_START or SECTION_BEGIN if a marker is
	 *	under point. If no marker is found, this function returns -1.
	 */
	int32						MarkerAt(
									BPoint point);

public:							// Operations

	void						ShowMarkers(
									bool show);

public:							// BScrollerTarget Implementation

	virtual void				Draw(
									BRect updateRect);

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

	virtual void				SetScrollValue(
									float scrollValue,
									orientation which);

public:							// CObserver Implementation

	virtual bool				Released(
									CObservable *subject);

	virtual void				Updated(
									BMessage *message);

private:						// Instance Data

	CEventTrack *				m_track;

	CStripFrameView *			m_frameView;

	bool						m_showMarkers;

	BBitmap *					m_leftMarker;

	BBitmap *					m_rightMarker;

	float						m_markerWidth;
};

#endif /* __C_RulerView_H__ */
