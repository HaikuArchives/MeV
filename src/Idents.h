/* ===================================================================== *
 * Idents.h (MeV)
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
 *		Curt Malouin (malouin)
 *
 * ---------------------------------------------------------------------
 * Purpose:
 *  Various menu and control ID's
 * ---------------------------------------------------------------------
 * History:
 *	1997		Talin
 *		Original implementation
 *	04/08/2000	cell
 *		General cleanup in preparation for initial SourceForge checkin
 *	04/30/2000	malouin
 *		EMenuIDs start at 'menA' to avoid conflict with other constants
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __Idents_H__
#define __Idents_H__

enum EMenuIDs	{
		// File menus
	MENU_NEW = 'menA',
	MENU_OPEN,
	MENU_SAVE,
	MENU_SAVE_AS,
	MENU_IMPORT,
	MENU_EXPORT,
	MENU_CLOSE,
	MENU_ABOUT,
	MENU_ABOUT_PLUGINS,
	MENU_QUIT,

		// Edit menus
	MENU_UNDO,
	MENU_REDO,
	MENU_CLEAR,
	MENU_DOC_PREFS,
	
		// Tracks menu
	MENU_CREATE_METERED_TRACK,
	MENU_CREATE_REAL_TRACK,
	MENU_DELETE_TRACK,

		// Window menus
	MENU_NEW_WINDOW,
	MENU_TRACKLIST,
	MENU_INSPECTOR,
	MENU_GRIDWINDOW,
	MENU_TRANSPORT,

		// Options menus
	MENU_VIEW_SETTINGS,
	MENU_VIRTUAL_CHANNELS,
	MENU_PROGRAM_SETTINGS,
	MENU_MIDI_CONFIG,
	
		// Player menus
	MENU_PLAY,
	MENU_PLAY_FROM_START,
	MENU_PLAY_SECTION,
	MENU_STOP,
	MENU_PAUSE,
	MENU_REW,
	MENU_FF,
	MENU_LOCATE_START,
	MENU_LOCATE_END,

		// Track editing menus
	MENU_SET_SECTION,			// Set section markers from 
};

enum EMessageIDs {

		// Dialog button IDs
	Apply_ID				= 'appl',
	Close_ID				= 'clos',
	Revert_ID			= 'revt',
	ApplyClose_ID		= 'apcl',

		// Misc control IDs
	ZoomOut_ID			= 'zout',
	ZoomIn_ID			= 'zin ',
	ToolBar_ID			= 'tbar',
	TrackName_ID			= 'tkNm',
	LoseFocus_ID			= 'lfoc',
	Connect_ID				= 'conn',
		// Drag and drop message command
	MeVDragMsg_ID		= 'MeV-',
	
		// Drag and drop message types
	DragTrack_ID			= 'trak',			// drag atrack
	DragEvent_ID			= 'evnt',			// drag an event, such as tempo
};

// ---------------------------------------------------------------------------
// A selection of BMessage command codes which inform the main application
// about various changes in the state of the real-time player



enum EPlayerStateChangeIDs {
	Player_ChangeTransportState		= 'pcts',		// player was stopped, started, etc.
	Player_ChangeTempo				= 'pctm',		// player tempo changed.
};

enum EBitmapIDs {
	SmallPlus_Image		= 128,
	SmallMinus_Image		= 129,
	GridTool_Image		= 130,
	ArrowTool_Image,
	PencilTool_Image,
	EraserTool_Image,
	LockTool_Image,
	SelRectTool_Image,
	TextTool_Image,
	MetrolTool_Image,
	ProgramTool_Image,
	TimeSigTool_Image,
	TrackTool_Image,
	RepeatTool_Image,
	EndTool_Image,
	SysExTool_Image,
	SmallClock_Image,

	SectionMarker_Image	= 160,

	BeginBtn_Image		= 300,
	RewindBtn_Image,
	PlayBtn_Image,
	FFBtn_Image,
	EndBtn_Image,
	StopBtn_Image,
	PauseBtn_Image,
	RecBtn_Image,
	Loop1_Image,
	Loop2_Image,
	PlayBeg_Image,
};

#endif /* __Idents_H__ */
