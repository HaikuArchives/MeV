/* ===================================================================== *
 * LinearWindow.cpp (MeV/User Interface)
 * ===================================================================== */

#include "LinearWindow.h"
#include "PlayerControl.h"
#include "Idents.h"
#include "MeVDoc.h"
#include "MeVApp.h"
#include "ResourceUtils.h"

#include "TimeIntervalEditor.h"
#include "QuickKeyMenuItem.h"
#include "TextDisplay.h"
#include "BorderView.h"
#include "ToolBar.h"

#include "LinearEditor.h"
#include "TrackCtlStrip.h"
#include "VelocityEditor.h"
#include "PitchBendEditor.h"

// Gnu C Library
#include <stdio.h>
// Interface Kit
#include <Bitmap.h>
#include <Box.h>
#include <MenuBar.h>
#include <PopUpMenu.h>
#include <TextControl.h>

class CImageAndTextMenuItem : 
	public BMenuItem
{
	
	const BBitmap			*bitmap;
	BPoint				imageSize;
	
public:

	CImageAndTextMenuItem(	const char	*inLabel,
							const BBitmap	 *inBitmap,
							BPoint		inSize,
							BMessage		*inMessage,
							char			inShortcutKey = 0,
							uint32		inModifiers = 0)
		: BMenuItem( inLabel, inMessage, inShortcutKey, inModifiers )
	{
		bitmap = inBitmap;
		imageSize = inSize;
	}

	void DrawContent()
	{
		BRect		r( Frame() );
		BFont		font;
		font_height	fh;
		
		Menu()->GetFont( &font );
		font.GetHeight( &fh );

		Menu()->MovePenTo( r.left + imageSize.x + 4, (r.bottom + r.top - fh.descent + fh.ascent)/2 );
		Menu()->SetDrawingMode( B_OP_OVER );
		Menu()->DrawString( Label() );
		
		if (bitmap != NULL)
		{
			BRect	br( bitmap->Bounds() );

			Menu()->DrawBitmapAsync(	bitmap,
									BPoint(	r.left + (imageSize.x - br.Width())/2,
											(r.top + r.bottom - br.Height())/2 ) );
		}
	}

	void GetContentSize( float *width, float *height )
	{
		BMenuItem::GetContentSize( width, height );
		*width += imageSize.x + 4;
		*height = MAX( *height, imageSize.y );
	}
};


// ---------------------------------------------------------------------------
// Linear window set-up

const float ToolArea_Height		= 50.0;
const float Tool_Width			= 148.0;

CLinearWindow::CLinearWindow( BRect frame, CMeVDoc &inDocument, CEventTrack *inTrack )
	: CTrackWindow( frame, inDocument, inTrack )
{
	BView			*topToolArea;
	BMenu			*menu;
	float			x = 2.0;

		// view rect should be same size as window rect but with left top at (0, 0)

	menus = new BMenuBar( BRect( 0,0,0,0 ), NULL );

	CreateFileMenu( menus );

		// Create the edit menu
	menu = new BMenu( "Edit" );
	menu->AddItem( undoMenu = new BMenuItem( "Undo", new BMessage( MENU_UNDO ), 'Z' ) );
	menu->AddItem( redoMenu = new BMenuItem( "Redo", new BMessage( MENU_REDO ), 'Z', B_SHIFT_KEY ) );
	menu->AddSeparatorItem();
	menu->AddItem( clearMenu = new BMenuItem( "Cut", new BMessage( B_CUT ), 'X' ) );
	menu->AddItem( clearMenu = new BMenuItem( "Copy", new BMessage( B_COPY ), 'C' ) );
	menu->AddItem( clearMenu = new BMenuItem( "Paste", new BMessage( B_PASTE ), 'V' ) );
	menu->AddItem( clearMenu = new CQuickKeyMenuItem( "Clear", new BMessage( MENU_CLEAR ), B_DELETE, "Del" ) );
	menu->AddItem( new BMenuItem( "Select All", new BMessage( B_SELECT_ALL ), 'A' ) );
	menu->AddSeparatorItem();
	menu->AddItem( new BMenuItem( "View Settings...", new BMessage( MENU_VIEW_SETTINGS ) ) );
	menu->AddItem( new BMenuItem( "Virtual Channels...", new BMessage( MENU_VIRTUAL_CHANNELS ) ) );
	menus->AddItem( menu );

		// Create the edit menu
	menu = new BMenu( "Play" );
	menu->AddItem( pauseMenu = new CQuickKeyMenuItem( "Pause", new BMessage( MENU_PAUSE ), B_SPACE, "Space" ) );
	menu->AddSeparatorItem();
	menu->AddItem( playMenu = new CQuickKeyMenuItem( "Start", new BMessage( MENU_PLAY ), B_ENTER, "Enter" ) );
// menu->AddItem( playSelectMenu = new CQuickKeyMenuItem( "Play Selection", new BMessage( MENU_PLAY_SELECT ), B_ENTER, B_SHIFT_KEY ) );
	menu->AddItem( playSelectMenu = new CQuickKeyMenuItem( "Play Section", new BMessage( MENU_PLAY_SECTION ), 'p', "p" ) );
	menu->AddSeparatorItem();
	menu->AddItem( setSectionMenu = new BMenuItem( "Set Section", new BMessage( MENU_SET_SECTION ), 'S', B_SHIFT_KEY ) );
	menus->AddItem( menu );
	
		// Create the plug-ins menu
	menus->AddItem( plugInMenu );

		// Create the edit menu
	windowMenu = new BMenu( "Window" );
	windowMenu->AddItem( new BMenuItem( "New Window", new BMessage( MENU_NEW_WINDOW ), 'W', B_SHIFT_KEY ) );
	windowMenu->AddSeparatorItem();
	windowMenu->AddItem( inspectorMenu = new BMenuItem( "", new BMessage( MENU_INSPECTOR ), 'I' ) );
	windowMenu->AddItem( gridWindowMenu = new BMenuItem( "", new BMessage( MENU_GRIDWINDOW ), 'G' ) );
	windowMenu->AddItem( transportMenu = new BMenuItem( "", new BMessage( MENU_TRANSPORT ), 'T' ) );
	windowMenu->AddSeparatorItem();
	menus->AddItem( windowMenu );

		// Add the menus
	AddChild( menus );

		// view rect should be same size as window rect but with left top at (0, 0)
		// Adjust UI coords for menu bar size.
	frame.OffsetTo( B_ORIGIN );
	Lock();
	frame.top = menus->Frame().bottom + 1;
	frame.bottom	-= B_H_SCROLL_BAR_HEIGHT;
	Unlock();
	
	BPoint isize( 20.0, 16.0 );

	BPopUpMenu		*createMenu = new BPopUpMenu( "", false, false );
	createMenu->AddItem( new CImageAndTextMenuItem(	"Default",
													ResourceUtils::LoadImage("PencilTool"),
													isize,
													new BMessage( 3000 + EvtType_Count ) ) );
	createMenu->AddItem( new CImageAndTextMenuItem(	"Program Change",
													ResourceUtils::LoadImage("ProgramTool"),
													isize,
													new BMessage( 3000 + EvtType_ProgramChange ) ) );
	createMenu->AddItem( new CImageAndTextMenuItem(	"Nested Track",
													ResourceUtils::LoadImage("TrackTool"),
													isize,
													new BMessage( 3000 + EvtType_Sequence ) ) );
	createMenu->AddItem( new CImageAndTextMenuItem(	"Repeat",
													ResourceUtils::LoadImage("RepeatTool"),
													isize,
													new BMessage( 3000 + EvtType_Repeat ) ) );
	createMenu->AddItem( new CImageAndTextMenuItem(	"Time Signature",
													ResourceUtils::LoadImage("TimeSigTool"),
													isize,
													new BMessage( 3000 + EvtType_TimeSig ) ) );
	createMenu->AddItem( new CImageAndTextMenuItem(	"System Exclusive",
													ResourceUtils::LoadImage("SysExTool"),
													isize,
													new BMessage( 3000 + EvtType_SysEx ) ) );
	createMenu->AddItem( new CImageAndTextMenuItem(	"Track End",
													ResourceUtils::LoadImage("EndTool"),
													isize,
													new BMessage( 3000 + EvtType_End ) ) );
	createMenu->SetTargetForItems( (CDocWindow *)this );

		// Add the left tool area.
	topToolArea = new CBorderView(BRect(frame.left, frame.top,
										frame.right + 2, frame.top + ToolArea_Height),
								  "", B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW, 0,
								  CBorderView::BEVEL_BORDER);
	AddChild( topToolArea );
	topToolArea->SetViewColor( 220, 220, 220 );
	
	toolBar = new CToolBar(	BRect( 2.0, 2.0, Tool_Width, 26.0 ),
							new BMessage( ToolBar_ID ),
							BPoint( 24.0, 24.0 ),
							false,
							0,
							B_WILL_DRAW );
	toolBar->SetTarget( (CDocWindow *)this );

	toolBar->AddTool(TOOL_GRID, true, ResourceUtils::LoadImage("GridTool"));
	toolBar->AddSeperator();

	toolBar->AddTool(TOOL_SELECT, false, ResourceUtils::LoadImage("ArrowTool"));
	toolBar->ExcludeTool(TOOL_SELECT, 0);

	toolBar->AddTool(TOOL_CREATE, false, ResourceUtils::LoadImage("PencilTool"),
					 NULL, NULL, createMenu);
	toolBar->ExcludeTool(TOOL_CREATE, 0);

	toolBar->AddTool(TOOL_ERASE, false, ResourceUtils::LoadImage("EraserTool"));
	toolBar->ExcludeTool(TOOL_ERASE, 0);
	toolBar->EnableTool(TOOL_ERASE, false);

	toolBar->AddTool(TOOL_TEXT, false, ResourceUtils::LoadImage("TextTool"));
	toolBar->ExcludeTool(TOOL_TEXT, 0);
	toolBar->EnableTool(TOOL_TEXT, false);

	toolBar->AddTool(TOOL_TEXT + 5, false, ResourceUtils::LoadImage("TextTool"));
	toolBar->ExcludeTool(TOOL_TEXT + 5, 0);
	toolBar->EnableTool(TOOL_TEXT + 5, false);

	toolBar->Select(TOOL_GRID, true, false);
	toolBar->Select(TOOL_SELECT, true, false);

	topToolArea->AddChild(toolBar);
	toolBar->SetViewColor(B_TRANSPARENT_32_BIT);

	trackNameCtl = new BTextControl(
		BRect( 1.0, 28.0, Tool_Width - 2.0, 46.0 ),
		"TrackName", "Name:", Track()->Name(), new BMessage( TrackName_ID ),
		B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW );
	topToolArea->AddChild( trackNameCtl );
	trackNameCtl->SetTarget( (CDocWindow *)this );
	trackNameCtl->SetViewColor( 220, 220, 220 );
	trackNameCtl->SetDivider( 31.0 );
	trackNameCtl->TextView()->AddFilter(
		new BMessageFilter(	B_PROGRAMMED_DELIVERY,
							B_LOCAL_SOURCE,
							B_KEY_DOWN,
							DefocusTextFilterFunc ) );

	x = Tool_Width + 2;

	BBox		*box;
	
	box = new BBox( BRect( x, 2.0, x + 200.0 + 8.0, 47.0 ) );
	topToolArea->AddChild( box );
	
	durationEditor = new CTimeIntervalEditor( BRect( 4, 4.0, 202.0, 41.0 ),
											"DurationEdit",
											new BMessage( 'dura' ) );
	box->AddChild( durationEditor );
	newEventDuration = Ticks_Per_QtrNote;
	durationEditor->SetTarget( (CDocWindow *)this );

	frame.top += ToolArea_Height + 1;

	CreateFrames( frame, inTrack );
	BRect		scrollFrame( stripScroll->Frame() );
	stripScroll->ResizeTo( scrollFrame.Width() - 100.0, scrollFrame.Height() );
	stripScroll->MoveTo(   scrollFrame.left    + 100.0, scrollFrame.top );

/*
	x += 212.0;
		// Now, add the menu field for the type of event being added.
		
	BMenu		*typeMenu = new BPopUpMenu( "Note" );
	typeMenu->AddItem( new BMenuItem( "Note",					new BMessage( 3000 + EvtType_Note ) ) );
	typeMenu->AddItem( new BMenuItem( "Channel Aftertouch",	new BMessage( 3000 + EvtType_ChannelATouch ) ) );
	typeMenu->AddItem( new BMenuItem( "Polyphonic Aftertouch", new BMessage( 3000 + EvtType_PolyATouch ) ) );
	typeMenu->AddItem( new BMenuItem( "Control Change",		new BMessage( 3000 + EvtType_Controller ) ) );
	typeMenu->AddItem( new BMenuItem( "Program Change",		new BMessage( 3000 + EvtType_ProgramChange ) ) );
	typeMenu->AddItem( new BMenuItem( "Pitch Bend",			new BMessage( 3000 + EvtType_PitchBend ) ) );
// typeMenu->AddItem( new BMenuItem( "Tempo Change",			new BMessage( 3000 + EvtType_PolyATouch ) ) );
	typeMenu->AddItem( new BMenuItem( "Nested Track",			new BMessage( 3000 + EvtType_Sequence ) ) );
// typeMenu->AddItem( new BMenuItem( "Conditional Branch",	new BMessage( 3000 + EvtType_PolyATouch ) ) );
// typeMenu->AddItem( new BMenuItem( "Branch Label",			new BMessage( 3000 + EvtType_PolyATouch ) ) );
	typeMenu->AddItem( new BMenuItem( "Repeat",				new BMessage( 3000 + EvtType_Repeat ) ) );
	typeMenu->AddItem( new BMenuItem( "Time Signature",		new BMessage( 3000 + EvtType_TimeSig ) ) );
//	typeMenu->AddItem( new BMenuItem( "Stop Event",			new BMessage( 3000 + EvtType_Stop ) ) );
	typeMenu->AddItem( new BMenuItem( "User Event",			new BMessage( 3000 + EvtType_UserEvent ) ) );
	typeMenu->AddItem( new BMenuItem( "Track End",				new BMessage( 3000 + EvtType_End ) ) );
	typeMenu->SetTargetForItems( (CDocWindow *)this );

	BMenuField	*newType = new BMenuField(	BRect( x, 1.0, x + 190.0, 24.0 ),
											"EventType", "Event Type:",
											typeMenu,
											B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW );
	topToolArea->AddChild( newType );
	newType->SetDivider( 60.0 );
*/
	
		// Add the left tool area.
	scrollFrame.right = 100.0;

	rgb_color	border,
				fill;
	BView		*v;
				
	fill.red = fill.green = fill.blue = 128;
	fill.alpha = 128;

	v = new CBorderView(scrollFrame, "", B_FOLLOW_LEFT | B_FOLLOW_BOTTOM,
						B_WILL_DRAW, &fill);
	AddChild( v );

		// Add the time display string view
	timeView = new CTextDisplay(	BRect(	1,
										1,
										scrollFrame.right - 1,
										scrollFrame.Height()), "", false );
	v->SetViewColor( border );
	v->AddChild( timeView );
	v->SetViewColor( B_TRANSPARENT_32_BIT );
	timeView->SetAlignment( B_ALIGN_RIGHT );
	timeView->SetFont( be_fixed_font );
	timeView->SetFontSize( 12 );

		// Now, create some strips for test purposes
	CStripView		*sv;

	frame.top += Ruler_Height;
	sv = new CLinearEditor(	*this, *stripFrame, frame );
	stripFrame->AddChildView( sv->TopView(), 80 );

	sv = new CVelocityEditor(	*this, *stripFrame, frame );
	stripFrame->AddChildView( sv->TopView(), 15 );

	sv = new CPitchBendEditor(	*this, *stripFrame, frame );
	stripFrame->AddChildView( sv->TopView(), 15 );

	sv = new CTrackCtlStrip(	*this, *stripFrame, frame, inTrack );
	stripFrame->AddChildView( sv->TopView(), 15 );
}

bool CLinearWindow::QuitRequested()
{
	Track()->SetName( trackNameCtl->Text() );
	return CTrackWindow::QuitRequested();
}

void CLinearWindow::MessageReceived( BMessage* theMessage )
{
	CMeVApp			&app = *(CMeVApp *)be_app;

	if (theMessage->what >= 3000 && theMessage->what <= 3000 + EvtType_Count)
	{
		BBitmap		*image;
	
		newEventType = (enum E_EventType)(theMessage->what - 3000);
		
		switch (newEventType) {
		case EvtType_ProgramChange:	image = ResourceUtils::LoadImage("ProgramTool"); break;
		case EvtType_TimeSig:		image = ResourceUtils::LoadImage("TimeSigTool"); break;
		case EvtType_Sequence:		image = ResourceUtils::LoadImage("TrackTool"); break;
		case EvtType_Repeat:		image = ResourceUtils::LoadImage("RepeatTool"); break;
		case EvtType_End:			image = ResourceUtils::LoadImage("EndTool"); break;
		case EvtType_SysEx:			image = ResourceUtils::LoadImage("SysExTool"); break;
		default:					image = ResourceUtils::LoadImage("PencilTool"); break;
		}
		
		if (image) toolBar->SetToolImage( TOOL_CREATE, image, NULL, NULL );
		return;
	}

	switch(theMessage->what) {
	case ToolBar_ID:
		int32		group;
		int32		tool,
					state;
		
		theMessage->FindInt32( "tool", &tool );
		theMessage->FindInt32( "state", &state );
		if (theMessage->FindInt32( "group", &group ) == B_OK)
		{
			if (group >= 0 && group < (sizeof(toolStates)/sizeof(toolStates[0])))
				toolStates[ group ] = tool;
		}
		else if (tool == TOOL_GRID)
		{
			Track()->EnableGridSnap( state );
		}
		break;
		
	case TrackName_ID:
		Track()->SetName( trackNameCtl->Text() );
		break;
		
	case 'dura':
		// REM: Do we also want to change duration of selected events?
		((CMeVDoc &)Document()).SetDefaultAttribute( EvAttr_Duration, durationEditor->Value() );
		newEventDuration = durationEditor->Value();
		break;
		
	case MENU_UNDO:	Track()->Undo(); break;
	case MENU_REDO:	Track()->Redo(); break;

	case B_CUT:
		if (CurrentFocus()) DispatchMessage( theMessage, CurrentFocus() );
		else { /* REM: Add CUT code */ };
		break;

	case B_COPY:
		if (CurrentFocus()) DispatchMessage( theMessage, CurrentFocus() );
		else { /* REM: Add CUT code */ };
		break;

	case B_PASTE:
		if (CurrentFocus()) DispatchMessage( theMessage, CurrentFocus() );
		else { /* REM: Add CUT code */ };
		break;

	case MENU_CLEAR:
		Track()->DeleteSelection();
		Document().SetModified();
		break;

	case B_SELECT_ALL:
		if (CurrentFocus()) DispatchMessage( theMessage, CurrentFocus() );
		else Track()->SelectAll();
		break;
	
	case MENU_NEW_WINDOW:
		CLinearWindow *window;
	
		window = new CLinearWindow( BRect( 60, 60, 340, 300 ), (CMeVDoc &)document, Track() );
		window->Show();
		break;
		
	case MENU_PROGRAM_SETTINGS:
		((CMeVApp *)be_app)->ShowPrefs();
		break;

	case MENU_MIDI_CONFIG:
		((CMeVApp *)be_app)->ShowMidiConfig();
		break;

	case MENU_VIEW_SETTINGS:
		ShowPrefs();
		break;
		
	case MENU_VIRTUAL_CHANNELS:
		((CMeVDoc &)Document()).ShowWindow( CMeVDoc::VChannel_Window );
		break;
		
	case MENU_PLAY:
		if (CPlayerControl::IsPlaying( (CMeVDoc *)&document ))
		{
			CPlayerControl::StopSong( (CMeVDoc *)&document );
			break;
		}

			// Start playing a song.
//		CPlayerControl::PlaySong(	(CMeVDoc *)&document,
//								0, 0, LocateTarget_Real, -1,
//								SyncType_SongInternal, (app.GetLoopFlag() ? PB_Loop : 0) );
		CPlayerControl::PlaySong(	(CMeVDoc *)&document,
								Track()->GetID(), 0, LocateTarget_Real, -1,
								SyncType_SongInternal, (app.GetLoopFlag() ? PB_Loop : 0) );
		break;
		
	case MENU_PLAY_SECTION:
	
			// Start playing a song.
		CPlayerControl::PlaySong(
			(CMeVDoc *)&document,
			Track()->GetID(),
			Track()->SectionStart(), LocateTarget_Metered,
			Track()->SectionEnd() - Track()->SectionStart(),
			SyncType_SongInternal,
			(app.GetLoopFlag() ? PB_Loop : 0) | PB_Folded );
		break;
		
	case MENU_SET_SECTION:

		if (Track()->SelectionType() != CTrack::Select_None)
		{
			Track()->SetSection( Track()->MinSelectTime(), Track()->MaxSelectTime() );
			Track()->NotifyUpdate( CTrack::Update_Section, NULL );
		}
		break;

// case MENU_PAUSE:
// 	break;
		
		case Select_ID: {
			bool		active = false;
			theMessage->FindBool( "active", 0, &active );
			UpdateActiveSelection( active );
			break;
		}		
		default: {
			CTrackWindow::MessageReceived( theMessage );
			break;
		}
	}
}

void CLinearWindow::DisplayMouseTime( CTrack *track, int32 time )
{
	long				majorUnit,
					minorUnit,
					extraTime;

	if (track == NULL)
	{
		timeBuf[ 0 ] = '\0';
	}
	else
	{
		track->SigMap().DecomposeTime( time, majorUnit, minorUnit, extraTime );
	
		if (track->ClockType() == ClockType_Real)
		{
			int32		hours = majorUnit / 60;
			int32		minutes = majorUnit - (hours * 60);
		
			sprintf( timeBuf, "%2ld:%2.2ld:%2.2ld.%2.2ld", hours, minutes, minorUnit, extraTime );
		}
		else
		{
			sprintf( timeBuf, "%4ld.%2.2ld.%4.4ld", majorUnit + 1, minorUnit, extraTime );
		}
	}
	
	timeView->SetText( timeBuf );
}

void CLinearWindow::MenusBeginning()
{
	CTrackWindow::MenusBeginning();
	setSectionMenu->SetEnabled( Track()->SelectionType() != CTrack::Select_None );
}

void CLinearWindow::OnUpdate( BMessage *inMsg )
{
	int32		trackHint;
	int32		trackID;

	if (inMsg->FindInt32( "TrackID", 0, &trackID ) != B_OK)		trackID = -1;
	if (inMsg->FindInt32( "TrackAttrs", 0, &trackHint ) != B_OK)	trackHint = 0;
	
	if (trackHint & CTrack::Update_Name)
	{
		trackNameCtl->SetText( Track()->Name() );
	}
}

