/* ===================================================================== *
 * DocWindow.cpp (MeV/Application Framework)
 * ===================================================================== */

#include "DocWindow.h"

// Gnu C Library
#include <stdio.h>
#include <string.h>
// Interface Kit
#include <Alert.h>
#include <Menu.h>
#include <MenuItem.h>

CDocWindow	*CDocWindow::activeDocWin;

CDocWindow::CDocWindow(
	BRect frame,
	CDocument &inDocument,
	const char *inTypeName,
	window_type wType,
	uint32 flags)
	: CAppWindow(frame, NULL, wType, 0 ),
	  document( inDocument )
{
	windowMenu = NULL;
	windowMenuStart = -1;

	CalcWindowTitle( inTypeName );
	document.Acquire();
	document.AddWindow( this );
	updateMenus = true;
}

CDocWindow::CDocWindow(
	CWindowState &inState,
	CDocument &inDocument,
	const char *inTypeName,
	window_type wType,
	uint32 flags )
	: CAppWindow( inState, inState.Rect(), NULL, wType, 0 ),
	  document( inDocument )
{
	windowMenu = NULL;
	windowMenuStart = -1;

	CalcWindowTitle( inTypeName );
	document.Acquire();
	document.AddWindow( this );
	updateMenus = true;
}

CDocWindow::~CDocWindow()
{
	document.RemoveWindow( this );
	CRefCountObject::Release( &document );
	be_app->Lock();
	if (this == activeDocWin) activeDocWin = NULL;
	be_app->Unlock();
}

void
CDocWindow::CalcWindowTitle(
	const char *inTypeName)
{
	char				name[ B_FILE_NAME_LENGTH + 32 ];
	document.GetName( name );

	if (inTypeName)
	{
		strncat( name, ": ", sizeof name );
		strncat( name, inTypeName, sizeof name );
		windowNumber = -1;
		SetTitle( name );
	}
	else
	{
		windowNumber = 0;
		SetTitle( name );
	}
}

void CDocWindow::RecalcWindowTitle()
{
	char				name[ B_FILE_NAME_LENGTH ];
	char				title[ B_FILE_NAME_LENGTH + 16 ];
	document.GetName( name );

	if (windowNumber > 0)
	{
		sprintf( title, "%s:%d", name, windowNumber );
		SetTitle( title );
	}
	else
	{
		SetTitle( name );
	}
}

void CDocWindow::BuildWindowMenu( BMenu *inMenu )
{
	if (inMenu != NULL)
	{
		if (windowMenuStart < 0)
		{
			windowMenuStart = inMenu->CountItems();
		}
		else
		{
			BMenuItem		*mi;
			
			while ((mi = inMenu->RemoveItem( windowMenuStart ))) delete mi;
		}
	
		document.BuildWindowMenu( inMenu, this );
		updateMenus = false;
	}
}

void CDocWindow::MessageReceived( BMessage *msg )
{
	if (msg->what == Activate_ID)	Activate();
	else CAppWindow::MessageReceived( msg );
}

void CDocWindow::MenusBeginning()
{
	if (updateMenus) BuildWindowMenu( windowMenu );

	CAppWindow::MenusBeginning();
}

bool CDocWindow::QuitRequested()
{
	if (document.WindowCount() == 1 && document.Modified())
	{
		BAlert		*alert;
		long			result;
		char			fName[ B_FILE_NAME_LENGTH ];
		char			text[ 64 + B_FILE_NAME_LENGTH ];
		
		document.GetName( fName );
		sprintf( text, "Save changes to the document \"%s\"?", fName );

		alert = new BAlert(  NULL, text,
							"Don't Save",
							"Cancel",
							"Save",
							B_WIDTH_AS_USUAL, B_INFO_ALERT); 
		alert->SetShortcut( 1, B_ESCAPE );
		result = alert->Go();
		
		if (result == 0) return CAppWindow::QuitRequested();
		if (result == 1) return false;
		if (result == 2)
		{
				// Here's where we have to save the document
			document.Save();
			return true;
		}
		return false;
	}
	return CAppWindow::QuitRequested();
}

	/**	Acquires the active selection token. */
void CDocWindow::AcquireSelectToken()
{
	be_app->Lock();
	if (this != activeDocWin)
	{
		if (activeDocWin)
		{
			BMessage		msg( Select_ID );
			
			msg.AddBool( "active", false );
			
			activeDocWin->PostMessage( &msg, activeDocWin );
		}
		activeDocWin = this;

		BMessage		msg( Select_ID );
		msg.AddBool( "active", true );
		PostMessage( &msg, this );
	}

	be_app->Unlock();
}

void CDocWindow::WindowActivated( bool active )
{
	CAppWindow::WindowActivated( active );

	if (active) AcquireSelectToken();
}
