/* ===================================================================== *
 * AssemblyWindow.cpp (MeV/UI)
 * ===================================================================== */

#include "AssemblyWindow.h"

#include "AssemblyRulerView.h"
#include "BitmapTool.h"
#include "BorderButton.h"
#include "DataSnap.h"
#include "EventTrack.h"
#include "IconMenuItem.h"
#include "Idents.h"
#include "LinearWindow.h"
#include "MenuTool.h"
#include "MeVApp.h"
#include "PlayerControl.h"
#include "QuickKeyMenuItem.h"
#include "ResourceUtils.h"
#include "ScreenUtils.h"
#include "Splitter.h"
#include "ToolBar.h"
#include "TrackCtlStrip.h"

// Gnu C Library
#include <stdio.h>
// Interface Kit
#include <Bitmap.h>
#include <Box.h>
#include <MenuBar.h>
#include <MenuField.h>
#include <PopUpMenu.h>
#include <StringView.h>
// Support Kit
#include <Debug.h>

// Debugging Macros
#define D_ALLOC(x) //PRINT (x)		// Constructor/Destructor
#define D_MESSAGE(x) //PRINT (x)	// MessageReceived()

// ---------------------------------------------------------------------------
// Constructor/Destructor

CAssemblyWindow::CAssemblyWindow(
	BRect frame,
	CMeVDoc *document,
	bool hasSettings)
	:	CTrackWindow(frame, document, true, 
					 (CEventTrack *)document->FindTrack(1),
					 hasSettings)
{
	D_ALLOC(("CAssemblyWindow::CAssemblyWindow(%s)\n",
			 hasSettings ? "hasSettings == true" : "hasSettings == false"));

	AddMenuBar();
	AddToolBar();

	// ++++++ retrieve this from window settings if possible
	m_toolStates[0] = CEventEditor::TOOL_SELECT;

	// add the default strips
	BRect rect(Bounds());
	rect.top = ToolBar()->Frame().bottom + 1.0;
	rect.bottom -= B_H_SCROLL_BAR_HEIGHT;

	AddFrameView(rect, (CTrack *)Document()->FindTrack(1));
	stripFrame->AddType("Metered");
	stripFrame->AddType("Real");

	if (!hasSettings)
	{
		// add default strip
		AddStrip("Metered", 1.0);
		stripFrame->PackStrips();
	}

	SetNewEventType(EvtType_Sequence);
}

CAssemblyWindow::~CAssemblyWindow()
{
	D_ALLOC(("CAssemblyWindow::CAssemblyWindow()\n"));
}

// ---------------------------------------------------------------------------
// CTrackWindow Implementation

void
CAssemblyWindow::MessageReceived(
	BMessage* message)
{
	switch (message->what)
	{
		case MENU_UNDO:
		{
			Track()->Undo();
			break;
		}
		case MENU_REDO:
		{
			Track()->Redo();
			break;
		}
		case B_CUT:
		{
			if (CurrentFocus())
			{
				DispatchMessage(message, CurrentFocus());
			}
			else
			{
				// Add CUT code here
			}
			break;
		}
		case B_COPY:
		{
			if (CurrentFocus())
			{
				DispatchMessage(message, CurrentFocus());
			}
			else
			{
				// Add COPY code here
			}
			break;
		}
		case B_PASTE:
		{
			if (CurrentFocus())
			{
				DispatchMessage( message, CurrentFocus() );
			}
			else
			{
				// Add PASTE code
			}
			break;
		}
		case MENU_CLEAR:
		{
			ActiveTrack()->DeleteSelection();
			Document()->SetModified();
			break;
		}
		case B_SELECT_ALL:
		{
			if (CurrentFocus()) DispatchMessage( message, CurrentFocus() );
			else Track()->SelectAll();
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
			CMeVApp *app = (CMeVApp *)be_app;
			CPlayerControl::PlaySong(Document(), 0, 0, LocateTarget_Real, -1,
									 SyncType_SongInternal,
									 (app->GetLoopFlag() ? PB_Loop : 0));
			break;
		}
		case MENU_PLAY_SECTION:
		{	
			// Start playing a song.
			CMeVApp *app = (CMeVApp *)be_app;
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
			if (Track()->SelectionType() != CTrack::Select_None)
			{
				Track()->SetSection(Track()->MinSelectTime(),
									Track()->MaxSelectTime());
				Track()->NotifyUpdate(CTrack::Update_Section, NULL);
			}
			break;
		}
		case CEventEditor::TOOL_GRID:
		{
			D_MESSAGE((" -> TOOL_GRID\n"));
			int32 value;
			if (message->FindInt32("value", &value) != B_OK)
			{
				return;
			}
			Track()->EnableGridSnap(value);
			break;
		}
		case CEventEditor::TOOL_SELECT:
		case CEventEditor::TOOL_CREATE:
		case CEventEditor::TOOL_ERASE:
		case CEventEditor::TOOL_TEXT:
		{
			D_MESSAGE((" -> TOOL_SELECT/CREATE/ERASE/TEXT\n"));
			m_toolStates[0] = message->what;
			break;
		}
		case 'cMet':
		{
			// ++++++ what is this ?
			stripFrame->SetClockType(ClockType_Metered);
			break;
		}
		case 'cRea':
		{
			// ++++++ what is this ?
			stripFrame->SetClockType(ClockType_Real);
			break;
		}
		default:
		{
			CTrackWindow::MessageReceived(message);
			break;
		}
	}
}

void
CAssemblyWindow::OnUpdate(
	BMessage *message)
{
	CTrackWindow::OnUpdate(message);
}

bool
CAssemblyWindow::AddStrip(
	BString type,
	float proportion)
{
	BRect rect(Bounds());
	rect.top = ToolBar()->Frame().bottom + 1.0;
	rect.bottom -= B_H_SCROLL_BAR_HEIGHT;

	CAssemblyRulerView *ruler = NULL;
	CStripView *strip = NULL;

	if (type == "Metered")
	{
		strip = new CTrackCtlStrip(*this, *stripFrame, rect,
								   (CEventTrack *)Document()->FindTrack(1),
								   "Metered");
		ruler = new CAssemblyRulerView(*this, stripFrame,
									   (CEventTrack *)Document()->FindTrack(1),
									   BRect(0.0, -1.0, rect.Width(),
									   		 CTrackWindow::DEFAULT_RULER_HEIGHT),
									   (char *)NULL, B_FOLLOW_LEFT_RIGHT,
									   B_WILL_DRAW);
	}
	else if (type == "Real")
	{
		strip = new CTrackCtlStrip(*this, *stripFrame, rect,
								   (CEventTrack *)Document()->FindTrack((int32)0),
								   "Real");
		ruler = new CAssemblyRulerView(*this, stripFrame,
									   (CEventTrack *)Document()->FindTrack((int32)0),
									   BRect(0.0, -1.0, rect.Width(), 
									   		 CTrackWindow::DEFAULT_RULER_HEIGHT),
									   (char *)NULL, B_FOLLOW_LEFT_RIGHT,
									   B_WILL_DRAW);
		ruler->ShowMarkers(false);
	}

	if (strip)
	{
		if (ruler)
			strip->SetRulerView(ruler);
		stripFrame->AddStrip(strip, proportion);
		return true;
	}

	CTrackWindow::AddStrip(type, proportion);
}

void
CAssemblyWindow::NewEventTypeChanged(
	event_type type)
{
	BBitmap *bitmap;
	switch (type)
	{
		case EvtType_Tempo:
		{
			bitmap = ResourceUtils::LoadImage("MetroTool");
			break;
		}
		case EvtType_TimeSig:
		{
			bitmap = ResourceUtils::LoadImage("TimeSigTool");
			break;
		}
		case EvtType_Repeat:
		{
			bitmap = ResourceUtils::LoadImage("RepeatTool");
			break;
		}
		case EvtType_End:
		{
			bitmap = ResourceUtils::LoadImage("EndTool");
			break;
		}
		default:
		{
			bitmap = ResourceUtils::LoadImage("PencilTool");
			break;
		}
	}
	CMenuTool *tool = dynamic_cast<CMenuTool *>
					  (ToolBar()->FindTool("Create"));
	if (tool && bitmap)
	{
		tool->SetBitmap(bitmap);
		ToolBar()->Invalidate(tool->Frame());
	}
}

// ---------------------------------------------------------------------------
// Internal Operations

void
CAssemblyWindow::AddMenuBar()
{
	BMenu *menu, *subMenu;
	BMenuItem *item;

	BMenuBar *menuBar = new BMenuBar(Bounds(), "General");

	// Create the 'File' menu
	CreateFileMenu(menuBar);

	// Create the edit menu
	menu = new BMenu("Edit");
	menu->AddItem(new BMenuItem("Undo", new BMessage(MENU_UNDO), 'Z'));
	menu->AddItem(new BMenuItem("Redo", new BMessage(MENU_REDO), 'Z', B_SHIFT_KEY));
	menu->AddSeparatorItem();
	menu->AddItem(item = new BMenuItem("Cut", new BMessage(B_CUT), 'X'));
	item->SetEnabled(false);
	menu->AddItem(item = new BMenuItem("Copy", new BMessage(B_COPY), 'C'));
	item->SetEnabled(false);
	menu->AddItem(item = new BMenuItem("Paste", new BMessage(B_PASTE), 'V'));
	item->SetEnabled(false);
	menu->AddItem(new CQuickKeyMenuItem("Clear", new BMessage(MENU_CLEAR), B_DELETE, "Del"));
	menu->AddItem(new BMenuItem("Select All", new BMessage(B_SELECT_ALL), 'A'));
	menuBar->AddItem(menu);

	// Create the 'Play' menu
	menu = new BMenu("Play");
	menu->AddItem(new CQuickKeyMenuItem("Pause", new BMessage(MENU_PAUSE), B_SPACE, "Space"));
	menu->AddSeparatorItem();
	menu->AddItem(new CQuickKeyMenuItem("Start", new BMessage(MENU_PLAY), B_ENTER, "Enter"));
	menu->AddItem(new CQuickKeyMenuItem("Play Section", new BMessage(MENU_PLAY_SECTION ), 'p', "p"));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem("Set Section", new BMessage(MENU_SET_SECTION), 'S', B_SHIFT_KEY));
	menuBar->AddItem(menu);
	
	// Create the 'View' menu
	menu = new BMenu("View");
	subMenu = new BMenu("Add Strip");
	menu->AddItem(subMenu);
	menuBar->AddItem(menu);

	// Create the 'Window' menu
	CreateWindowMenu(menuBar);

	// Add the menus
	AddChild(menuBar);
}

void
CAssemblyWindow::AddToolBar()
{
	// make the pop up menu for 'Create' tool
	BPopUpMenu *createMenu = new BPopUpMenu("", false, false);
	createMenu->SetFont(be_plain_font);
	BMessage *message = new BMessage(NEW_EVENT_TYPE_CHANGED);
	message->AddInt32("type", EvtType_Count);
	createMenu->AddItem(new CIconMenuItem("Default", message,
										  ResourceUtils::LoadImage("PencilTool")));
	createMenu->AddSeparatorItem();
	message = new BMessage(*message);
	message->ReplaceInt32("type", EvtType_Tempo);
	createMenu->AddItem(new CIconMenuItem("Tempo", message,
										  ResourceUtils::LoadImage("MetroTool")));
	message = new BMessage(*message);
	message->ReplaceInt32("type", EvtType_TimeSig);
	createMenu->AddItem(new CIconMenuItem("Time Signature", message,
										  ResourceUtils::LoadImage("TimeSigTool")));
	message = new BMessage(*message);
	message->ReplaceInt32("type", EvtType_Repeat);
	createMenu->AddItem(new CIconMenuItem("Repeat", message,
										  ResourceUtils::LoadImage("RepeatTool")));
	message = new BMessage(*message);
	message->ReplaceInt32("type", EvtType_End);
	createMenu->AddItem(new CIconMenuItem("Part End", message,
										  ResourceUtils::LoadImage("EndTool")));
	createMenu->SetTargetForItems((CDocWindow *)this);

	BRect rect(Bounds());
	if (KeyMenuBar())
		rect.top = KeyMenuBar()->Frame().bottom + 1.0;
	rect.right += 1.0;

	// add the tool bar
	CToolBar *toolBar = new CToolBar(rect, "General");
	CBitmapTool *tool;
	toolBar->AddTool(tool = new CBitmapTool("Snap To Grid",
											ResourceUtils::LoadImage("GridTool"),
											new BMessage(CEventEditor::TOOL_GRID)));
	tool->SetValue(B_CONTROL_ON);
	toolBar->AddSeparator();

	toolBar->AddTool(tool = new CBitmapTool("Select",
											ResourceUtils::LoadImage("ArrowTool"),
											new BMessage(CEventEditor::TOOL_SELECT)));
	tool->SetValue(B_CONTROL_ON);
	toolBar->AddTool(new CMenuTool("Create", ResourceUtils::LoadImage("PencilTool"),
								   createMenu, new BMessage(CEventEditor::TOOL_CREATE)));
	toolBar->AddTool(tool = new CBitmapTool("Eraser",
											ResourceUtils::LoadImage("EraserTool"),
											new BMessage(CEventEditor::TOOL_ERASE)));
	toolBar->MakeRadioGroup("Select", "Eraser", true);
	
	SetToolBar(toolBar);
}

void
CAssemblyWindow::AddFrameView(
	BRect frame,
	CTrack *track)
{
	// Create the frame for the strips, and the scroll bar
	stripFrame = new CStripFrameView(BRect(frame), (char *)NULL,
									 track, B_FOLLOW_ALL);

	stripScroll = new CScroller(BRect(frame.left - 1, frame.bottom + 1,
									  frame.right - 41, frame.bottom + 15),
								NULL, stripFrame, 0.0, 0.0, B_HORIZONTAL);

	BControl *magButton;
	magButton = new CBorderButton(BRect(frame.right - 27, frame.bottom + 1,
										frame.right - 13, frame.bottom + 15),
								  NULL, ResourceUtils::LoadImage("SmallPlus"),
								  new BMessage(ZoomIn_ID), true,
								  B_FOLLOW_BOTTOM | B_FOLLOW_RIGHT,
								  B_WILL_DRAW);
	magButton->SetTarget((CDocWindow *)this);
	AddChild(magButton);

	magButton = new CBorderButton(BRect(frame.right - 41, frame.bottom + 1,
										frame.right - 27, frame.bottom + 15),
								  NULL, ResourceUtils::LoadImage("SmallMinus"),
								  new BMessage(ZoomOut_ID), true,
								  B_FOLLOW_BOTTOM | B_FOLLOW_RIGHT,
								  B_WILL_DRAW);
	magButton->SetTarget((CDocWindow *)this);
	AddChild(magButton);

	// add views to window
	AddChild(stripScroll);
	AddChild(stripFrame);

	CTrackWindow::AddFrameView(frame, track);
}

// END - AssemblyWindow.cpp
