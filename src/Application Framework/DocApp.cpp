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

// ---------------------------------------------------------------------------
//	Global resource loading functions

void *LoadResource(type_code type, long id, size_t *data_size)
{
	return ((CDocApp *)be_app)->resFile->FindResource( type, id, data_size );
}

void *LoadResource(type_code type, const char *name, size_t *data_size)
{
	return ((CDocApp *)be_app)->resFile->FindResource( type, name, data_size );
}

struct BitmapRes {
	int32		left,
				top,
				right,
				bottom,
				format,
				length;
};

BBitmap *LoadBitmap( type_code type, long id )
{
	void			*res;
	size_t		data_size;
	
	res = LoadResource( type, id, &data_size );
	if (res)
	{
		BitmapRes	*br = (BitmapRes *)res;
		BBitmap		*bb;
		
		swap_data( 
			B_INT32_TYPE,
			br,
			6 * sizeof(int32),
			B_SWAP_BENDIAN_TO_HOST );
		
		bb = new BBitmap(	BRect( br->left, br->top, br->right, br->bottom ),
							(color_space)br->format );
		if (bb != NULL)
		{
			bb->SetBits( br + 1, br->length, 0, (color_space)br->format );
			delete res;
			return bb;
		}
		delete res;
	}
	return NULL;
}

BBitmap *LoadBitmap( type_code type, const char *name )
{
	void			*res;
	size_t		data_size;
	
	res = LoadResource( type, name, &data_size );
	if (res)
	{
		BitmapRes	*br = (BitmapRes *)res;
		BBitmap		*bb;
		
		swap_data( 
			B_INT32_TYPE,
			br,
			6 * sizeof(int32),
			B_SWAP_BENDIAN_TO_HOST );
		
		bb = new BBitmap(	BRect( br->left, br->top, br->right, br->bottom ),
							(color_space)br->format );
		if (bb != NULL)
		{
			bb->SetBits( br + 1, br->length, 0, (color_space)br->format );
			delete res;
			return bb;
		}
		delete res;
	}
	return NULL;
}

char *LookupErrorText( status_t error )
{
	switch (error) {
	case B_NO_ERROR:
		return "No error occured.";
		
	case B_BAD_VALUE:
		return "An erroneous input value was entered.";

	case B_ENTRY_NOT_FOUND:
		return "The file or directory could not be found. Please check the spelling of the directory and file names.";

	case B_PERMISSION_DENIED:
		return "Permission for this operation was denied.";
		
	case B_NO_MEMORY:
		return "There was not enough memory to complete the operation.";
		
	default:
		return "An error has been detected of a type...never before encountered, Captain.";
	}
}
