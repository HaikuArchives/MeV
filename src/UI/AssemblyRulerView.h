/* ===================================================================== *
 * TrackWindow.h (MeV/User Interface)
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
 *  Ruler view for assembly window
 * ---------------------------------------------------------------------
 * History:
 *	1997		Talin
 *		Original implementation
 *	04/08/2000	cell
 *		General cleanup in preparation for initial SourceForge checkin
 *	04/21/2000	cell
 *		Separated from TrackWindow
 *		Updated to use SetMouseEventMask()
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __C_AssemblyRulerView_H__
#define __C_AssemblyRulerView_H__

#include "RulerView.h"
#include "Observer.h"

class CEventTrack;
class BBitmap;

//const int Ruler_Height	 = 12;

class CAssemblyRulerView :
	public CRulerView,
	public CObserver
{

public:							// Constants

	enum messages
	{
								MARKER_MOVED = 'mARm'
	};

public:							// Constructor/Destructor

								CAssemblyRulerView(
									BLooper &looper,
									CTrackEditFrame *frameView,
									CEventTrack	*track,
									BRect frame,
									const char *name,
									ulong resizeMask,
									ulong flags);

public:							// CRulerView Implementation

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

public:							// CObserver Implementation

	virtual void				OnUpdate(
									BMessage *message);

public:							// Operations

	void						ShowMarkers(
									bool show);

private:						// Instance Data

	bool						m_showMarkers;

	BBitmap *					m_markerBitmap;
};

#endif /* __C_TrackWindow_H__ */
