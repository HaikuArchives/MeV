/* ===================================================================== *
 * TrackWindow.cpp (MeV/User Interface)
 * ===================================================================== */

#include "TrackWindow.h"
#include "MeVApp.h"
#include "Idents.h"
#include "StripView.h"
#include "EventEditor.h"
#include "PlayerControl.h"
#include "StdEventOps.h"
#include "OperatorWindow.h"
#include "MeVDoc.h"
#include "BorderButton.h"
#include "QuickKeyMenuItem.h"
#include "AssemblyRulerView.h"
#include "ResourceUtils.h"

#include "Junk.h"

// Gnu C Library
#include <stdio.h>
// Interface Kit
#include <MenuBar.h>
// Support Kit
#include <Debug.h>
#include <String.h>

// Debugging Macros
#define D_ALLOC(x) //PRINT (x)			// Constructor/Destructor
#define D_MESSAGE(x) //PRINT (x)		// MessageReceived()
#define D_INTERNAL(x) //PRINT (x)		// Internal Operations

// ---------------------------------------------------------------------------
// Constructor/Destructor

CTrackWindow::CTrackWindow(
	BRect frame,
	CMeVDoc &inDocument,
	CEventTrack *inTrack)
	:	CDocWindow(frame, (CDocument &)inDocument),
		CObserver(*this, &inDocument),
		stripFrame(NULL),
		stripScroll(NULL),
		trackOp(NULL),
		newEventType(EvtType_Count),
		newEventDuration(Ticks_Per_QtrNote * 4),
		prefsWinState(BRect(40.0, 40.0, 500.0, 300.0))
{
	D_ALLOC(("TrackWindow::TrackWindow()\n"));

	track = inTrack;
	track->Acquire();
	
	BMessage *trackMsg = new BMessage('0000');
	trackMsg->AddInt32("TrackID", track->GetID());
	trackMsg->AddInt32("DocumentID", (int32)&inDocument);

/*	plugInMenu = new BMenu( "Filters" );
	plugInMenu->AddItem( new BMenuItem( "Event Operators...", new BMessage( 'oper' ) ) );
	plugInMenu->AddSeparatorItem();
	plugInMenuInstance.SetBaseMenu( plugInMenu );
	plugInMenuInstance.SetMessageAttributes( trackMsg );
*/
	SetPulseRate(100000);
}

CTrackWindow::~CTrackWindow()
{
	D_ALLOC(("CTrackWindow::~CTrackWindow()\n"));

	CRefCountObject::Release(trackOp);
	CRefCountObject::Release(track);
}

// ---------------------------------------------------------------------------
// CDocWindow Implementation

void
CTrackWindow::MenusBeginning()
{
	BMenuItem *item = NULL;
	BString itemLabel;
	const char *description;

	// Set up Undo menu
	item = KeyMenuBar()->FindItem(MENU_UNDO);
	description = NULL;
	if (Track()->CanUndo())
	{
		itemLabel = "Undo";
		description = Track()->UndoDescription();
		if (description)
		{
			itemLabel << " " << description;
		}
		if (item)
		{
			item->SetLabel(itemLabel.String());
			item->SetEnabled(true);
		}
	}
	else if (item)
	{
		item->SetLabel("Undo");
		item->SetEnabled(false);
	}

	// Set up Redo menu
	item = KeyMenuBar()->FindItem(MENU_REDO);
	description = NULL;
	if (Track()->CanRedo())
	{
		itemLabel = "Redo";
		description = Track()->RedoDescription();
		if (description)
		{
			itemLabel << " " << description;
		}
		if (item)
		{
			item->SetLabel(itemLabel.String());
			item->SetEnabled(true);
		}
	}
	else if (item)
	{
		item->SetLabel("Redo");
		item->SetEnabled(false);
	}
	
	// Set up Clear menu
	item = KeyMenuBar()->FindItem("Clear");
	if (item)
	{
		item->SetEnabled(ActiveTrack()->SelectionType() != CTrack::Select_None);

	// Set up Window menu
		item = KeyMenuBar()->FindItem("Show Event Inspector");
		item->SetMarked(dynamic_cast<CMeVApp *>(be_app)->Inspector());
		item = KeyMenuBar()->FindItem("Show Grid Window");
		item->SetMarked(dynamic_cast<CMeVApp *>(be_app)->GridWindow());
		item = KeyMenuBar()->FindItem("Show Transport Controls");
		item->SetMarked(dynamic_cast<CMeVApp *>(be_app)->TransportWindow());
	}
//	plugInMenuInstance.CheckMenusChanged();

	CDocWindow::MenusBeginning();
}

void
CTrackWindow::MessageReceived(
	BMessage *message)
{
	switch (message->what)
	{
		case MENU_QUIT:
		{
			be_app->PostMessage( B_QUIT_REQUESTED );
			break;
		}
		case MENU_ABOUT:
		{
			be_app->PostMessage( B_ABOUT_REQUESTED );
			break;
		}
		case MENU_NEW:
		{
			((CDocApp *)be_app)->NewDocument();
			break;
		}
		case MENU_OPEN:
		{
			((CDocApp *)be_app)->OpenDocument();
			break;
		}
		case MENU_SAVE:
		{
			document.Save();
			break;
		}
		case MENU_SAVE_AS:
		{
			document.SaveAs();
			break;
		}
		case MENU_IMPORT:
		{
			((CMeVApp *)be_app)->ImportDocument();
			break;
		}
		case MENU_EXPORT:
		{
			Document().Export( message );
			break;
		}
		case MENU_INSPECTOR:
		{
			((CMeVApp *)be_app)->ShowInspector( ((CMeVApp *)be_app)->Inspector() == NULL );
			Activate();				// In case window was deactivated
			break;
		}
		case MENU_GRIDWINDOW:
		{
			((CMeVApp *)be_app)->ShowGridWindow( ((CMeVApp *)be_app)->GridWindow() == NULL );
			Activate();				// In case window was deactivated
			break;
		}
		case MENU_TRANSPORT:
		{
			((CMeVApp *)be_app)->ShowTransportWindow( ((CMeVApp *)be_app)->TransportWindow() == NULL );
			Activate();				// In case window was deactivated
			break;
		}
		case MENU_ABOUT_PLUGINS:
		{
			be_app->PostMessage( message );
			break;
		}
		case ZoomOut_ID:
		{
			stripFrame->ZoomOut();
			break;
		}
		case ZoomIn_ID:
		{
			stripFrame->ZoomIn();
			break;
		}
		case MENU_PAUSE:
		{
			// Start playing a song.
			CPlayerControl::TogglePauseState( (CMeVDoc *)&document );
			break;
		}
		case LoseFocus_ID:
		{
			// Remove focus from pesky controls.
			if (CurrentFocus()) CurrentFocus()->MakeFocus( false );
			break;
		}
		case B_KEY_DOWN:
		{
			int32		key;
			int32		modifiers;
			CEventTrack	*tk;
			
			tk = ActiveTrack();
	
			message->FindInt32("raw_char", &key);
			message->FindInt32("modifiers", &modifiers);
	
			if (key == B_LEFT_ARROW || key == B_RIGHT_ARROW)
			{
				if (tk->SelectionType() != CTrack::Select_None )
				{
					int32		delta;
					EventOp		*op;
					
					delta = tk->GridSnapEnabled() ? tk->TimeGridSize() : 1;
						
					if (modifiers & B_SHIFT_KEY)
					{
						op = new DurationOffsetOp( key == B_RIGHT_ARROW ? delta : -delta );
	
						tk->ModifySelectedEvents(
							NULL, *op, "Change Duration", EvAttr_Duration );
					}
					else
					{
						if (key == B_LEFT_ARROW)
						{
							if (delta > tk->MinSelectTime()) break;
							delta = -delta;
						}
						op = new TimeOffsetOp( delta );
	
						tk->ModifySelectedEvents(
							NULL, *op, "Move", EvAttr_None );
					}
					
					CRefCountObject::Release( op );
				}
			}
			else if (key == B_UP_ARROW || key == B_DOWN_ARROW)
			{
				if (tk->CurrentEvent() != NULL )
				{
					enum E_EventAttribute		attr = EvAttr_None;
					int32					delta;
					
					if (	key == B_UP_ARROW) delta = -1;
					else delta = 1;
							
					switch (tk->CurrentEvent()->Command()) {
					case EvtType_Note:
						attr = EvAttr_Pitch;
						delta = -delta;
						if (modifiers & B_SHIFT_KEY) delta *= 12;
						break;
					case EvtType_Repeat:
					case EvtType_Sequence:
					case EvtType_TimeSig:
						attr = EvAttr_VPos;
						break;
					}
					
					if (attr != EvAttr_None)
					{
						EventOp *op = CreateOffsetOp( attr, delta, 0 );
						if (op)
						{
							tk->ModifySelectedEvents(
								NULL, *op, "Modify Events", attr );
							CRefCountObject::Release( op );
		
							if (gPrefs.FeedbackEnabled( attr, false )
								&&	tk->SelectionCount() == 1)
							{
								CPlayerControl::DoAudioFeedback(
									&Document(),
									attr,
									tk->CurrentEvent()->GetAttribute( attr ),
									tk->CurrentEvent() );
							}
						}
					}
				}
			}
		
			if (!(modifiers & B_COMMAND_KEY))
			{
					// REM: Make sure delete menu is enabled before
					// checking menus. We ought to call MenusBeginning, but
					// I'm not sure that's safe...
				BMenuItem *item = KeyMenuBar()->FindItem("Clear");
				item->SetEnabled(ActiveTrack()->SelectionType() != CTrack::Select_None);
	
				if (key == B_BACKSPACE) key = B_DELETE;
				if (CQuickKeyMenuItem::TriggerShortcutMenu( menus, key ))
					break;
			}
			CDocWindow::MessageReceived( message );
			break;
		}			
		case 'echo':
		{
			int32 attr, delta, value;
			bool finalFlag, cancelFlag;
			EventOp *op = NULL;

			message->FindInt32("attr", &attr);
			message->FindInt32("delta", &delta);
			message->FindInt32("value", &value);
			finalFlag = message->HasBool("final");
			cancelFlag = message->HasBool("cancel");
			op = CreateOffsetOp((enum E_EventAttribute)attr, delta, value);
			if (op)
			{
				SetPendingOperation(op);
				CRefCountObject::Release(op);
			}
			if (finalFlag || cancelFlag)
			{
				FinishTrackOperation(finalFlag);
			}
			break;
		}
		case 'oper':
		{
			BWindow *w;
			w = ((CMeVDoc &)Document()).ShowWindow(CMeVDoc::Operator_Window);
			((COperatorWindow *)w)->SetTrack(track);
			break;
		}
		case Update_ID:
		case Delete_ID:
		{
			CObserver::MessageReceived(message);
			break;
		}
		default:
		{
			CDocWindow::MessageReceived(message);
			break;
		}
	}
}

void
CTrackWindow::WindowActivated(
	bool active)
{
	CDocWindow::WindowActivated(active);
	if (active)
	{
		SetPulseRate(70000);
		CMeVApp::WatchTrack(Track());
	}
}

void
CTrackWindow::UpdateActiveSelection(
	bool inActive)
{
		// Forward messages about changing selection to child views.
	int		ct = stripFrame->CountStrips();
	
	for (int i = 0; i < ct; i++)
	{
		BView		*strip = stripFrame->StripAt( i ),
					*view;
						
		for (view = strip->ChildAt( 0 ); view != NULL; view = view->NextSibling())
		{
			CStripView	*sv;
	
			if ((sv = dynamic_cast<CStripView *>(view)))
			{
				sv->SetSelectionVisible( inActive );
			}
		}
	}
}

void
CTrackWindow::SetPendingOperation(
	EventOp *inOp)
{
		// Forward messages about changing selection to child views.
	int		ct = stripFrame->CountStrips();
	StSubjectLock			stLock( *ActiveTrack(), Lock_Shared );

	for (int i = 0; i < ct; i++)
	{
		BView		*strip = stripFrame->StripAt( i ),
					*view;
						
		for (view = strip->ChildAt( 0 ); view != NULL; view = view->NextSibling())
		{
			CEventEditor		*ee;
	
			if ((ee = dynamic_cast<CEventEditor *>(view)))
			{
				if (ee->SupportsShadowing())
				{
					if (trackOp)	ee->InvalidateSelection( *trackOp );
					if (inOp)	ee->InvalidateSelection( *inOp );
				}
			}
		}
	}
	
	if (inOp)	inOp->Acquire();
	CRefCountObject::Release( trackOp );
	trackOp = inOp;
}

void
CTrackWindow::FinishTrackOperation(
	int32 inCommit)
{
		// Forward messages about changing selection to child views.
	int		ct = stripFrame->CountStrips();
	StSubjectLock			stLock( *ActiveTrack(), Lock_Exclusive );

	for (int i = 0; i < ct; i++)
	{
		BView		*strip = stripFrame->StripAt( i ),
					*view;
						
		for (view = strip->ChildAt( 0 ); view != NULL; view = view->NextSibling())
		{
			CEventEditor		*ee;
	
			if ((ee = dynamic_cast<CEventEditor *>(view)))
			{
				if (trackOp)	ee->InvalidateSelection( *trackOp );
			}
		}
	}
	
	if (trackOp)
	{
			/**	Apply an operator to the selected events. */
		if (inCommit)
		{
			ActiveTrack()->ModifySelectedEvents( NULL, *trackOp, trackOp->UndoDescription() );
		}
		CRefCountObject::Release( trackOp );
	}
	trackOp = NULL;
}

void
CTrackWindow::ShowPrefs()
{
	prefsWinState.Lock();
	if (!prefsWinState.Activate())
	{
		BWindow		*w;
	
		w = new CLEditorPrefsWindow( prefsWinState, Track() );
		w->Show();
	}
	prefsWinState.Unlock();
}

void
CTrackWindow::CreateFileMenu(
	BMenuBar *menus)
{
	BMenu			*menu,
					*submenu;

		// Create the file menu
	menu = new BMenu( "File" );
	menu->AddItem( new BMenuItem( "New", new BMessage( MENU_NEW ) ) );
	menu->AddItem( new BMenuItem( "Open...", new BMessage( MENU_OPEN ), 'O' ) );
	menu->AddItem( new BMenuItem( "Close Window", new BMessage( B_QUIT_REQUESTED ), 'W' ) );
	menu->AddSeparatorItem();
	menu->AddItem( new BMenuItem( "Save", new BMessage( MENU_SAVE ), 'S' ) );
	menu->AddItem( new BMenuItem( "Save As...", new BMessage( MENU_SAVE_AS ) ) );
	menu->AddSeparatorItem();

	submenu = new BMenu( "Export" );
	((CMeVApp *)be_app)->BuildExportMenu( submenu );
	if (submenu->CountItems() <= 0) submenu->SetEnabled( false );

	menu->AddItem( new BMenuItem( "Import...", new BMessage( MENU_IMPORT ) ) );
	menu->AddItem( new BMenuItem( submenu ) );
	menu->AddSeparatorItem();
	menu->AddItem( new BMenuItem( "About MeV", new BMessage( MENU_ABOUT ), 0 ) );
	menu->AddItem( new BMenuItem( "About Plug-Ins", new BMessage( MENU_ABOUT_PLUGINS ), 0 ) );
	menu->AddItem( new BMenuItem( "MIDI Configuration..", new BMessage( MENU_MIDI_CONFIG ), 'P', B_SHIFT_KEY ) );
	menu->AddItem( new BMenuItem( "Preferences..", new BMessage( MENU_PROGRAM_SETTINGS ), 'P' ) );
	menu->AddSeparatorItem();
	menu->AddItem( new BMenuItem( "Quit", new BMessage( MENU_QUIT ), 'Q' ) );
	menus->AddItem( menu );
}
	
void
CTrackWindow::CreateFrames(
	BRect frame,
	CTrack *inTrack)
{
	CScrollerTarget	*ruler;
	BView			*pad;

		// Create the frame for the strips, and the scroll bar
	stripFrame = new CTrackEditFrame(
		BRect(	frame.left,
				frame.top + Ruler_Height,
				frame.right,
				frame.bottom),
		(char *)NULL,
		inTrack,
		B_FOLLOW_ALL );

	ruler = new CAssemblyRulerView(
		*this,
		stripFrame,
		(CEventTrack *)inTrack,
		BRect(	frame.left + 21,
				frame.top,
				frame.right - 14,
				frame.top + Ruler_Height - 1 ),
		(char *)NULL,
		B_FOLLOW_LEFT_RIGHT,
		B_WILL_DRAW );
		
	rgb_color fill = ui_color(B_PANEL_BACKGROUND_COLOR);
				
	pad = new CBorderView(BRect(frame.left - 1, frame.top - 1,
								frame.left + 20, frame.top + Ruler_Height - 1),
						  "", B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW, 
						  &fill);
		
	AddChild( pad );

	pad = new CBorderView(BRect(frame.right - 13, frame.top - 1,
								frame.right + 1, frame.top + Ruler_Height - 1),
						  "", B_FOLLOW_RIGHT | B_FOLLOW_TOP, B_WILL_DRAW,
						  &fill);
	AddChild(pad);

	stripScroll = new CScroller(
		BRect(	frame.left		- 1,
				frame.bottom	+ 1,
				frame.right		- 41,
				frame.bottom	+ 15 ),
		NULL,
		stripFrame,
		0.0, 0.0, B_HORIZONTAL );

	BControl		*magButton;

	magButton = new CBorderButton(
		BRect(	frame.right - 27,
				frame.bottom + 1,
				frame.right - 13,
				frame.bottom + 15 ),
		NULL, ResourceUtils::LoadImage("SmallPlus"),
		new BMessage( ZoomIn_ID ),
		B_FOLLOW_BOTTOM | B_FOLLOW_RIGHT,
		B_WILL_DRAW );

	magButton->SetTarget( (CDocWindow *)this );
	AddChild( magButton );

	magButton = new CBorderButton(
		BRect(	frame.right - 41,
				frame.bottom + 1,
				frame.right - 27,
				frame.bottom + 15 ),
		NULL, ResourceUtils::LoadImage("SmallMinus"),
		new BMessage( ZoomOut_ID ),
		B_FOLLOW_BOTTOM | B_FOLLOW_RIGHT,
		B_WILL_DRAW );

	magButton->SetTarget( (CDocWindow *)this );
	AddChild( magButton );

		// add views to window
	AddChild( ruler );
	AddChild( stripScroll );
	AddChild( stripFrame );
}

void
CTrackWindow::OnDeleteRequested(
	BMessage *message)
{
	PostMessage(B_QUIT_REQUESTED);
}

void
CTrackWindow::OnUpdate(
	BMessage *message)
{
	int32		trackHint;
	int32		docHint;
	int32		trackID;

	if (message->FindInt32( "TrackID",    0, &trackID ) != B_OK)		trackID = -1;
	if (message->FindInt32( "TrackAttrs", 0, &trackHint ) != B_OK)	trackHint = 0;
	if (message->FindInt32( "DocAttrs",   0, &docHint ) != B_OK)		docHint = 0;
	
	if (trackHint & CTrack::Update_Name)
	{
//		trackList->Invalidate();
	}
	else if (docHint & CMeVDoc::Update_Name)
	{
	}
}

filter_result
DefocusTextFilterFunc(
	BMessage *msg,
	BHandler **target,
	BMessageFilter *messageFilter)
{
	int32		key;
	int32		modifiers;
		
	BLooper		*looper = messageFilter->Looper();
	
	msg->FindInt32( "raw_char", &key );
	msg->FindInt32( "modifiers", &modifiers );
	
	if (key == B_ENTER || key == B_TAB)
	{
		BMessage		msg( LoseFocus_ID );
		looper->PostMessage( &msg );
	}
	return B_DISPATCH_MESSAGE;
}

// END - TrackWindow.cpp
