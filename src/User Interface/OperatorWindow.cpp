/* ===================================================================== *
 * OperatorWindow.cpp (MeV/User Interface)
 * ===================================================================== */

#include "OperatorWindow.h"
#include "MultiColumnListView.h"
#include "Idents.h"
#include "EventOp.h"
#include "EventTrack.h"
#include "MeVPlugin.h"
#include "StWindowUtils.h"
#include "MeVApp.h"

// Interface Kit
#include <Button.h>
#include <MenuBar.h>

class COperatorListItem : public CMultiColumnListItem {
	friend class		COperatorWindow;

	EventOp			*op;
	bool				realTime,
					trackPtr,
					listen,
					listenAll,
					saveListen,
					saveListenAll;

public:
	COperatorListItem( EventOp *inOp )
		: CMultiColumnListItem( this )
	{
		op = inOp;
		realTime = inOp->RealTimeOK();
		listen = listenAll = trackPtr = false;
	}

	int32 GetFieldIntData( int32 inIndex )
	{
		switch (inIndex) {
		case 0:	return (realTime && trackPtr) ? (listen ? 1 : 0) : 2;
		case 1: return realTime ? (listenAll ? 1 : 0) : 2;
		default:
			return 0;
		}
		return 2;
	}

	/*const*/ char *GetFieldStringData( int32 inIndex )
	{
		if (inIndex == 2)
		{
			if (op->Name()) return (char *)op->Name();
			else return (char *)op->UndoDescription();
		}
		else if (inIndex == 3) return (char *)op->CreatorName();

		return NULL;
	}

	void *GetFieldData( int32 inIndex )
	{
		return NULL;
	}
};

COperatorWindow::COperatorWindow( CWindowState &inState, CMeVDoc &inDocument )
	: CDocWindow( inState, inDocument, "Event Operators", B_TITLED_WINDOW, B_NOT_H_RESIZABLE ),
	  CObserver( *this, &inDocument ),
	  plugInMenuInstance( ((CMeVApp *)be_app)->operWindowPlugIns )
{
	BRect			r( inState.Rect() );
	int32			x, y;
	BButton			*bb;
	BMenu			*menu;
	
	watchTrack = NULL;
	
	SetSizeLimits( r.Width(), r.Width(), 100.0, 5000.0 );
	
		// view rect should be same size as window rect but with left top at (0, 0)

	menus = new BMenuBar( BRect( 0,0,0,0 ), NULL );

		// Create the file menu
	menu = new BMenu( "File" );
#if 0
	menu->AddItem( new BMenuItem( "Load Virtual Channels...", new BMessage( MENU_ABOUT ), 'L' ) );
	menu->AddItem( new BMenuItem( "Save Virtual Channels...", new BMessage( MENU_ABOUT ), 'S' ) );
	menu->AddItem( new BMenuItem( "Save As Default", new BMessage( MENU_ABOUT ) ) );
#endif
	menu->AddItem( new BMenuItem( "Close Window", new BMessage( B_QUIT_REQUESTED ), 'W' ) );
	menus->AddItem( menu );

		// Create the edit menu
	menu = new BMenu( "Edit" );
	menu->AddItem( new BMenuItem( "Copy", new BMessage( B_COPY ), 'C' ) );
	menu->AddItem( new BMenuItem( "Paste", new BMessage( B_COPY ), 'V' ) );
	menus->AddItem( menu );

	BMessage		*trackMsg = new BMessage( '0000' );
	trackMsg->AddInt32( "DocumentID", (int32)&inDocument );

	plugInMenu = new BMenu( "Plug-Ins" );
	plugInMenuInstance.SetBaseMenu( plugInMenu );
	plugInMenuInstance.SetMessageAttributes( trackMsg );
	menus->AddItem( plugInMenu );

		// REM: Create the windows menu here (to be able to re-open track windows...)

		// Add the menus
	AddChild( menus );

	r.OffsetTo( B_ORIGIN );

	BView		*bv = new BView( r, NULL, B_FOLLOW_ALL_SIDES, B_WILL_DRAW );
	
	AddChild( bv );
	bv->SetViewColor( 220, 220, 220 );
	
	operList = new CMultiColumnListView(
		BRect( r.left + 7, r.top + 24, r.right - 7, r.bottom - 46 ),
		NULL, B_MULTIPLE_SELECTION_LIST, B_FOLLOW_ALL );

	CColumnField *cf;
	
	new CCheckmarkColumnField		( *operList, 50, 0, "Listen" );
	new CCheckmarkColumnField		( *operList, 50, 0, "Listen All" );
	cf = new CStringColumnField	( *operList, 50, 2, "Name" );
	cf->SetAlignment( B_ALIGN_LEFT );
	cf = new CStringColumnField	( *operList, 50, 1, "Kind" );
	cf->SetAlignment( B_ALIGN_LEFT );
	
	operList->SetSelectionMessage( new BMessage( Select_ID ) );
	operList->SetSelectionMessage( new BMessage( Select_ID ) );
	operList->SetColumnClickMessage( new BMessage( ClickCheckbox_ID ) );
	
	bv->AddChild( operList );

	for (int i = 0; i < inDocument.CountOperators(); i++)
	{
			// Add item sorted by some field...
		operList->AddItem( new COperatorListItem( inDocument.OperatorAt( i ) ) );
	}
	
	x = static_cast<int32>(r.right) - 14;
	y = static_cast<int32>(r.bottom) - 35;

	x -= 90;
	bb = new BButton(	BRect( x, y, x + 80, y + 25 ),
						"Close", "Close",
						new BMessage( Close_ID ),
						B_FOLLOW_RIGHT | B_FOLLOW_BOTTOM );
	bv->AddChild( bb );
	bb->MakeDefault( true );

	x -= 96;
	applyButton = new BButton(	BRect( x, y, x + 80, y + 25 ),
							"Apply", "Apply",
							new BMessage( Apply_ID ),
							B_FOLLOW_RIGHT | B_FOLLOW_BOTTOM );
	bv->AddChild( applyButton );

#if 0
	x -= 90;
	bb = new BButton(	BRect( x, y, x + 80, y + 25 ),
						"Revert", "Revert",
						new BMessage( Revert_ID ),
						B_FOLLOW_RIGHT | B_FOLLOW_BOTTOM );
	bv->AddChild( bb );
#endif
	
	x -= 90;
	editButton = new BButton(	BRect( x, y, x + 80, y + 25 ),
							"Edit...", "Edit...",
							new BMessage( Edit_ID ),
							B_FOLLOW_RIGHT | B_FOLLOW_BOTTOM );
	bv->AddChild( editButton );
	applyButton->SetEnabled( false );
	editButton->SetEnabled( false );
}

COperatorWindow::~COperatorWindow()
{
	DeleteListItems( operList );
}

void COperatorWindow::MessageReceived( BMessage *msg )
{
	CMeVDoc			*doc = (CMeVDoc *)&document;
	int32			index;
	COperatorListItem	*li;

	switch (msg->what) {
	case Select_ID:
//		applyButton->SetEnabled( true );

		index = operList->CurrentSelection();
		li = (COperatorListItem *)operList->ItemAt( index );
		if (	li == NULL
			|| li->op == NULL
			|| li->op->Creator() == NULL
			|| li->op->Creator()->EditOperator( li->op, true ) == false)
		{
			editButton->SetEnabled( false );
		}
		else editButton->SetEnabled( true );
		break;
	
	case Close_ID:
		PostMessage( B_QUIT_REQUESTED );
		break;

	case Apply_ID:
		break;
		
	case Edit_ID:
	
			// Edit the operator
		index = operList->CurrentSelection();
		li = (COperatorListItem *)operList->ItemAt( index );
		if (	li != NULL && li->op != NULL && li->op->Creator() != NULL)
			li->op->Creator()->EditOperator( li->op, false );
		break;

#if 0
	case Revert_ID:
		break;
#endif

	case ClickCheckbox_ID:
		int32				col,
							row;
					
		msg->FindInt32( "cell", 0, &row );
		msg->FindInt32( "cell", 1, &col );

		li = (COperatorListItem *)operList->ItemAt( row );
		if (li->realTime == false) break;

		if (col == 0)
		{
			if (watchTrack != NULL)
			{
					// Add/Remove this operator to the track and recompile lists.
				li->listen = !li->listen;
				watchTrack->SetOperatorActive( li->op, li->listen );
				operList->InvalidateItem( row );

				// Notify everyone that things have changed.
			}
		}
		else if (col == 1)
		{
				// Add this operator to global track list, and recompile lists.
			li->listenAll = !li->listenAll;
			doc->SetOperatorActive( li->op, li->listenAll );
			operList->InvalidateItem( row );

				// Notify everyone that things have changed.
		}
		break;

	case Update_ID:
	case Delete_ID:
		CObserver::MessageReceived( msg );
		break;

	default:
		CDocWindow::MessageReceived( msg );
		break;
	}
}

void COperatorWindow::MenusBeginning()
{
	plugInMenuInstance.CheckMenusChanged();
	CDocWindow::MenusBeginning();
}

void COperatorWindow::SetTrack( CEventTrack *inViewTrack )
{
	StSubjectLock	lock( *inViewTrack, Lock_Shared );
	StWindowLocker	wLock( this );
	CMeVDoc			*doc = (CMeVDoc *)&document;

	if (watchTrack != inViewTrack)
	{
		watchTrack = inViewTrack;
	
		for (int i = 0; i < operList->CountItems(); i++)
		{
			COperatorListItem	*li = (COperatorListItem *)operList->ItemAt( i );
			bool					saveTkPt = li->trackPtr;
		
			li->saveListen		= li->listen;
			li->saveListenAll	= li->listenAll;
			
			if (inViewTrack)
			{
				li->listen = inViewTrack->OperatorIndex( li->op ) >= 0;
				li->trackPtr = true;
			}
			else
			{
				li->listen = li->trackPtr = false;
			}
			
			li->listenAll = doc->ActiveOperatorIndex( li->op ) >= 0;
			
			if (		li->saveListen	!= li->listen
				||	li->saveListenAll!= li->listenAll
				||	saveTkPt			!= li->trackPtr)
			{
				operList->InvalidateItem( i );
			}
		}	
	}
}

void COperatorWindow::OnUpdate( BMessage *inMsg )
{
	int32		docHint;
	CMeVDoc		*doc = (CMeVDoc *)Document();

	if (inMsg->FindInt32( "DocAttrs",   0, &docHint ) != B_OK)		docHint = 0;
	
	if (docHint & CMeVDoc::Update_OperList)
	{
		operList->Hide();
		DeleteListItems( operList );
		for (int i = 0; i < doc->CountOperators(); i++)
		{
				// Add item sorted by some field...
			operList->AddItem( new COperatorListItem( doc->OperatorAt( i ) ) );
		}
		operList->Show();
	}
	else if (docHint & CMeVDoc::Update_Operator)
	{
		operList->Invalidate();
	}
}
