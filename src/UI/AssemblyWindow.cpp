/* ===================================================================== *
 * AssemblyWindow.cpp (MeV/User Interface)
 * ===================================================================== */

#include "AssemblyWindow.h"
#include "AssemblyRulerView.h"
#include "LinearWindow.h"
#include "PlayerControl.h"
#include "Idents.h"
#include "MeVDoc.h"
#include "MeVApp.h"
#include "ScreenUtils.h"
#include "QuickKeyMenuItem.h"
#include "MultiColumnListView.h"
#include "TrackCtlStrip.h"
#include "DataSnap.h"
// User Interface
#include "BitmapTool.h"
#include "Splitter.h"
#include "ToolBar.h"

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
	CMeVDoc &document )
	:	CTrackWindow(frame, document, (CEventTrack *)document.FindTrack(1))
{
	D_ALLOC(("CAssemblyWindow::CAssemblyWindow()\n"));

	AddMenuBar();
	AddToolBar();

	BRect rect(Bounds());
	rect.top = m_toolBar->Frame().bottom + 1.0;
	rect.bottom -= B_H_SCROLL_BAR_HEIGHT;
	CreateFrames(rect, (CTrack *)document.FindTrack(1));

	// Now, create some strips for test purposes
	CStripView *sv;
	CAssemblyRulerView *realRuler;
	CScrollerTarget	*oldRuler = stripFrame->Ruler();

	sv = new CTrackCtlStrip(*this, *stripFrame, rect,
							(CEventTrack *)document.FindTrack(1), "Metrical");
	sv->SetRemovable(false);
	stripFrame->AddChildView(sv->TopView(), 50);

	realRuler = new CAssemblyRulerView(*this, stripFrame,
									   (CEventTrack *)document.FindTrack((int32)0),
									   BRect(0.0, 0.0, rect.Width() - 14, 
									   CTrackWindow::DEFAULT_RULER_HEIGHT - 1),
									   (char *)NULL, B_FOLLOW_LEFT_RIGHT,
									   B_WILL_DRAW);
	realRuler->ShowMarkers(false);
	stripFrame->SetRuler(oldRuler);

	CEventEditor *ee;
	ee = new CTrackCtlStrip(*this, *stripFrame, frame,
							(CEventTrack *)document.FindTrack((int32)0), "Real");
	ee->SetRuler(realRuler);
	stripFrame->AddChildView(ee->TopView(), 8);

	newEventType = EvtType_Sequence;
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
	CMeVDoc &doc = (CMeVDoc &)Document();
	CMeVApp	&app = *(CMeVApp *)be_app;

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
			doc.SetModified();
			break;
		}
		case B_SELECT_ALL:
		{
			if (CurrentFocus()) DispatchMessage( message, CurrentFocus() );
			else Track()->SelectAll();
			break;
		}
		case MENU_NEW_WINDOW:
		{
			CAssemblyWindow *window;
		
			window = new CAssemblyWindow( UScreenUtils::StackOnScreen( 540, 300 ), doc );
			window->Show();
			break;
		}
		case MENU_PROGRAM_SETTINGS:
		{
			((CMeVApp *)be_app)->ShowPrefs();
			break;
		}
		case MENU_MIDI_CONFIG:
		{
			((CMeVApp *)be_app)->ShowMidiConfig();
			break;
		}
		case MENU_VIEW_SETTINGS:
		{
			ShowPrefs();
			break;
		}
		case MENU_VIRTUAL_CHANNELS:
		{
			doc.ShowWindow( CMeVDoc::VChannel_Window );
			break;
		}
		case MENU_PLAY:
		{
			if (CPlayerControl::IsPlaying( (CMeVDoc *)&document ))
			{
				CPlayerControl::StopSong( (CMeVDoc *)&document );
				break;
			}
			// Start playing a song.
			CPlayerControl::PlaySong(	(CMeVDoc *)&doc,
									0, 0, LocateTarget_Real, -1,
									SyncType_SongInternal,
									(app.GetLoopFlag() ? PB_Loop : 0) );
			break;
		}
		case MENU_PLAY_SECTION:
		{	
			// Start playing a song.
			CPlayerControl::PlaySong(
				(CMeVDoc *)&document,
				Track()->GetID(),
				Track()->SectionStart(), LocateTarget_Metered,
				Track()->SectionEnd() - Track()->SectionStart(),
				SyncType_SongInternal,
				(app.GetLoopFlag() ? PB_Loop : 0) | PB_Folded );
			break;
		}
		case MENU_SET_SECTION:
		{
			if (Track()->SelectionType() != CTrack::Select_None)
			{
				Track()->SetSection( Track()->MinSelectTime(), Track()->MaxSelectTime() );
				Track()->NotifyUpdate( CTrack::Update_Section, NULL );
			}
			break;
		}
		case TOOL_GRID:
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
		case TOOL_SELECT:
		case TOOL_CREATE:
		case TOOL_ERASE:
		case TOOL_TEXT:
		{
			D_MESSAGE((" -> TOOL_SELECT/CREATE/ERASE/TEXT\n"));
			m_toolStates[0] = message->what;
			break;
		}
		case Select_ID:
		{
			bool active = false;	
			message->FindBool("active", 0, &active);
			UpdateActiveSelection(active);
			break;
		}	
		case 'cMet':
		{
			stripFrame->SetFrameClockType( ClockType_Metered );
			break;
		}
		case 'cRea':
		{
			stripFrame->SetFrameClockType( ClockType_Real );
			break;
		}
		default:
		{
			CTrackWindow::MessageReceived(message);
			break;
		}
	}
}

//	Update inspector info when we get an observer update message.
//	Overridden from the CObserver class.
void
CAssemblyWindow::OnUpdate(
	BMessage *inMsg)
{
	CTrackWindow::OnUpdate( inMsg );
}

// ---------------------------------------------------------------------------
// Internal Operations

void
CAssemblyWindow::AddMenuBar()
{
	BMenu *menu;
	BMenuItem *item;

	menus = new BMenuBar(Bounds(), "General");

	// Create the 'File' menu
	CreateFileMenu(menus);

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
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem("View Settings...", new BMessage(MENU_VIEW_SETTINGS)));
	menu->AddItem(new BMenuItem("Virtual Channels...", new BMessage(MENU_VIRTUAL_CHANNELS)));
	menus->AddItem(menu);

	// Create the 'Play' menu
	menu = new BMenu("Play");
	menu->AddItem(new CQuickKeyMenuItem("Pause", new BMessage(MENU_PAUSE), B_SPACE, "Space"));
	menu->AddSeparatorItem();
	menu->AddItem(new CQuickKeyMenuItem("Start", new BMessage(MENU_PLAY), B_ENTER, "Enter"));
	menu->AddItem(new CQuickKeyMenuItem("Play Section", new BMessage(MENU_PLAY_SECTION ), 'p', "p"));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem("Set Section", new BMessage(MENU_SET_SECTION), 'S', B_SHIFT_KEY));
	menus->AddItem(menu);
	
	// Create the 'Window' menu
	windowMenu = new BMenu("Window");
	windowMenu->AddItem(new BMenuItem("New Window", new BMessage(MENU_NEW_WINDOW), 'W', B_SHIFT_KEY));
	windowMenu->AddSeparatorItem();
	windowMenu->AddItem(new BMenuItem("Show Tracks Window",
									  new BMessage(MENU_TRACKLIST), 'L'));
	windowMenu->AddItem(new BMenuItem("Show Event Inspector",
									  new BMessage(MENU_INSPECTOR), 'I'));
	windowMenu->AddItem(new BMenuItem("Show Grid Window",
									  new BMessage(MENU_GRIDWINDOW), 'G'));
	windowMenu->AddItem(new BMenuItem("Show Transport Controls",
									  new BMessage(MENU_TRANSPORT), 'T'));
	windowMenu->AddSeparatorItem();
	menus->AddItem(windowMenu);

	// Add the menus
	AddChild( menus );
}

void
CAssemblyWindow::AddToolBar()
{
	BRect rect(Bounds());
	if (menus)
		rect.top = menus->Frame().bottom + 1.0;
	rect.right += 1.0;

	// add the tool bar
	m_toolBar = new CToolBar(rect, "General");
	CBitmapTool *tool;
	m_toolBar->AddTool(tool = new CBitmapTool("Snap To Grid",
											  ResourceUtils::LoadImage("GridTool"),
											  new BMessage(TOOL_GRID)));
	tool->SetValue(B_CONTROL_ON);
	m_toolBar->AddSeparator();

	m_toolBar->AddTool(tool = new CBitmapTool("Select",
											  ResourceUtils::LoadImage("ArrowTool"),
											  new BMessage(TOOL_SELECT)));
	tool->SetValue(B_CONTROL_ON);
	m_toolBar->AddTool(tool = new CBitmapTool("Eraser",
											  ResourceUtils::LoadImage("EraserTool"),
											  new BMessage(TOOL_ERASE)));
	m_toolBar->MakeRadioGroup("Select", "Eraser", true);
	
	AddChild(m_toolBar);
}

// END - AssemblyWindow.cpp
