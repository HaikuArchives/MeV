/* ===================================================================== *
 * LinearWindow.cpp (MeV/User Interface)
 * ===================================================================== */

#include "LinearWindow.h"
#include "PlayerControl.h"
#include "Idents.h"
#include "MeVDoc.h"
#include "MeVApp.h"
#include "ResourceUtils.h"
#include "QuickKeyMenuItem.h"
#include "TextDisplay.h"
#include "BorderView.h"
#include "LinearEditor.h"
#include "TrackCtlStrip.h"
#include "VelocityEditor.h"
#include "PitchBendEditor.h"
// User Interface
#include "BitmapTool.h"
#include "IconMenuItem.h"
#include "MenuTool.h"
#include "ToolBar.h"

// Gnu C Library
#include <stdio.h>
// Interface Kit
#include <Bitmap.h>
#include <Box.h>
#include <MenuBar.h>
#include <PopUpMenu.h>
#include <TextControl.h>
// Support Kit
#include <Debug.h>

// Debugging Macros
#define D_ALLOC(x) //PRINT (x)			// Constructor/Destructor
#define D_MESSAGE(x) //PRINT (x)		// MessageReceived()
#define D_INTERNAL(x) //PRINT (x)		// Internal Operations

// ---------------------------------------------------------------------------
// Constructor/Destructor

CLinearWindow::CLinearWindow(
	BRect frame,
	CMeVDoc &document,
	CEventTrack *track )
	:	CTrackWindow(frame, document, track )
{
	BRect rect(Bounds());

	AddMenuBar();
	AddToolBar();

	rect.top = m_toolBar->Frame().bottom + 1.0;
	rect.bottom -= B_H_SCROLL_BAR_HEIGHT;
	CreateFrames(rect, track);
	BRect scrollFrame(stripScroll->Frame());
	stripScroll->ResizeTo(scrollFrame.Width() - 100.0, scrollFrame.Height());
	stripScroll->MoveTo(scrollFrame.left + 100.0, scrollFrame.top);

	// Add the time display string view
	scrollFrame.right = 100.0;
	BView *view = new CBorderView(scrollFrame, "", B_FOLLOW_LEFT | B_FOLLOW_BOTTOM,
								  B_WILL_DRAW);
	AddChild(view);
	m_timeView = new CTextDisplay(BRect(1.0, 1.0, scrollFrame.right,
								  scrollFrame.Height()),
								  "", false );
	m_timeView->SetAlignment(B_ALIGN_RIGHT);
	m_timeView->SetFont(be_fixed_font);
	m_timeView->SetFontSize(10);
	view->AddChild(m_timeView);

	// Now, create some strips for test purposes
	CStripView *sv;
	rect.top += CTrackWindow::DEFAULT_RULER_HEIGHT;
	sv = new CLinearEditor(*this, *stripFrame, rect);
	stripFrame->AddChildView(sv->TopView(), 80);
	sv = new CVelocityEditor(*this, *stripFrame, rect);
	stripFrame->AddChildView(sv->TopView(), 15);
	sv = new CPitchBendEditor(*this, *stripFrame, rect);
	stripFrame->AddChildView(sv->TopView(), 15);
	sv = new CTrackCtlStrip(*this, *stripFrame, rect, track);
	stripFrame->AddChildView(sv->TopView(), 15);
}

// ---------------------------------------------------------------------------
// CTrackWindow Implementation

void
CLinearWindow::DisplayMouseTime(
	CTrack *track,
	int32 time)
{
	long majorUnit, minorUnit, extraTime;

	if (track == NULL)
	{
		m_timeBuf[0] = '\0';
	}
	else
	{
		track->SigMap().DecomposeTime(time, majorUnit, minorUnit, extraTime);
		if (track->ClockType() == ClockType_Real)
		{
			int32 hours = majorUnit / 60;
			int32 minutes = majorUnit - (hours * 60);
			sprintf(m_timeBuf, "%2ld:%2.2ld:%2.2ld.%2.2ld", hours, minutes, minorUnit, extraTime);
		}
		else
		{
			sprintf(m_timeBuf, "%4ld.%2.2ld.%4.4ld", majorUnit + 1, minorUnit, extraTime);
		}
	}
	m_timeView->SetText(m_timeBuf);
}

void
CLinearWindow::MenusBeginning()
{
	CTrackWindow::MenusBeginning();
	BMenuItem *item = KeyMenuBar()->FindItem("Set Section");
	item->SetEnabled(Track()->SelectionType() != CTrack::Select_None);
}

void
CLinearWindow::MessageReceived(
	BMessage* message)
{
	D_MESSAGE(("CLinearWindow::MessageReceived()\n"));

	CMeVApp &app = *(CMeVApp *)be_app;

	switch (message->what)
	{
		case NEW_EVENT_TYPE_CHANGED:
		{
			D_MESSAGE((" -> NEW_EVENT_TYPE_CHANGED\n"));
			int32 type;
			if (message->FindInt32("type", &type) != B_OK)
			{
				return;
			}
			newEventType = (enum E_EventType)(type);
			BBitmap *bitmap;
			switch (newEventType)
			{
				case EvtType_ProgramChange:
				{
					bitmap = ResourceUtils::LoadImage("ProgramTool");
					break;
				}
				case EvtType_TimeSig:
				{
					bitmap = ResourceUtils::LoadImage("TimeSigTool");
					break;
				}
				case EvtType_Sequence:
				{
					bitmap = ResourceUtils::LoadImage("TrackTool");
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
				case EvtType_SysEx:
				{
					bitmap = ResourceUtils::LoadImage("SysExTool");
					break;
				}
				default:
				{
					bitmap = ResourceUtils::LoadImage("PencilTool");
					break;
				}
			}
			CMenuTool *tool = dynamic_cast<CMenuTool *>(m_toolBar->FindTool("Create"));
			if (tool && bitmap)
			{
				tool->SetBitmap(bitmap);
				m_toolBar->Invalidate(tool->Frame());
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
/*		case 'dura':
			// REM: Do we also want to change duration of selected events?
			((CMeVDoc &)Document()).SetDefaultAttribute( EvAttr_Duration, durationEditor->Value() );
			newEventDuration = durationEditor->Value();
			break;
*/		
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
			if (CurrentFocus()) DispatchMessage( message, CurrentFocus() );
			else { /* REM: Add CUT code */ };
			break;
		}
		case B_COPY:
		{
			if (CurrentFocus()) DispatchMessage( message, CurrentFocus() );
			else { /* REM: Add CUT code */ };
			break;
		}
		case B_PASTE:
		{
			if (CurrentFocus()) DispatchMessage( message, CurrentFocus() );
			else { /* REM: Add CUT code */ };
			break;
		}
		case MENU_CLEAR:
		{
			Track()->DeleteSelection();
			Document().SetModified();
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
			CLinearWindow *window;
			window = new CLinearWindow( BRect( 60, 60, 340, 300 ), (CMeVDoc &)document, Track() );
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
			((CMeVDoc &)Document()).ShowWindow( CMeVDoc::VChannel_Window );
			break;
		}
		case MENU_PLAY:
		{
			if (CPlayerControl::IsPlaying( (CMeVDoc *)&document ))
			{
				CPlayerControl::StopSong( (CMeVDoc *)&document );
				break;
			}
			//Start playing a song.
	/*		CPlayerControl::PlaySong(	(CMeVDoc *)&document,
									0, 0, LocateTarget_Real, -1,
									SyncType_SongInternal, (app.GetLoopFlag() ? PB_Loop : 0) );
	*/		CPlayerControl::PlaySong(	(CMeVDoc *)&document,
									Track()->GetID(), 0, LocateTarget_Real, -1,
									SyncType_SongInternal, (app.GetLoopFlag() ? PB_Loop : 0) );
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
		case MENU_PAUSE:
		{
		 	break;
		}
		case Select_ID:
		{
			bool		active = false;
			message->FindBool( "active", 0, &active );
			UpdateActiveSelection( active );
			break;
		}		
		default:
		{
			CTrackWindow::MessageReceived( message );
			break;
		}
	}
}

void
CLinearWindow::OnUpdate(
	BMessage *message)
{
	int32 trackHint;
	if (message->FindInt32("TrackAttrs", 0, &trackHint) != B_OK)
	{
		trackHint = 0;
	}

	int32 trackID;
	if (message->FindInt32("TrackID", 0, &trackID) != B_OK)
	{
		trackID = -1;
	}

	if (trackHint & CTrack::Update_Name)
	{
//		trackNameCtl->SetText( Track()->Name() );
	}
}

// ---------------------------------------------------------------------------
// Internal Operations

void
CLinearWindow::AddMenuBar()
{
	D_INTERNAL(("CLinearWindow::AddMenuBar()\n"));

	BMenu *menu;
	menus = new BMenuBar(Bounds(), NULL);

	// Create the 'File' menu
	CreateFileMenu(menus);

	// Create the 'Edit' menu
	menu = new BMenu("Edit");
	menu->AddItem(new BMenuItem("Undo", new BMessage(MENU_UNDO), 'Z'));
	menu->AddItem(new BMenuItem("Redo", new BMessage(MENU_REDO), 'Z', B_SHIFT_KEY));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem("Cut", new BMessage(B_CUT), 'X'));
	menu->AddItem(new BMenuItem("Copy", new BMessage(B_COPY), 'C'));
	menu->AddItem(new BMenuItem("Paste", new BMessage(B_PASTE), 'V'));
	menu->AddItem(new CQuickKeyMenuItem("Clear", new BMessage(MENU_CLEAR), 
										B_DELETE, "Del"));
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
// menu->AddItem( playSelectMenu = new CQuickKeyMenuItem( "Play Selection", new BMessage( MENU_PLAY_SELECT ), B_ENTER, B_SHIFT_KEY ) );
	menu->AddItem(new CQuickKeyMenuItem("Play Section", new BMessage(MENU_PLAY_SECTION), 'p', "p"));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem("Set Section", new BMessage(MENU_SET_SECTION), 'S', B_SHIFT_KEY));
	menus->AddItem(menu);
	
	// Create the plug-ins menu
//	menus->AddItem(plugInMenu);

	// Create the 'Window' menu
	windowMenu = new BMenu("Window");
	windowMenu->AddItem(new BMenuItem( "New Window", new BMessage(MENU_NEW_WINDOW), 'W', B_SHIFT_KEY));
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
	AddChild(menus);
}

void
CLinearWindow::AddToolBar()
{
	D_INTERNAL(("CLinearWindow::AddToolBar()\n"));

	// make the pop up menu for 'Create' tool
	BPopUpMenu *createMenu = new BPopUpMenu("", false, false);
	createMenu->SetFont(be_plain_font);
	BMessage *message = new BMessage(NEW_EVENT_TYPE_CHANGED);
	message->AddInt32("type", EvtType_Count);
	createMenu->AddItem(new CIconMenuItem("Default", message,
										  ResourceUtils::LoadImage("PencilTool")));
	message = new BMessage(*message);
	message->ReplaceInt32("type", EvtType_ProgramChange);
	createMenu->AddItem(new CIconMenuItem("Program Change", message,
										  ResourceUtils::LoadImage("ProgramTool")));
	message = new BMessage(*message);
	message->ReplaceInt32("type", EvtType_Sequence);
	createMenu->AddItem(new CIconMenuItem("Nested Track", message,
										  ResourceUtils::LoadImage("TrackTool")));
	message = new BMessage(*message);
	message->ReplaceInt32("type", EvtType_Repeat);
	createMenu->AddItem(new CIconMenuItem("Repeat", message,
										  ResourceUtils::LoadImage("RepeatTool")));
	message = new BMessage(*message);
	message->ReplaceInt32("type", EvtType_TimeSig);
	createMenu->AddItem(new CIconMenuItem("Time Signature", message,
										  ResourceUtils::LoadImage("TimeSigTool")));
	message = new BMessage(*message);
	message->ReplaceInt32("type", EvtType_SysEx);
	createMenu->AddItem(new CIconMenuItem("System Exclusive", message,
										  ResourceUtils::LoadImage("SysExTool")));
	message = new BMessage(*message);
	message->ReplaceInt32("type", EvtType_End);
	createMenu->AddItem(new CIconMenuItem("Track End", message,
										  ResourceUtils::LoadImage("EndTool")));
	createMenu->SetTargetForItems((CDocWindow *)this);


	BRect rect(Bounds());
	if (menus)
		rect.top = menus->Frame().bottom + 1.0;
	rect.right += 1.0;

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
	m_toolBar->AddTool(new CMenuTool("Create", ResourceUtils::LoadImage("PencilTool"),
								   createMenu, new BMessage(TOOL_CREATE)));
	m_toolBar->AddTool(tool = new CBitmapTool("Erase",
											ResourceUtils::LoadImage("EraserTool"),
											new BMessage(TOOL_ERASE)));
	tool->SetEnabled(false);
	m_toolBar->AddTool(tool = new CBitmapTool("Text",
											ResourceUtils::LoadImage("TextTool"),
											new BMessage(TOOL_TEXT)));
	tool->SetEnabled(false);
	m_toolBar->MakeRadioGroup("Select", "Text", true);

	AddChild(m_toolBar);
}

// END - LinearWindow.cpp
