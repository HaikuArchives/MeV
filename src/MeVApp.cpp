/* ===================================================================== *
 * MeVApp.cpp (MeV)
 * ===================================================================== */

#include "MeV.h"
#include "MeVApp.h"
#include "MeVDoc.h"
#include "MeVPlugin.h"
#include "DocWindow.h"
#include "StripFrameView.h"
#include "InspectorWindow.h"
#include "TransportWindow.h"
#include "MidiConfigWindow.h"
#include "Junk.h"
#include "Splash.h"
#include "Idents.h"
#include "EventOp.h"
#include "LinearWindow.h"
#include "AssemblyWindow.h"
#include "GridWindow.h"
#include "Spinner.h"
#include "ScreenUtils.h"
#include "PlayerControl.h"
#include "MidiDeviceInfo.h"

// Gnu C Library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// Interface Kit
#include <Box.h>
#include <MenuField.h>
#include <PopUpMenu.h>
#include <ScrollView.h>
#include <StringView.h>
#include <TextControl.h>
// Storage Kit
#include <FilePanel.h>
#include <NodeInfo.h>
#include <Path.h>

#define INSPECTOR_NAME		"Inspector"
#define GRID_NAME			"Grid"
#define TRANSPORT_NAME		"Transport"
#define APP_PREFS_NAME		"Prefs"
#define MIDI_CONFIG_NAME	"Midi Config"
#define ABOUT_NAME			"About Box"
#define ABOUT_PI_NAME		"About Plug-ins"

CGlobalPrefs				gPrefs;
char						gPlugInName[ B_FILE_NAME_LENGTH ];

extern void CalcHighlightColor( rgb_color &in, rgb_color &out );


	// Images global to application.
BBitmap		*smallPlusImage = NULL,
			*smallMinusImage = NULL,
			*gridToolImage = NULL,
			*arrowToolImage = NULL,
			*pencilToolImage = NULL,
			*metroToolImage = NULL,
			*eraserToolImage = NULL,
			*selRectToolImage = NULL,
			*textToolImage = NULL,
			*sectionMarkerImage = NULL,
			*beginBtnImage = NULL,
			*rewindBtnImage = NULL,
			*playBtnImage = NULL,
			*ffBtnImage = NULL,
			*endBtnImage = NULL,
			*stopBtnImage = NULL,
			*pauseBtnImage = NULL,
			*recBtnImage = NULL,
			*loop1Image = NULL,
			*loop2Image = NULL,
			*playBegImage = NULL,
			*programToolImage = NULL,
			*timeSigToolImage = NULL,
			*trackToolImage = NULL,
			*repeatToolImage = NULL,
			*endToolImage = NULL,
			*sysexToolImage = NULL,
			*smallClockImage = NULL;

CTrack		*CMeVApp::activeTrack;

int main(int argc, char *argv[])
{
	CMeVApp	app;
	app.Run();

	return 0;
}

static bool ReadWindowState( BMessage &msg, char *prefsName, CWindowState &ioState, bool defaultOpen )
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

static void WriteWindowState( BMessage &msg, char *prefsName, CWindowState &wState )
{
	char			oName[ 48 ];

	sprintf( oName, "%s Open", prefsName );

	msg.RemoveName( prefsName );
	msg.AddPoint( prefsName, wState.Pos() );

	msg.RemoveName( oName );
	msg.AddBool( oName, wState.IsOpen() );
}

class CMeVRefFilter : public BRefFilter {
	bool Filter( const entry_ref *ref, BNode *node, struct stat *st, const char *filetype )
	{
		return (strstr( filetype, "MeV" ) != NULL);
	}
};

class CImportRefFilter : public BRefFilter {
	bool Filter( const entry_ref *ref, BNode *node, struct stat *st, const char *filetype )
	{
		BList			&list = ((CMeVApp *)be_app)->importerList;
	
		for (int i = 0; i < list.CountItems(); i++)
		{
			MeVPlugIn	*pi = (MeVPlugIn *)list.ItemAt( i );

			if (pi->DetectFileType( ref, node, st, filetype ) >= 0) return true;
		}
		return false;
	}
};

static rgb_color defaultColorTable[ 16 ] = {

	{ 255, 128, 128 },			// Midi Channel 1
	{ 255, 128,   0 },			// Midi Channel 2
	{ 255, 255,   0 },			// Midi Channel 3
	{ 128, 255,   0 },			// Midi Channel 4
	{   0, 255, 128 },			// Midi Channel 5
	{   0, 192,   0 },			// Midi Channel 6
	{   0, 160, 160 },			// Midi Channel 7
	{   0, 192, 160 },			// Midi Channel 8
	{ 128, 255, 255 },			// Midi Channel 9
	{  47, 130, 255 },			// Midi Channel 10
	{ 128, 128, 255 },			// Midi Channel 11
	{ 200,   0, 255 },			// Midi Channel 12
	{ 255,   0, 255 },			// Midi Channel 13
	{ 255, 128, 255 },			// Midi Channel 14
	{ 192, 192, 192 },			// Midi Channel 15
	{ 128, 128,   0 },			// Midi Channel 16
};

CMeVApp::CMeVApp()
	:	CDocApp( "application/x-vnd.SylvanTechnicalArts-MeV" ),
		prefs( "x-sylvan-MeV" ),
		winSettings( prefs, "settings/windows", true ),
		editSettings( prefs, "settings/edit", true ),
		midiSettings( prefs, "settings/midi", true ),
		vtableSettings( prefs, "settings/vtable", true ),
		inspectorState(BRect(0.0,
							 406.0,
							 CInspectorWindow::DEFAULT_DIMENSIONS.Width(),
							 CInspectorWindow::DEFAULT_DIMENSIONS.Height())),
		gridWinState(BRect(0.0 + CInspectorWindow::DEFAULT_DIMENSIONS.Width(),
						   406.0 - CGridWindow::DEFAULT_DIMENSIONS.Height(),
						   CGridWindow::DEFAULT_DIMENSIONS.Width(),
						   CGridWindow::DEFAULT_DIMENSIONS.Height())),
		transportState( BRect( 0.0, 406.0, Transport_Width, Transport_Height ) ),
		midiConfigWinState( BRect( 40, 40, 500, 400 ) ),
		appPrefsWinState( BRect( 40, 40, 500, 300 ) ),
		aboutWinState( BRect( 40, 40, 500, 300 ) ),
		aboutPluginWinState( BRect( 80, 80, 450, 250 ) )
{
		// Iterate through all the plugins...
	BDirectory			addOnDir( "/boot/home/config/add-ons/MeV" );
	bool					inspectorOpen = true,
						gridWindowOpen = false,
						transportOpen = false,
						appPrefsOpen = false,
						midiConfigOpen = false,
						aboutOpen = false,
						aboutPlugOpen = false;
						

	CSplashWindow::DisplayStatus( "Initializing Defaults..." );
	
	Event::InitTables();
	activeTrack = NULL;
	filter = new CMeVRefFilter;
	importFilter = new CImportRefFilter;
	loopFlag = false;
	
		// Initialize mini-dialog windows
	deviceAttrsWindow = NULL;
	portAttrsWindow = NULL;
	patchAttrsWindow = NULL;

		// Initialize default global prefs
	gPrefs.feedbackDragMask = -1;
	gPrefs.feedbackAdjustMask = -1;
	gPrefs.feedbackDelay = 50;
	gPrefs.inclusiveSelection = false;
	gPrefs.firstMeasureNumber = 1;
	gPrefs.appPrefsPanel = 0;
	gPrefs.lEditorPrefsPanel = 0;
	
		// Initialize import/export file panels
	exportPanel = NULL;
	importPanel = NULL;

	CSplashWindow::DisplayStatus( "Starting MIDI Player..." );
	CPlayerControl::InitPlayer();

		// Initialize default virtual channel table
	for (int i = 0; i < Max_VChannels; i++)
	{
		VChannelEntry	&vc = defaultVCTable[ i ];
		
		vc.fillColor	= defaultColorTable[ i & 15 ];
		if (i & 16)
		{
			vc.fillColor.red		= vc.fillColor.red	* 3 / 4;
			vc.fillColor.green	= vc.fillColor.green	* 3 / 4;
			vc.fillColor.blue	= vc.fillColor.blue	* 3 / 4;
		}
		vc.port		= i/16;
		vc.channel	= (i&15) + 1;
		vc.flags		= VChannelEntry::transposable;
		vc.velocityContour = vc.VUMeter = 0;
		CalcHighlightColor( vc.fillColor, vc.highlightColor );
	}

	CSplashWindow::DisplayStatus( "Scanning for Plug-ins..." );

	//printf("Scanning for plug-ins\n");

		// Load in add-ons
	if (addOnDir.InitCheck() == B_NO_ERROR)
	{
		BEntry		entry;
		
		//printf("Looping\n");

		for (;;)
		{
			BPath		path;
			image_id		id;
			MeVPlugIn	*pi;
			MeVPlugIn 	*(*func_create)();

			if (addOnDir.GetNextEntry( &entry ) == B_ENTRY_NOT_FOUND) break;
			
			entry.GetPath( &path );
			
			//printf(" %s\n", path.Path());	

			id = load_add_on( path.Path() );
			if (id == B_ERROR) continue;
			
			entry.GetName( gPlugInName );

			//printf(" checking for symbol\n");	

			if (get_image_symbol(	id,
								"CreatePlugin",  //"CreatePlugin__Fv",
								B_SYMBOL_TYPE_TEXT,
								(void **)&func_create ) == B_NO_ERROR)
			{
				//printf("   found it!\n");	
				CSplashWindow::DisplayStatus( "Loading %s...", gPlugInName );
				pi = (func_create)();
// 			plugInList.AddItem( pi );
			}
		}
	}
	
	CSplashWindow::DisplayStatus( "Reading window preferences..." );

		// Load in application preferences
	if (!winSettings.InitCheck())
	{
		BMessage		&prefMessage = winSettings.GetMessage();

		inspectorOpen		= ReadWindowState( prefMessage, INSPECTOR_NAME, inspectorState, true );
		gridWindowOpen	= ReadWindowState( prefMessage, GRID_NAME,		  gridWinState, false );
		transportOpen		= ReadWindowState( prefMessage, TRANSPORT_NAME, transportState, false );
		appPrefsOpen		= ReadWindowState( prefMessage, APP_PREFS_NAME, appPrefsWinState, false );
		midiConfigOpen	= ReadWindowState( prefMessage, MIDI_CONFIG_NAME, midiConfigWinState, false );
		aboutOpen		= ReadWindowState( prefMessage, ABOUT_NAME,     aboutWinState, false );
		aboutPlugOpen		= ReadWindowState( prefMessage, ABOUT_PI_NAME,  aboutPluginWinState, false );
	}
	
	CSplashWindow::DisplayStatus( "Reading editor preferences..." );

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

	CSplashWindow::DisplayStatus( "Reading MIDI preferences..." );

		// Load in application preferences
	if (!midiSettings.InitCheck())
	{
		BMessage		&prefMessage = midiSettings.GetMessage();
		
		SetDevicePrefs( &prefMessage );
	}
	
	CSplashWindow::DisplayStatus( "Reading virtual channel preferences..." );

		// Load default virtual channel table...
	if (!vtableSettings.InitCheck())
	{
		BMessage		&prefMessage = vtableSettings.GetMessage();
				
		for (int i = 0; i < Max_VChannels; i++)
		{
			int8				b;
			bool				t;
		
			VChannelEntry	&vc = defaultVCTable[ i ];

			if (prefMessage.FindInt8( "Port", i, &b ) == B_OK) vc.port = b;
			if (prefMessage.FindInt8( "Channel", i, &b ) == B_OK) vc.channel = b;
			if (prefMessage.FindInt8( "Red", i, &b ) == B_OK) vc.fillColor.red = b;
			if (prefMessage.FindInt8( "Green", i, &b ) == B_OK) vc.fillColor.green = b;
			if (prefMessage.FindInt8( "Blue", i, &b ) == B_OK) vc.fillColor.blue = b;
			if (prefMessage.FindInt8( "Contour", i, &b ) == B_OK) vc.velocityContour = b;

			if (prefMessage.FindBool( "Transpose", i, &t ) == B_OK)
			{
				if (t) vc.flags |= VChannelEntry::transposable;
				else vc.flags &= ~VChannelEntry::transposable;
			}

			if (prefMessage.FindBool( "Mute", i, &t ) == B_OK)
			{
				if (t) vc.flags |= VChannelEntry::mute;
				else vc.flags &= ~VChannelEntry::mute;
			}
		
			CalcHighlightColor( vc.fillColor, vc.highlightColor );
		}	
	}
	
	CalcDeviceTable();

	CSplashWindow::HideSplash();

	if (inspectorOpen) ShowInspector( true );
	if (gridWindowOpen) ShowGridWindow( true );
	if (transportOpen) ShowTransportWindow( true );
	if (appPrefsOpen) ShowPrefs();
	if (midiConfigOpen) ShowMidiConfig();
}

CMeVApp::~CMeVApp()
{
	if (openPanel) openPanel->SetRefFilter( NULL );
	delete filter;
	delete importFilter;

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
		BMessage		&prefMessage = midiSettings.GetMessage();

		GetDevicePrefs( &prefMessage );
		midiSettings.Save();
	}

	for (int i = 0; i < defaultOperatorList.CountItems(); i++)
	{
		EventOp		*op = (EventOp *)defaultOperatorList.ItemAt( i );

		CRefCountObject::Release( op );
	}

	for (int i = 0; i < deviceList.CountItems(); i++)
	{
		MIDIDeviceInfo	*mdi = (MIDIDeviceInfo *)deviceList.ItemAt( i );

		delete mdi;
	}
	
	CancelEditDeviceAttrs();
	CancelEditPortAttrs();
	CancelEditPatchAttrs();
}

CDocument *CMeVApp::NewDocument( bool inShowWindow, entry_ref *inRef )
{
	CMeVDoc			*doc;

	if (inRef)	doc = new CMeVDoc( *this, *inRef );
	else			doc = new CMeVDoc( *this );
	
		// If document did not initialize OK, then fail.
	if (!doc->InitCheck())
	{
		delete doc;
		return NULL;
	}

	if (inShowWindow) doc->ShowWindow( CMeVDoc::Assembly_Window );
	return doc;
}

	/** Import a new document */
void CMeVApp::ImportDocument()
{
	BFilePanel		*fp = GetImportPanel( &be_app_messenger );

	fp->SetMessage( new BMessage( 'impt' ) );
	fp->SetRefFilter( importFilter );
	fp->Show();
}

void CMeVApp::ShowInspector( bool inShow )
{
	inspectorState.Lock();

	if (inShow)
	{
		if (!inspectorState.Activate())
		{
			BWindow		*w;
		
			w = new CInspectorWindow( inspectorState.Rect().LeftTop(), inspectorState );
			w->Show();
		}
	}
	else
	{
		inspectorState.Close();
	}
	
	inspectorState.Unlock();
}

void CMeVApp::ShowGridWindow( bool inShow )
{
	gridWinState.Lock();

	if (inShow)
	{
		if (!gridWinState.Activate())
		{
			BWindow		*w;
		
			w = new CGridWindow( gridWinState.Rect().LeftTop(), gridWinState );
			w->Show();
		}
	}
	else
	{
		gridWinState.Close();
	}
	
	gridWinState.Unlock();
}

void CMeVApp::ShowTransportWindow( bool inShow )
{
	transportState.Lock();

	if (inShow)
	{
		if (!transportState.Activate())
		{
			BWindow		*w;
		
			w = new CTransportWindow( transportState.Rect().LeftTop(), transportState );
			w->Show();
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
		BWindow		*w;
	
		w = new CAppPrefsWindow( appPrefsWinState );
		w->Show();
	}
	appPrefsWinState.Unlock();
}

void CMeVApp::ShowMidiConfig()
{
	midiConfigWinState.Lock();
	if (!midiConfigWinState.Activate())
	{
		BWindow		*w;
	
		w = new CMidiConfigWindow( midiConfigWinState );
		w->Show();
	}
	midiConfigWinState.Unlock();
}

void CMeVApp::WatchTrack( CEventTrack *inTrack )
{
	CMeVApp		*mApp = (CMeVApp *)be_app;
	CAppWindow	*w;
	
	if (activeTrack == inTrack) return;
	
	if (activeTrack) CRefCountObject::Release( activeTrack );
	activeTrack = inTrack;
	if (activeTrack) activeTrack->Acquire();

	mApp->inspectorState.Lock();
	w = mApp->inspectorState.Window();
	if (w)
	{
		((CInspectorWindow *)w)->WatchTrack( inTrack );
	}
	mApp->inspectorState.Unlock();

	mApp->gridWinState.Lock();
	w = mApp->gridWinState.Window();
	if (w)
	{
		((CGridWindow *)w)->WatchTrack( inTrack );
	}
	mApp->gridWinState.Unlock();

	mApp->transportState.Lock();
	w = mApp->transportState.Window();
	if (w)
	{
		((CTransportWindow *)w)->WatchTrack( inTrack );
	}
	mApp->transportState.Unlock();
}

void CMeVApp::AboutRequested()
{
	BWindow			*w;

	w = new CAboutWindow( aboutWinState );
	w->Show();
}

void CMeVApp::MessageReceived( BMessage *inMsg )
{
	BWindow			*w;

	switch (inMsg->what) {
	case MENU_ABOUT_PLUGINS:
		w = new CAboutPluginWindow( aboutPluginWinState );
		w->Show();
		break;
		
	case Player_ChangeTransportState:
	
			// If there's a transport window, forward this message to it.
		transportState.Lock();
		w = transportState.Window();
		if (w) w->PostMessage( inMsg );
		transportState.Unlock();
		break;
		
		// Import panel message
	case 'impt':

		int32			refCount;
		type_code		type;
							
			// For each reference
		if (inMsg->GetInfo( "refs", &type, &refCount ) == B_NO_ERROR)
		{
			for (int j = 0; j < refCount; j++)
			{
				entry_ref	ref;
				
				if (inMsg->FindRef( "refs", j, &ref ) == B_NO_ERROR)
				{
					BList			&list = ((CMeVApp *)be_app)->importerList;

						// See if there's a plug-in which can identify it...
					for (int i = 0; i < list.CountItems(); i++)
					{
						MeVPlugIn	*pi = (MeVPlugIn *)list.ItemAt( i );
						int32		kind;
						struct stat	st;
						char			filetype[ B_MIME_TYPE_LENGTH ];
						
						BNode		node( &ref );
						if (node.InitCheck() != B_NO_ERROR) continue;
						
						BNodeInfo	ni( &node );
						if (ni.InitCheck() != B_NO_ERROR) continue;
						if (ni.GetType( filetype ) != B_NO_ERROR) continue;
						if (node.GetStat( &st ) != B_NO_ERROR) continue;
						
						kind = pi->DetectFileType( &ref, &node, &st, filetype );
						if (kind < 0) continue;

						pi->OnImport( NULL, &ref, kind );
						break;
					}
				}
			}
		}
		break;

		// Export panel message
	case 'expt':
		{
			CMeVDoc 	*hDoc = NULL;
			entry_ref 	saveRef;
			char 		*saveName;
			BMessage	pluginMsg;
			MeVPlugIn	*pluginPtr;
	
			if (	inMsg->FindPointer( "Document" , (void **)&hDoc ) == B_NO_ERROR &&
				inMsg->FindRef( "directory" , &saveRef ) == B_NO_ERROR &&
				inMsg->FindString( "name" , (const char **)&saveName ) == B_NO_ERROR &&
				inMsg->FindPointer( "plugin", (void **)&pluginPtr ) == B_NO_ERROR &&
				inMsg->FindMessage( "msg", &pluginMsg ) == B_NO_ERROR)
			{
				entry_ref fileRef;
				
				BDirectory theDir( &saveRef );
				BEntry theEntry( &theDir, saveName );
				
//				if (theEntry.GetRef( &fileRef ) == B_NO_ERROR)
//					pluginPtr->OnExport( &pluginMsg, (int32)*hDoc, &fileRef );
			}
		}
		break;
		
	default:
		CDocApp::MessageReceived( inMsg );
		break;
	}
}

	/**	Add an operator to the list of default operators. */
void CMeVApp::AddDefaultOperator( EventOp *inOp )
{
	Lock();

	if (!defaultOperatorList.HasItem( inOp ))
	{
		inOp->Acquire();
		defaultOperatorList.AddItem( inOp );
	}
	
	Unlock();
}

	/**	Return the Nth operator (Increases reference count). */
EventOp *CMeVApp::OperatorAt( int32 index )
{
	void		*ptr = defaultOperatorList.ItemAt( index );
	
	if (ptr) return (EventOp *)((EventOp *)ptr)->Acquire();
	return NULL;
}

bool CMeVApp::QuitRequested()
{
		// save out application preferences before windows close.
	if (!winSettings.InitCheck())
	{
		BMessage		&prefMessage = winSettings.GetMessage();

		WriteWindowState( prefMessage, INSPECTOR_NAME,	inspectorState );
		WriteWindowState( prefMessage, GRID_NAME,		gridWinState );
		WriteWindowState( prefMessage, TRANSPORT_NAME,	transportState );
		WriteWindowState( prefMessage, APP_PREFS_NAME,	appPrefsWinState );
		WriteWindowState( prefMessage, MIDI_CONFIG_NAME,midiConfigWinState );
		WriteWindowState( prefMessage, ABOUT_NAME,		aboutWinState );
		WriteWindowState( prefMessage, ABOUT_PI_NAME,	aboutPluginWinState );
		winSettings.Save();
	}
	return true;
}

CTrack *CMeVApp::ActiveTrack()
{
	return activeTrack ? (CTrack *)activeTrack->Acquire() : NULL;
}

	/* Overrides file panel creation to install the filter */
BFilePanel *CMeVApp::CreateOpenPanel()
{
	BFilePanel		*p = CDocApp::CreateOpenPanel();
	
	p->SetRefFilter( filter );
	return p;
}

class CDeviceAttrsDialog : public CMiniDialog {
	CSpinner		*lcSpinner,
				*hcSpinner,
				*bcSpinner;
	BTextControl	*name;
	CTextDisplay	*chRange,
				*baseDisp;
	MIDIDeviceInfo *info;
	char			rangeText[ 16 ],
				baseText[ 16 ];
	int32		portNum;
				
	void ShowChannels();
	void MessageReceived( BMessage* theMessage );
	bool QuitRequested()
	{
		((CMeVApp *)be_app)->deviceAttrsWindow = NULL;
		return true;
	}

public:
	CDeviceAttrsDialog( BWindow *parent );
	void SetDevice( MIDIDeviceInfo *info, int32 inPortNum );
};

CDeviceAttrsDialog::CDeviceAttrsDialog( BWindow *parent )
	: CMiniDialog( 300, 76, parent, "MIDI Instrument Attributes" )
{
		// Create text strings:

		//	Device name
	background->AddChild( new BStringView( BRect( 8, 1, 150, 16 ), "", "Instrument Name" ) );

		//	Device name edit box
	name = new BTextControl( BRect( 6, 20, 150, 40 ), "Name", "", "", new BMessage( 'name' ),
		B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW );
	background->AddChild( name );
	name->SetTarget( this );
	name->SetViewColor( 220, 220, 220 );
	name->SetDivider( 0.0 );
	name->MakeFocus();

		//	Low Channel Text
	background->AddChild( new BStringView( BRect( 160, 1, 246, 16 ), "", "Channel Range" ) );

		//	Low Channel (Spinner)
	lcSpinner = new CSpinner( BRect( 160, 20, 176, 40 ), "Low", new BMessage( 'lchn' ) );
	lcSpinner->SetRange( 0, 15 );
	background->AddChild( lcSpinner );
//	lcSpinner->SetTarget( this );

	chRange = new CTextDisplay( BRect( 180, 20, 220, 40 ), "Channels" );
	chRange->SetAlignment( B_ALIGN_CENTER );
	background->AddChild( chRange );

		//	High Channel Text
		//	High Channel (Spinner)
	hcSpinner = new CSpinner( BRect( 224, 20, 240, 40 ), "Low", new BMessage( 'hchn' ) );
	hcSpinner->SetRange( 0, 15 );
	background->AddChild( hcSpinner );
//	hcSpinner->SetTarget( this );

		//	Base Channel Text
	background->AddChild( new BStringView( BRect( 250, 1, 296, 16 ), "", "Base Ch." ) );

	baseDisp = new CTextDisplay( BRect( 250, 20, 270, 40 ), "BaseDisp" );
	baseDisp->SetAlignment( B_ALIGN_CENTER );
	background->AddChild( baseDisp );

		//	Base Channel (Spinner)
	bcSpinner = new CSpinner( BRect( 274, 20, 290, 40 ), "Base", new BMessage( 'bchn' ) );
	bcSpinner->SetRange( 0, 15 );
	background->AddChild( bcSpinner );
//	bcSpinner->SetTarget( this );
}

void CDeviceAttrsDialog::ShowChannels()
{
	sprintf( rangeText, "%ld - %ld", lcSpinner->Value() + 1, hcSpinner->Value() + 1 );
	chRange->SetText( rangeText );

	sprintf( baseText, "%ld", bcSpinner->Value() + 1 );
	baseDisp->SetText( baseText );
}

void CDeviceAttrsDialog::SetDevice( MIDIDeviceInfo *inInfo, int32 inPortNum )
{
	info = inInfo;
	
	portNum = inPortNum;

	if (info)
	{
		name->SetText( info->name );
		lcSpinner->SetValue( info->lowChannel );
		hcSpinner->SetValue( info->highChannel );
		bcSpinner->SetValue( info->baseChannel );
	}
	else
	{
		name->SetText( "" );
		lcSpinner->SetValue( 0 );
		hcSpinner->SetValue( 0 );
		bcSpinner->SetValue( 0 );
	}
	
	ShowChannels();
}

void CDeviceAttrsDialog::MessageReceived( BMessage* theMessage )
{
	CMeVApp *app = (CMeVApp *)be_app;

	switch(theMessage->what) {
	case 'lchn':
	case 'hchn':
		if (hcSpinner->Value() < lcSpinner->Value()) hcSpinner->SetValue( lcSpinner->Value() );
		ShowChannels();
		break;
	
	case 'bchn':
		ShowChannels();
		break;
		
	case Apply_ID:

#if 0
		app->appPrefsWinState.Lock();
		if (app->appPrefsWinState.Window() != NULL)
		{
			BMessage		msg( 'chdv' );
			
			msg.AddPointer( "device", info );
			msg.AddString( "name", name->Text() );
			msg.AddInt8( "low", lcSpinner->Value() );
			msg.AddInt8( "high", hcSpinner->Value() );
			msg.AddInt8( "base", bcSpinner->Value() );
			msg.AddInt8( "port", portNum );
		
			app->appPrefsWinState.Window()->PostMessage( &msg );
		}
		app->appPrefsWinState.Unlock();
#else
		app->midiConfigWinState.Lock();
		if (app->midiConfigWinState.Window() != NULL)
		{
			BMessage		msg( 'chdv' );
			
			msg.AddPointer( "device", info );
			msg.AddString( "name", name->Text() );
			msg.AddInt8( "low", lcSpinner->Value() );
			msg.AddInt8( "high", hcSpinner->Value() );
			msg.AddInt8( "base", bcSpinner->Value() );
			msg.AddInt8( "port", portNum );
		
			app->midiConfigWinState.Window()->PostMessage( &msg );
		}
		app->midiConfigWinState.Unlock();
#endif

		// REM: Notify prefs window to rebuild it's list.

		PostMessage( B_QUIT_REQUESTED );
		break;

	case Close_ID:
		PostMessage( B_QUIT_REQUESTED );
		break;

	default:
		CMiniDialog::MessageReceived( theMessage );
		break;
	}		
}

void CMeVApp::EditDeviceAttrs( MIDIDeviceInfo *inDevInfo, int32 inPortNum )
{
		//	Open device attrs window...
	if (deviceAttrsWindow == NULL)
	{
#if 0
		appPrefsWinState.Lock();
		if (appPrefsWinState.Window() != NULL)
		{
			deviceAttrsWindow = new CDeviceAttrsDialog( appPrefsWinState.Window() );
		}
		appPrefsWinState.Unlock();
#else
		midiConfigWinState.Lock();
		if (midiConfigWinState.Window() != NULL)
		{
			deviceAttrsWindow = new CDeviceAttrsDialog( midiConfigWinState.Window() );
		}
		midiConfigWinState.Unlock();
#endif

	}
	
	((CDeviceAttrsDialog *)deviceAttrsWindow)->SetDevice( inDevInfo, inPortNum );
	deviceAttrsWindow->Show();
}

void CMeVApp::CancelEditDeviceAttrs()
{
	if (deviceAttrsWindow) deviceAttrsWindow->Quit();
	deviceAttrsWindow = NULL;
}

class CPortAttrsDialog : public CMiniDialog {
	BTextControl	*description,
				*name;
	int32		portNum;

	void MessageReceived( BMessage* theMessage );
	bool QuitRequested()
	{
		((CMeVApp *)be_app)->portAttrsWindow = NULL;
		return true;
	}

public:
	CPortAttrsDialog( BWindow *parent );
	void SetPort( int32 inPortIndex );
};

CPortAttrsDialog::CPortAttrsDialog( BWindow *parent )
	: CMiniDialog( 300, 76, parent, "Port Attributes" )
{
		//	Port name
	background->AddChild( new BStringView( BRect( 8, 1, 150, 16 ), "", "Port Description" ) );

		//	Port description
	description = new BTextControl( BRect( 6, 20, 184, 40 ), "Description", "", "", new BMessage( 'desc' ),
		B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW );
	background->AddChild( description );
	description->SetTarget( this );
	description->SetViewColor( 220, 220, 220 );
	description->SetDivider( 0.0 );
	description->MakeFocus();

	background->AddChild( new BStringView( BRect( 192, 1, 250, 16 ), "", "Port Name" ) );

	name = new BTextControl( BRect( 190, 20, 240, 40 ), "Name", "", "", new BMessage( 'name' ),
		B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW );
	background->AddChild( name );
	name->SetTarget( this );
	name->SetViewColor( 220, 220, 220 );
	name->SetDivider( 0.0 );

	BMenu		*devMenu = new BPopUpMenu( "synth" );
	devMenu->AddItem( new BMenuItem( "synth",			new BMessage( 3000 ) ) );
	devMenu->AddItem( new BMenuItem( "midi1",			new BMessage( 3001 ) ) );
	devMenu->AddItem( new BMenuItem( "midi2",			new BMessage( 3002 ) ) );
	devMenu->SetTargetForItems( (CDocWindow *)this );

	BMenuField	*devType = new BMenuField(	BRect( 244, 20, 292, 40 ),
											NULL, NULL,
											devMenu,
											B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW );
	background->AddChild( devType );
}

void CPortAttrsDialog::SetPort( int32 inPortIndex )
{
	portNum = inPortIndex;
	description->SetText( CPlayerControl::PortName( portNum ) );
	name->SetText( CPlayerControl::PortDevice( portNum ) );
}

void CPortAttrsDialog::MessageReceived( BMessage* theMessage )
{
	CMeVApp *app = (CMeVApp *)be_app;

	switch(theMessage->what) {
	case 3000:	name->SetText( "synth" ); break;
	case 3001:	name->SetText( "midi1" ); break;
	case 3002:	name->SetText( "midi2" ); break;

	case Apply_ID:
#if 0
		app->appPrefsWinState.Lock();
		if (app->appPrefsWinState.Window() != NULL)
		{
			BMessage		msg( 'chpt' );
			
			msg.AddInt32( "port", portNum );
			msg.AddString( "name", name->Text() );
			msg.AddString( "description", description->Text() );
		
			app->appPrefsWinState.Window()->PostMessage( &msg );
		}
		app->appPrefsWinState.Unlock();
#else
		app->midiConfigWinState.Lock();
		if (app->midiConfigWinState.Window() != NULL)
		{
			BMessage		msg( 'chpt' );
			
			msg.AddInt32( "port", portNum );
			msg.AddString( "name", name->Text() );
			msg.AddString( "description", description->Text() );
		
			app->midiConfigWinState.Window()->PostMessage( &msg );
		}
		app->midiConfigWinState.Unlock();
#endif

#if 0
		if (CPlayerControl::SetPortDevice( portNum, (char *)name->Text() ) == false)
		{
			((CDocApp *)be_app)->Error( "Invalid device name" );
			return;
		}

		CPlayerControl::SetPortName( portNum, (char *)description->Text() );
#endif
		
		// REM: Need to tell list to refresh...

		PostMessage( B_QUIT_REQUESTED );
		break;

	case Close_ID:
		PostMessage( B_QUIT_REQUESTED );
		break;

	default:
		CMiniDialog::MessageReceived( theMessage );
		break;
	}
}

void CMeVApp::EditPortAttrs( int32 inPortIndex )
{
	if (portAttrsWindow == NULL)
	{
#if 0
		appPrefsWinState.Lock();
		if (appPrefsWinState.Window() != NULL)
		{
			portAttrsWindow = new CPortAttrsDialog( appPrefsWinState.Window() );
		}
		appPrefsWinState.Unlock();
#else
		midiConfigWinState.Lock();
		if (midiConfigWinState.Window() != NULL)
		{
			portAttrsWindow = new CPortAttrsDialog( midiConfigWinState.Window() );
		}
		midiConfigWinState.Unlock();
#endif
	}

	((CPortAttrsDialog *)portAttrsWindow)->SetPort( inPortIndex );
	portAttrsWindow->Show();
}

void CMeVApp::CancelEditPortAttrs()
{
	if (portAttrsWindow) portAttrsWindow->Quit();
	portAttrsWindow = NULL;
}

class CPatchAttrsDialog : public CMiniDialog {
	MIDIDeviceInfo	*mdi;
	BTextControl		*description,
					*bankString,
					*indexString;

	void MessageReceived( BMessage* theMessage );
	bool QuitRequested()
	{
		((CMeVApp *)be_app)->patchAttrsWindow = NULL;
		return true;
	}

public:
	CPatchAttrsDialog( BWindow *parent );
	void SetPatch( MIDIDeviceInfo *mdi );
};

CPatchAttrsDialog::CPatchAttrsDialog( BWindow *parent )
	: CMiniDialog( 300, 76, parent, "Program Attributes" )
{
		//	names
	background->AddChild( new BStringView( BRect( 8, 1, 40, 16 ), "", "Bank" ) );
	background->AddChild( new BStringView( BRect( 40, 1, 72, 16 ), "", "#" ) );
	background->AddChild( new BStringView( BRect( 80, 1, 300, 16 ), "", "Program Name" ) );

		//	Patch description
	bankString = new BTextControl( BRect( 6, 20, 40, 40 ), "Bank", "", "", new BMessage( 'desc' ),
		B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_NAVIGABLE );
	background->AddChild( bankString );
	bankString->SetTarget( this );
	bankString->SetViewColor( 220, 220, 220 );
	bankString->SetDivider( 0.0 );
	bankString->MakeFocus();
	bankString->SetText( "0" );

		//	Patch description
	indexString = new BTextControl( BRect( 38, 20, 72, 40 ), "Description", "", "", new BMessage( 'desc' ),
		B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_NAVIGABLE );
	background->AddChild( indexString );
	indexString->SetTarget( this );
	indexString->SetViewColor( 220, 220, 220 );
	indexString->SetDivider( 0.0 );
	indexString->SetText( "0" );
	
		// Allow only numbers...
/*	for (int i = 0; i < 256; i++)
	{
		if (i < '0' || i > '9')
		{
			bankString->DisallowChar( i );
			indexString->DisallowChar( i );
		}
	} */

		//	Patch description
	description = new BTextControl( BRect( 78, 20, 294, 40 ), "Description", "", "", new BMessage( 'desc' ),
		B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_NAVIGABLE );
	background->AddChild( description );
	description->SetTarget( this );
	description->SetViewColor( 220, 220, 220 );
	description->SetDivider( 0.0 );
}

void CPatchAttrsDialog::SetPatch( MIDIDeviceInfo *inMDI )
{
	mdi = inMDI;
}

void CPatchAttrsDialog::MessageReceived( BMessage* theMessage )
{
	CMeVApp *app = (CMeVApp *)be_app;

	switch(theMessage->what) {
		case Apply_ID: {
			int32		bankNum = strtol( bankString->Text(), NULL, 0 ),
						patchNum = strtol( indexString->Text(), NULL, 0 );
	
			if (mdi->GetPatch( bankNum, patchNum ))
			{
				CDocApp::Error( "That MIDI program is already defined." );
				break;
			}
	
			app->midiConfigWinState.Lock();
			if (app->midiConfigWinState.Window() != NULL)
			{
				BMessage		msg( 'chpg' );
				
				msg.AddInt32( "index", patchNum );
				msg.AddInt32( "bank", bankNum );
				msg.AddString( "description", description->Text() );
			
				app->midiConfigWinState.Window()->PostMessage( &msg );
			}
			app->midiConfigWinState.Unlock();
			
			PostMessage( B_QUIT_REQUESTED );
			break;
		}	
		case Close_ID: {
			PostMessage( B_QUIT_REQUESTED );
			break;
		}
		default: {
			CMiniDialog::MessageReceived( theMessage );
			break;
		}
	}
}

void CMeVApp::EditPatchAttrs( MIDIDeviceInfo *mdi )
{
	if (patchAttrsWindow == NULL)
	{
		midiConfigWinState.Lock();
		if (midiConfigWinState.Window() != NULL)
		{
			patchAttrsWindow = new CPatchAttrsDialog( midiConfigWinState.Window() );
		}
		midiConfigWinState.Unlock();
	}
	else patchAttrsWindow->Activate();

	((CPatchAttrsDialog *)patchAttrsWindow)->SetPatch( mdi );
	patchAttrsWindow->Show();
}

void CMeVApp::CancelEditPatchAttrs()
{
	if (patchAttrsWindow) patchAttrsWindow->Quit();
	patchAttrsWindow = NULL;
}

	/** Add a new MIDI device to the device table. */
MIDIDeviceInfo *CMeVApp::NewDevice()
{
	MIDIDeviceInfo		*info = new MIDIDeviceInfo;
	
	deviceList.AddItem( info );
	return info;
}

	/** Delete a MIDI device from the device table. */
void CMeVApp::DeleteDevice( MIDIDeviceInfo *inDevInfo )
{
	if (deviceList.RemoveItem( inDevInfo )) delete inDevInfo;
}

void CMeVApp::CalcDeviceTable()
{
	for (uint32 i = 0; i < Max_MidiPorts; i++)
	{
		for (uint32 j = 0; j < 16; j++)
		{
			deviceTable[ i ][ j ] = NULL;
		}
	}

	for (int i = 0; i < deviceList.CountItems(); i++)
	{
		MIDIDeviceInfo	*mdi = DeviceAt( i );
		
		for (int j = mdi->lowChannel; j <= mdi->highChannel; j++)
		{
			if (j < 0) continue;
			if (j > 15) break;
			
			deviceTable[ mdi->portNum ][ j ] = mdi;
		}
	}
	
	// REM: We'll want to kick off any observers that are looking at VChannels...
}

void CMeVApp::GetDevicePrefs( BMessage *msg )
{
	msg->RemoveName( "portDescription" );
	msg->RemoveName( "portDevice" );

	for (uint32 i = 0; i < Max_MidiPorts; i++)
	{
		if (CPlayerControl::PortName( i ))
			msg->AddString( "portDescription", CPlayerControl::PortName( i ) );
		else msg->AddString( "portDescription", "" );

		if (CPlayerControl::PortDevice( i ))
			msg->AddString( "portDevice", CPlayerControl::PortDevice( i ) );
		else msg->AddString( "portDevice", "" );
	}
	
	msg->RemoveName( "instrumentName" );
	msg->RemoveName( "instrumentPort" );
	msg->RemoveName( "instrumentLowChannel" );
	msg->RemoveName( "instrumentHighChannel" );
	msg->RemoveName( "instrumentBaseChannel" );

	for (int32 i = 0; i < deviceList.CountItems(); i++)
	{
		MIDIDeviceInfo	*mdi = DeviceAt( i );
		
		msg->AddString( "instrumentName", mdi->name );
		msg->AddInt8( "instrumentPort", mdi->portNum );
		msg->AddInt8( "instrumentLowChannel", mdi->lowChannel );
		msg->AddInt8( "instrumentHighChannel", mdi->highChannel );
		msg->AddInt8( "instrumentBaseChannel", mdi->baseChannel );
		
		char				patchNameTag[ 32 ],
						patchIndexTag[ 32 ],
						patchBankTag[ 32 ];
						
		sprintf( patchNameTag, "patchName.%ld", i );
		sprintf( patchIndexTag, "patchIndex.%ld", i );
		sprintf( patchBankTag, "patchBank.%ld", i );

		msg->RemoveName( patchNameTag );
		msg->RemoveName( patchIndexTag );
		msg->RemoveName( patchBankTag );

		CDictionary<uint32,PatchInfo>::Iterator	iter( mdi->patches );
		uint32			*key;
	
			// Save all patch banks to prefs file.
		for (key = iter.First(); key; key = iter.Next())
		{
			PatchInfo		*pi = iter.Value();
			msg->AddString( patchNameTag, pi->Name() );
			msg->AddInt8( patchIndexTag, *key & 0x7f );
			msg->AddInt16( patchBankTag, *key >> 7 );
		}
	}
}

#if 1

char *internalNames[ 128 ] = {
	"Acoustic Grand",
	"Bright Grand",
	"Electric Grand",
	"Honky Tonk",
	"Electric Piano 1",
	"Electric Piano 2",
	"Harpsichord",
	"Clavichord",
	"Celesta",
	"Glockenspiel",
	"Music Box",
	"Vibraphone",
	"Marimba",
	"Xylophone",
	"Tubular Bells",
	"Dulcimer",
	"Drawbar Organ",
	"Percussive Organ",
	"Rock Organ",
	"Church Organ",
	"Reed Organ",
	"Accordian",
	"Harmonica",
	"Tango Accordian",
	
	  /* Guitars */
	"Acoustic Guitar Nylon",
 	"Acoustic Guitar Steel",
 	"Electric Guitar Jazz",
 	"Electric Guitar Clean",
 	"Electric Guitar Muted",
 	"Overdriven Guitar",
 	"Distortion Guitar",
 	"Guitar Harmonics",
  
	  /* Basses */
 	"Acoustic Bass",
 	"Electric Bass Finger",
 	"Electric Bass Pick",
 	"Fretless Bass",
 	"Slap Bass 1",
 	"Slap Bass 2",
 	"Synth Bass 1",
 	"Synth Bass 2",

	  /* Strings */
 	"Violin",
 	"Viola",
 	"Cello",
 	"Contrabass",
 	"Tremolo StringS",
 	"Pissicato StringS",
 	"Orchestral StringS",
 	"Timpani",

	  /* Ensemble strings and voices */
 	"String Ensemble 1",
 	"String Ensemble 2",
 	"Synth StringS 1",
 	"Synth StringS 2",
 	"Voice Aah",
 	"Voice Ooh",
 	"Synth Voice",
 	"Orchestra Hit",

	  /* Brass */
 	"Trumpet",
 	"Trombone",
 	"Tuba",
 	"Muted Trumpet",
 	"French Horn",
 	"Brass Section",
 	"Synth Brass 1",
 	"Synth Brass 2",

	  /* Reeds */
 	"Soprano Sax",
 	"Alto Sax",
 	"Tenor Sax",
 	"Baritone Sax",
 	"Oboe",
 	"English Horn",
 	"Bassoon",
 	"Clarinet",

	  /* Pipes */
 	"Piccolo",
 	"Flute",
 	"Recorder",
 	"Pan Flute",
 	"Blown Bottle",
 	"Shakuhachi",
 	"Whistle",
 	"Ocarina",

	  /* Synth Leads*/
 	"Square Wave",
 	"Sawtooth Wave",
 	"Calliope",
 	"Chiff",
 	"Charang",
 	"Voice",
 	"Fifths",
 	"Bass Lead",
  
	  /* Synth Pads */
 	"New Age",
 	"Warm",
 	"Polysynth",
 	"Choir",
 	"Bowed",
 	"Metallic",
 	"Halo",
 	"Sweep",

	  /* Effects */
 	"FX 1",
 	"FX 2",
 	"FX 3",
 	"FX 4",
 	"FX 5",
 	"FX 6",
 	"FX 7",
 	"FX 8",

  /* Ethnic */
 	"Sitar",
 	"Banjo",
 	"Shamisen",
 	"Koto",
 	"Kalimba",
 	"Bagpipe",
 	"Fiddle",
 	"Shanai",

  /* Percussion */
 	"Tinkle Bell",
 	"Agogo",
 	"Steel Drums",
 	"Woodblock",
 	"Taiko Drums",
 	"Melodic Tom",
 	"Synth Drum",
 	"Reverse Cymbal",

  /* Sound Effects */
 	"Fret Noise",
 	"Breath Noise",
 	"Seashore",
 	"Bird Tweet",
 	"Telephone",
 	"Helicopter",
 	"Applause",
 	"Gunshot"
};

#endif


void CMeVApp::SetDevicePrefs( BMessage *msg )
{
	int32		count;
	type_code	type;

		// Delete the list of devices...
	for (int i = 0; i < deviceList.CountItems(); i++)
	{
		MIDIDeviceInfo	*mdi = (MIDIDeviceInfo *)deviceList.ItemAt( i );
		
		delete mdi;
	}
	deviceList.MakeEmpty();

	if (msg->GetInfo( "portDevice", &type, &count ) == B_NO_ERROR)
	{
		char		*description,
				*device;
	
		for (uint32 i = 0; i < Max_MidiPorts; i++)
		{
			if (	msg->FindString( "portDevice", i, (const char **)&device ) == B_NO_ERROR
				&& msg->FindString( "portDescription", i, (const char **)&description ) == B_NO_ERROR)
			{
				CSplashWindow::DisplayStatus( "Opening device: %s...", description );
				CPlayerControl::SetPortDevice( i, device );
				CPlayerControl::SetPortName( i, description );
			}
			else
			{
				CPlayerControl::SetPortDevice( i, "" );
				CPlayerControl::SetPortName( i, "<No Device>" );
			}
		}
	}
	else
	{
			// Default set of ports and instruments...

		CPlayerControl::SetPortName( 0, "Internal MIDI Synth" );
		CPlayerControl::SetPortDevice( 0, "synth" );
	
		CPlayerControl::SetPortName( 1, "MIDI Port 1" );
		CPlayerControl::SetPortDevice( 1, "midi1" );
	
		CPlayerControl::SetPortName( 2, "MIDI Port 2" );
		CPlayerControl::SetPortDevice( 2, "midi2" );
	
			// Create a default MIDI instrument in case preferences loading went awry.
		MIDIDeviceInfo	*mdi = NewDevice();
		mdi->SetPort( 0 );
		mdi->SetName( "Internal Voices" );
		mdi->SetChannels( 0, 15, 0 );
		
#if 1
			// Create test patches for the internal voice instrument.
		for (int i = 0; i < 128; i++)
			mdi->SetPatchName( 0, i, internalNames[ i ] );
#endif

			// Erase all other devices from table...
		for (uint32 i = 3; i < Max_MidiPorts; i++)
		{
			CPlayerControl::SetPortDevice( i, "" );
			CPlayerControl::SetPortName( i, "<No Device>" );
		}
	}

	if (msg->GetInfo( "instrumentName", &type, &count ) == B_NO_ERROR)
	{
		for (int32 i = 0; i < count; i++)
		{
			char		*name;
					
			if (msg->FindString( "instrumentName", i, (const char **)&name ) == B_NO_ERROR)
			{
				MIDIDeviceInfo	*mdi = NewDevice();
				int8				port = 0,
								lowChannel = 0,
								highChannel = 0,
								baseChannel = 0;
//				char				patchListName[ 32 ];
				int32			patchCount;
								
				mdi->SetName( name );

				if (msg->FindInt8( "instrumentPort", i, &port ) == B_NO_ERROR)
					mdi->SetPort( port );

				if (	msg->FindInt8( "instrumentLowChannel", i, &lowChannel ) == B_NO_ERROR
					&& msg->FindInt8( "instrumentHighChannel", i, &highChannel ) == B_NO_ERROR
					&& msg->FindInt8( "instrumentBaseChannel", i, &baseChannel ) == B_NO_ERROR)
				{
					mdi->SetChannels( lowChannel, highChannel, baseChannel );
				}
				
				char				patchNameTag[ 32 ],
								patchIndexTag[ 32 ],
								patchBankTag[ 32 ];
						
				sprintf( patchNameTag, "patchName.%ld", i );
				sprintf( patchIndexTag, "patchIndex.%ld", i );
				sprintf( patchBankTag, "patchBank.%ld", i );
				
				mdi->patches.MakeEmpty();

				if (msg->GetInfo( patchNameTag, &type, &patchCount ) == B_NO_ERROR)
				{
					for (int j = 0; j < patchCount; j++)
					{
						char		*patchName;
						int8		patchIndex;
						int16	patchBank;

						if (msg->FindString( patchNameTag, j, (const char **)&patchName ) == B_NO_ERROR
							&& msg->FindInt8( patchIndexTag, j, &patchIndex ) == B_NO_ERROR
							&& msg->FindInt16( patchBankTag, j, &patchBank ) == B_NO_ERROR)
						{
							mdi->SetPatchName( patchBank, patchIndex, patchName );
						}
					}
				}
			}
		}
	}
	
	CalcDeviceTable();
}

BFilePanel *CMeVApp::GetImportPanel( BMessenger *msngr )
{
	if (importPanel == NULL)
	{
		entry_ref			ref;
		appDir.GetRef( &ref );
		
			// Create a new import panel
		importPanel = new BFilePanel(	B_OPEN_PANEL, msngr, &ref, B_FILE_NODE, false );
	}
	else if (importPanel->IsShowing()) return NULL;
	
	importPanel->SetMessage( new BMessage( B_REFS_RECEIVED ) );
	importPanel->SetButtonLabel( B_DEFAULT_BUTTON, "Import" );
	importPanel->SetTarget( *msngr );
	importPanel->SetRefFilter( NULL );
	return importPanel;
}

BFilePanel *CMeVApp::GetExportPanel( BMessenger *msngr )
{
	if (exportPanel == NULL)
	{
		entry_ref			ref;
		appDir.GetRef( &ref );
		
			// Create a new export panel
		exportPanel = new BFilePanel( B_SAVE_PANEL, msngr, &ref, B_FILE_NODE, false );
	}
	else if (exportPanel->IsShowing()) return NULL;
	
	exportPanel->SetButtonLabel( B_DEFAULT_BUTTON, "Export" );
	exportPanel->SetTarget( *msngr );
	exportPanel->SetRefFilter( NULL );
	return exportPanel;
}

void CMeVApp::SetDefaultVCTable( VChannelTable &inTable )
{
	memcpy( defaultVCTable, inTable, sizeof defaultVCTable );

		// Save default virtual channel table
	if (!vtableSettings.InitCheck())
	{
		BMessage		&prefMessage = vtableSettings.GetMessage();
		
		prefMessage.MakeEmpty();

		for (int i = 0; i < Max_VChannels; i++)
		{
			VChannelEntry	&vc = defaultVCTable[ i ];

			prefMessage.AddInt8( "Port", vc.port );
			prefMessage.AddInt8( "Channel", vc.channel );
			prefMessage.AddInt8( "Red", vc.fillColor.red );
			prefMessage.AddInt8( "Green", vc.fillColor.green );
			prefMessage.AddInt8( "Blue", vc.fillColor.blue );
			prefMessage.AddInt8( "Contour", vc.velocityContour );

			prefMessage.AddBool( "Transpose", vc.flags & VChannelEntry::transposable ? true : false );
			prefMessage.AddBool( "Mute", vc.flags & VChannelEntry::mute ? true : false );
		}	
		vtableSettings.Save();
	}
}

void CMeVApp::BuildExportMenu( BMenu *inMenu )
{
	for (int i = 0; i < exporterList.CountItems(); i++)
	{
		ExportInfo	*ei = (ExportInfo *)exporterList.ItemAt( i );
		BMessage		*msg = new BMessage( MENU_EXPORT );
		
		msg->AddPointer( "plugin", ei->plugIn );
		msg->AddMessage( "msg", ei->msg );

		inMenu->AddItem( new BMenuItem( ei->menuText, msg ) );
	}
}

CAboutWindow::CAboutWindow( CWindowState &inState )
	: CAppWindow( inState, inState.Rect(), "About MeV", B_TITLED_WINDOW, B_NOT_RESIZABLE | B_NOT_ZOOMABLE )
{
	BRect		r( Frame() );
	
	r.OffsetTo( B_ORIGIN );

/*	BButton		*bb;
	bb = new BButton(	BRect(	r.right - 80,
								r.bottom + 9,
								r.right - 4,
								r.bottom + 34 ),
						"Close", "Close",
						new BMessage( ApplyClose_ID ),
						B_FOLLOW_LEFT | B_FOLLOW_BOTTOM ) );
	bb->MakeDefault( true );
	AddChild( bb ); */
}

CAboutPluginWindow::CAboutPluginWindow( CWindowState &inState )
	: CAppWindow( inState, inState.Rect(), "About MeV Plug-Ins", B_TITLED_WINDOW, B_NOT_RESIZABLE | B_NOT_ZOOMABLE )
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
	
	pList->SetSelectionMessage( new BMessage( Select_ID ) );
	
	BScrollView *sv = new BScrollView(	NULL,
										pList,
										B_FOLLOW_TOP_BOTTOM | B_FOLLOW_LEFT,
										0,
										false, true,
										B_PLAIN_BORDER );
	background->AddChild( sv );
	
	for (int i = 0; i < app->plugInList.CountItems(); i++)
	{
		MeVPlugIn		*pi = (MeVPlugIn *)app->plugInList.ItemAt( i );
		
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

void CAboutPluginWindow::MessageReceived( BMessage *msg )
{
	switch (msg->what) {
		case Select_ID: {
			int num = pList->CurrentSelection();
			MeVPlugIn	*pi = (MeVPlugIn *)((CMeVApp *)be_app)->plugInList.ItemAt( num );
			textView->SetText( pi->AboutText() ); 
			break;
		}
		default: {
			CAppWindow::MessageReceived( msg );
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

BBitmap *LoadImage( BBitmap *&outBM, int32 inID )
{
	if (outBM == NULL)
	{
		outBM = LoadBitmap( 'BMAP', inID );
	}
	return outBM;
}
