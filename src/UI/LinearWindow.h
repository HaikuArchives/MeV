/* ===================================================================== *
 * LinearWindow.h (MeV/User Interface)
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
 *  Editor Window for linear editor strips
 * ---------------------------------------------------------------------
 * History:
 *	1997		Talin
 *		Original implementation
 *	04/08/2000	cell
 *		General cleanup in preparation for initial SourceForge checkin
 *	04/30/2000	cell
 *		Removed internal CImageAndTextMenuItem class, now using the new
 *		CIconMenuItem class for the same thing
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __C_LinearWindow_H__
#define __C_LinearWindow_H__

#include "TrackWindow.h"
// ---------------------------------------------------------------------------
// Linear editor window

class CTimeIntervalEditor;
class CTextDisplay;
class CToolBar;

class CLinearWindow : 
	public CTrackWindow
{
	
public:							// Constructor/Destructor

								CLinearWindow(
									BRect frame,
									CMeVDoc *document,
									CEventTrack *track);

public:							// Accessors

	// Returns current toolbar setting
	int32						CurrentTool()
								{ return m_toolStates[0]; }

public:							// CTrackWindow Implementation

	virtual void				DisplayMouseTime(
									CTrack *track,
									int32 time);

	virtual void				MenusBeginning();

	virtual void				MessageReceived(
									BMessage *message);

	virtual void				OnUpdate(
									BMessage *message);

protected:

	virtual bool				AddStrip(
									BString type,
									float proportion = 0.3);

	virtual void				NewEventTypeChanged(
									event_type type);

protected:						// Internal Operations

	void						AddMenuBar();

	void						AddToolBar();

	void						AddFrameView(
									BRect frame,
									CTrack *track);

private:						// Instance Data

	char						m_timeBuf[16];

	CToolBar *					m_toolBar;

	uint8						m_toolStates[1];

	CTextDisplay *				m_timeView;
};

#endif /* __C_LinearWindow_H__ */
