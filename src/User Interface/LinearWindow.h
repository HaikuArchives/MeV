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
	
	char					timeBuf[ 16 ];
	CToolBar				*toolBar;
	uint8				toolStates[ 1 ];
	BTextControl			*trackNameCtl;
	CTimeIntervalEditor	*durationEditor;
	BMenuItem			*playSelectMenu;
	CTextDisplay			*timeView;

	void MessageReceived( BMessage* theMessage );
	void DisplayMouseTime( CTrack *track, int32 time );
	void MenusBeginning();
	void OnUpdate( BMessage *inMsg );
	bool QuitRequested();

public:
	CLinearWindow( BRect frame, CMeVDoc &inDocument, CEventTrack *inTrack );

		/**	Returns current toolbar setting. */
	int32 CurrentTool() { return toolStates[ 0 ]; }
};

#endif /* __C_LinearWindow_H__ */
