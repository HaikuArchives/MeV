/* ===================================================================== *
 * TrackWindow.cpp (MeV/UI)
 * ===================================================================== */

#include "TrackWindow.h"

#include "AssemblyRulerView.h"
#include "BorderButton.h"
#include "EventEditor.h"
#include "EventTrack.h"
#include "Idents.h"
#include "IFFReader.h"
#include "IFFWriter.h"
#include "MeVApp.h"
#include "MeVDoc.h"
#include "MeVFileID.h"
#include "OperatorWindow.h"
#include "PlayerControl.h"
#include "QuickKeyMenuItem.h"
#include "ResourceUtils.h"
#include "StdEventOps.h"
#include "StripFrameView.h"
#include "StripView.h"
#include "TextDisplay.h"
#include "Tool.h"
#include "ToolBar.h"

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
	CEventTrack *inTrack,
	bool hasSettings)
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
	D_ALLOC(("TrackWindow::TrackWindow(new)\n"));

	track = inTrack;
	track->Acquire();
	
	SetPulseRate(100000);
}

CTrackWindow::~CTrackWindow()
{
	D_ALLOC(("CTrackWindow::~CTrackWindow()\n"));

	CRefCountObject::Release(trackOp);
	CRefCountObject::Release(track);
}

// ---------------------------------------------------------------------------
// Hook Functions

void
CTrackWindow::AddFrameView(
	BRect frame,
	CTrack *track)
{
	BRect scrollRect(stripScroll->Frame());
	stripScroll->ResizeTo(scrollRect.Width() - 120.0, scrollRect.Height());
	stripScroll->MoveTo(scrollRect.left + 120.0, scrollRect.top);

	BRect infoRect(scrollRect);
	infoRect.top = infoRect.bottom - B_H_SCROLL_BAR_HEIGHT;
	infoRect.right = 119.0;
	BView *view = new CBorderView(infoRect, "", B_FOLLOW_LEFT | B_FOLLOW_BOTTOM,
								  B_WILL_DRAW);

	// Add the vertical position info view
	infoRect = view->Bounds();
	infoRect.right = 59.0;
	m_vPosInfoView = new CTextDisplay(infoRect.InsetByCopy(1.0, 1.0),
									  "", false);
	m_vPosInfoView->SetAlignment(B_ALIGN_RIGHT);
	m_vPosInfoView->SetFont(be_fixed_font);
	m_vPosInfoView->SetFontSize(10);
	view->AddChild(m_vPosInfoView);

	// Add the horizontal position info view
	infoRect.OffsetBy(infoRect.Width(), 0.0);
	m_hPosInfoView = new CTextDisplay(infoRect.InsetByCopy(1.0, 1.0),
									  "", false );
	m_hPosInfoView->SetAlignment(B_ALIGN_RIGHT);
	m_hPosInfoView->SetFont(be_fixed_font);
	m_hPosInfoView->SetFontSize(10);
	view->AddChild(m_hPosInfoView);

	AddChild(view);
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

void
CTrackWindow::SetHorizontalPositionInfo(
	BString text)
{
	PRINT(("CTrackWindow::SetHorizontalPositionInfo(%s)\n",
			text.String()));

	m_hPosInfoView->SetText(text.String());
	m_hPosInfoView->Invalidate();
}

void
CTrackWindow::SetHorizontalPositionInfo(
	CTrack *track,
	int32 time)
{
	if (track == NULL)
	{
		SetHorizontalPositionInfo("");
	}
	else
	{
		BString text;
		long majorUnit, minorUnit, extraTime;
		track->SigMap().DecomposeTime(time, majorUnit, minorUnit, extraTime);
		if (track->ClockType() == ClockType_Real)
		{
			int32 hours = majorUnit / 60;
			text << hours << ":" << majorUnit - (hours * 60) << ":";
			text << minorUnit << ":" << extraTime;
		}
		else
		{
			text << majorUnit + 1 << ":" << minorUnit << ":" << extraTime;
		}
		SetHorizontalPositionInfo(text);
	}
}

void
CTrackWindow::SetVerticalPositionInfo(
	BString text)
{
	PRINT(("CTrackWindow::SetVerticalPositionInfo(%s)\n",
			text.String()));

	m_vPosInfoView->SetText(text.String());
	m_vPosInfoView->Invalidate();
}

// ---------------------------------------------------------------------------
// Serialization

void
CTrackWindow::ExportSettings(
	BMessage *settings) const
{
	settings->AddRect("frame", Frame());
	stripFrame->ExportSettings(settings);
}

void
CTrackWindow::ImportSettings(
	const BMessage *settings)
{
	BRect frame(Frame());
	settings->FindRect("frame", &frame);
	ResizeTo(frame.Width(), frame.Height());
	MoveTo(frame.LeftTop());

	stripFrame->ImportSettings(settings);
}

void
CTrackWindow::ReadState(
	CIFFReader &reader,
	BMessage *settings)
{
	BRect frame;
	reader >> frame.left >> frame.top >> frame.right >> frame.bottom;
	settings->AddRect("frame", frame);

	CStripFrameView::ReadState(reader, settings);
}

void
CTrackWindow::WriteState(
	CIFFWriter &writer,
	const BMessage *settings)
{
	ASSERT(settings != NULL);

	BRect frame;
	settings->FindRect("frame", &frame);
	writer << frame.left << frame.top << frame.right << frame.bottom;

	CStripFrameView::WriteState(writer, settings);
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
			bool show = (Document()->Application()->TrackList() == NULL);
			Document()->Application()->ShowTrackList(show);
			// In case window was deactivated
			Activate();
			break;
		}
		case MENU_INSPECTOR:
		{
			bool show = (Document()->Application()->Inspector() == NULL);
			Document()->Application()->ShowInspector(show);
			// In case window was deactivated
			Activate();
			break;
		}
		case MENU_GRIDWINDOW:
		{
			bool show = (Document()->Application()->GridWindow() == NULL);
			Document()->Application()->ShowGridWindow(show);
			// In case window was deactivated
			Activate();
			break;
		}
		case MENU_TRANSPORT:
		{
			bool show = (Document()->Application()->TransportWindow() == NULL);
			Document()->Application()->ShowTransportWindow(show);
			// In case window was deactivated
			Activate();
			break;
		}
		case ZoomOut_ID:
		{
			stripFrame->ZoomBy(-1);
			break;
		}
		case ZoomIn_ID:
		{
			stripFrame->ZoomBy(1);
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

bool
CTrackWindow::QuitRequested()
{
	BMessage *message = Track()->GetWindowSettings();
	if (message != NULL)
		delete message;
	message = new BMessage();
	ExportSettings(message);
	Track()->SetWindowSettings(message);

	return CDocWindow::QuitRequested();
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
}

// ---------------------------------------------------------------------------
// Internal Operations

void
CTrackWindow::CreateFileMenu(
	BMenuBar *menuBar)
{
	BMenu *menu, *submenu;
	BMenuItem *item;

	// Create the file menu
	menu = new BMenu("File");
	menu->AddItem(item = new BMenuItem("New", new BMessage(MENU_NEW)));
	item->SetTarget(be_app);
	menu->AddItem(item = new BMenuItem("Open...", new BMessage(MENU_OPEN), 'O'));
	item->SetTarget(be_app);
	menu->AddItem(new BMenuItem("Close Window", new BMessage(B_QUIT_REQUESTED), 'W'));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem("Save", new BMessage(MENU_SAVE), 'S'));
	menu->AddItem(new BMenuItem("Save As...", new BMessage(MENU_SAVE_AS)));
	menu->AddSeparatorItem();

	menu->AddItem(item = new BMenuItem("Import...", new BMessage(MENU_IMPORT)));
	item->SetTarget(be_app);
	submenu = new BMenu("Export");
	Document()->Application()->BuildExportMenu(submenu);
	if (submenu->CountItems() <= 0)
		submenu->SetEnabled(false);
	menu->AddItem(new BMenuItem(submenu));
	menu->AddSeparatorItem();

	menu->AddItem(item = new BMenuItem("Preferences..", new BMessage(MENU_PROGRAM_SETTINGS)));
	item->SetTarget(be_app);
	menu->AddItem(item = new BMenuItem("Help..", NULL));
	item->SetEnabled(false);
	menu->AddItem(new BMenuItem("About MeV...", new BMessage(MENU_ABOUT), 0));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem("Quit", new BMessage(MENU_QUIT), 'Q'));
	menuBar->AddItem(menu);
}
	
void
CTrackWindow::CreateWindowMenu(
	BMenuBar *menuBar)
{
	BMenu *menu;

	// Create the file menu
	menu = new BMenu("Window");
	menu->AddItem(new BMenuItem("Show Parts",
								new BMessage(MENU_TRACKLIST), 'L'));
	menu->AddItem(new BMenuItem("Show Inspector",
								new BMessage(MENU_INSPECTOR), 'I'));
	menu->AddItem(new BMenuItem("Show Grid",
								new BMessage(MENU_GRIDWINDOW), 'G'));
	menu->AddItem(new BMenuItem("Show Transport",
								new BMessage(MENU_TRANSPORT), 'T'));
	menu->AddSeparatorItem();
	SetWindowMenu(menu);
	menuBar->AddItem(menu);
}

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

// ---------------------------------------------------------------------------
// Filter Function

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
