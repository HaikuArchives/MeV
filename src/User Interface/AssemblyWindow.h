/* ===================================================================== *
 * AssemblyWindow.h (MeV/User Interface)
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
 *  Editor Window for song assembly
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

#ifndef __C_AssemblyWindow_H__
#define __C_AssemblyWindow_H__

#include "TrackWindow.h"
#include "MeVDoc.h"

// ---------------------------------------------------------------------------
// Assembly editor window

//class CTimeIntervalEditor;
class CTextDisplay;
class CToolBar;
class CMultiColumnListView;
//class CStripFrameView;
class CColumnField;
//class CDragPalette;

class CAssemblyWindow : 
	public CTrackWindow 
{
	
public:							// Constructor/Destructor

								CAssemblyWindow(
									BRect frame,
									CMeVDoc &inDocument);

								~CAssemblyWindow();

public:							// CTrackWindow Implementation

	// For windows which edit dual tracks, select which one
	// has selected events
	CEventTrack *				ActiveTrack()
								{
									return Document().ActiveMaster();
								}

	// Returns current toolbar setting
	int32						CurrentTool()
								{
									return toolStates[0];
								}

	virtual void				MessageReceived(
									BMessage *message);

	virtual void				OnUpdate(
									BMessage *message);

protected:						// Internal Methods

	void						AddMenuBar();

	void						AddToolBar();

	void						BuildTrackList();

	void						ShowTrackInfo();

private:						// Instance Data

	CToolBar *					toolBar;

	uint8						toolStates[1];

	CMultiColumnListView *		trackList;

	CColumnField *				typeColumn;

	CColumnField *				nameColumn;

//	CTextDisplay *				eventCount;
//
//	CTextDisplay *				channelsUsed;
//
//	CDragPalette *				eventPalette;
	
	enum {
		TrackEdit_ID = 'tkEd',
	};
};

#endif /* __C_AssemblyWindow_H__ */
