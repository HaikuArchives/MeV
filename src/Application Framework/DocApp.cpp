/* ===================================================================== *
 * DocApp.cpp (MeV/Application Framework)
 * ===================================================================== */

#include "DocApp.h"

// Interface Kit
#include <Alert.h>
#include <Bitmap.h>
// Storage Kit
#include <FilePanel.h>
#include <Resources.h>

// ---------------------------------------------------------------------------
//	Document Application methods

CDocApp::CDocApp( const char *inSignature ) : BApplication( inSignature )
{
	app_info info;
	be_app->GetAppInfo( &info );
	openPanel = NULL;

	// Get the parent directory of the application
	BEntry appEntry( &info.ref );
	appEntry.GetParent( &appDir );

	// Get the resource file of the application
	resFile = new BResources();
	BFile appFile( &info.ref, B_READ_ONLY );
	resFile->SetTo( &appFile );

	messenger = new BMessenger( this );
}

CDocApp::~CDocApp()
{
	delete messenger;
	delete openPanel;
}

void CDocApp::AddDocument( CDocument *inDoc ) 
{
	documents.AddItem( inDoc );
}

	//	Remove a document. If all documents removed, then
	//	quit the application...
void CDocApp::RemoveDocument( CDocument *inDoc )
{
	documents.RemoveItem( inDoc );
	if (documents.CountItems() == 0)
	{
		PostMessage( B_QUIT_REQUESTED );
	}
}

	//	Process icons which were dropped on the app.
void CDocApp::RefsReceived( BMessage *inMsg )
{ 
	uint32		type;
	int32		count;
	entry_ref	ref;

	inMsg->GetInfo( "refs", &type, &count );
	if (type == B_REF_TYPE)
	{
		for ( long i = --count; i >= 0; i-- )
		{
			if ( inMsg->FindRef( "refs", i, &ref ) == B_OK )
			{
				CRefCountObject::Release( NewDocument( true, &ref ) );
			}
		}
	}
}

	//	If no documents are open, then open a blank one.
void CDocApp::ReadyToRun()
{
	if (documents.CountItems() == 0)
	{
		CDocument	*doc = NewDocument( true, NULL );
		
			//	Release reference because newly created window has a ref.
		CRefCountObject::Release( doc );
	}
}

	//	Error message routine
void CDocApp::Error( char *inErrMsg )
{
	BAlert		*alert;
		
	alert = new BAlert(  NULL, inErrMsg,
						"OK",
						NULL,
						NULL,
						B_WIDTH_AS_USUAL, B_WARNING_ALERT); 
	alert->Go();
}

void CDocApp::MessageReceived( BMessage *inMsg )
{
	if (	inMsg->what == B_SAVE_REQUESTED
		|| inMsg->what == B_CANCEL)
	{
		void			*docPtr;
		
		if (inMsg->FindPointer( "Document", &docPtr ) != B_NO_ERROR)
		{
			BApplication::MessageReceived( inMsg );
			return;
		}
	
		entry_ref		eref;
		const char		*name;
		CDocument	*doc = (CDocument *)docPtr;
			
		if (inMsg->what == B_SAVE_REQUESTED)
		{
			if (inMsg->FindRef( "directory", 0, &eref ) == B_OK
				&& inMsg->FindString( "name", 0, &name ) == B_OK)
			{
				BDirectory	dir( &eref );
				if (dir.InitCheck() != B_NO_ERROR) return;
	
				doc->docLocation.SetTo( &dir, name );
				doc->named = true;
				doc->SaveDocument();
			}
		}
		else
		{
				// Panel is hidden, release document
			CRefCountObject::Release( doc );
		}
	}
	else BApplication::MessageReceived( inMsg );
}

void CDocApp::OpenDocument()
{
	if (openPanel == NULL) openPanel = CreateOpenPanel();
	openPanel->Show();
}

BFilePanel *CDocApp::CreateOpenPanel()
{
	BFilePanel		*openPanel;
	entry_ref			ref;
	
	appDir.GetRef( &ref );
		
		// Create a new load panel
	openPanel = new BFilePanel(	B_OPEN_PANEL, Messenger(),
							&ref, B_FILE_NODE, false );

#if 0
		// Set the save panel to point to the directory from where we loaded.
	BEntry		pEntry;
	
	if (docLocation.GetParent( &pEntry ) == B_NO_ERROR)
	{
		savePanel->SetPanelDirectory( &pEntry );
	}
#endif
	
	return openPanel;
}
