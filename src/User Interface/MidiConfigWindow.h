/* ===================================================================== *
 * MidiConfigWindow.h (MeV/User Interface)
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
 *  MIDI configuration window
 * ---------------------------------------------------------------------
 * History:
 *	1997		Talin
 *		Original implementation
 *	04/08/2000	cell
 *		General cleanup in preparation for initial SourceForge checkin
 *  05/13/2000  dwalton
 		-Added prototype softport system.
 		 
 * ---------------------------------------------------------------------
 * To Do:
 * 		-'adsp' only works when an item is highlighted.
 		-add softport should have corrisponding delete softport.
 * ===================================================================== */

#ifndef __C_MidiConfigWindow_H__
#define __C_MidiConfigWindow_H__

#include "MeVApp.h"
#include "WindowState.h"

class CMultiColumnListView;
class CDevListItem;

class CMidiConfigWindow :
	public CAppWindow {

protected:
	BOutlineListView	*devList;
	BButton			*attrButton,
					*addButton,
					*subButton,
					*addpButton,
					*subpButton,
					*addspButton;
	BMessage			deviceListSave;
	BTextControl		*patchName;
	CMultiColumnListView *patchList;
	BMessenger		messenger;
	BMenuItem		*saveMenu,
					*loadMenu;
	
	void MessageReceived( BMessage *msg );

	void BuildPortList();
	void BuildPatchList( MIDIDeviceInfo *mdi );
	void ClearPatchList();
	void InsertDevice( CDevListItem *item );
	bool QuitRequested();

public:
	CMidiConfigWindow( CWindowState &inState );
	~CMidiConfigWindow();
};

#endif /* __C_MidiConfigWindow_H__ */