/* ===================================================================== *
 * TrackWindow.cpp (MeV/User Interface)
 * ===================================================================== */

#include "TrackWindow.h"

#include "AssemblyRulerView.h"
#include "BorderButton.h"
#include "EventEditor.h"
#include "EventTrack.h"
#include "Idents.h"
#include "MeVApp.h"
#include "MeVDoc.h"
#include "OperatorWindow.h"
#include "PlayerControl.h"
#include "QuickKeyMenuItem.h"
#include "ResourceUtils.h"
#include "StdEventOps.h"
#include "StripView.h"
#include "Tool.h"
#include "ToolBar.h"
#include "TrackEditFrame.h"
#include "TrackListWindow.h"

// Gnu C Library
#include <stdio.h>
// Application Kit
#include <MessageFilter.h>
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
// Constants

const float
CTrackWindow::DEFAULT_RULER_HEIGHT = 12.0;

// ---------------------------------------------------------------------------
// Constructor/Destructor

CTrackWindow::CTrackWindow(
	BRect frame,
	CMeVDoc *document,
	CEventTrack *inTrack)
	:	CDocWindow(frame, document,
				   (inTrack && inTrack->GetID() > 1) ? inTrack->Name() : NULL,
				   B_DOCUMENT_WINDOW,
				   B_ASYNCHRONOUS_CONTROLS),
		CObserver(*this, document),
		stripFrame(NULL),
		stripScroll(NULL),
		trackOp(NULL),
		m_newEventType(EvtType_Count)
{
	D_ALLOC(("TrackWindow::TrackWindow()\n"));

	track = inTrack;
	track->Acquire();
	
	BMessage *trackMsg = new BMessage('0000');
	trackMsg->AddInt32("TrackID", track->GetID());
	trackMsg->AddInt32("DocumentID", (int32)document);

	SetPulseRate(100000);
}

CTrackWindow::~CTrackWindow()
{
	D_ALLOC(("CTrackWindow::~CTrackWindow()\n"));

	CRefCountObject::Release(trackOp);
	CRefCountObject::Release(track);
}

// ---------------------------------------------------------------------------
// Accessors

int32
CTrackWindow::NewEventDuration() const
{
	if (ActiveTrack()->GridSnapEnabled())
		return ActiveTrack()->TimeGridSize();
	else
		return Ticks_Per_QtrNote;
}

event_type
CTrackWindow::NewEventType(
	event_type defaultType) const
{
	if (m_newEventType >= EvtType_Count)
		return defaultType;

	return m_newEventType;
}

// ---------------------------------------------------------------------------
// Operations

void
CTrackWindow::SetPendingOperation(
	EventOp *inOp)
{
	// Forward messages about changing selection to child views.
	StSubjectLock stLock(*ActiveTrack(), Lock_Shared);

	for (int32 i = 0; i < stripFrame->CountStrips(); i++)
	{
		CEventEditor *ee = dynamic_cast<CEventEditor *>(stripFrame->StripAt(i));
		if (ee && ee->SupportsShadowing())
		{
			if (trackOp)
				ee->InvalidateSelection(*trackOp);
			if (inOp)
				ee->InvalidateSelection(*inOp);
		}
	}

	if (inOp)
		inOp->Acquire();
	CRefCountObject::Release(trackOp);
	trackOp = inOp;
}

void
CTrackWindow::FinishTrackOperation(
	int32 inCommit)
{
	// Forward messages about changing selection to child views.
	StSubjectLock stLock(*ActiveTrack(), Lock_Exclusive);

	for (int32 i = 0; i < stripFrame->CountStrips(); i++)
	{
		CEventEditor *ee = dynamic_cast<CEventEditor *>(stripFrame->StripAt(i));
		if (ee && ee->SupportsShadowing())
		{
			if (trackOp)
				ee->InvalidateSelection(*trackOp);
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
	item = KeyMenuBar()->FindItem(MENU_CLEAR);
	item->SetEnabled(ActiveTrack()->SelectionType() != CTrack::Select_None);

	// Set up 'View' menu
	item = KeyMenuBar()->FindItem("Add Strip");
	if (item)
	{
		BMenu *subMenu = item->Submenu();
		if (subMenu->CountItems() == 0)
		{
			for (int32 i = 0; i < stripFrame->CountTypes(); i++)
			{
				BMessage *message = new BMessage(CStripView::ADD_STRIP);
				message->AddString("type", stripFrame->TypeAt(i));
				subMenu->AddItem(new BMenuItem(stripFrame->TypeAt(i).String(),
											   message));
			}
		}
	}

	// Set up Window menu
	CMeVApp *app = dynamic_cast<CMeVApp *>(be_app);
	item = KeyMenuBar()->FindItem(MENU_TRACKLIST);
	if (item)
		item->SetMarked(app->TrackList());
	item = KeyMenuBar()->FindItem(MENU_INSPECTOR);
	if (item)
		item->SetMarked(app->Inspector());
	item = KeyMenuBar()->FindItem(MENU_GRIDWINDOW);
	if (item)
		item->SetMarked(app->GridWindow());
	item = KeyMenuBar()->FindItem(MENU_TRANSPORT);
	if (item)
		item->SetMarked(app->TransportWindow());

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
			Document()->Save();
			break;
		}
		case MENU_SAVE_AS:
		{
			Document()->SaveAs();
			break;
		}
		case MENU_IMPORT:
		{
			((CMeVApp *)be_app)->ImportDocument();
			break;
		}
		case MENU_EXPORT:
		{
			Document()->Export(message);
			break;
		}
		case CStripView::ADD_STRIP:
		{
			BString type;
			if (message->FindString("type", &type) == B_OK)
			{
				if (AddStrip(type))
					stripFrame->PackStrips();
			}
			break;
		}
		case NEW_EVENT_TYPE_CHANGED:
		{
			int32 type;
			if (message->FindInt32("type", &type) != B_OK)
				return;

			if (type != m_newEventType)
			{
				m_newEventType = static_cast<event_type>(type);
				NewEventTypeChanged(m_newEventType);
			}
			CTool *tool = ToolBar()->FindTool("Create");
			if (tool)
				tool->SetValue(B_CONTROL_ON);
			break;
		}
		case MENU_TRACKLIST:
		{
			((CMeVApp *)be_app)->ShowTrackList(((CMeVApp *)be_app)->TrackList() == NULL);
			Activate();				// In case window was deactivated
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
			CPlayerControl::TogglePauseState(Document());
			break;
		}
		case LoseFocus_ID:
		{
			// Remove focus from pesky controls.
			if (CurrentFocus())
				CurrentFocus()->MakeFocus(false);
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
								CPlayerControl::DoAudioFeedback(Document(),
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
				if (CQuickKeyMenuItem::TriggerShortcutMenu(KeyMenuBar(), key))
					break;
			}
			CDocWindow::MessageReceived(message);
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
			w = Document()->ShowWindow(CMeVDoc::Operator_Window);
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

// ---------------------------------------------------------------------------
// CObserver Implementation

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
	int32 trackHint;
	int32 docHint;
	int32 trackID;

	if (message->FindInt32("TrackID", 0, &trackID) != B_OK)
		trackID = -1;
	if (message->FindInt32("TrackAttrs", 0, &trackHint) != B_OK)
		trackHint = 0;
	if (message->FindInt32("DocAttrs", 0, &docHint) != B_OK)
		docHint = 0;
}

// ---------------------------------------------------------------------------
// Internal Operations

void
CTrackWindow::UpdateActiveSelection(
	bool active)
{
	// Forward messages about changing selection to child views.
	for (int32 i = 0; i < stripFrame->CountStrips(); i++)
	{
		stripFrame->StripAt(i)->SetSelectionVisible(active);
	}
}

void
CTrackWindow::CreateFileMenu(
	BMenuBar *menus)
{
	BMenu *menu, *submenu;
	BMenuItem *item;

	// Create the file menu
	menu = new BMenu("File");
	menu->AddItem(new BMenuItem("New", new BMessage(MENU_NEW)));
	menu->AddItem(new BMenuItem("Open...", new BMessage(MENU_OPEN), 'O'));
	menu->AddItem(new BMenuItem("Close Window", new BMessage(B_QUIT_REQUESTED), 'W'));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem("Save", new BMessage(MENU_SAVE), 'S'));
	menu->AddItem(new BMenuItem("Save As...", new BMessage(MENU_SAVE_AS)));
	menu->AddSeparatorItem();

	submenu = new BMenu("Export");
	Document()->Application()->BuildExportMenu(submenu);
	if (submenu->CountItems() <= 0)
		submenu->SetEnabled(false);

	menu->AddItem(new BMenuItem("Import...", new BMessage(MENU_IMPORT)));
	menu->AddItem(new BMenuItem(submenu));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem("Preferences..", new BMessage(MENU_PROGRAM_SETTINGS)));
	menu->AddItem(item = new BMenuItem("Help..", NULL));
	item->SetEnabled(false);
	menu->AddItem(new BMenuItem("About MeV...", new BMessage(MENU_ABOUT), 0));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem("Quit", new BMessage(MENU_QUIT), 'Q'));
	menus->AddItem(menu);
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
