/* ===================================================================== *
 * GridWindow.h (MeV/UI)
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
 *  Controls track grid settings.
 * ---------------------------------------------------------------------
 * History:
 *	1997		Talin
 *		Original implementation
 *	04/08/2000	cell
 *		General cleanup in preparation for initial SourceForge checkin
 *  04/15/2000	cell
 *		Now uses B_FLOATING_WINDOW type instead of the PaletteWindow
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __C_GridWindow_H__
#define __C_GridWindow_H__

#include "Observer.h"
#include "AppWindow.h"

class CEventTrack;
class CTimeIntervalControl;

	// Floating preference editors for editor windows...
	// 	Grid size with track...

class CGridWindow :
	public CAppWindow
{

public:						// Constants

	enum messages
	{
							GRID_INTERVAL_CHANGED = 'mUGc'
	};
	
	static const BRect		DEFAULT_DIMENSIONS;

public:						// Constructor/Destructor

							CGridWindow(
								BPoint position,
								CWindowState &state );

	virtual					~CGridWindow();

public:						// Operations

	/** Inspect the current event of the track. */
	void					WatchTrack(
								CEventTrack *track);

public:						// CAppWindow Implementation

	virtual void			MessageReceived(
								BMessage *message);

	/** If the app wants us to stop looking at the track, then oblige it. */
	virtual void			SubjectReleased(
								CObservable *subject);

	/** Update inspector info when we get an observer update message. */
	virtual void			SubjectUpdated(
								BMessage *message);

private:					// Instance Data

	CTimeIntervalControl	*m_intervalControl;

	CEventTrack				*m_track;
};

#endif /* __C_GridWindow_H__ */
