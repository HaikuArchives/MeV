/* ===================================================================== *
 * MidiConfigWindow.cpp (MeV/User Interface)
 * ===================================================================== */

#include "MidiConfigWindow.h"
#include "MultiColumnListView.h"
#include "MidiDeviceInfo.h"
#include "Event.h"
#include "PlayerControl.h"
#include "Idents.h"
#include "BeFileReader.h"

// Gnu C Library
#include <stdio.h>
#include <ctype.h>
// Interface Kit
#include <Button.h>
#include <MenuBar.h>
#include <OutlineListView.h>
#include <ScrollView.h>
#include <StringView.h>
#include <TextControl.h>
// Storage Kit
#include <FilePanel.h>
// Support Kit
#include <Debug.h>

class CListDataItem :
	public BListItem
{

	float		fBaselineOffset;
	BList			children;
public:

	CListDataItem( uint32 outlineLevel = 0, bool expanded = true )
		: BListItem( outlineLevel, expanded ) {}
		
	virtual const char *Text() = 0;

	void	 DrawItem( BView *owner, BRect frame, bool complete = false );
	void	 Update( BView *owner, const BFont *font );
};

void	CListDataItem::DrawItem( BView *owner, BRect frame, bool complete )
{
	if ( complete || IsSelected() )
	{
		if ( IsSelected() )	owner->SetHighColor( 192, 192, 192 );
		else				owner->SetHighColor( owner->ViewColor() );

		owner->FillRect( frame, B_SOLID_HIGH );
	}

	if (IsEnabled())		owner->SetHighColor( 0, 0, 0 );
	else					owner->SetHighColor( 127, 127, 127 );

	owner->DrawString( Text(), BPoint( frame.left + 2, frame.top + fBaselineOffset ) );
}

void	 CListDataItem::Update( BView *owner, const BFont *font )
{
	BListItem::Update( owner, font );

	font_height	fh;

	owner->GetFontHeight( &fh );
	fBaselineOffset = (Height() - fh.descent + fh.ascent) / 2;
}

class CPortListItem : public CListDataItem {
public:
	int32		portNum;

	CPortListItem( int inPortNum ) : CListDataItem( 0, true )
	{
		portNum = inPortNum;
	}
	
	const char *Text() { return CPlayerControl::PortName( portNum ); }
};

class CDevListItem : public CListDataItem {
public:
	MIDIDeviceInfo	*mdi;
	char				name[ 64 ];

	CDevListItem( MIDIDeviceInfo *inInfo ) : CListDataItem( 1, false )
	{
		mdi = inInfo;
	}

	const char *Text()
	{
		if (mdi->lowChannel < mdi->highChannel)
			sprintf( name, "%s [%d-%d]", mdi->name, mdi->lowChannel + 1, mdi->highChannel + 1 );
		else sprintf( name, "%s [%d]", mdi->name, mdi->lowChannel + 1 );

		return name;
	}
};

class CPatchListItem : public CMultiColumnListItem {
	friend int comparePatches( const void *_a, const void *_b );
public:

	MIDIDeviceInfo	*mdi;
	int32			patchBank,
					patchNum;

	CPatchListItem( MIDIDeviceInfo *inMDI, int32 inBank, int32 inIndex )
		: CMultiColumnListItem( NULL )
	{
		mdi = inMDI;
		patchBank = inBank;
		patchNum = inIndex;
	}
	
	int32 GetFieldIntData( int32 inIndex )
	{
		if (inIndex == 0) return patchBank;
		if (inIndex == 1) return patchNum;
		return 0;
	}

	char *GetFieldStringData( int32 inIndex )
	{
		return (char *)mdi->GetPatchName( patchBank, patchNum );
	}

	void *GetFieldData( int32 inIndex )
	{
		return NULL;
	}
};

CMidiConfigWindow::CMidiConfigWindow( CWindowState &inState )
	: CAppWindow( inState, inState.Rect(), "MIDI Configuration", B_TITLED_WINDOW, B_NOT_H_RESIZABLE /* | B_NOT_ZOOMABLE */ ),
	  messenger( this )
{
	BView		*background;
	BRect		r( inState.Rect() );
	BButton		*bb;
	BMenuBar	*menus;
	BMenu		*menu;
	
	r.OffsetTo( B_ORIGIN );

		// view rect should be same size as window rect but with left top at (0, 0)
	menus = new BMenuBar( BRect( 0,0,0,0 ), NULL );
		// Create the file menu
	menu = new BMenu( "File" );
	menu->AddItem( loadMenu = new BMenuItem( "Load Program Names...", new BMessage( MENU_OPEN ), 'O' ) );
	menu->AddItem( saveMenu = new BMenuItem( "Save Program Names...", new BMessage( MENU_SAVE ), 'S' ) );
	menu->AddSeparatorItem();
	menu->AddItem( new BMenuItem( "Close Window", new BMessage( B_QUIT_REQUESTED ), 'W' ) );
	menu->AddItem( new BMenuItem( "Quit", new BMessage( MENU_QUIT ), 'Q' ) );
	menus->AddItem( menu );

	menu = new BMenu( "Edit" );
	menu->AddItem( new BMenuItem( "Clear program list", new BMessage( 'clrl' ) ) );
	menus->AddItem( menu );

		// Add the menus
	AddChild( menus );

		// Save current device list in prefs structure
	((CMeVApp *)be_app)->GetDevicePrefs( &deviceListSave );

		// Grey background
	background = new BView( r, NULL, B_FOLLOW_ALL_SIDES, B_WILL_DRAW );
	AddChild( background );
	background->SetViewColor( 220, 220, 220 );

		// String label of device list
	background->AddChild( new BStringView( BRect( 8, 21, 200, 36 ), "", "Hardware Ports & Instruments" ) );

		// Device and instrument list
	devList = new BOutlineListView( BRect( 8, 38, 184, r.bottom - 92 ), NULL, B_SINGLE_SELECTION_LIST );
	devList->SetSelectionMessage( new BMessage( 'dev#' ) );
	devList->SetInvocationMessage( new BMessage( 'attr' ) );
	BScrollView *scv = new BScrollView(	NULL,
									devList,
									B_FOLLOW_TOP_BOTTOM | B_FOLLOW_LEFT,
									0,
									false, true,
									B_PLAIN_BORDER );
	background->AddChild( scv );
	
		// Buttons associated with device list
	background->AddChild( attrButton = new BButton(	BRect( 6, r.bottom - 88, 200, r.bottom - 70 ),
												"EditAttributes", "Edit Attributes...",
												new BMessage( 'attr' ), B_FOLLOW_BOTTOM ) );

	background->AddChild( addButton = new BButton(	BRect( 6, r.bottom - 62, 98, r.bottom - 44 ),
												"AddInst", "Add Instrument...",
												new BMessage( 'addi' ), B_FOLLOW_BOTTOM ) );

	background->AddChild( subButton = new BButton(	BRect( 100, r.bottom - 62, 200, r.bottom - 44 ),
												"SubInst", "Remove Instrument",
												new BMessage( 'remi' ), B_FOLLOW_BOTTOM ) );

		// String label for patch list
	background->AddChild( new BStringView( BRect( 208, 21, 400, 36 ), "", "Program Names" ) );

		// List of instrument patches
	patchList = new CMultiColumnListView(
		BRect( 208, 37, r.right - 7, r.bottom - 66 ), NULL, B_SINGLE_SELECTION_LIST, B_FOLLOW_ALL );

	CColumnField *cf;
	new CNumericColumnField		( *patchList, 32,  0, "Bank" );
	new CNumericColumnField		( *patchList, 32,  0, "#" );
	cf = new CStringColumnField		( *patchList, 10, 1, "Patch Name" );
	cf->SetAlignment( B_ALIGN_LEFT );

	patchList->SetSelectionMessage( new BMessage( 'pat#' ) );
	background->AddChild( patchList );

		// Patch name edit box
	patchName = new BTextControl( BRect( 204, r.bottom - 59, 352, r.bottom - 40 ), "Name", "", "", new BMessage( 'name' ),
		B_FOLLOW_LEFT | B_FOLLOW_BOTTOM, B_WILL_DRAW );
	background->AddChild( patchName );
	patchName->SetTarget( this );
	patchName->SetViewColor( 220, 220, 220 );
	patchName->SetDivider( 0.0 );
	patchName->MakeFocus();

		// Buttons associated with patches
	background->AddChild( addpButton = new BButton(	BRect( 354, r.bottom - 62, 402, r.bottom - 44 ),
												"AddProg", "Add..",
												new BMessage( 'addp' ), B_FOLLOW_BOTTOM ) );

	background->AddChild( subpButton = new BButton(	BRect( 404, r.bottom - 62, 453, r.bottom - 44 ),
												"SubProg", "Remove",
												new BMessage( 'remp' ), B_FOLLOW_BOTTOM ) );

		// Buttons associated with dialog as a whole.
	background->AddChild( new BButton(	BRect(	r.right - 82 - 105, r.bottom - 30, r.right - 6 - 105, r.bottom - 10 ),
									"Revert", "Revert",
									new BMessage( Revert_ID ),
									B_FOLLOW_LEFT | B_FOLLOW_BOTTOM ) );

	background->AddChild( bb = new BButton(	BRect(	r.right - 100, r.bottom - 30, r.right - 10, r.bottom - 10 ),
										"Close", "Close",
										new BMessage( Close_ID ),
										B_FOLLOW_LEFT | B_FOLLOW_BOTTOM ) );
	bb->MakeDefault( true );

	BuildPortList();

	subButton->SetEnabled( false );
	subpButton->SetEnabled( false );
	addpButton->SetEnabled( false );
}

CMidiConfigWindow::~CMidiConfigWindow()
{
	DeleteListItems( patchList );
}

bool CMidiConfigWindow::QuitRequested()
{
	((CMeVApp *)be_app)->CancelEditDeviceAttrs();
	((CMeVApp *)be_app)->CancelEditPortAttrs();
	return true;
}

void CMidiConfigWindow::InsertDevice( CDevListItem *item )
{
	CPortListItem		*pli;
	CDevListItem		*dli;
	MIDIDeviceInfo	*info = item->mdi;
	int				i;

	int devCount = devList->CountItems();
	for (i = 0; i < devCount; i++)
	{
		CListDataItem	*cp = (CListDataItem *)devList->ItemAt( i );

		if ((pli = dynamic_cast<CPortListItem *>(cp)))
		{
			if (pli->portNum > info->portNum) break;
		}
		else if ((dli = dynamic_cast<CDevListItem *>(cp)))
		{
			if (dli->mdi->portNum > info->portNum) break;
			if (dli->mdi->portNum < info->portNum) continue;
			if (dli->mdi->lowChannel > info->lowChannel) break;
			if (dli->mdi->lowChannel < info->lowChannel) continue;
			if (dli->mdi->highChannel > info->highChannel) break;
			if (dli->mdi->highChannel < info->highChannel) continue;
			if (strcmp( dli->mdi->name, info->name ) > 0) break;
		}
	}
	
	devList->AddItem( item, i );
}

/*	Functions we'd like to have:
	For any list item, find the next/previous item at the same level, regardless of expansion/collapse.
	For any list item, find the first child item.
	A way to count the total number of child items (recursive) under an item.
	A way to get the Nth item at a particular level.
	A way to move an item and all of it's child items to another point in the list at the same level.
*/

void CMidiConfigWindow::BuildPortList()
{
	CMeVApp		*app = (CMeVApp *)be_app;
	int			devCount;
	int32		devSel = devList->CurrentSelection();
	CListDataItem	*cp = (CPortListItem *)devList->ItemAt( devSel ),
				*selItem = NULL;
	CPortListItem	*pli = dynamic_cast<CPortListItem *>(cp);
	CDevListItem	*dli = dynamic_cast<CDevListItem *>(cp);

	DeleteListItems( devList );

	for (int i = 0; i < 16; i++)
	{
		devList->AddItem( cp = new CPortListItem( i ) );
		if (pli && pli->portNum == i) selItem = cp;
	}

	devCount = app->CountDevices();
	for (int j = 0; j < devCount; j++)
	{
		MIDIDeviceInfo	*mdi = app->DeviceAt( j );
		CDevListItem		*d;

		InsertDevice( d = new CDevListItem( mdi ) );
		if (dli && dli->mdi == mdi) selItem = d;
	}

	if (selItem)	devList->Select( devList->IndexOf( selItem ) );
	else			devList->Select( 0 );
}

int comparePatches( const void *_a, const void *_b )
{
	CPatchListItem		*a = *(CPatchListItem **)_a,
					*b = *(CPatchListItem **)_b;

	if (a->patchBank > b->patchBank) return 1;
	if (a->patchBank < b->patchBank) return -1;
	if (a->patchNum > b->patchNum) return 1;	
	if (a->patchNum < b->patchNum) return -1;
	return 0;
}

void CMidiConfigWindow::BuildPatchList( MIDIDeviceInfo *mdi )
{
//	CMeVApp			*app = (CMeVApp *)be_app;
	CPatchListItem		*pli;
	CDictionary<uint32,PatchInfo>::Iterator	iter( mdi->patches );
	BList				tempList;

	subButton->SetEnabled( true );
	addpButton->SetEnabled( true );
	saveMenu->SetEnabled( true );
	loadMenu->SetEnabled( true );

	uint32			*key;
	
		// Dereference all patch banks...
	for (key = iter.First(); key; key = iter.Next())
	{
		int32		patch = *key & 0x7f,
					bank = *key >> 7;
		pli = new CPatchListItem( mdi, bank, patch );
		
		tempList.AddItem( pli );
	}
	
	tempList.SortItems( comparePatches );

	patchList->Hide();
	DeleteListItems( patchList );
	patchList->AddList( &tempList );
	patchList->Show();
}

void CMidiConfigWindow::ClearPatchList()
{
	subButton->SetEnabled( false );
	subpButton->SetEnabled( false );
	addpButton->SetEnabled( false );
	saveMenu->SetEnabled( false );
	loadMenu->SetEnabled( false );

	patchList->Hide();
	DeleteListItems( patchList );
	patchList->Show();
}

void CMidiConfigWindow::MessageReceived( BMessage *msg )
{
	int32			devIndex = devList->CurrentSelection(),
					patIndex = patchList->CurrentSelection();
	CMeVApp			*app = (CMeVApp *)be_app;
	CListDataItem		*cp = (CPortListItem *)devList->ItemAt( devIndex );
	CPortListItem		*pli = NULL;
	CDevListItem		*dli = NULL;
	CPatchListItem		*pai = (CPatchListItem *)patchList->ItemAt( patIndex );
	MIDIDeviceInfo	*info;
	int32			portIndex;
	BFilePanel		*fp;

	switch (msg->what) {
	case MENU_OPEN:
			// Open the file requester and all that bloody business...
			// It's a text file this time...
			// REM: How do we know which one was selected at the time?
		if ((fp = app->GetImportPanel( &messenger )))
		{
			fp->Show();
		}
		break;
	
	case MENU_SAVE:
			// Open the file requester and all that bloody business...
			// It's a text file this time...
			// REM: How do we know which one was selected at the time?
		if ((fp = app->GetExportPanel( &messenger )))
		{
			fp->Show();
		}
		break;
	
	case MENU_QUIT:
		be_app->PostMessage( B_QUIT_REQUESTED );
		break;
		
	case B_SAVE_REQUESTED:
		if ((dli = dynamic_cast<CDevListItem *>(cp)))
		{
			const char		*name;
			entry_ref		eref;

			if (	msg->FindRef( "directory", 0, &eref ) == B_OK
				&& msg->FindString( "name", 0, &name ) == B_OK)
			{
				BDirectory	dir( &eref );
				CheckBeError( dir.InitCheck() );
			
				BFile			file( &dir, name, B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE );
				CheckBeError( file.InitCheck() );
				
					//	Now, we're ready to write the data I think
				for (int i = 0; i < patchList->CountItems(); i++)
				{
					char			line[ 128 ];
					int			len;

					pai = (CPatchListItem *)patchList->ItemAt( i );
					len = sprintf(	line, "%ld\t%ld\t%s\n",
								pai->patchBank, pai->patchNum, 
								pai->mdi->GetPatchName( pai->patchBank, pai->patchNum ) );
								
					file.Write( line, len );
				}
			}
		}
		break;
		
	case B_REFS_RECEIVED:
		if ((dli = dynamic_cast<CDevListItem *>(cp)))
		{
			entry_ref		eref;

			if (	msg->FindRef( "refs", 0, &eref ) == B_OK)
			{
				BFile			file( &eref, B_READ_ONLY );
				CheckBeError( file.InitCheck() );

				char		line[ 256 ];
				int32	len = 0,
						actual = 0,
						scan = 0,
						cr = 0;
				bool		first = true;

				for (;;)
				{
					actual = file.Read( line + len, sizeof line - len - 1 );
					if (actual <= 0)
					{
						if (scan >= len) break;
					}
					else len += actual;
					scan = 0;

					while (scan < 128 && scan < len)
					{
						int32		bank = 0, index = 0;

						for (cr = scan; cr < len; cr++)
						{
							if (line[ cr ] == '\n') break;
						}
						
						if (cr <= scan)
						{
							scan++;
							continue;
						}

							// Skip WS
						while (isspace( line[ scan ] ) && scan < cr) scan++;
						
							// Parse bank #
						while (isdigit( line[ scan ] ) && scan < cr)
						{
							bank = bank * 10 + line[ scan ] - '0'; scan++;
						}
						
							// Skip WS
						while (isspace( line[ scan ] ) && scan < cr) scan++;
						
							// Parse bank #
						while (isdigit( line[ scan ] ) && scan < cr)
						{
							index = index * 10 + line[ scan ] - '0'; scan++;
						}
						
							// Skip WS
						while (line[ scan ] == '\t' && scan < cr) scan++;

							// Parse name
						if (scan < cr)
						{
							if (first)
							{
								dli->mdi->patches.MakeEmpty();
								first = false;
							}

							line[ cr++ ] = 0;
							dli->mdi->SetPatchName( bank, index, &line[ scan ] );
						}
						
						scan = cr;
					}

						// Eliminate line from buffer
					if (scan < len) 
					{
						memmove( line, &line[ scan ], len - scan );
						len -= scan;
					}
					else len = 0;

					scan = 0;
				}

				BuildPatchList( dli->mdi );
			}
		}
		break;
	
	case Close_ID:
		PostMessage( B_QUIT_REQUESTED );
		break;
		
	case Revert_ID:
		((CMeVApp *)be_app)->SetDevicePrefs( &deviceListSave );

		BuildPortList();
		
		devIndex = devList->CurrentSelection();
		cp = (CPortListItem *)devList->ItemAt( devIndex );

		if ((dli = dynamic_cast<CDevListItem *>(cp)))
		{
			BuildPatchList( dli->mdi );
		}
		else
		{
			ClearPatchList();
		}
		break;

	case 'dev#':

		if ((dli = dynamic_cast<CDevListItem *>(cp)))
		{
			BuildPatchList( dli->mdi );
		}
		else
		{
			ClearPatchList();
		}
		break;
		
	case 'pat#':
		if (pai)
		{
			subpButton->SetEnabled( true );
			patchName->SetText( pai->GetFieldStringData( 2 ) );
		}
		break;

	case 'attr':
		if ((pli = dynamic_cast<CPortListItem *>(cp)))
		{
				// Open up port attrs
			app->EditPortAttrs( pli->portNum );
		}
		else if ((dli = dynamic_cast<CDevListItem *>(cp)))
		{
				// Open up instrument attrs
			app->EditDeviceAttrs( dli->mdi, dli->mdi->portNum );
		}
		break;
	
	case 'addi':
		if ((pli = dynamic_cast<CPortListItem *>(cp)))
		{
				// Add instrument
			portIndex = pli->portNum;
		}
		else if ((dli = dynamic_cast<CDevListItem *>(cp)))
		{
				// Add patch bank
			portIndex = dli->mdi->portNum;
		}
		else break;

			// Add an instrument to that port...
		app->EditDeviceAttrs( NULL, portIndex );
		break;
	
	case 'remi':
		if ((dli = dynamic_cast<CDevListItem *>(cp)))
		{
			((CMeVApp *)be_app)->CancelEditDeviceAttrs();

				// delete this instrument
			app->DeleteDevice( dli->mdi );
			devList->RemoveItem( cp );
			delete cp;
		}
		break;
		
	case 'remp':
		if (pai)
		{
			ASSERT( pai->mdi );
			ASSERT( pai->mdi->GetPatch( pai->patchBank, pai->patchNum ) );
			ASSERT( patchList->HasItem( pai ) );
		
				// delete this patch
			((CMeVApp *)be_app)->CancelEditPatchAttrs();
			pai->mdi->DeletePatch( pai->patchBank, pai->patchNum );
			patchList->RemoveItem( pai );
		}
		break;
		
	case 'name':
		if (pai)
		{
				// Change the name of the patch.
			pai->mdi->SetPatchName( pai->patchBank, pai->patchNum, patchName->Text() );
			patchList->InvalidateItem( patchList->IndexOf( pai ) );
		}
		break;
	
	case 'addp':
		if ((dli = dynamic_cast<CDevListItem *>(cp)))
		{
				// Add an patch to this instrument
			app->EditPatchAttrs( dli->mdi );
		}
		break;
		
	case 'chpt':		// Port changed...
		if (msg->FindInt32( "port", &portIndex ) == B_NO_ERROR)
		{
			char *description, *name;

			if (	msg->FindString( "name", (const char **)&name ) != B_NO_ERROR
				|| 	msg->FindString( "description", (const char **)&description ) != B_NO_ERROR)
					break;

			if (CPlayerControl::SetPortDevice( portIndex, name ) == false)
			{
				app->Error( "Invalid device name" );
				return;
			}

			CPlayerControl::SetPortName( portIndex, description );
			
				// Find record for device...
			for (int i = 0; i < devList->CountItems(); i++)
			{
				cp = (CListDataItem *)devList->ItemAt( i );
				if ((pli = dynamic_cast<CPortListItem *>(cp)))
				{
					if (portIndex == pli->portNum) break;
				}
			}
				
			if (pli != NULL)
			{
				devList->InvalidateItem( devList->IndexOf( pli ) );
			}
		}
		break;

	case 'chdv':		// Device changed...
	
		if (msg->FindPointer( "device", (void **)&info ) == B_NO_ERROR)
		{
			char	*name;
			int8	lowChannel,
					highChannel,
					baseChannel,
					portNum;

			if (	msg->FindString( "name", (const char **)&name ) != B_NO_ERROR
				|| 	msg->FindInt8( "low", &lowChannel ) != B_NO_ERROR
				|| 	msg->FindInt8( "high", &highChannel ) != B_NO_ERROR
				|| 	msg->FindInt8( "base", &baseChannel ) != B_NO_ERROR
				|| 	msg->FindInt8( "port", &portNum ) != B_NO_ERROR)
					break;
		
			if (info == NULL)
			{
				info = app->NewDevice();
				info->SetPort( portNum );
				dli = new CDevListItem( info );
			}
			else
			{
					// Find record for device...
				for (int i = 0; i < devList->CountItems(); i++)
				{
					cp = (CListDataItem *)devList->ItemAt( i );
					if ((dli = dynamic_cast<CDevListItem *>(cp)))
					{
						if (info == dli->mdi) break;
					}
				}
				
				if (info == NULL) break;
				devList->RemoveItem( dli );
			}

			info->SetName( name );
			info->SetChannels( lowChannel, highChannel, baseChannel );

			InsertDevice( dli );
			devList->Select( devList->IndexOf( dli ) );
			app->CalcDeviceTable();
		}
		break;

	case 'chpg':		// Patch changed...
		int32		index,
					bank;
		char			*name;
	
		if (	pai != NULL
			&& msg->FindInt32( "index", &index ) == B_NO_ERROR
			&& msg->FindInt32( "bank", &bank ) == B_NO_ERROR
			&& msg->FindString( "description", (const char **)&name ) == B_NO_ERROR)
		{
			if ((dli = dynamic_cast<CDevListItem *>(cp)))
			{
				dli->mdi->SetPatchName( bank, index, name );
				BuildPatchList( dli->mdi );
			}
		}
		break;

	case 'clrl':
		if ((dli = dynamic_cast<CDevListItem *>(cp)))
		{
			dli->mdi->patches.MakeEmpty();
			BuildPatchList( dli->mdi );
		}
		break;
		
	default:
		CAppWindow::MessageReceived( msg );
		break;
	}
}
