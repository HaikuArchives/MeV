/* ===================================================================== *
 * LinearWindow.cpp (MeV/User Interface)
 * ===================================================================== */

#include "LinearWindow.h"

#include "AssemblyRulerView.h"
#include "BorderButton.h"
#include "EventTrack.h"
#include "PlayerControl.h"
#include "Idents.h"
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
	CMeVDoc *document,
	CEventTrack *track)
	:	CTrackWindow(frame, document, track)
{
	BRect rect(Bounds());

	AddMenuBar();
	AddToolBar();

	rect.top = ToolBar()->Frame().bottom + 1.0;
	rect.bottom -= B_H_SCROLL_BAR_HEIGHT;
	AddFrameView(rect, track);

	stripFrame->AddType("Piano Roll");
	stripFrame->AddType("Velocity");
	stripFrame->AddType("Pitch Bend");
	stripFrame->AddType("Sequence");

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
	AddStrip("Piano Roll", 0.5);
	AddStrip("Velocity", 0.25);
	AddStrip("Sequence", 0.25);
	stripFrame->PackStrips();
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

	BMenuItem *item;

	item = KeyMenuBar()->FindItem("Set Section");
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
			CMenuTool *tool = dynamic_cast<CMenuTool *>
							  (ToolBar()->FindTool("Create"));
			if (tool && bitmap)
			{
				tool->SetBitmap(bitmap);
				ToolBar()->Invalidate(tool->Frame());
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
			Document()->SetModified();
			break;
		}
		case B_SELECT_ALL:
		{
			if (CurrentFocus())
				DispatchMessage(message, CurrentFocus());
			else
				Track()->SelectAll();
			break;
		}
		case MENU_NEW_WINDOW:
		{
			CLinearWindow *window;
			window = new CLinearWindow(BRect(60, 60, 340, 300),
									   Document(), Track() );
			window->Show();
			break;
		}
		case MENU_PROGRAM_SETTINGS:
		{
			((CMeVApp *)be_app)->ShowPrefs();
			break;
		}
		case MENU_PLAY:
		{
			if (CPlayerControl::IsPlaying(Document()))
			{
				CPlayerControl::StopSong(Document());
				break;
			}
			CPlayerControl::PlaySong(Document(), Track()->GetID(), 0,
									 LocateTarget_Real, -1,
									 SyncType_SongInternal,
									 (app.GetLoopFlag() ? PB_Loop : 0) );
			break;
		}		
		case MENU_PLAY_SECTION:
		{
			// Start playing a song.
			CPlayerControl::PlaySong(Document(), Track()->GetID(),
									 Track()->SectionStart(),
									 LocateTarget_Metered,
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
		trackHint = 0;

	int32 trackID;
	if (message->FindInt32("TrackID", 0, &trackID) != B_OK)
		trackID = -1;

	if (trackHint & CTrack::Update_Name)
		CalcWindowTitle(track->Name());
}

// ---------------------------------------------------------------------------
// Internal Operations

void
CLinearWindow::AddMenuBar()
{
	D_INTERNAL(("CLinearWindow::AddMenuBar()\n"));

	BMenu *menu, *subMenu;
	BMenuBar *menuBar = new BMenuBar(Bounds(), NULL);

	// Create the 'File' menu
	CreateFileMenu(menuBar);

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
	menuBar->AddItem(menu);

	// Create the 'Play' menu
	menu = new BMenu("Play");
	menu->AddItem(new CQuickKeyMenuItem("Pause", new BMessage(MENU_PAUSE), B_SPACE, "Space"));
	menu->AddSeparatorItem();
	menu->AddItem(new CQuickKeyMenuItem("Start", new BMessage(MENU_PLAY), B_ENTER, "Enter"));
	menu->AddItem(new CQuickKeyMenuItem("Play Section", new BMessage(MENU_PLAY_SECTION), 'p', "p"));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem("Set Section", new BMessage(MENU_SET_SECTION), 'S', B_SHIFT_KEY));
	menuBar->AddItem(menu);
	
	// Create the 'View' menu
	menu = new BMenu("View");
	subMenu = new BMenu("Add Strip");
	menu->AddItem(subMenu);
	menuBar->AddItem(menu);

	// Create the 'Window' menu
	menu = new BMenu("Window");
	menu->AddItem(new BMenuItem("New Window",
								new BMessage(MENU_NEW_WINDOW), 'W',
								B_SHIFT_KEY));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem("Show Tracks Window",
								new BMessage(MENU_TRACKLIST), 'L'));
	menu->AddItem(new BMenuItem("Show Event Inspector",
								new BMessage(MENU_INSPECTOR), 'I'));
	menu->AddItem(new BMenuItem("Show Grid Window",
								new BMessage(MENU_GRIDWINDOW), 'G'));
	menu->AddItem(new BMenuItem("Show Transport Controls",
								new BMessage(MENU_TRANSPORT), 'T'));
	menu->AddSeparatorItem();
	SetWindowMenu(menu);
	menuBar->AddItem(menu);

	// Add the menus
	AddChild(menuBar);
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
	if (KeyMenuBar())
		rect.top = KeyMenuBar()->Frame().bottom + 1.0;
	rect.right += 1.0;

	CToolBar *toolBar = new CToolBar(rect, "General");
	CBitmapTool *tool;
	toolBar->AddTool(tool = new CBitmapTool("Snap To Grid",
											ResourceUtils::LoadImage("GridTool"),
											new BMessage(TOOL_GRID)));
	tool->SetValue(B_CONTROL_ON);
	toolBar->AddSeparator();

	toolBar->AddTool(tool = new CBitmapTool("Select",
											ResourceUtils::LoadImage("ArrowTool"),
											new BMessage(TOOL_SELECT)));
	tool->SetValue(B_CONTROL_ON);
	toolBar->AddTool(new CMenuTool("Create", ResourceUtils::LoadImage("PencilTool"),
								   createMenu, new BMessage(TOOL_CREATE)));
	toolBar->AddTool(tool = new CBitmapTool("Erase",
											ResourceUtils::LoadImage("EraserTool"),
											new BMessage(TOOL_ERASE)));
	toolBar->AddTool(tool = new CBitmapTool("Text",
											ResourceUtils::LoadImage("TextTool"),
											new BMessage(TOOL_TEXT)));
	tool->SetEnabled(false);
	toolBar->MakeRadioGroup("Select", "Text", true);

	SetToolBar(toolBar);
}

void
CLinearWindow::AddFrameView(
	BRect frame,
	CTrack *track)
{
	// Create the frame for the strips, and the scroll bar
	stripFrame = new CTrackEditFrame(BRect(frame.left,
										   frame.top + DEFAULT_RULER_HEIGHT,
										   frame.right, frame.bottom),
									 (char *)NULL, track, B_FOLLOW_ALL);

	CScrollerTarget	*ruler;
	ruler = new CAssemblyRulerView(*this, stripFrame, (CEventTrack *)track,
								   BRect(frame.left + 21, frame.top,
										 frame.right - 14, 
										 frame.top + DEFAULT_RULER_HEIGHT - 1),
								   (char *)NULL, B_FOLLOW_LEFT_RIGHT,
								   B_WILL_DRAW);

	BView *pad;
	rgb_color fill = ui_color(B_PANEL_BACKGROUND_COLOR);
	pad = new CBorderView(BRect(frame.left - 1, frame.top - 1,
								frame.left + 20, 
								frame.top + DEFAULT_RULER_HEIGHT - 1),
						  "", B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW, 
						  &fill);
	AddChild(pad);

	pad = new CBorderView(BRect(frame.right - 13, frame.top - 1,
								frame.right + 1, 
								frame.top + DEFAULT_RULER_HEIGHT - 1),
						  "", B_FOLLOW_RIGHT | B_FOLLOW_TOP, B_WILL_DRAW,
						  &fill);
	AddChild(pad);

	stripScroll = new CScroller(BRect(frame.left - 1, frame.bottom + 1,
									  frame.right - 41, frame.bottom + 15),
								NULL, stripFrame, 0.0, 0.0, B_HORIZONTAL);

	BControl *magButton;
	magButton = new CBorderButton(BRect(frame.right - 27, frame.bottom + 1,
										frame.right - 13, frame.bottom + 15),
								  NULL, ResourceUtils::LoadImage("SmallPlus"),
								  new BMessage(ZoomIn_ID),
								  B_FOLLOW_BOTTOM | B_FOLLOW_RIGHT,
								  B_WILL_DRAW);
	magButton->SetTarget((CDocWindow *)this);
	AddChild(magButton);

	magButton = new CBorderButton(BRect(frame.right - 41, frame.bottom + 1,
										frame.right - 27, frame.bottom + 15),
								  NULL, ResourceUtils::LoadImage("SmallMinus"),
								  new BMessage(ZoomOut_ID),
								  B_FOLLOW_BOTTOM | B_FOLLOW_RIGHT,
								  B_WILL_DRAW);
	magButton->SetTarget((CDocWindow *)this);
	AddChild(magButton);

	// add views to window
	AddChild(ruler);
	AddChild(stripScroll);
	AddChild(stripFrame);
}

bool
CLinearWindow::AddStrip(
	BString type,
	float proportion)
{
	BRect rect(Bounds());
	rect.top = ToolBar()->Frame().bottom
			   + CTrackWindow::DEFAULT_RULER_HEIGHT + 1.0;
	rect.bottom -= B_H_SCROLL_BAR_HEIGHT;

	CStripView *strip = NULL;

	if (type == "Piano Roll")
	{
		strip = new CLinearEditor(*this, *stripFrame, rect);
	}
	else if (type == "Velocity")
	{
		strip = new CVelocityEditor(*this, *stripFrame, rect);
	}
	else if (type == "Pitch Bend")
	{
		strip = new CPitchBendEditor(*this, *stripFrame, rect);
	}
	else if (type == "Control Change")
	{
		// nyi
	}
	else if (type == "Sequence")
	{
		strip = new CTrackCtlStrip(*this, *stripFrame, rect, Track(),
								   "Sequence");
	}

	if (strip)
	{
		stripFrame->AddStrip(strip, proportion);
		return true;
	}

	CTrackWindow::AddStrip(type, proportion);
}

// END - LinearWindow.cpp
