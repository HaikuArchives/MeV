/* ===================================================================== *
 * Document.cpp (MeV/Application Framework)
 * ===================================================================== */

#include "DocApp.h"
#include "DocWindow.h"

// Gnu C Library
#include <stdio.h>
// Interface Kit
#include <Menu.h>
#include <MenuItem.h>
// Storage Kit
#include <FilePanel.h>

int32		CDocument::newDocCount = 0;

// ---------------------------------------------------------------------------
//	Document methods

CDocument::CDocument( CDocApp &inApp ) : app( inApp )
{
	char			name[ 32 ];

	app.AddDocument( this );
	modified		= false;
	named		= false;
	valid			= false;
	savePanel		= NULL;
	
		//	Get a unique name
	if (++newDocCount == 1)
		sprintf( name, "./Untitled" );
	else sprintf( name, "./Untitled-%ld", newDocCount );
	
		//	Um, where should the default directory be?
		//	A preferences item?
	docLocation.SetTo( name );
}

CDocument::CDocument( CDocApp &inApp, entry_ref &inRef )
	: app( inApp ), docLocation( &inRef )
{
	app.AddDocument( this );
	modified		= false;
	named		= true;
}

CDocument::~CDocument()
{
	delete savePanel;
	app.RemoveDocument( this );
}

long CDocument::GetUniqueWindowNum()
{
	long			trialID = 1,
				prevID = -1;

		//	Loop until we can run through the entire list
		//	without a collision.
	while (trialID != prevID)
	{
		prevID = trialID;

			//	If we collide, then increase the ID by one
		for (int i = 0; i < windows.CountItems(); i++)
		{
			CDocWindow		*w = (CDocWindow *)windows.ItemAt( i );
			
			if (w->windowNumber <= 0) continue;
			if (trialID == w->windowNumber) trialID++;
		}
	}

	return trialID;
}

void CDocument::AddWindow( CDocWindow *inWin )
{
	int32				plainWindowCount = 0;

	StSubjectLock( *this, Lock_Exclusive );

	windows.AddItem( inWin );
	for (int i = 0; i < windows.CountItems(); i++)
	{
		CDocWindow		*w = (CDocWindow *)windows.ItemAt( i );
		w->updateMenus = true;
		if (w->windowNumber >= 0) plainWindowCount++;
	}

	if (plainWindowCount > 1)
	{
		for (int i = 0; i < windows.CountItems(); i++)
		{
			CDocWindow		*w = (CDocWindow *)windows.ItemAt( i );

			if (w->windowNumber == 0)
			{
				w->windowNumber = GetUniqueWindowNum();
				w->RecalcWindowTitle();
			}
		}
	}
}

void CDocument::RemoveWindow( CDocWindow *inWin )
{
	int32				plainWindowCount = 0;

	StSubjectLock( *this, Lock_Exclusive );

	windows.RemoveItem( inWin );
	for (int i = 0; i < windows.CountItems(); i++)
	{
		CDocWindow		*w = (CDocWindow *)windows.ItemAt( i );
		
		if (w->windowNumber >= 0) plainWindowCount++;
		
		w->updateMenus = true;
	}
	
	if (plainWindowCount == 1)
	{
		for (int i = 0; i < windows.CountItems(); i++)
		{
			CDocWindow		*w = (CDocWindow *)windows.ItemAt( i );
		
			if (w->windowNumber > 0)
			{
				w->windowNumber = 0;
				w->RecalcWindowTitle();
			}
		}
	}

		//	If there are any other observers hanging on to this, then
		//	ask them to please observe something else.
	if (windows.CountItems() == 0) RequestDelete();
}

bool CDocument::GetName( char *outName )
{
	return (docLocation.GetName( outName ) == B_NO_ERROR);
}

void CDocument::BuildWindowMenu( BMenu *inMenu, CDocWindow *inSelected )
{
	StSubjectLock( *this, Lock_Shared );
	
	for (int i = 0; i < windows.CountItems(); i++)
	{
		CDocWindow		*w = (CDocWindow *)windows.ItemAt( i );
		BMenuItem		*mi;
		
		mi = new BMenuItem( w->Title(), new BMessage( Activate_ID ), 0, 0 );
		mi->SetTarget( w );
		if (w == inSelected) mi->SetMarked( true );

		inMenu->AddItem( mi );
	}
}

void CDocument::Save()
{
	if (named == false)
	{
		SaveAs();
		return;
	}
	
	SaveDocument();
}

void CDocument::SaveAs()
{
	if (savePanel == NULL) savePanel = CreateSavePanel();
	CRefCountObject::Acquire();
	savePanel->Show();
}

BFilePanel *CDocument::CreateSavePanel()
{
	BMessage		saveMsg( B_SAVE_REQUESTED );
	BFilePanel		*savePanel;
		
	saveMsg.AddPointer( "Document", this );

		// Create a new save panel
	savePanel = new BFilePanel(	B_SAVE_PANEL,
							((CDocApp *)be_app)->Messenger(),
							NULL, B_FILE_NODE, false, &saveMsg );

		// Set the save panel to point to the directory from where we loaded.
	BEntry		pEntry;
	
	if (docLocation.GetParent( &pEntry ) == B_NO_ERROR)
	{
		savePanel->SetPanelDirectory( &pEntry );
	}
	
	return savePanel;
}

#if 0
OnDocNameChanged()
{
	StSubjectLock	this;

		//	REM: Iterate through all windows and call OnDocNameChanged...
		//	Or, is that better done through the observer mechanism? Probably!
	OnDocNameChanged();
}
#endif
