/* ===================================================================== *
 * LinearWindow.cpp (MeV/UI)
 * ===================================================================== */

#include "LinearWindow.h"

#include "BitmapTool.h"
#include "BorderButton.h"
#include "BorderView.h"
#include "EventTrack.h"
#include "IconMenuItem.h"
#include "Idents.h"
#include "LinearEditor.h"
#include "MenuTool.h"
#include "MeVApp.h"
#include "PitchBendEditor.h"
#include "PlayerControl.h"
#include "QuickKeyMenuItem.h"
#include "ResourceUtils.h"
#include "RulerView.h"
#include "TextDisplay.h"
#include "ToolBar.h"
#include "TrackCtlStrip.h"
#include "VelocityEditor.h"

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
	CEventTrack *track,
	bool hasSettings)
	:	CTrackWindow(frame, document, false, track, hasSettings)
{
	BRect rect(Bounds());

	AddMenuBar();
	AddToolBar();

	// ++++++ retrieve this from window settings if possible
	m_toolStates[0] = CEventEditor::TOOL_SELECT;

	rect.top = ToolBar()->Frame().bottom + 1.0;
	rect.bottom -= B_H_SCROLL_BAR_HEIGHT;
	AddFrameView(rect, track);

	stripFrame->AddType("Piano Roll");
	stripFrame->AddType("Velocity");
	stripFrame->AddType("Pitch Bend");
	stripFrame->AddType("Sequence");
	if (!hasSettings)
	{
		// create default strips
		AddStrip("Piano Roll", 0.5);
		AddStrip("Velocity", 0.25);
		AddStrip("Sequence", 0.25);
		stripFrame->PackStrips();
	}
}

// ---------------------------------------------------------------------------
// CTrackWindow Implementation

void
CLinearWindow::MessageReceived(
	BMessage* message)
{
	D_MESSAGE(("CLinearWindow::MessageReceived()\n"));

	switch (message->what)
	{
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
		default:
		{
			CTrackWindow::MessageReceived(message);
			break;
		}
	}
}

bool
CLinearWindow::SubjectReleased(
	CObservable *subject)
{
	if (subject == Track())
	{
		PostMessage(B_QUIT_REQUESTED, this);
	}
	
	return CTrackWindow::SubjectReleased(subject);
}

void
CLinearWindow::SubjectUpdated(
	BMessage *message)
{
	int32 trackHint;
	if (message->FindInt32("TrackAttrs", 0, &trackHint) != B_OK)
		trackHint = 0;

	int32 trackID;
	if (message->FindInt32("TrackID", 0, &trackID) != B_OK)
		trackID = -1;

	if (trackHint & CTrack::Update_Name)
		CalcWindowTitle(NULL, track->Name());
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
		strip = new CLinearEditor(*stripFrame, rect);
	}
	else if (type == "Velocity")
	{
		strip = new CVelocityEditor(*stripFrame, rect);
	}
	else if (type == "Pitch Bend")
	{
		strip = new CPitchBendEditor(*stripFrame, rect);
	}
	else if (type == "Control Change")
	{
		// nyi
	}
	else if (type == "Sequence")
	{
		strip = new CTrackCtlStrip(*stripFrame, rect, Track(),
								   "Sequence");
	}
	else if (type == "Record Strip")
	{
		//strip = new CRecordStrip (*this, *stripFrame, rect,Track());
	}
	if (strip)
	{
		stripFrame->AddStrip(strip, proportion);
		return true;
	}

	CTrackWindow::AddStrip(type, proportion);
}

void
CLinearWindow::NewEventTypeChanged(
	event_type type)
{
	BBitmap *bitmap;
	switch (type)
	{
		case EvtType_ProgramChange:
		{
			bitmap = ResourceUtils::LoadImage("ProgramTool");
			break;
		}
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
	menu->AddSeparatorItem();
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
	CreateWindowMenu(menuBar);

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
	CIconMenuItem *item;
	BMessage *message = new BMessage(NEW_EVENT_TYPE_CHANGED);
	message->AddInt32("type", EvtType_Count);
	createMenu->AddItem(new CIconMenuItem("Default", message,
										  ResourceUtils::LoadImage("PencilTool")));
	createMenu->AddSeparatorItem();
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
	createMenu->AddSeparatorItem();
	message = new BMessage(*message);
	message->ReplaceInt32("type", EvtType_ProgramChange);
	createMenu->AddItem(new CIconMenuItem("Program Change", message,
										  ResourceUtils::LoadImage("ProgramTool")));
	message = new BMessage(*message);
	message->ReplaceInt32("type", EvtType_SysEx);
	createMenu->AddItem(item = new CIconMenuItem("System Exclusive", message,
												 ResourceUtils::LoadImage("SysExTool")));
	item->SetEnabled(false);
	createMenu->SetTargetForItems((CDocWindow *)this);

	BRect rect(Bounds());
	if (KeyMenuBar())
		rect.top = KeyMenuBar()->Frame().bottom + 1.0;
	rect.right += 1.0;

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
	toolBar->AddTool(tool = new CBitmapTool("Erase",
											ResourceUtils::LoadImage("EraserTool"),
											new BMessage(CEventEditor::TOOL_ERASE)));
	toolBar->AddTool(tool = new CBitmapTool("Text",
											ResourceUtils::LoadImage("TextTool"),
											new BMessage(CEventEditor::TOOL_TEXT)));
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
	stripFrame = new CStripFrameView(BRect(frame.left,
										   frame.top + DEFAULT_RULER_HEIGHT + 1.0,
										   frame.right, frame.bottom),
									 (char *)NULL, track, B_FOLLOW_ALL);

	CScrollerTarget	*ruler;
	ruler = new CRulerView(BRect(frame.left + 21.0, frame.top,
								 frame.right - 14.0,
								 frame.top + DEFAULT_RULER_HEIGHT),
						   NULL,
						   *this, stripFrame, (CEventTrack *)track,
						   B_FOLLOW_LEFT_RIGHT,
						   B_WILL_DRAW);

	BView *pad;
	rgb_color fill = ui_color(B_PANEL_BACKGROUND_COLOR);
	pad = new CBorderView(BRect(frame.left - 1, frame.top - 1.0,
								frame.left + 20, 
								frame.top + DEFAULT_RULER_HEIGHT),
						  "", B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW, 
						  &fill);
	AddChild(pad);

	pad = new CBorderView(BRect(frame.right - 13, frame.top - 1.0,
								frame.right + 1, 
								frame.top + DEFAULT_RULER_HEIGHT),
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
	AddChild(ruler);
	AddChild(stripScroll);
	AddChild(stripFrame);

	CTrackWindow::AddFrameView(frame, track);
}

// END - LinearWindow.cpp
