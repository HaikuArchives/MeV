/* ===================================================================== *
 * RulerView.h (MeV/User Interface)
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
 *  Ruler View, associated with track editr frames
 * ---------------------------------------------------------------------
 * History:
 *	1997		Talin
 *		Original implementation
 *	04/08/2000	cell
 *		General cleanup in preparation for initial SourceForge checkin
 *	04/21/2000	cell
 *		Separated from TradkEditFrame.h
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __C_RulerView_H__
#define __C_RulerView_H__

#include "Scroller.h"

class CTrackEditFrame;

class CRulerView :
	public CScrollerTarget
{

public:							// Constructor/Destructor

								CRulerView(
									BRect frame,
									const char *name,
									CTrackEditFrame *frameView,
									ulong resizingModeMask,
									ulong flags );

public:							// Operations

	void						SetScrollValue(
									float scrollValue,
									orientation which);

protected:						// Instance Data

	CTrackEditFrame *			m_frameView;
};

#endif /* __C_TrackEditFrame_H__ */
