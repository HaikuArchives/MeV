/* ===================================================================== *
 * TrackWindow.cpp (MeV/UI)
 * ===================================================================== */

#include "TrackWindow.h"

#include "BorderButton.h"
#include "EventEditor.h"
#include "EventTrack.h"
#include "IconMenuItem.h"
#include "Idents.h"
#include "IFFReader.h"
#include "IFFWriter.h"
#include "MeVApp.h"
#include "MeVDoc.h"
#include "MeVFileID.h"
#include "OperatorWindow.h"
#include "PlayerControl.h"
#include "PositionInfoBar.h"
#include "QuickKeyMenuItem.h"
#include "RecentDocumentsMenu.h"
#include "ResourceUtils.h"
#include "StdEventOps.h"
#include "StripFrameView.h"
#include "StripView.h"
#include "Tool.h"
#include "ToolBar.h"

// Gnu C Library
#include <stdio.h>
// Application Kit
#include <MessageFilter.h>
#include <Roster.h>
// Interface Kit
#include <Bitmap.h>
#include <MenuBar.h>
#include <StringView.h>
// Storage Kit
#include <Entry.h>
#include <Node.h>
#include <NodeInfo.h>
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
	bool isMaster,
	CEventTrack *track,
	bool hasSettings)
	:	CDocWindow(frame, document, isMaster,
				   (track && track->GetID() > 1) ? track->Name() : NULL,
				   B_DOCUMENT_WINDOW,
				   B_ASYNCHRONOUS_CONTROLS),
		stripFrame(NULL),
		track(track),
		stripScroll(NULL),
		trackOp(NULL),
		m_newEventType(EvtType_Count)
{
	D_ALLOC(("TrackWindow::TrackWindow(new)\n"));

	track->AddObserver(this);
	
	SetPulseRate(100000);
}

CTrackWindow::~CTrackWindow()
{
	D_ALLOC(("CTrackWindow::~CTrackWindow()\n"));

	if (track != NULL)
		track->RemoveObserver(this);
}

// ---------------------------------------------------------------------------
// Hook Functions

void
CTrackWindow::FrameResized(
	float width,
	float height)
{
	float minWidth, minHeight, maxWidth, maxHeight;
	GetSizeLimits(&minWidth, &maxWidth, &minHeight, &maxHeight);
	
	minWidth = m_posInfoBar->Frame().Width() + 6 * B_V_SCROLL_BAR_WIDTH + 3;
	minHeight = KeyMenuBar()->Frame().Height() + ToolBar()->Frame().Height() +
	stripFrame->MinimumHeight() + m_posInfoBar->Frame().Height();
	
	SetSizeLimits(minWidth, maxWidth, minHeight, maxHeight);
	CDocWindow::FrameResized(width, height);
}

void
CTrackWindow::AddFrameView(
	BRect frame,
	CTrack *track)
{
	BRect scrollRect(stripScroll->Frame());
	stripScroll->ResizeTo(scrollRect.Width() - 240.0, scrollRect.Height());
	stripScroll->MoveTo(scrollRect.left + 240.0, scrollRect.top);

	BRect infoRect(scrollRect);
	infoRect.top = infoRect.bottom - B_H_SCROLL_BAR_HEIGHT;
	infoRect.right = 238.0;
	m_posInfoBar = new CPositionInfoBar(infoRect, stripScroll);
	m_posInfoBar->SetMinimumWidth(140.0);
	AddChild(m_posInfoBar);
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
	m_posInfoBar->SetText(B_HORIZONTAL, text);
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
			text << majorUnit + 1 << ":" << minorUnit + 1 << ":";
			text << extraTime + 1;
		}
		SetHorizontalPositionInfo(text);
	}
}

void
CTrackWindow::SetVerticalPositionInfo(
	BString text)
{
	m_posInfoBar->SetText(B_VERTICAL, text);
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
			itemLabel << ": " << description;
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
			itemLabel << ": " << description;
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
	
	// Set up 'Edit' menu
	item = KeyMenuBar()->FindItem(MENU_CLEAR);
	item->SetEnabled(ActiveTrack()->SelectionType() != CTrack::Select_None);

	// Set up 'Play' menu
	item = KeyMenuBar()->FindItem(MENU_PLAY_SECTION);
	if (item)
		item->SetEnabled(Track()->SectionStart() < Track()->SectionEnd());
	item = KeyMenuBar()->FindItem(MENU_SET_SECTION);
	if (item)
		item->SetEnabled(Track()->SelectionType() != CTrack::Select_None);

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
				BString name = stripFrame->TypeAt(i);
				BBitmap *icon = NULL;
				stripFrame->GetIconForType(i, &icon);
				message->AddString("type", name.String());
				subMenu->AddItem(new CIconMenuItem(name.String(), message,
												   icon));
			}
		}
	}

	// Set up Window menu
	CMeVApp *app = Document()->Application();
	item = KeyMenuBar()->FindItem(MENU_PARTS_WINDOW);
	if (item)
		item->SetMarked(app->TrackList());
	item = KeyMenuBar()->FindItem(MENU_MIX_WINDOW);
	if (item)
		item->SetMarked(Document()->IsWindowOpen(CMeVDoc::MIX_WINDOW));
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
		case MENU_PLAY:
		{
			if (CPlayerControl::IsPlaying(Document()))
			{
				CPlayerControl::StopSong(Document());
				break;
			}
			// Start playing a song.
			CMeVApp *app = Document()->Application();
			CPlayerControl::PlaySong(Document(), 0, 0, LocateTarget_Real, -1,
									 SyncType_SongInternal,
									 (app->GetLoopFlag() ? PB_Loop : 0));
			break;
		}
		case MENU_PLAY_SECTION:
		{	
			// Start playing a section.
			CMeVApp *app = Document()->Application();
			CPlayerControl::PlaySong(Document(), Track()->GetID(),
									 Track()->SectionStart(),
									 LocateTarget_Metered,
									 Track()->SectionEnd() - Track()->SectionStart(),
									 SyncType_SongInternal,
									 (app->GetLoopFlag() ? PB_Loop : 0) | PB_Folded );
			break;
		}
		case MENU_SET_SECTION:
		{
			StSubjectLock lock(*Track(), Lock_Exclusive);
			if (Track()->SelectionType() != CTrack::Select_None)
			{
				Track()->SetSection(Track()->MinSelectTime(),
									Track()->MaxSelectTime());
				Track()->NotifyUpdate(CTrack::Update_Section, NULL);
			}
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
		case MENU_PARTS_WINDOW:
		{
			bool show = (Document()->Application()->TrackList() == NULL);
			Document()->Application()->ShowTrackList(show);
			// In case window was deactivated
			Activate();
			break;
		}
		case MENU_MIX_WINDOW:
		{
			bool visible = Document()->IsWindowOpen(CMeVDoc::MIX_WINDOW);
			Document()->ShowWindow(CMeVDoc::MIX_WINDOW, !visible);
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
			PRINT((" B_KEY_DOWN\n"));

			int32 key;
			message->FindInt32("raw_char", &key);
			int32 modifiers;
			message->FindInt32("modifiers", &modifiers);
			if (!(modifiers & B_COMMAND_KEY))
			{
				// REM: Make sure delete menu is enabled before
				// checking menus. We ought to call MenusBeginning, but
				// I'm not sure that's safe...
				BMenuItem *item = KeyMenuBar()->FindItem("Clear");
				item->SetEnabled(ActiveTrack()->SelectionType() != CTrack::Select_None);
	
				if (key == B_BACKSPACE)
					key = B_DELETE;
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
	if (Track() != NULL)
	{
		BMessage *message = Track()->GetWindowSettings();
		if (message != NULL)
			delete message;
		message = new BMessage();
		ExportSettings(message);
		Track()->SetWindowSettings(message);

		Track()->RemoveObserver(this);

		track = NULL;
	}

	return CDocWindow::QuitRequested();
}

bool
CTrackWindow::SubjectReleased(
	CObservable *subject)
{
	D_OBSERVE(("CTrackWindow<%p>::SubjectReleased()\n", this));

	if (subject == track)
	{
		track->RemoveObserver(this);
		track = NULL;
		return true;
	}

	return CDocWindow::SubjectReleased(subject);
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
	UpdateActiveSelection(active);
}

// ---------------------------------------------------------------------------
// Internal Operations

void
CTrackWindow::CreateFileMenu(
	BMenuBar *menuBar)
{
	BMenu *menu, *submenu;
	BMenuItem *item;
	BMessage refList;

	// Create the file menu
	menu = new BMenu("File");
	menu->AddItem(item = new BMenuItem("New", new BMessage(MENU_NEW)));
	item->SetTarget(be_app);

	submenu = new CRecentDocumentsMenu("Open" B_UTF8_ELLIPSIS,
									   new BMessage(B_REFS_RECEIVED),
									   6, CMeVDoc::MimeType()->Type());
	submenu->SetTargetForItems(be_app);
	menu->AddItem(item = new BMenuItem(submenu, new BMessage(MENU_OPEN)));
	item->SetShortcut('O', B_COMMAND_KEY);
	item->SetTarget(be_app);

	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem("Save", new BMessage(MENU_SAVE), 'S'));
	menu->AddItem(new BMenuItem("Save As" B_UTF8_ELLIPSIS,
								new BMessage(MENU_SAVE_AS)));
	menu->AddItem(new BMenuItem("Close", new BMessage(B_QUIT_REQUESTED),
								'W'));
	menu->AddSeparatorItem();

	const char *importFormats[2] = { "audio/x-midi", "audio/midi" };
	be_roster->GetRecentDocuments(&refList, 6, importFormats, 2);
	submenu = new CRecentDocumentsMenu("Import" B_UTF8_ELLIPSIS,
									   new BMessage(B_REFS_RECEIVED),
									   6, importFormats, 2);
	submenu->SetTargetForItems(be_app);
	menu->AddItem(item = new BMenuItem(submenu, new BMessage(MENU_IMPORT)));
	item->SetTarget(be_app);

	submenu = new BMenu("Export");
	Document()->Application()->BuildExportMenu(submenu);
	if (submenu->CountItems() <= 0)
		submenu->SetEnabled(false);
	menu->AddItem(new BMenuItem(submenu));
	menu->AddSeparatorItem();

	menu->AddItem(item = new BMenuItem("Preferences" B_UTF8_ELLIPSIS,
									   new BMessage(MENU_PROGRAM_SETTINGS)));
	item->SetTarget(be_app);
	menu->AddItem(item = new BMenuItem("Help" B_UTF8_ELLIPSIS,
									   new BMessage(MENU_HELP)));
	item->SetTarget(be_app);
	menu->AddItem(new BMenuItem("About MeV" B_UTF8_ELLIPSIS,
								new BMessage(MENU_ABOUT)));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem("Quit", new BMessage(MENU_QUIT), 'Q'));
	menuBar->AddItem(menu);
}
	
void
CTrackWindow::CreateWindowMenu(
	BMenuBar *menuBar)
{
	BMenu *menu;

	menu = new BMenu("Window");
	menu->AddItem(new BMenuItem("Parts",
								new BMessage(MENU_PARTS_WINDOW), 'P'));
	menu->AddItem(new BMenuItem("Mix",
								new BMessage(MENU_MIX_WINDOW), 'M'));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem("Inspector",
								new BMessage(MENU_INSPECTOR), 'I'));
	menu->AddItem(new BMenuItem("Grid",
								new BMessage(MENU_GRIDWINDOW), 'G'));
	menu->AddItem(new BMenuItem("Transport",
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
