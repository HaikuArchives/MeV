/* ===================================================================== *
 * MeVApp.h (MeV)
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
 *  MeV application object
 * ---------------------------------------------------------------------
 * History:
 *	1997		Talin
 *		Original implementation
 *	04/08/2000	cell
 *		General cleanup in preparation for initial SourceForge checkin
 *	04/21/2000	cell
 *		Moved the CAboutWindow class into its own source files
 *	05/13/2000  dwalton
 *		Added two default mev soft ports to first time load.
 *  05/13/2000	dwalton
 *		extended CPortAttrsDialog to add support for midi kit2.
 *		including menubeginning midi consumer search.
 *	06/05/2000	malouin
 *		Changed CMeVRefFilter and CImportRefFilter so that
 *		directories are visible in open file panels.
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __C_MeVApp_H__
#define __C_MeVApp_H__

#include "DocApp.h"
#include "WindowState.h"
#include "DynamicMenu.h"
#include "Preferences.h"
#include "VChannel.h"
#include "MeV.h"

class CDocWindow;
class CStripFrameView;
class CScroller;
class CStripView;
class CInspectorWindow;
class CTrackListWindow;
class CTransportWindow;
class CGridWindow;
class CEventTrack;
class EventOp;

const float		Transport_Width = 277.0,
				Transport_Height = 64.0;

// ---------------------------------------------------------------------------
// The main application class

class CTrack;
class MIDIDeviceInfo;

class CMeVApp : public CDocApp {
	
	friend class CInspectorWindow;
	friend class CAboutPluginWindow;
	friend class MeVPlugIn;
	friend class MeVDocRef;
	friend class CTrackWindow;
	friend class CPortAttrsDialog;
	friend class COperatorWindow;
	friend class CDeviceAttrsDialog;
	friend class CPatchAttrsDialog;
	friend struct PatchBank;
	friend class CImportRefFilter;

	struct ExportInfo {
		BMessage			*msg;
		MeVPlugIn		*plugIn;
		char				*menuText;
	};

	Preferences			prefs;
	PreferenceSet 		winSettings,
				 		editSettings,
				 		midiSettings,
				 		vtableSettings;

	CWindowState		trackListState,
						inspectorState,
						gridWinState,
						transportState,
						midiConfigWinState,
						appPrefsWinState,
						aboutPluginWinState;
	BWindow				*deviceAttrsWindow,
						*portAttrsWindow,
						*patchAttrsWindow;
	BList					plugInList;
	BList					defaultOperatorList;
	BList					deviceList;
	BList					patchList;
	VChannelTable			defaultVCTable;

		// Menu lists for plug-ins
	CDynamicMenuDef		assemWindowPlugIns,
						trackWindowPlugIns,
						operWindowPlugIns;
	BList					importerList,
						exporterList;
						
	void MessageReceived( BMessage *inMsg );
	void AboutRequested();
	bool QuitRequested();

	BFilePanel			*exportPanel,
						*importPanel;
	
	static CTrack			*activeTrack;
	BRefFilter				*filter;
	BRefFilter				*importFilter;
	bool					loopFlag;			// Turn looping on/off
	
	MIDIDeviceInfo	*deviceTable[ 16 ][ 16 ];

public:
		/**	Constructor. */
	CMeVApp();
	~CMeVApp();
	
		/**	Create a new document. */
	CDocument *NewDocument( bool inShowWindow = true, entry_ref *inRef = NULL );
	
		/** Import a new document */
	void ImportDocument();

		/** return the address of the tracks list window. */
	CTrackListWindow *TrackList() { return (CTrackListWindow *)trackListState.Window(); }

		/**	Show or hide the tracks list window. */
	void ShowTrackList( bool inShow );
	
		/**	Return the address of the inspector window. */
	CInspectorWindow *Inspector() { return (CInspectorWindow *)inspectorState.Window(); }
	
		/**	Show or hide the inspector window. */
	void ShowInspector( bool inShow );
	
		/**	Return the address of the grid snap window. */
	CGridWindow *GridWindow() { return (CGridWindow *)gridWinState.Window(); }
	
		/**	Show or hide the grid snap window. */
	void ShowGridWindow( bool inShow );
	
		/**	Return the address of the transport window. */
	CTransportWindow *TransportWindow() { return (CTransportWindow *)transportState.Window(); }
	
		/**	Show or hide the transport window. */
	void ShowTransportWindow( bool inShow );
	
		/**	Show the application preferences window. */
	void ShowPrefs();
	
		/**	Show the MIDI Configuration window */
	void ShowMidiConfig();

		/**	Tell the inspector to watch the current event on this track. */
	static void WatchTrack( CEventTrack *inTrack );
	
		/**	Add an operator to the list of default operators. */
	void AddDefaultOperator( EventOp *inOp );

		/**	Return the number of operators. */
	int32 CountOperators() { return defaultOperatorList.CountItems(); }
	
		/**	Return the Nth operator (Increases reference count). */
	EventOp *OperatorAt( int32 index );
	
		/**	Return the Nth operator (Increases reference count). */
	int32 IndexOfOperator( EventOp *op ) { return defaultOperatorList.IndexOf( op ); }
	
		/**	Return pointer to active track. */
	static CTrack *ActiveTrack();

		/** Overrides file panel creation to install the filter */
	virtual BFilePanel *CreateOpenPanel();
	
		/** Returns the number of devices in the instrument table. */
	int32 CountDevices() { return deviceList.CountItems(); }
	
		/** Return information about the Nth MIDI device */
	MIDIDeviceInfo *DeviceAt( int32 index )
	{	
		return (MIDIDeviceInfo *)deviceList.ItemAt( index );
	}

		/** Add a new MIDI device to the device table. */
	MIDIDeviceInfo *NewDevice();

		/** Delete a MIDI device from the device table. */
	void DeleteDevice( MIDIDeviceInfo *inDevInfo );
	
		/** Open a device attrs window. */
	void EditDeviceAttrs( MIDIDeviceInfo *inDevInfo, int32 inPortNum );

		/** Cancel the device attrs window. */
	void CancelEditDeviceAttrs();

		/** Open a port attrs window. */
	void EditPortAttrs( int32 inPortIndex );

		/** Cancel the port attrs window. */
	void CancelEditPortAttrs();
	
		/** Open a patch attrs window. */
	void EditPatchAttrs( MIDIDeviceInfo *inDevice );

		/** Cancel the patch attrs window. */
	void CancelEditPatchAttrs();
	
		/** Return the MIDI device associated with this port and channel */
	MIDIDeviceInfo *LookupInstrument( uint32 port, uint32 hChannel )
	{
		if (port >= Max_MidiPorts || hChannel >= 16) return NULL;
		return deviceTable[ port ][ hChannel ];
	}
	
		/** Rebuild the device table */
	void CalcDeviceTable();
	
		/* Encode device preference information into a BMessage. (Memento) */
	void GetDevicePrefs( BMessage *msg );

		/* Decode device preference information from a BMessage. (Memento) */
	void SetDevicePrefs( BMessage *msg );

		/** Creates a file panel for import operations. */
	BFilePanel *GetImportPanel( BMessenger *msngr );

		/** Creates a file panel for export operations. */
	BFilePanel *GetExportPanel( BMessenger *msngr );

		/**	Get the state of the "loop" flag. */
	bool GetLoopFlag() { return loopFlag; }
	
		/** Set the state of the "loop" flag. */
	void SetLoopFlag( bool inLoop ) { loopFlag = inLoop; }
	
		/** Get the default Virtual Channel table. */
	void GetDefaultVCTable( VChannelTable &outTable )
	{
		memcpy( outTable, defaultVCTable, sizeof outTable );
	}
		
		/** Set the default Virtual Channel table. */
	void SetDefaultVCTable( VChannelTable &inTable );
	
		/** Build the export menu */
	void BuildExportMenu( BMenu *inMenu );
};

// ---------------------------------------------------------------------------
// Window about plug-ins

class CAboutPluginWindow : public CAppWindow {
	BListView		*pList;
	BTextView		*textView;

	void MessageReceived( BMessage *msg );
public:
	CAboutPluginWindow( CWindowState &inState );
};

// ---------------------------------------------------------------------------
// Global preferences

struct CGlobalPrefs {
	enum FeedbackTypes {
		FB_Pitch			= 1<<0,
		FB_Velocity		= 1<<1,
		FB_Program		= 1<<2,
		FB_Control		= 1<<3,
		FB_Channel		= 1<<4
	};

	uint32				feedbackDragMask,
						feedbackAdjustMask;
	uint32				feedbackDelay;
	
	bool					inclusiveSelection;

	uint8				firstMeasureNumber;		// first measure is 0 or 1
	
	int16				appPrefsPanel,				// which app prefs panel is selected,
						lEditorPrefsPanel;			// which LEditor prefs panel is selected.
	
	bool FeedbackEnabled( int32 inAttribute, bool inDrag = true );
};

extern CGlobalPrefs		gPrefs;

#endif /* __C_MeVApp_H__ */
