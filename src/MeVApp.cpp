/* ===================================================================== *
 * MeVApp.cpp (MeV)
 * ===================================================================== */

#include "MeVApp.h"

#include "AssemblyWindow.h"
#include "BeFileReader.h"
#include "CursorCache.h"
#include "DocWindow.h"
#include "EventOp.h"
#include "EventTrack.h"
#include "GridWindow.h"
#include "Idents.h"
#include "InspectorWindow.h"
#include "LinearWindow.h"
#include "MidiModule.h"
#include "MeV.h"
#include "MeVDoc.h"
#include "MeVModule.h"
#include "MeVPlugin.h"
#include "PlayerControl.h"
#include "PreferencesWindow.h"
#include "ScreenUtils.h"
#include "StripFrameView.h"
#include "TrackListWindow.h"
#include "TransportWindow.h"

// Gnu C Library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// Application Kit
#include <Roster.h>
// Interface Kit
#include <Alert.h>
#include <Box.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <ScrollView.h>
#include <TextView.h>
// Storage Kit
#include <AppFileInfo.h>
#include <FilePanel.h>
#include <NodeInfo.h>
#include <Path.h>
// Support Kit
#include <Debug.h>

// Debugging Macros
#define D_ALLOC(x) //PRINT(x)			// Constructor/Destructor
#define D_MODULE(x) //PRINT(x)			// Module Management
#define D_WINDOW(x) //PRINT(x)			// Window Management

#define TRACKLIST_NAME		"Part List"
#define INSPECTOR_NAME		"Inspector"
#define GRID_NAME			"Grid"
#define TRANSPORT_NAME		"Transport"
#define APP_PREFS_NAME		"Prefs"
#define MIDI_CONFIG_NAME	"Midi Config"
#define ABOUT_NAME			"About Box"
#define ABOUT_PI_NAME		"About Plug-ins"

CGlobalPrefs				gPrefs;
char						gPlugInName[B_FILE_NAME_LENGTH];

CTrack *
CMeVApp::activeTrack;

int main(int argc, char *argv[])
{
	CMeVApp	app;
	app.Run();

	return 0;
}

static bool ReadWindowState( BMessage &msg, const char *prefsName, CWindowState &ioState, bool defaultOpen )
{
	BPoint		p;
	bool			b;
	char			oName[ 48 ];
	
	sprintf( oName, "%s Open", prefsName );

	if (msg.FindPoint( prefsName, &p ) == B_OK)
	{
		ioState.SetPos( p );
	}

	if (msg.FindBool( oName, &b ) == B_OK)
	{
		return b;
	}
	else return defaultOpen;
}

static void WriteWindowState( BMessage &msg, const char *prefsName, CWindowState &wState )
{
	char			oName[ 48 ];

	sprintf( oName, "%s Open", prefsName );

	msg.RemoveName( prefsName );
	msg.AddPoint( prefsName, wState.Pos() );

	msg.RemoveName( oName );
	msg.AddBool( oName, wState.IsOpen() );
}

class CMeVRefFilter
	:	public BRefFilter
{

public:						// BRefFilter Implementation

	bool					Filter(
								const entry_ref *ref,
								BNode *node,
								struct stat *st,
								const char *filetype)
							{ return (strstr(filetype, "MeV") != NULL || node->IsDirectory()); }
};

class CImportRefFilter
	:	public BRefFilter
{

public:						// BRefFilter Implementation

	bool					Filter(
								const entry_ref *ref,
								BNode *node,
								struct stat *st,
								const char *filetype);
};

bool
CImportRefFilter::Filter(
	const entry_ref *ref,
	BNode *node,
	struct stat *st,
	const char *fileType)
{
	BList &list = ((CMeVApp *)be_app)->importerList;

	for (int i = 0; i < list.CountItems(); i++)
	{
		MeVPlugIn *plugin = (MeVPlugIn *)list.ItemAt(i);
		if (node->IsDirectory()
		 || (plugin->DetectFileType(ref, node, st, fileType) >= 0))
			return true;
	}
	return false;
}

// ---------------------------------------------------------------------------
// Constructor/Destructor

CMeVApp::CMeVApp()
	:	CDocApp("application/x-vnd.BeUnited.MeV"),
		prefs( "x-vnd.BeUnited.MeV" ),
		winSettings( prefs, "settings/windows", true ),
		editSettings( prefs, "settings/edit", true ),
		midiSettings( prefs, "settings/midi", true ),
		vtableSettings( prefs, "settings/vtable", true ),
		trackListState(BRect(620.0, 100.0,
							 CTrackListWindow::DEFAULT_DIMENSIONS.Width(),
							 CTrackListWindow::DEFAULT_DIMENSIONS.Height())),
		inspectorState(BRect(50.0, 460.0,
							 CInspectorWindow::DEFAULT_DIMENSIONS.Width(),
							 CInspectorWindow::DEFAULT_DIMENSIONS.Height())),
		gridWinState(BRect(0.0 + CInspectorWindow::DEFAULT_DIMENSIONS.Width(),
						   406.0 - CGridWindow::DEFAULT_DIMENSIONS.Height(),
						   CGridWindow::DEFAULT_DIMENSIONS.Width(),
						   CGridWindow::DEFAULT_DIMENSIONS.Height())),
		transportState(BRect(540.0, 462.0, Transport_Width, Transport_Height)),
		appPrefsWinState(BRect(40, 40, 500, 300)),
		aboutPluginWinState(BRect(80, 80, 450, 250))
{
	// Check if the MIME types are installed
	UpdateMimeDatabase();

	bool trackListOpen = true,
		 inspectorOpen = true,
		 gridWindowOpen = false,
		 transportOpen = true,
		 appPrefsOpen = false,
		 aboutPlugOpen = false;

	CEvent::InitTables();
	activeTrack = NULL;
	filter = new CMeVRefFilter;
	m_importFilter = new CImportRefFilter;
	loopFlag = false;
	
	// Initialize default global prefs
	gPrefs.feedbackDragMask = ULONG_MAX;
	gPrefs.feedbackAdjustMask = ULONG_MAX;
	gPrefs.feedbackDelay = 50;
	gPrefs.inclusiveSelection = true;
	gPrefs.firstMeasureNumber = 1;
	gPrefs.appPrefsPanel = 0;
	gPrefs.lEditorPrefsPanel = 0;
	
	// Initialize import/export file panels
	m_exportPanel = NULL;
	m_importPanel = NULL;

	AddModule(Midi::CMidiModule::Instance());

	CPlayerControl::InitPlayer();

	LoadAddOns();
	
	// Load in application preferences
	if (!winSettings.InitCheck())
	{
		D_ALLOC((" -> retrieve window settings...\n"));

		BMessage &prefMessage = winSettings.GetMessage();
		trackListOpen = ReadWindowState(prefMessage, TRACKLIST_NAME,
										trackListState, true);
		inspectorOpen = ReadWindowState(prefMessage, INSPECTOR_NAME,
										inspectorState, true);
		gridWindowOpen = ReadWindowState(prefMessage, GRID_NAME,
										 gridWinState, false);
		transportOpen = ReadWindowState(prefMessage, TRANSPORT_NAME,
										transportState, true);
		appPrefsOpen = ReadWindowState(prefMessage, APP_PREFS_NAME,
									   appPrefsWinState, false);
		aboutPlugOpen = ReadWindowState(prefMessage, ABOUT_PI_NAME,
										aboutPluginWinState, false);
	}

	if (!editSettings.InitCheck())
	{
		BMessage		&prefMessage = editSettings.GetMessage();
		int32		v;
		bool			b;
		
		if (prefMessage.FindInt32( "Feedback Delay", &v ) == B_OK)
			gPrefs.feedbackDelay = v;
		
		if (prefMessage.FindInt32( "Feedback Drag Mask", &v ) == B_OK)
			gPrefs.feedbackDragMask = v;
		
		if (prefMessage.FindInt32( "Feedback Adjust Mask", &v ) == B_OK)
			gPrefs.feedbackAdjustMask = v;
		
		if (prefMessage.FindBool( "Inclusive Select", &b ) == B_OK)
			gPrefs.inclusiveSelection = b;
		
		if (prefMessage.FindBool( "Start At Measure 1", &b ) == B_OK)
			gPrefs.firstMeasureNumber = b ? 1 : 0;

		if (prefMessage.FindBool( "Loop", &b ) == B_OK)
			loopFlag = b ? 1 : 0;

		if (prefMessage.FindInt32( "APrefsPanel", &v ) == B_OK)
			gPrefs.appPrefsPanel = v;

		if (prefMessage.FindInt32( "LEPrefsPanel", &v ) == B_OK)
			gPrefs.lEditorPrefsPanel = v;
	}

	// Load in application preferences
	if (!midiSettings.InitCheck())
	{
		// +++ what goes in here ?
	}
	
	if (trackListOpen)
		ShowTrackList(true);
	if (inspectorOpen)
		ShowInspector(true);
	if (gridWindowOpen)
		ShowGridWindow(true);
	if (transportOpen)
		ShowTransportWindow(true);
	if (appPrefsOpen)
		ShowPrefs();
}

CMeVApp::~CMeVApp()
{
	delete filter;
	delete m_importFilter;

	CCursorCache::Release();

	if (!editSettings.InitCheck())
	{
		BMessage		&prefMessage = editSettings.GetMessage();
		
		prefMessage.RemoveName( "Feedback Delay" );
		prefMessage.AddInt32( "Feedback Delay", gPrefs.feedbackDelay );
		
		prefMessage.RemoveName( "Feedback Drag Mask" );
		prefMessage.AddInt32( "Feedback Drag Mask", gPrefs.feedbackDragMask );
		
		prefMessage.RemoveName( "Feedback Adjust Mask" );
		prefMessage.AddInt32( "Feedback Adjust Mask", gPrefs.feedbackAdjustMask );
		
		prefMessage.RemoveName( "Inclusive Select" );
		prefMessage.AddBool( "Inclusive Select", gPrefs.inclusiveSelection );
		
		prefMessage.RemoveName( "Start At Measure 1" );
		prefMessage.AddBool( "Start At Measure 1", gPrefs.firstMeasureNumber != 0 );

		prefMessage.RemoveName( "Loop" );
		prefMessage.AddBool( "Loop", loopFlag != 0 );

		prefMessage.RemoveName( "APrefsPanel" );
		prefMessage.AddInt32( "APrefsPanel", gPrefs.appPrefsPanel );
		
		prefMessage.RemoveName( "LEPrefsPanel" );
		prefMessage.AddInt32( "LEPrefsPanel", gPrefs.lEditorPrefsPanel );
		
		editSettings.Save();
	}

	if (!midiSettings.InitCheck())
	{
		midiSettings.Save();
	}

	for (int i = 0; i < defaultOperatorList.CountItems(); i++)
	{
		EventOp		*op = (EventOp *)defaultOperatorList.ItemAt( i );

		CRefCountObject::Release( op );
	}

	for (int i = 0; i < deviceList.CountItems(); i++)
	{
	}

	RemoveModule('MIDI');
}

// ---------------------------------------------------------------------------
// Accessors

CTrack *
CMeVApp::ActiveTrack()
{
	return activeTrack;
}

// ---------------------------------------------------------------------------
// Operations

void
CMeVApp::HelpRequested()
{
	app_info appInfo;
	if (GetAppInfo(&appInfo) != B_OK)
		return;

	BEntry entry(&appInfo.ref);
	BPath path;
	if ((entry.GetPath(&path) != B_OK)
	 || (path.GetParent(&path) != B_OK))
		return;

	path.Append("docs/index.html");
	entry.SetTo(path.Path());
	if (entry.InitCheck() != B_OK)
		return;

	entry_ref ref;
	entry.GetRef(&ref);
	be_roster->Launch(&ref);
}

void
CMeVApp::WatchTrack(
	CEventTrack *track)
{
	CMeVApp *app = (CMeVApp *)be_app;
	CAppWindow *window;

	if (activeTrack == track)
		// nothing to do
		return;
	
	activeTrack = track;

	BMessage message(WATCH_TRACK);
	message.AddPointer("mev:track", (void *)track);
	BMessenger messenger;

	app->trackListState.Lock();
	messenger = BMessenger(NULL, app->trackListState.Window());
	messenger.SendMessage(&message);
	app->trackListState.Unlock();

	app->inspectorState.Lock();
	messenger = BMessenger(NULL, app->inspectorState.Window());
	messenger.SendMessage(&message);
	app->inspectorState.Unlock();

	app->gridWinState.Lock();
	messenger = BMessenger(NULL, app->gridWinState.Window());
	messenger.SendMessage(&message);
	app->gridWinState.Unlock();

	app->transportState.Lock();
	messenger = BMessenger(NULL, app->transportState.Window());
	messenger.SendMessage(&message);
	app->transportState.Unlock();
}

// ---------------------------------------------------------------------------
// Module Management

void
CMeVApp::AddModule(
	CMeVModule *module)
{
	D_MODULE(("CMeVApp::AddModule()\n"));

	m_modules[module->Type()] = module;
}

CMeVModule *
CMeVApp::ModuleFor(
	unsigned long type) const
{
	D_MODULE(("CMeVApp::ModuleFor(%ld)\n", type));

	module_map::const_iterator i = m_modules.find(type);
	if (i != m_modules.end())
		return i->second;
	else
		return NULL;
}

void
CMeVApp::RemoveModule(
	uint32 type)
{
	D_MODULE(("CMeVApp::RemoveModule(%ld)\n", type));

	CMeVModule *module = ModuleFor(type);
	if (module != NULL)
	{
		module->Lock();
		module->Quit();
		m_modules.erase(type);
	}
}

// ---------------------------------------------------------------------------
// Window Management

void
CMeVApp::ShowTrackList( bool inShow )
{
	trackListState.Lock();

	if (inShow)
	{
		if (!trackListState.Activate())
		{
			CTrackListWindow *window;
			window = new CTrackListWindow(trackListState.Rect().LeftTop(),
										  trackListState);
			window->Show();
			if (activeTrack != NULL)
			{
				BMessenger messenger(NULL, window);
				BMessage message(WATCH_TRACK);
				message.AddPointer("mev:track", (void *)activeTrack);
				messenger.SendMessage(&message);
			}
		}
	}
	else
	{
		trackListState.Close();
	}

	trackListState.Unlock();
}

void
CMeVApp::ShowInspector(
	bool show)
{
	inspectorState.Lock();

	if (show)
	{
		if (!inspectorState.Activate())
		{
			CInspectorWindow *window;
			window = new CInspectorWindow(inspectorState.Rect().LeftTop(),
										  inspectorState);
			window->Show();
			if (activeTrack != NULL)
			{
				BMessenger messenger(NULL, window);
				BMessage message(WATCH_TRACK);
				message.AddPointer("mev:track", (void *)activeTrack);
				messenger.SendMessage(&message);
			}
		}
	}
	else
	{
		inspectorState.Close();
	}
	
	inspectorState.Unlock();
}

void
CMeVApp::ShowGridWindow(
	bool show)
{
	gridWinState.Lock();

	if (show)
	{
		if (!gridWinState.Activate())
		{
			CGridWindow *window;
			window = new CGridWindow(gridWinState.Rect().LeftTop(),
									 gridWinState);
			window->Show();
			if (activeTrack != NULL)
			{
				BMessenger messenger(NULL, window);
				BMessage message(WATCH_TRACK);
				message.AddPointer("mev:track", (void *)activeTrack);
				messenger.SendMessage(&message);
			}
		}
	}
	else
	{
		gridWinState.Close();
	}
	
	gridWinState.Unlock();
}

void
CMeVApp::ShowTransportWindow(
	bool show)
{
	D_WINDOW(("CMeVApp::ShowTransportWindow(%s)\n",
			  show ? "true" : "false"));

	transportState.Lock();

	if (show)
	{
		if (!transportState.Activate())
		{
			CTransportWindow *window;
			window = new CTransportWindow(transportState.Rect().LeftTop(),
										  transportState);
			window->Show();
			if (activeTrack != NULL)
			{
				BMessenger messenger(NULL, window);
				BMessage message(WATCH_TRACK);
				message.AddPointer("mev:track", (void *)activeTrack);
				messenger.SendMessage(&message);
			}
		}
	}
	else
	{
		transportState.Close();
	}
	
	transportState.Unlock();
}

void CMeVApp::ShowPrefs()
{
	appPrefsWinState.Lock();
	if (!appPrefsWinState.Activate())
	{
		BWindow *window = new CPreferencesWindow(appPrefsWinState);
		window->Show();
	}
	appPrefsWinState.Unlock();
}

// ---------------------------------------------------------------------------
// File Import/Export

BFilePanel *
CMeVApp::GetImportPanel(
	BMessenger *messenger)
{
	if (m_importPanel == NULL)
	{
		// Create a new import panel
		m_importPanel = new BFilePanel(B_OPEN_PANEL, messenger, NULL,
									   B_FILE_NODE, false);
		m_importPanel->SetButtonLabel(B_DEFAULT_BUTTON, "Import");
		m_importPanel->Window()->SetTitle("MeV: Import");
	}
	else if (m_importPanel->IsShowing())
	{
		return NULL;
	}

	return m_importPanel;
}

BFilePanel *
CMeVApp::GetExportPanel(
	BMessenger *msngr)
{
	if (m_exportPanel == NULL)
	{
		// Create a new export panel
		m_exportPanel = new BFilePanel(B_SAVE_PANEL, msngr, NULL, B_FILE_NODE,
									   false);
		m_exportPanel->SetButtonLabel(B_DEFAULT_BUTTON, "Export");
		m_exportPanel->Window()->SetTitle("MeV: Export");
	}
	else if (m_exportPanel->IsShowing())
		return NULL;

	m_exportPanel->SetTarget(*msngr);
	m_exportPanel->SetRefFilter(NULL);
	return m_exportPanel;
}

void
CMeVApp::ImportDocument()
{
	BFilePanel *filePanel = GetImportPanel(&be_app_messenger);

	filePanel->SetMessage(new BMessage(IMPORT_REQUESTED));
//	filePanel->SetRefFilter(m_importFilter);
	filePanel->Show();
}

void
CMeVApp::HandleImport(
	entry_ref *ref)
{
	// See if there's a plug-in which can identify it...
	for (int i = 0; i < importerList.CountItems(); i++)
	{
		MeVPlugIn *plugin = (MeVPlugIn *)importerList.ItemAt(i);
		int32 kind;
		struct stat	st;
		char filetype[B_MIME_TYPE_LENGTH];

		BNode node(ref);
		if (node.InitCheck() != B_NO_ERROR)
			continue;

		BNodeInfo ni(&node);
		if (ni.InitCheck() != B_NO_ERROR)
			continue;
		if (ni.GetType(filetype) != B_NO_ERROR)
			continue;
		if (node.GetStat(&st) != B_NO_ERROR)
			continue;
	
		kind = plugin->DetectFileType(ref, &node, &st, 
									  filetype);
		if (kind < 0)
			continue;

		plugin->OnImport(NULL, ref, kind);
		break;
	}
}

// ---------------------------------------------------------------------------
// Operator Management

void
CMeVApp::AddDefaultOperator(
	EventOp *inOp)
{
	Lock();

	if (!defaultOperatorList.HasItem( inOp ))
	{
		inOp->Acquire();
		defaultOperatorList.AddItem( inOp );
	}
	
	Unlock();
}

EventOp *
CMeVApp::OperatorAt(
	int32 index)
{
	void		*ptr = defaultOperatorList.ItemAt( index );
	
	if (ptr) return (EventOp *)((EventOp *)ptr)->Acquire();
	return NULL;
}

// ---------------------------------------------------------------------------
// CDocApp Implementation

void
CMeVApp::AboutRequested()
{
	BString aboutText = "MeV (Musical Environment)";

	app_info appInfo;
	if (GetAppInfo(&appInfo) == B_OK)
	{
		BFile appFile(&appInfo.ref, B_READ_ONLY);
		BAppFileInfo appFileInfo(&appFile);
		version_info versionInfo;
		if (appFileInfo.GetVersionInfo(&versionInfo, B_APP_VERSION_KIND) == B_OK)
			aboutText << " " << versionInfo.short_info;
	}
	aboutText << "\nBuild Date: " << __DATE__ << "\n";

	// add url
	aboutText << "\nThe MeV Homepage:\n";
	aboutText << "http://mev.sourceforge.net/\n";

	// add team info
	aboutText << "\nThe MeV Team:\n";
	aboutText << "Christopher Lenz, ";
	aboutText << "Curt Malouin, ";
	aboutText << "Dan Walton, ";
	aboutText << "Eric Moon, ";
	aboutText << "Jamie Krutz, ";
	aboutText << "Talin\n ";

	// add copyright & legal stuff
	aboutText << "\nThis Software is distributed under the "
				 "Mozilla Public License 1.1 on an 'AS IS' "
				 "basis, WITHOUT WARRANTY OF ANY KIND, either "
				 "express or implied. See the License for the "
				 "specific language governing rights and limitations "
				 "under the License.\n";

	BAlert *alert = new BAlert("About MeV", aboutText.String(), "OK");
	alert->Go(0);
}

BFilePanel *
CMeVApp::CreateOpenPanel()
{
	BFilePanel		*p = CDocApp::CreateOpenPanel();
	
	p->SetRefFilter( filter );
	return p;
}

void
CMeVApp::MessageReceived(
	BMessage *message)
{
	switch (message->what)
	{
		case MENU_HELP:
		{
			HelpRequested();
			break;
		}
		case MENU_ABOUT_PLUGINS:
		{
			CAboutPluginWindow *window = new CAboutPluginWindow(aboutPluginWinState);
			window->Show();
			break;
		}
		case MENU_PROGRAM_SETTINGS:
		{
			ShowPrefs();
			break;
		}
		case Player_ChangeTransportState:
		{
			// If there's a transport window, forward this message to it.
			transportState.Lock();
			BWindow *window = transportState.Window();
			if (window)
				window->PostMessage(message);
			transportState.Unlock();
			break;
		}		
		case MENU_NEW:
		{
			NewDocument();
			break;
		}
		case MENU_OPEN:
		{
			OpenDocument();
			break;
		}
		case MENU_IMPORT:
		{
			ImportDocument();
			break;
		}
		case IMPORT_REQUESTED:
		{
			int32 refCount;
			type_code type;
			
			// For each reference
			if (message->GetInfo("refs", &type, &refCount) == B_NO_ERROR)
			{
				for (int j = 0; j < refCount; j++)
				{
					entry_ref ref;
					if (message->FindRef("refs", j, &ref) == B_NO_ERROR)
					{
						HandleImport(&ref);
					}
				}
			}
			break;
		}
		case EXPORT_REQUESTED:
		{
			CMeVDoc *doc = NULL;
			entry_ref saveRef;
			BString saveName;
			BMessage pluginMsg;
			MeVPlugIn *plugin;
			if ((message->FindPointer("Document" , (void **)&doc) == B_OK)
			 && (message->FindRef("directory" , &saveRef) == B_OK)
			 && (message->FindString("name" , &saveName) == B_OK)
			 && (message->FindPointer("plugin", (void **)&plugin) == B_OK)
			 && (message->FindMessage("msg", &pluginMsg) == B_OK))
			{
				entry_ref ref;
				BDirectory dir(&saveRef);
				BEntry entry(&dir, saveName.String());
				if (entry.GetRef(&ref) == B_NO_ERROR)
					plugin->OnExport(&pluginMsg, (int32)doc, &ref);
			}
			break;
		}
		default:
		{
			CDocApp::MessageReceived(message);
		}
	}
}

CDocument *
CMeVApp::NewDocument(
	bool showWindow,
	entry_ref *ref)
{
	CMeVDoc *doc;

	if (ref)
	{
		BFile file(ref, B_READ_ONLY);
		status_t error = file.InitCheck();
		if (error)
		{
			const char *msg;
			switch (error)
			{
				case B_BAD_VALUE:
				{
					msg = "The directory or path name you specified was invalid.";
					break;
				}
				case B_ENTRY_NOT_FOUND:
				{
					msg = "The file could not be found. Please check the spelling of the directory and file names.";
					break;
				}
				case B_PERMISSION_DENIED:
				{
					msg = "You do not have permission to read that file.";
					break;
				}
				case B_NO_MEMORY:
				{
					msg = "There was not enough memory to complete the operation.";
					break;
				}
				default:
				{
					msg = "An error has been detected of a type never before encountered, Captain.";
				}
			}		
			CDocApp::Error(msg);
			return NULL;
		}

		// Create reader and IFF reader.
		CBeFileReader reader(file);
		CIFFReader iffReader(reader);
		doc = new CMeVDoc(this, *ref, iffReader);
	}
	else
	{
		doc = new CMeVDoc(this);
	}

	// If document did not initialize OK, then fail.
	if (!doc->InitCheck())
	{
		delete doc;
		return NULL;
	}

	if (showWindow)
		doc->ShowWindow(CMeVDoc::ASSEMBLY_WINDOW);

	return doc;
}

void
CMeVApp::RefsReceived(
	BMessage *message)
{ 
	uint32 type;
	int32 count;
	message->GetInfo("refs", &type, &count);
	if (type == B_REF_TYPE)
	{
		for (int32 i = 0; i < count; i++)
		{
			entry_ref ref;
			if (message->FindRef("refs", i, &ref) == B_OK)
			{
				BEntry entry(&ref, true);
				entry.GetRef(&ref);

				BNode node(&ref);
				if (node.InitCheck() != B_OK)
					return;
				BNodeInfo nodeInfo(&node);
				if (nodeInfo.InitCheck() != B_OK)
					return;
				char mimeString[B_MIME_TYPE_LENGTH];
				if (nodeInfo.GetType(mimeString) == B_OK)
				{
					BMimeType mimeType(mimeString);
					if (mimeType == *CMeVDoc::MimeType())
					{
						CDocApp::RefsReceived(message);
					}
					else
					{
						HandleImport(&ref);
					}
				}
			}
		}
	}
}

bool
CMeVApp::QuitRequested()
{
	if (!CDocApp::QuitRequested())
		return false;

	// save out application preferences before windows close.
	if (!winSettings.InitCheck())
	{
		BMessage &prefMessage = winSettings.GetMessage();

		WriteWindowState(prefMessage, TRACKLIST_NAME, trackListState );
		WriteWindowState(prefMessage, INSPECTOR_NAME, inspectorState );
		WriteWindowState(prefMessage, GRID_NAME, gridWinState );
		WriteWindowState(prefMessage, TRANSPORT_NAME, transportState );
		WriteWindowState(prefMessage, APP_PREFS_NAME, appPrefsWinState );
		WriteWindowState(prefMessage, ABOUT_PI_NAME, aboutPluginWinState );
		winSettings.Save();
	}
	return true;
}

// ---------------------------------------------------------------------------
// Internal Operations

void
CMeVApp::BuildExportMenu(
	BMenu *menu)
{
	for (int i = 0; i < exporterList.CountItems(); i++)
	{
		ExportInfo *ei = (ExportInfo *)exporterList.ItemAt(i);
		BMessage *msg = new BMessage(MENU_EXPORT);

		msg->AddPointer("plugin", ei->plugIn);
		msg->AddMessage("msg", ei->msg);

		menu->AddItem(new BMenuItem(ei->menuText, msg));
	}
}

void
CMeVApp::LoadAddOns()
{
	// get application info
	app_info appInfo;
	if (GetAppInfo(&appInfo) != B_OK)
		return;

	// get application parent directory
	BDirectory appDir;
	BEntry appEntry(&appInfo.ref);
	if ((appEntry.InitCheck() != B_OK)
	 || (appEntry.GetParent(&appDir) != B_OK))
	 	return;

	// get add-ons directory inside app directory
	BEntry addOnDirEntry;
	if (appDir.FindEntry("add-ons", &addOnDirEntry, true) != B_OK)
		return;
	BDirectory addOnDir(&addOnDirEntry);	

	// iterate through the files in that dir, and load add-ons
	if (addOnDir.InitCheck() == B_NO_ERROR)
	{
		BEntry entry;	
		while (addOnDir.GetNextEntry(&entry) == B_OK)
		{
			BPath path;
			image_id image;
			MeVPlugIn *plugin;
			MeVPlugIn *(*func_create)();

			entry.GetPath(&path);
			image = load_add_on(path.Path());
			if (!image)
				continue;
			
			entry.GetName(gPlugInName);

			if (get_image_symbol(image, "CreatePlugin",
								 B_SYMBOL_TYPE_TEXT,
								 (void **)&func_create) == B_OK)
			{
				plugin = (func_create)();
 				m_plugins.AddItem(plugin);
			}
		}
	}
}

void
CMeVApp::UpdateMimeDatabase()
{
	BMimeType *docType = CMeVDoc::MimeType();
	if (!docType->IsInstalled())
		docType->Install();

	delete docType;
}

// ---------------------------------------------------------------------------
// Class: CAboutPluginWindow

CAboutPluginWindow::CAboutPluginWindow(
	CWindowState &inState)
	:	CAppWindow(inState, inState.Rect(), "About MeV Plug-Ins", B_TITLED_WINDOW, B_NOT_RESIZABLE | B_NOT_ZOOMABLE)
{
	BRect		r( Frame() );
	BView		*background;
	CMeVApp		*app = (CMeVApp *)be_app;
	BBox			*bb;

	r.OffsetTo( B_ORIGIN );

	background = new BView( r, NULL, B_FOLLOW_ALL_SIDES, B_WILL_DRAW );
	
	AddChild( background );
	background->SetViewColor( 220, 220, 220 );
	
	r.InsetBy( 7.0, 7.0 );

	pList = new BListView(	BRect(	r.left, r.top + 1,
									140 - B_V_SCROLL_BAR_WIDTH, r.bottom - 1 ),
									NULL, B_SINGLE_SELECTION_LIST, B_FOLLOW_ALL );
	
	pList->SetSelectionMessage( new BMessage(PLUGIN_SELECTED) );
	
	BScrollView *sv = new BScrollView(	NULL,
										pList,
										B_FOLLOW_TOP_BOTTOM | B_FOLLOW_LEFT,
										0,
										false, true,
										B_PLAIN_BORDER );
	background->AddChild( sv );
	
	for (int i = 0; i < app->m_plugins.CountItems(); i++)
	{
		MeVPlugIn		*pi = (MeVPlugIn *)app->m_plugins.ItemAt( i );
		
		pList->AddItem( new BStringItem( pi->Name() ) );
	}

	r.left = 149;
	
	bb = new BBox( r, NULL, B_FOLLOW_ALL, B_WILL_DRAW, B_FANCY_BORDER );
	background->AddChild( bb );
	
	textView = new BTextView(	BRect( 10, 10, r.Width() - 10, r.Height() - 10 ),
							"Description",	
							BRect( 0, 0, r.Width() - 20, r.Height() - 20 ),
							B_FOLLOW_ALL, B_WILL_DRAW );
	textView->MakeSelectable( false );
	textView->MakeEditable( false );
	textView->SetAlignment( B_ALIGN_CENTER );
	bb->AddChild( textView );
	textView->SetViewColor( 220, 220, 220 );
}

void
CAboutPluginWindow::MessageReceived(
	BMessage *message)
{
	switch (message->what)
	{
		case PLUGIN_SELECTED:
		{
			int num = pList->CurrentSelection();
			MeVPlugIn *pi = (MeVPlugIn *)((CMeVApp *)be_app)->m_plugins.ItemAt( num );
			textView->SetText(pi->AboutText());
			break;
		}
		default:
		{
			CAppWindow::MessageReceived(message);
			break;
		}
	}
}

bool CGlobalPrefs::FeedbackEnabled( int32 inAttribute, bool inDrag )
{
	uint32		mask = inDrag ? feedbackDragMask : feedbackAdjustMask;
	
	switch (inAttribute) {
	case EvAttr_Pitch:
		return (mask & FB_Pitch) != 0;

	case EvAttr_AttackVelocity:
	case EvAttr_ReleaseVelocity:
		return (mask & FB_Velocity) != 0;

	case EvAttr_Program:
	case EvAttr_ProgramBank:
		return (mask & FB_Program	) != 0;

	case EvAttr_ControllerNumber:
	case EvAttr_ControllerValue8:
	case EvAttr_ControllerValue16:
	case EvAttr_Modulation:
		return (mask & FB_Control) != 0;

	case EvAttr_Channel:
		return (mask & FB_Channel) != 0;
	}
	return false;
}
