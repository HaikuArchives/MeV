/* ===================================================================== *
 * VChannelWindow.cpp (MeV/User Interface)
 * ===================================================================== */

#include "VChannelWindow.h"
#include "MultiColumnListView.h"
#include "TextSlider.h"
#include "Idents.h"
#include "MeVFileID.h"
#include "MeVApp.h"
#include "PlayerControl.h"
#include "MidiDeviceInfo.h"
#include "BeFileWriter.h"
#include "BeFileReader.h"
#include "IFFWriter.h"
#include "IFFReader.h"

// Gnu C Library
#include <stdio.h>
// Interface Kit
#include <Button.h>
#include <ColorControl.h>
#include <StringView.h>
// Storage Kit
#include <NodeInfo.h>

class CVCRefFilter : public BRefFilter {
	bool Filter( const entry_ref *ref, BNode *node, struct stat *st, const char *filetype )
	{
		return (strstr( filetype, "MeV" ) != NULL || node->IsDirectory());
	}
};

class CVChannelListItem : public CMultiColumnListItem {
	int32				rowIndex;
	char					nameBuf[ 48 ];

public:
	CVChannelListItem( VChannelEntry &inEntry, int32 inIndex )
		: CMultiColumnListItem( &inEntry )
	{
		rowIndex = inIndex;
	}
	
	int32 GetFieldIntData( int32 inIndex )
	{
		VChannelEntry	*vc = (VChannelEntry *)rowData;
	
		switch (inIndex) {
		case 0:	return rowIndex + 1;
		case 2: return vc->port + 1;
		case 3: return vc->channel;
		case 5: return (vc->flags & VChannelEntry::transposable) ? 1 : 0;
		case 6: return (vc->flags & VChannelEntry::mute) ? 1 : 0;
		default:
			return 0;
		}
	}

	char *GetFieldStringData( int32 inIndex )
	{
		VChannelEntry	*vc = (VChannelEntry *)rowData;

		if (inIndex == 1)
		{
			MIDIDeviceInfo	*mdi = ((CMeVApp *)be_app)->LookupInstrument( vc->port, vc->channel - 1 );
			char				*name;
			
			if (mdi != NULL)	name = mdi->name;
			else
			{
				name = CPlayerControl::PortName( vc->port );
				if (name == NULL) return "--";
			}
			
			sprintf( nameBuf, "%s, channel %d", name, vc->channel );
			return nameBuf;
		}
		return NULL;
	}

	void *GetFieldData( int32 inIndex )
	{
		VChannelEntry	*vc = (VChannelEntry *)rowData;
	
		if (inIndex == 4) return &vc->fillColor;
		return NULL;
	}
};

	// REM: Add a menu bar to load, save, cut, copy, etc... virtual channels.
	// REM: Give window a minimum size.

CVChannelWindow::CVChannelWindow( CWindowState &inState, CMeVDoc &inDocument )
	: CDocWindow( inState, inDocument, "Virtual Channels", B_TITLED_WINDOW, B_NOT_H_RESIZABLE ),
	  messenger( this )
{
	BRect			r( inState.Rect() );
	int32			x, y;
	BButton			*bb;
	BMenu			*menu;
	
	filter = new CVCRefFilter;

	memcpy( saveTable, &inDocument.GetVChannel( 0 ), sizeof saveTable );
	
	SetSizeLimits( r.Width(), r.Width(), 100.0, 5000.0 );
	
		// view rect should be same size as window rect but with left top at (0, 0)

	menus = new BMenuBar( BRect( 0,0,0,0 ), NULL );

		// Create the file menu
	menu = new BMenu( "File" );
	menu->AddSeparatorItem();
	menu->AddItem( new BMenuItem( "Load Virtual Channels...", new BMessage( MENU_OPEN ), 'L' ) );
	menu->AddItem( new BMenuItem( "Save Virtual Channels...", new BMessage( MENU_SAVE ), 'S' ) );
	menu->AddItem( new BMenuItem( "Save As Default", new BMessage( 'sdef' ) ) );
	menu->AddSeparatorItem();
	menu->AddItem( new BMenuItem( "Close Window", new BMessage( B_QUIT_REQUESTED ), 'W' ) );
	menus->AddItem( menu );

		// Create the edit menu
	menu = new BMenu( "Edit" );
	menu->AddItem( new BMenuItem( "Copy", new BMessage( B_COPY ), 'C' ) );
	menu->AddItem( new BMenuItem( "Paste", new BMessage( B_COPY ), 'V' ) );
	menus->AddItem( menu );

		// REM: Create the windows menu here (to be able to re-open track windows...)

		// Add the menus
	AddChild( menus );

	r.OffsetTo( B_ORIGIN );

	BView		*bv = new BView( r, NULL, B_FOLLOW_ALL_SIDES, B_WILL_DRAW );
	
	AddChild( bv );
	bv->SetViewColor( 220, 220, 220 );
	
	channelList = new CMultiColumnListView(
		BRect( r.left + 7, r.top + 24, r.right - 7, r.bottom - 160 ),
		NULL, B_SINGLE_SELECTION_LIST, B_FOLLOW_ALL );

	CColumnField *cf;
	
	new CNumericColumnField		( *channelList, 24,  0, "#" );
	cf = new CStringColumnField	( *channelList, 10, 1, "Name" );
	cf->SetAlignment( B_ALIGN_LEFT );
	new CNumericColumnField		( *channelList, 36, 0, "Port" );
	new CNumericColumnField		( *channelList, 50, 0, "Channel" );
	new CColorSwatchColumnField	( *channelList, 50, 0, "Color" );
	new CCheckmarkColumnField		( *channelList, 60, 0, "Transpose" );
	new CCheckmarkColumnField		( *channelList, 40, 0, "Mute" );
	
	channelList->SetSelectionMessage( new BMessage( SelectChannel_ID ) );
	channelList->SetSelectionMessage( new BMessage( SelectChannel_ID ) );
	channelList->SetColumnClickMessage( new BMessage( ClickCheckbox_ID ) );
	
	bv->AddChild( channelList );

	for (int i = 0; i < Max_VChannels; i++)
	{
		channelList->AddItem( new CVChannelListItem( inDocument.GetVChannel( i ), i ) );
	}
	
	x = r.right - 14;
	y = r.bottom - 35;

	x -= 90;
	bb = new BButton(	BRect( x, y, x + 90, y + 25 ),
						"Close", "Close",
						new BMessage( Close_ID ),
						B_FOLLOW_RIGHT | B_FOLLOW_BOTTOM );
	bv->AddChild( bb );
	bb->MakeDefault( true );

	x -= 90;
	bb = new BButton(	BRect( x, y, x + 80, y + 25 ),
						"Revert", "Revert",
						new BMessage( Revert_ID ),
						B_FOLLOW_RIGHT | B_FOLLOW_BOTTOM );
	bv->AddChild( bb );
	
	channelColor = new BColorControl(	BPoint( 7, r.bottom - 152 ),
									B_CELLS_16x16, 3,
									"color", new BMessage( Color_ID ) );
	channelColor->SetResizingMode( B_FOLLOW_LEFT | B_FOLLOW_BOTTOM );
	bv->AddChild( channelColor );

	y = r.bottom - 152;

	BStringView	*sv;
	sv = new BStringView( BRect( 200, y, 250, y + 13 ), "", "Port:", B_FOLLOW_BOTTOM );
	sv->SetAlignment( B_ALIGN_RIGHT );
	bv->AddChild( sv );
	
	portSlider = new CTextSlider( BRect( 250, y, r.right - 8, y + 13 ), new BMessage( PortSlider_ID ), NULL, B_FOLLOW_BOTTOM );
	bv->AddChild( portSlider );
//	portSlider->SetTarget( (BLooper *)this, NULL );
	portSlider->SetRange( 1, 16 );
	y += 20;

	sv = new BStringView( BRect( 200, y, 250, y + 13 ), "", "Channel:", B_FOLLOW_BOTTOM );
	sv->SetAlignment( B_ALIGN_RIGHT );
	bv->AddChild( sv );

	channelSlider = new CTextSlider( BRect( 250, y, r.right - 8, y + 13 ), new BMessage( ChannelSlider_ID ), NULL, B_FOLLOW_BOTTOM );
	bv->AddChild( channelSlider );
//	channelSlider->SetTarget( (BLooper *)this, NULL );
	channelSlider->SetRange( 1, 16 );
	y += 20;
}

void CVChannelWindow::MessageReceived( BMessage *msg )
{
	int32			selChan = channelList->CurrentSelection();
	VChannelEntry		&vc = ((CMeVDoc *)Document())->GetVChannel( selChan );
	CMeVApp			*app = (CMeVApp *)be_app;
	CMeVDoc			*doc = (CMeVDoc *)Document();
	int32			col,
					row;
	BFilePanel		*fp;
	char				*name;
	entry_ref			eref;
	bool				doHint = false;

	switch (msg->what) {
	case Revert_ID:
		memcpy( &doc->GetVChannel( 0 ), saveTable, sizeof saveTable );
		channelList->Invalidate();
		doHint = true;
		// Fall thru

	case SelectChannel_ID:
		portSlider->SetValue( vc.port + 1 );
		channelSlider->SetValue( vc.channel );
		channelColor->SetValue( vc.fillColor );	
		break;
		
	case Close_ID:
		PostMessage( B_QUIT_REQUESTED );
		break;
	
	case Color_ID:
		vc.fillColor = channelColor->ValueAsColor();
		CalcHighlightColor( vc.fillColor, vc.highlightColor );

		channelList->InvalidateItem( selChan );
		doHint = true;
		break;
		
	case ClickCheckbox_ID:
					
		msg->FindInt32( "cell", 0, &row );
		msg->FindInt32( "cell", 1, &col );
		
		VChannelEntry	*vc2;
		
		vc2 = &doc->GetVChannel( row );
		if (col == 5)
		{
			vc2->flags ^= VChannelEntry::transposable;
			channelList->InvalidateItem( row );
		}
		else if (col == 6)
		{
			vc2->flags ^= VChannelEntry::mute;
			channelList->InvalidateItem( row );
		}
		break;
		
	case PortSlider_ID:
		vc.port = portSlider->Value() - 1;
		channelList->InvalidateItem( selChan );
		break;

	case ChannelSlider_ID:
		vc.channel = channelSlider->Value();
		channelList->InvalidateItem( selChan );
		break;
		
	case 'sdef':
//		app->SetDefaultVCTable( doc->GetVChannel( 0 ) );
		break;
		
	case MENU_OPEN:
		if ((fp = app->GetImportPanel( &messenger )))
		{
			fp->SetRefFilter( filter );
			fp->Show();
		}
		break;
	
	case MENU_SAVE:
		if ((fp = app->GetExportPanel( &messenger )))
		{
			fp->Show();
		}
		break;

	case B_SAVE_REQUESTED:

			// Write the VCTable to a MeV file.
		if (	msg->FindRef( "directory", 0, &eref ) == B_OK
			&& msg->FindString( "name", 0, (const char **)&name ) == B_OK)
		{
			BDirectory	dir( &eref );
			CheckBeError( dir.InitCheck() );
		
			BFile			file( &dir, name, B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE );
			CheckBeError( file.InitCheck() );

				// Create reader and IFF reader.
			CBeFileWriter	writer( file );
			CIFFWriter	iffWriter( writer );
		
			iffWriter.Push( Form_ID );
			iffWriter << (long)MeV_ID;

			doc->WriteVCTable( iffWriter );

			BNodeInfo	ni( &file );
	
			if (ni.InitCheck() == B_NO_ERROR)
			{
				ni.SetType("application/x-vnd.MeV-VTable");
				ni.SetPreferredApp("application/x-vnd.MeV");
			}
		}
		break;
		
	case B_REFS_RECEIVED:

			// Read the VCTable from a MeV file.
		if (	msg->FindRef( "refs", 0, &eref ) == B_OK)
		{
			BFile			file( &eref, B_READ_ONLY );
			CheckBeError( file.InitCheck() );

				// Create reader and IFF reader.
			CBeFileReader	reader( file );
			CIFFReader	iffReader( reader );
			int32		formType;
	
			iffReader.ChunkID( 1, &formType );
	
			for (;;) {
				if (iffReader.NextChunk() == false) break;
				if (iffReader.ChunkID( 0, &formType ) == VCTable_ID)
					doc->ReadVCTable( iffReader );	// Virtual channel table
			}
		}

			// Reset control values
		channelList->Invalidate();
		portSlider->SetValue( vc.port + 1 );
		channelSlider->SetValue( vc.channel );
		channelColor->SetValue( vc.fillColor );	
		doHint = true;
		break;
	}
	
	if (doHint)
	{
		CUpdateHint		hint;
	
		// Channel attribute has changed
		// Umm, we need to post an update to ALL tracks...!
		hint.AddInt8( "channel", selChan );
		doc->PostUpdateAllTracks( &hint );
	}
}
