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

#include "AppWindow.h"
#include "DocApp.h"
#include "WindowState.h"
#include "DynamicMenu.h"
#include "Preferences.h"
#include "Destination.h"

class CDocWindow;
class CStripFrameView;
class CScroller;
class CStripView;
class CInspectorWindow;
class MIDIDeviceInfo;
class CTrack;
class CTrackListWindow;
class CTransportWindow;
class CGridWindow;
class CEventTrack;
class EventOp;

const float		Transport_Width = 277.0,
				Transport_Height = 69.0;

class CMeVApp
	:	public CDocApp
{
	// +++ far too many friends here !!!
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

public:							// Constants

	enum messages
	{
								IMPORT_REQUESTED = 'mevA',

								EXPORT_REQUESTED
	};

public:							// Constructor/Destructor

								CMeVApp();

								~CMeVApp();

public:							// Accessors

	/**	Return pointer to active track. */
	static CTrack *				ActiveTrack();

	/**	Get/set the state of the "loop" flag. */
	bool						GetLoopFlag()
								{ return loopFlag; }
	void						SetLoopFlag(
									bool loop)
								{ loopFlag = loop; }
	
public:							// Operations

	void						HelpRequested();

	/**	Tell the inspector to watch the current event on this track. */
	static void					WatchTrack(
									CEventTrack *track);
	
public:							// Device Management

	/** Returns the number of devices in the instrument table. */
	int32						CountDevices() const
								{ return deviceList.CountItems(); }
	
	/** Return information about the Nth MIDI device */
	MIDIDeviceInfo *			DeviceAt(
									int32 index) const
								{ return (MIDIDeviceInfo *)deviceList.ItemAt(index); }

	/** Return the MIDI device associated with this port and channel */
	MIDIDeviceInfo *			LookupInstrument(
									uint32 port,
									uint32 hChannel) const;

	/** Add a new MIDI device to the device table. */
	MIDIDeviceInfo *			NewDevice();

public:							// Operator Management

	/**	Add an operator to the list of default operators. */
	void						AddDefaultOperator(
									EventOp *inOp);

	/**	Return the number of operators. */
	int32						CountOperators() const
								{ return defaultOperatorList.CountItems(); }
	
	/**	Return the Nth operator (Increases reference count). */
	EventOp *					OperatorAt(
									int32 index);
	
	/**	Return the Nth operator (Increases reference count). */
	int32						IndexOfOperator(
									EventOp *op)
								{ return defaultOperatorList.IndexOf(op); }

public:							// Window Management

	/** return the address of the tracks list window. */
	CTrackListWindow *			TrackList() const
								{ return (CTrackListWindow *)trackListState.Window(); }

	/**	Show or hide the tracks list window. */
	void						ShowTrackList(
									bool show);

	/**	Return the address of the inspector window. */
	CInspectorWindow *			Inspector() const
								{ return (CInspectorWindow *)inspectorState.Window(); }

	/**	Show or hide the inspector window. */
	void						ShowInspector(
									bool show);

	/**	Return the address of the grid snap window. */
	CGridWindow *				GridWindow() const
								{ return (CGridWindow *)gridWinState.Window(); }

	/**	Show or hide the grid snap window. */
	void						ShowGridWindow(
									bool show);

	/**	Return the address of the transport window. */
	CTransportWindow *			TransportWindow() const
								{ return (CTransportWindow *)transportState.Window(); }
	
	/**	Show or hide the transport window. */
	void						ShowTransportWindow(
									bool show);

	/**	Show the application preferences window. */
	void						ShowPrefs();
	
public:							// File Import/Export

	/** Creates a file panel for import operations. */
	BFilePanel *				GetImportPanel(
									BMessenger *messenger);

	/** Creates a file panel for export operations. */
	BFilePanel *				GetExportPanel(
									BMessenger *messenger);

	/** Import a new document */
	void						ImportDocument();
	void						HandleImport(
									entry_ref *ref);

public:							// CDocApp Implementation

	virtual void				AboutRequested();

	/** Overrides file panel creation to install the ref filter */
	virtual BFilePanel *		CreateOpenPanel();
	
	virtual void				MessageReceived(
									BMessage *message);

	/**	Create a new document. */
	virtual CDocument *			NewDocument(
									bool showWindow = true,
									entry_ref *ref = NULL);
	
	virtual void				RefsReceived(
									BMessage *message);

	virtual bool				QuitRequested();

private:						// Internal Operations

	/** Build the export menu */
	void						BuildExportMenu(
									BMenu *menu);

	/** called at launch, loads all the add-ons it can find. */
	void						LoadAddOns();

	/** Install the document mime type if not already done. */
	void						UpdateMimeDatabase();

private:						// Instance Data

	Preferences					prefs;
	PreferenceSet 				winSettings,
				 				editSettings,
				 				midiSettings,
				 				vtableSettings;

	CWindowState				trackListState,
								inspectorState,
								gridWinState,
								transportState,
								appPrefsWinState,
								aboutPluginWinState;

	BList						m_plugins;

	BList						defaultOperatorList;

	BList						deviceList;

	/** Menu lists for plug-ins */
//	CDynamicMenuDef				assemWindowPlugIns,
//								trackWindowPlugIns,
//								operWindowPlugIns;
	BList						importerList,
								exporterList;

	BFilePanel *				m_exportPanel;
	BFilePanel *				m_importPanel;
	
	static CTrack *				activeTrack;
	BRefFilter *				filter;
	BRefFilter *				m_importFilter;

	/** Turn looping on/off */
	bool						loopFlag;

	MIDIDeviceInfo *			deviceTable[16][16];
};

// ---------------------------------------------------------------------------
// Window about plug-ins

class CAboutPluginWindow
	:	public CAppWindow
{

public:							// Constants

	enum messages
	{
								PLUGIN_SELECTED = 'apwA'
	};

public:							// Constructor/Destructor

								CAboutPluginWindow(
									CWindowState &state);

public:							// CAppWindow Implementation

	virtual void				MessageReceived(
									BMessage *message);

	virtual void				SubjectReleased(
									CObservable *subject)
								{ }

private:						// Instance Data

	BListView *					pList;

	BTextView *					textView;
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
