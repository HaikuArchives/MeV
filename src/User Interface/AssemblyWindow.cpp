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
// A palette of draggable items...

/*class CDragPalette : public BControl {

	BList			icons;

public:
	CDragPalette(	BRect		frame,
				const char	*name,
				BMessage		*msg,
				uint32		resizingMode,
				uint32		flags )
		: BControl( frame, name, NULL, msg, resizingMode, flags )
	{
	}
      	
	void AttachedToWindow()
	{
		BControl::AttachedToWindow();
		SetViewColor( B_TRANSPARENT_32_BIT );
	}
	
	void AddIcon( BBitmap *inMap )
	{
		icons.AddItem( inMap );
	}

	void Draw( BRect inUpdateRect )
	{
		BRect		r( Bounds() );
		int			count = icons.CountItems();
		
		SetHighColor( 0xe8, 0xe8, 0xe8 );
		FillRect( r );
		SetHighColor( 0x80, 0x80, 0x80 );
		StrokeRect( r );
		SetDrawingMode( B_OP_OVER );
		
		if (count > 0)
		{
			float		width = ((r.Width() - 1) / count) - 1;
			float		x = r.left + 1;

			for (int i = 0; i < count; i++)
			{
				BBitmap	*bm = (BBitmap *)icons.ItemAt( i );
				BRect	br( bm->Bounds() );
	
				if (i > 0)
				{
					StrokeLine( BPoint( x, r.top ), BPoint( x, r.bottom ) );
					x += 1;
				}

				DrawBitmapAsync( bm, BPoint(	x + (width - br.Width())/2,
											(r.top + r.bottom - br.Height())/2 ) );
											
				x += width;
			}
		}
	}
	
	void MouseDown( BPoint where )
	{
		int			count = icons.CountItems();
		BRect		r( Bounds() );
		
		if (count > 0)
		{
			float		width = ((r.Width() - 1) / count) - 1;
//			float		x = r.left + 1;
			
			int	index = static_cast<int>(floor((where.x - r.left) / width));
			
			if (index >= 0 && index < count)
			{
				SetValue( index );
				Invoke();
			}
		}
	}
};*/

// ---------------------------------------------------------------------------
// Track list view items

class CTrackListItem : public CMultiColumnListItem {

public:
	CTrackListItem( CTrack *inTrack )
		: CMultiColumnListItem( inTrack )
	{
	}
	
	int32 GetFieldIntData( int32 inIndex )
	{
		CTrack	*tk = (CTrack *)rowData;

		if (inIndex == 2) return tk->Muted();
		if (inIndex == 3) return tk->Solo();
		return 0;
	}

	char *GetFieldStringData( int32 inIndex )
	{
		CTrack	*tk = (CTrack *)rowData;
	
		if (inIndex == 0) return "Midi";
		else if (inIndex == 1) return (char *)tk->Name();
		return NULL;
	}

	void *GetFieldData( int32 inIndex )
	{
//		CTrack	*tk = (CTrack *)rowData;
		return NULL;
	}
};

class CTrackListView : public CMultiColumnListView {
	CMeVDoc				*doc;

public:
		/**	Constructor */
	CTrackListView(
		CMeVDoc			*inDoc,
		BRect			frame,
		const char		*name,
		list_view_type	type = B_SINGLE_SELECTION_LIST,
		uint32			resizeMask = B_FOLLOW_LEFT | B_FOLLOW_TOP,
		uint32			flags = B_WILL_DRAW | B_FRAME_EVENTS )
		: CMultiColumnListView( frame, name, type, resizeMask, flags )
	{
		doc = inDoc;
	}
	
	virtual bool IsDragAcceptable( const BMessage *dragMsg )
	{
		int32			msgType;
		int32			row;
		void				*dragDoc;

		if (		dragMsg != NULL
			&&	dragMsg->what == MeVDragMsg_ID
			&&	dragMsg->FindInt32( "Type", 0, &msgType ) == B_OK
			&&	dragMsg->FindInt32( "Index", 0, &row ) == B_OK
			&&	dragMsg->FindPointer( "Document", 0, &dragDoc ) == B_OK
			&&	msgType == DragTrack_ID
			&&	dragDoc == doc )
		{
			return true;
		}
		return false;
	}
	
	virtual void OnDrop( BMessage *dragMsg, int32 index )
	{
		int32			msgType;
		int32			row;
		void				*dragDoc;

		if (		dragMsg != NULL
			&&	dragMsg->what == MeVDragMsg_ID
			&&	dragMsg->FindInt32( "Type", 0, &msgType ) == B_OK
			&&	dragMsg->FindInt32( "Index", 0, &row ) == B_OK
			&&	dragMsg->FindPointer( "Document", 0, &dragDoc ) == B_OK
			&&	msgType == DragTrack_ID
			&&	dragDoc == doc )
		{
			if (index == row || index == row + 1) return;

			if (index > row) index--;
			
			doc->ChangeTrackOrder( row, index );
			Select( index );
		}
	}
};

// ---------------------------------------------------------------------------
// Constructor/Destructor

CAssemblyWindow::CAssemblyWindow(
	BRect frame,
	CMeVDoc &document )
	:	CTrackWindow(frame, document, (CEventTrack *)document.FindTrack(1))
{
	BRect rect(Bounds());

	AddMenuBar();
	AddToolBar();

	rect.top = toolBar->Frame().bottom + 1.0;
	rect.right = 190;

	// Add the track list.
	trackList = new CTrackListView(&document, rect, "",
								   B_SINGLE_SELECTION_LIST,
								   B_FOLLOW_LEFT | B_FOLLOW_TOP_BOTTOM);
	trackList->SetInvocationMessage(new BMessage(TrackEdit_ID));
	trackList->SetColumnClickMessage(new BMessage('tClk'));
	trackList->SetSelectionMessage(new BMessage('tSel'));
	trackList->StayFocused(false);
	typeColumn = new CStringColumnField(*trackList, 28,  0, "Type");
	nameColumn = new CStringColumnField(*trackList, 10, 1, "Name");
	nameColumn->SetAlignment(B_ALIGN_LEFT);
	nameColumn->SetDraggable(true);
	new CCheckmarkColumnField (*trackList, 12, 0, "M");
	new CCheckmarkColumnField (*trackList, 12, 0, "S");
	new CCheckmarkColumnField (*trackList, 12, 0, "R");
	AddChild(trackList);
	BuildTrackList();

	rect.left = rect.right + CSplitter::V_SPLITTER_WIDTH;
	rect.right = Bounds().right;
	CreateFrames(rect, (CTrack *)document.FindTrack(1));

	// Now, create some strips for test purposes
	CStripView *sv;
	CAssemblyRulerView *realRuler;
	CScrollerTarget	*oldRuler = stripFrame->Ruler();

	sv = new CTrackCtlStrip(*this, *stripFrame, rect,
							(CEventTrack *)document.FindTrack(1), "Metrical");
	stripFrame->AddChildView(sv->TopView(), 50);

	realRuler = new CAssemblyRulerView(
		*this,
		stripFrame,
		(CEventTrack *)document.FindTrack( (int32)0 ),
		BRect(	0.0, 0.0, rect.Width() - 14, Ruler_Height - 1 ),
		(char *)NULL,
		B_FOLLOW_LEFT_RIGHT,
		B_WILL_DRAW );
	realRuler->ShowMarkers( false );
	stripFrame->SetRuler( oldRuler );

	// Add splitter
	rect.left = trackList->Frame().right;
	rect.right = rect.left + CSplitter::V_SPLITTER_WIDTH;
	CSplitter *splitter = new CSplitter(rect, trackList, stripFrame,
										B_VERTICAL, B_FOLLOW_LEFT | B_FOLLOW_TOP_BOTTOM);
	AddChild(splitter);

	newEventType = EvtType_Sequence;

		// Adjust UI coords for menu bar size.
/*	frame.OffsetTo( B_ORIGIN );
	Lock();
	frame.top = menus->Frame().bottom + 1;
	Unlock();

		// Add the left tool area.
	leftToolArea = new CBorderView(BRect(0.0, frame.top, 
										 ToolArea_Width - CSplitter::V_SPLITTER_WIDTH,
										 frame.bottom + 2 ),
								   "", B_FOLLOW_LEFT | B_FOLLOW_TOP_BOTTOM, B_WILL_DRAW,
								   0, CBorderView::BEVEL_BORDER);
	AddChild(leftToolArea);
	leftToolArea->SetViewColor(220, 220, 220, 255);
	

	yPos += 3;


	leftToolArea->AddChild(toolBar);

	// add event palette
	eventPalette = new CDragPalette(	BRect( 	84.0, yPos, ToolArea_Width - 5.0, yPos + 24.0 ),
								"DraggableEvents",
								new BMessage( 'evdr' ),
								B_FOLLOW_LEFT | B_FOLLOW_BOTTOM,
								B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE );
	eventPalette->AddIcon(m_metroToolBitmap);
	eventPalette->AddIcon(m_programToolBitmap);
	eventPalette->AddIcon(m_timeSigToolBitmap);
	eventPalette->AddIcon(m_textToolBitmap);
	leftToolArea->AddChild(eventPalette);
	yPos += 28.0;

//	BStringView *stv;

	stv = new BStringView( BRect( 4.0, yPos, 80, yPos + 16.0 ), "", "Event Count:", B_FOLLOW_LEFT | B_FOLLOW_BOTTOM );
	stv->SetAlignment( B_ALIGN_RIGHT );
	leftToolArea->AddChild( stv );
	eventCount = new CTextDisplay(	BRect( 84.0, yPos, ToolArea_Width - 5.0, yPos + 16.0 ),
								"Event Count", true, B_FOLLOW_LEFT | B_FOLLOW_BOTTOM );
	eventCount->SetAlignment( B_ALIGN_LEFT );
	leftToolArea->AddChild( eventCount );

	yPos += 20;
	stv = new BStringView( BRect( 4.0, yPos, 80, yPos + 16.0 ), "", "Channels Used:", B_FOLLOW_LEFT | B_FOLLOW_BOTTOM );
	stv->SetAlignment( B_ALIGN_RIGHT );
	leftToolArea->AddChild( stv );
	channelsUsed = new CTextDisplay(	BRect( 84.0, yPos, ToolArea_Width - 5.0, yPos + 16.0 ),
									"Channels Used", true, B_FOLLOW_LEFT | B_FOLLOW_BOTTOM );
	channelsUsed->SetAlignment( B_ALIGN_LEFT );
	leftToolArea->AddChild( channelsUsed );
	
	yPos += 20;
	
	BBox *bb;
	bb = new BBox( BRect( -2.0, yPos, ToolArea_Width + 4.0, yPos + 1.0 ), NULL, B_FOLLOW_LEFT | B_FOLLOW_BOTTOM, B_WILL_DRAW );
	leftToolArea->AddChild( bb );
	yPos += 3;
	
	BMenu		*clockMenu = new BPopUpMenu( "Musical Time" );
	clockMenu->AddItem( new BMenuItem( "Musical Time",	new BMessage( 'cMet' ) ) );
	clockMenu->AddItem( new BMenuItem( "Real Time",		new BMessage( 'cRea' ) ) );
	BMenuField	*clockButton = new BMenuField(	BRect( 10.0, yPos, ToolArea_Width - 5.0, yPos + 14.0 ),
											"ViewAs", "View time as:",
											clockMenu,
											B_FOLLOW_LEFT | B_FOLLOW_BOTTOM, B_WILL_DRAW );
	leftToolArea->AddChild( clockButton );
	clockButton->SetDivider( 72.0 );


	frame.bottom		-= B_H_SCROLL_BAR_HEIGHT;
	frame.left		+= ToolArea_Width + 1.0 + CSplitter::V_SPLITTER_WIDTH;


	CEventEditor *ee;
	ee = new CTrackCtlStrip(*this, *stripFrame, frame, 
							(CEventTrack *)document.FindTrack((int32)0), "Real");
	ee->SetRuler( realRuler );
	stripFrame->AddChildView( ee->TopView(), 8 );
*/
}

CAssemblyWindow::~CAssemblyWindow()
{
	DeleteListItems(trackList);
}

// ---------------------------------------------------------------------------
// CTrackWindow Implementation

void
CAssemblyWindow::MessageReceived(
	BMessage* message)
{
	CMeVDoc			&doc = (CMeVDoc &)Document();
	CMeVApp			&app = *(CMeVApp *)be_app;
	CTrackListItem		*tli;
	int32			sel;

	switch (message->what)
	{
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
			toolStates[0] = message->what;
			break;
		}
		case TrackEdit_ID:
		{
			sel = trackList->CurrentSelection();
			if (sel >= 0)
			{
				tli = (CTrackListItem *)trackList->ItemAt( sel );
				if (tli != NULL)
				{
					CLinearWindow *window;
					window = new CLinearWindow(	UScreenUtils::StackOnScreen( 540, 370 ),
												doc,
												(CEventTrack *)tli->RowData() );
					window->Show();
				}
			}
			break;
		}		
		case 'tSel':
		{
			ShowTrackInfo();
			break;
		}
		case 'tClk':
		{
			int32		col, row;
			CTrack		*tk;

			ShowTrackInfo();

			if (message->FindInt32( "cell", 0, &row ) != B_OK) break;
			if (message->FindInt32( "cell", 1, &col ) != B_OK) break;

			tli = (CTrackListItem *)trackList->ItemAt( row );
			if (row < 0 || tli == NULL) break;

			tk = (CTrack *)tli->RowData();

			if (col == 1)
			{
				BRect			r, r2;
				BMessage			dragMsg( MeVDragMsg_ID );
				r = trackList->ItemFrame( row );
				BPoint			m;
				uint32			b;
				const char		*str = tk->Name();
				
				dragMsg.AddInt32( "Type", DragTrack_ID );
				dragMsg.AddInt32( "TrackID", tk->GetID() );
				dragMsg.AddInt32( "Index", row );
				dragMsg.AddPointer( "Document", &doc );
	
				r.left += typeColumn->Width() + 1;
				r.right = r.left + nameColumn->Width();
				
				trackList->GetMouse( &m, &b );
				
				r2 = r;
				r2.OffsetTo( B_ORIGIN );
	
				BBitmap			*bMap;			// Drag and drop bitmap
				BView			*view;
			
				bMap = new BBitmap( r2, B_COLOR_8_BIT, true, true );
				view = new BView( r2, NULL, 0, 0 );
				bMap->AddChild( view );
				bMap->Lock();
	
				view->SetHighColor( 128, 128, 255 );
				view->FillRect( r2 );	
				view->SetHighColor( 192, 192, 255 );
				view->FillRect( BRect( r2.left, r2.top, r2.right - 1, r2.top ) );	
				view->FillRect( BRect( r2.left, r2.top, r2.left, r2.bottom - 1 ) );
	
				view->SetHighColor( 64, 64, 192 );
				view->FillRect( BRect( r2.right, r2.top + 1, r2.right, r2.bottom ) );	
				view->FillRect( BRect( r2.left + 1, r2.bottom, r2.right, r2.bottom ) );
	
				view->SetHighColor( 0, 0, 0 );
				view->SetLowColor( 128, 128, 255 );
	// 		int32 y = (r2.top + r2.bottom - list.fh.descent + list.fh.ascent) / 2;
				view->DrawString( str, BPoint( r2.left + 3, 10 ) );
	
				bMap->Unlock();
				bMap->RemoveChild( view );
				delete view;
				
				trackList->DragMessage( &dragMsg, bMap, BPoint( m.x - r.left, m.y - r.top ), NULL );
			}
			else if (col == 2)
			{
					// Toggle mute option
				tk->SetMuted( !tk->Muted() );
			}
			
			break;
		}		
/*	case 'evdr':			// drag a new event
	{
		int32		eventType;
		BBitmap		*bitmap;			// Drag and drop bitmap
		int32 index = eventPalette->Value();
		
		switch (index) {
			case 0:
			{
				bitmap = ResourceUtils::LoadImage("MetroTool");
				eventType = EvtType_Tempo;
				break;
			}
			case 1:
			{
				bitmap = ResourceUtils::LoadImage("ProgramTool");
				eventType = EvtType_ProgramChange;
				break;
			}
			case 2:
			{
				bitmap = ResourceUtils::LoadImage("TimeSigTool");
				eventType = EvtType_TimeSig;
				break;
			}
			default:
			{
				bitmap = ResourceUtils::LoadImage("TextTool");
				eventType = EvtType_Text;
				break;
			}
		}

		if (bitmap != NULL)
		{
			BRect br(bitmap->Bounds() );
			BPoint hSpot((br.left + br.right) / 2, (br.top + br.bottom) / 2);
			BMessage dragMsg(MeVDragMsg_ID);
//			BBitmap *bCopy = new BBitmap(bitmap);
			dragMsg.AddInt32("Type", DragEvent_ID);
			dragMsg.AddInt32("EventType", eventType);
			dragMsg.AddPointer("Document", &doc);
			eventPalette->DragMessage(&dragMsg, bitmap, hSpot, NULL);
		}
		break;
	}*/
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
		CAssemblyWindow *window;
	
		window = new CAssemblyWindow( UScreenUtils::StackOnScreen( 540, 300 ), doc );
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
		doc.ShowWindow( CMeVDoc::VChannel_Window );
		break;
		
	case MENU_CREATE_METERED_TRACK:
			// REM: How to undo?
	
		doc.NewTrack( TrackType_Event, ClockType_Metered );
		doc.NotifyUpdate( CMeVDoc::Update_AddTrack, NULL );
		doc.SetModified();
		break;

	case MENU_CREATE_REAL_TRACK:
			// REM: How to undo?
	
		doc.NewTrack( TrackType_Event, ClockType_Real );
		doc.NotifyUpdate( CMeVDoc::Update_AddTrack, NULL );
		doc.SetModified();
		break;

	case MENU_DELETE_TRACK:
		
		if ((sel = trackList->CurrentSelection()) >= 0)
		{
			CTrackListItem	*tli = (CTrackListItem *)trackList->ItemAt( sel );
			CTrack			*tk = (CTrack *)tli->RowData();
			CTrackDeleteUndoAction	*undoAction;
			
			undoAction = new CTrackDeleteUndoAction( tk );
			Track()->AddUndoAction( undoAction );
		}
		break;

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
		case Select_ID:
		{
			bool		active = false;	
			message->FindBool( "active", 0, &active );
			UpdateActiveSelection( active );
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
	int32		trackHint,
				docHint;
	int32		trackID;
	
	CTrackWindow::OnUpdate( inMsg );

	if (inMsg->FindInt32( "TrackID", 0, &trackID ) != B_OK)		trackID = -1;
	if (inMsg->FindInt32( "TrackAttrs", 0, &trackHint ) != B_OK)	trackHint = 0;
	if (inMsg->FindInt32( "DocAttrs", 0, &docHint ) != B_OK)		docHint = 0;

	if (trackHint != 0 || docHint != 0)
	{
		if (trackID >= 0)
		{
			for (int i = 0; i < trackList->CountItems(); i++)
			{
				CTrackListItem	*tli = (CTrackListItem *)trackList->ItemAt( i );
				
				if (((CTrack *)tli->RowData())->GetID() == trackID)
				{
					if (trackHint & (CTrack::Update_Name | CTrack::Update_Flags))
						trackList->InvalidateItem( i );
					
					if (trackHint & (CTrack::Update_Summary))
					{
						if (i == trackList->CurrentSelection()) ShowTrackInfo();
					}
				}
			}
		}
		
//		if (docHint & CMeVDoc::Update_TempoMap)
//			stripFrame->Invalidate();
		
		if (docHint & (CMeVDoc::Update_AddTrack|CMeVDoc::Update_DelTrack|CMeVDoc::Update_TrackOrder))
		{
			BuildTrackList();
		}
	}
}


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
	BMenu *subMenu = new BMenu("Create");
	subMenu->AddItem(new BMenuItem("Metered Track", new BMessage(MENU_CREATE_METERED_TRACK), 'N'));
	subMenu->AddItem(new BMenuItem("Real Track", new BMessage(MENU_CREATE_REAL_TRACK)));
	menu->AddItem(subMenu);
	menu->AddItem(new BMenuItem("Delete Track", new BMessage(MENU_DELETE_TRACK)));
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
	windowMenu->AddItem(new BMenuItem("Show Event Inspector", new BMessage(MENU_INSPECTOR), 'I'));
	windowMenu->AddItem(new BMenuItem("Show Grid Window", new BMessage(MENU_GRIDWINDOW), 'G'));
	windowMenu->AddItem(new BMenuItem("Show Transport Controls", new BMessage(MENU_TRANSPORT), 'T'));
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
	{
		rect.top = menus->Frame().bottom + 1.0;
	}
	// add the tool bar
	toolBar = new CToolBar(rect, "General");
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
	toolBar->AddTool(tool = new CBitmapTool("Eraser",
											ResourceUtils::LoadImage("EraserTool"),
											new BMessage(TOOL_ERASE)));
	tool->SetEnabled(false);
	toolBar->MakeRadioGroup("Select", "Eraser", true);
	
	AddChild(toolBar);
}

void
CAssemblyWindow::BuildTrackList()
{
	int32		ct = trackList->CountItems(),
				i;
	int32		sel = trackList->CurrentSelection();
	
	trackList->DeselectAll();
	
	for (i = 0; i < ct; i++)
	{
		CTrackListItem	*tli = (CTrackListItem *)trackList->ItemAt( i );
		
		delete tli;
	}
	trackList->MakeEmpty();

	int32		nTracks = Document().CountTracks();
	
	for (i = 0; i < nTracks; i++)
	{
		CTrack	*tk = Document().TrackAt( i );
		
		if (!tk->Deleted())
		{
			trackList->AddItem( new CTrackListItem( tk ) );
		}
	}
	trackList->Select( sel );
	trackList->Invalidate();
}

void
CAssemblyWindow::ShowTrackInfo()
{
/*	int32			sel = trackList->CurrentSelection();
	CTrackListItem		*tli;
	CTrack			*tk;
	CEventTrack		*eTrk;

	if (sel >= 0)
	{
		tli = (CTrackListItem *)trackList->ItemAt( sel );
		if (tli != NULL)
		{
			tk = (CTrack *)tli->RowData();
			
			if ((eTrk = dynamic_cast<CEventTrack *>(tk)))
			{
				char			text[ 64 ];
				int			length = 0,
							run = 0;
				
				sprintf( text, "%ld", eTrk->CountItems() );
				eventCount->SetText( text );
				
				strcpy( text, "None" );
				
				for (int i = 0; i < Max_VChannels + 1; i++)
				{
					if (eTrk->IsChannelUsed( i ) || i >= Max_VChannels)
					{
						run++;
					}
					else if (run > 0)
					{
						if (length > 0)		text[ length++ ] = ',';

						if (run == 1)		length += sprintf( &text[ length ], "%d", i );
						else if (run == 2)	length += sprintf( &text[ length ], "%d,%d", i - 1, i );
						else				length += sprintf( &text[ length ], "%d-%d", i - run + 1, i );

						run = 0;
	
						if (length > 50)
						{
							sprintf( text, "Many..." );
							break;
						}
					}
				}

				channelsUsed->SetText( text );
				return;
			}
		}
	}
	
	eventCount->SetText( "" );
	channelsUsed->SetText( "" );
*/
}
