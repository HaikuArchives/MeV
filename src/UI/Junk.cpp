/* ===================================================================== *
 * Junk.cpp (MeV/User Interface)
 * ===================================================================== */

#include "Junk.h"

#include "ScreenUtils.h"

// Gnu C Library
#include <stdio.h>
// Interface Kit
#include <Box.h>
#include <Button.h>
#include <CheckBox.h>
#include <ScrollView.h>
#include <StringView.h>

// =============================================================================

enum E_TimeComponents {
	TC_Milliseconds,
	TC_Seconds,
	TC_Minutes,
	TC_Hours,
	TC_Clocks,
	TC_Beats,
	TC_Measures,
};

inline int32 udiv( int32 dividend, int32 divisor )
{
	if (dividend >= 0) return dividend / divisor;
	else return (dividend - divisor + 1) / divisor;
}

CTimeEditControl::CTimeEditControl(
	BRect		inFrame,
	BMessage		*inMessage,
	uint32		inResizingMode,
	uint32		inFlags )
	: BControl( inFrame, NULL, NULL, inMessage, inResizingMode, inFlags )
{
	beatSize = 240;
	measureSize = beatSize * 4;
	measureBase = 0;

	SetValue( 0 );
	
	clockType = ClockType_Metered;
}

	// REM: This is still too flickery for my taste.

void CTimeEditControl::Draw( BRect inUpdateRect )
{
	BRect		r( Bounds() );
	BFont		font1( be_bold_font ),
				font2( be_plain_font );
	escapement_delta ed;
	
	ed.space = ed.nonspace = 0.0;
         				
	font1.SetSize( 14.0 );
	font2.SetSize( 9.0 );

//	font1.SetSpacing( B_BITMAP_SPACING );
	font1.SetSpacing( B_FIXED_SPACING );
	SetLowColor( Parent()->ViewColor() );
	SetDrawingMode( B_OP_COPY );

	SetHighColor( Parent()->ViewColor() );
	FillRect( BRect( r.left, r.top, r.left, r.top ) );
	FillRect( BRect( r.left, r.bottom, r.left, r.bottom ) );
	FillRect( BRect( r.right, r.top, r.right, r.top ) );
	FillRect( BRect( r.right, r.bottom, r.right, r.bottom ) );

	SetHighColor( 100, 100, 100 );
	FillRect( BRect( r.left + 1, r.top, r.right - 1, r.top ) );
	FillRect( BRect( r.left + 1, r.bottom, r.right - 1, r.bottom ) );
	FillRect( BRect( r.left, r.top + 1, r.left, r.bottom - 1 ) );
	FillRect( BRect( r.right, r.top + 1, r.right, r.bottom - 1 ) );
	
	SetHighColor( 255, 255, 255 );
	FillRect( BRect( r.left + 1, r.top + 1, r.right - 1, r.top + 2 ) );
	FillRect( BRect( r.left + 1, r.top + 3, r.left + 1, r.bottom - 3 ) );

	SetHighColor( 200, 200, 200 );
	FillRect( BRect( r.left + 1, r.bottom - 2, r.right - 1, r.bottom - 2 ) );
	
	SetHighColor( 180, 180, 180 );
	FillRect( BRect( r.left + 1, r.bottom - 1, r.right - 1, r.bottom - 1 ) );
	FillRect( BRect( r.right - 1, r.top + 3, r.right - 1, r.bottom - 3 ) );
	
	Sync();

	if (clockType == ClockType_Metered)
	{
		int32		measures, beats, clocks;
	
		clocks = Value() - measureBase;
		measures = udiv( clocks, measureSize );
		clocks -= measures * measureSize;
		beats = clocks / beatSize;
		clocks -= beats * beatSize;
		
		sprintf(	text, "%4.4ld:%2.2ld:%3.3ld", measures, beats, clocks );

		SetFont( &font1 );

		SetHighColor( Parent()->ViewColor() );
		FillRect( BRect( r.left + 2, r.top + 2, r.right - 2, r.top + 12 ) );

		MovePenTo( r.left + 4, r.top + 13 );
		SetHighColor( 0, 0, 255 );
		DrawString( &text[ 0 ], 6, &ed );
		SetHighColor( 0, 0, 0 );
		DrawString( &text[ 6 ], 12 - 6, &ed );
		SetFont( &font2 );
		SetHighColor( Parent()->ViewColor() );
		FillRect( BRect( r.left + 2, r.top + 13, r.right - 2, r.bottom - 2 ) );
		SetHighColor( 0, 0, 0 );
		MovePenTo( r.left + 6, r.top + 21 );
		DrawString( "Meas.  Beat  Tick" );
	}
	else
	{
		int32		hours, minutes, seconds, ms;
		
		ms = Value();
		hours = udiv( ms, 60 * 60 * 1000 );
		ms -= hours * 60 * 60 * 1000;
		minutes = ms / (60 * 1000);
		ms -= minutes * 60 * 1000;
		seconds = ms / 1000;
		ms -= seconds * 1000;
	
		sprintf( text, "%2.2ld:%2.2ld:%2.2ld.%3.3ld", hours, minutes, seconds, ms );

		SetFont( &font1 );

		SetHighColor( Parent()->ViewColor() );
		FillRect( BRect( r.left + 2, r.top + 2, r.right - 2, r.top + 12 ) );

		MovePenTo( r.left + 2, r.top + 13 );
		SetHighColor( 0, 0, 255 );
		DrawString( &text[ 0 ], 6, &ed );
		SetHighColor( 0, 0, 0 );
		DrawString( &text[ 6 ], 12 - 6, &ed );
		SetFont( &font2 );
		SetHighColor( Parent()->ViewColor() );
		FillRect( BRect( r.left + 2, r.top + 13, r.right - 2, r.bottom - 2 ) );
		SetHighColor( 0, 0, 0 );
		MovePenTo( r.left + 5, r.top + 21 );
		DrawString( "Hr.   Min.  Sec.   ms" );
	}
}

void CTimeEditControl::SetValue( int32 value )
{
	if ( value != Value() )
	{
		BControl::SetValue( value );
	}
}

void CTimeEditControl::MouseDown( BPoint point )
{
	if (IsEnabled())
	{
		mousePos = point;
	
			// spawn a thread to drag the slider
		thread_id tid;
		tid = spawn_thread( drag_entry, "", B_NORMAL_PRIORITY, this );
		resume_thread( tid );
	}
}

long CTimeEditControl::drag_entry( void *arg )
{
	return ((CTimeEditControl *) arg)->Drag();
}

long CTimeEditControl::Drag()
{
	// First, we need to determine which digit was clicked on...

	return B_OK;
}

// =============================================================================

CPrefsWindow::CPrefsWindow( CWindowState &inState, char *name, uint32 flags )
	: CAppWindow( inState, inState.Rect(), name, B_TITLED_WINDOW, flags )
{
	BRect		r( inState.Rect() );
	
	r.OffsetTo( B_ORIGIN );

	background = new BView( r, NULL, B_FOLLOW_ALL_SIDES, B_WILL_DRAW );
	
	AddChild( background );
	background->SetViewColor( 220, 220, 220 );
	
	r.InsetBy( 5.0, 5.0 );
	r.bottom -= 35;
	
	BButton		*bb;
	
	background->AddChild( bb = new BButton(	BRect(	r.right - 176,
													r.bottom + 9,
													r.right - 102,
													r.bottom + 34 ),
											"Revert", "Revert",
											new BMessage( Revert_ID ),
											B_FOLLOW_LEFT | B_FOLLOW_BOTTOM ) );

	background->AddChild( bb = new BButton(	BRect(	r.right - 90,
													r.bottom + 9,
													r.right - 4,
													r.bottom + 34 ),
											"Close", "Close",
											new BMessage( Close_ID ),
											B_FOLLOW_LEFT | B_FOLLOW_BOTTOM ) );
	bb->MakeDefault( true );

	currentPanel = -1;
	panelCount = 0;

	panelList = new BListView(	BRect(	r.left + 2, r.top + 7,
										110 - B_V_SCROLL_BAR_WIDTH, r.bottom ),
										NULL, B_SINGLE_SELECTION_LIST );
	
	panelList->SetSelectionMessage( new BMessage( Select_ID ) );
	
	BScrollView *sv = new BScrollView(	NULL,
										panelList,
										B_FOLLOW_TOP_BOTTOM | B_FOLLOW_LEFT,
										0,
										false, true,
										B_PLAIN_BORDER );
	background->AddChild( sv );
//	panelList->Select( currentPanel );	
}

CPrefsWindow::~CPrefsWindow()
{
	DeleteListItems( panelList );
}

BView *CPrefsWindow::AddPanel( char *name )
{
	BRect		r( background->Frame() );
	BBox			*p;
	
	if (panelCount >= Max_Panels) return NULL;
	
	p = panels[ panelCount ] = new BBox(	BRect(	118,
												r.top + 5.0,
												r.right - 5.0,
												r.bottom - 38.0 ),
										NULL,
										B_FOLLOW_ALL );

	p->Hide();
	background->AddChild( p );
	p->SetLabel( name );
	panelList->AddItem( new BStringItem( name ) );
	
	panelCount++;
	
	return p;
}

int32 CPrefsWindow::GetPanelNum()
{
	return currentPanel;
}

void CPrefsWindow::SetPanelNum( int inPanelNum )
{
	if (inPanelNum <= 0) inPanelNum = 0;
	else if (inPanelNum >= panelCount) inPanelNum = panelCount - 1;

	if (inPanelNum != currentPanel)
	{
		if (currentPanel >= 0) panels[ currentPanel ]->Hide();
		currentPanel = inPanelNum;
		panelList->Select( inPanelNum );
		panels[ currentPanel ]->Show();
	}
}

void CPrefsWindow::MessageReceived( BMessage *msg )
{
	switch (msg->what) {
	case Select_ID:
		SetPanelNum( panelList->CurrentSelection() );
		break;
	}
}

const int32 New_Value = 'newv';

CAppPrefsWindow::CAppPrefsWindow( CWindowState &inState )
	: CPrefsWindow( inState, "MeV Preferences", B_NOT_RESIZABLE | B_NOT_ZOOMABLE )
{
	BStringView	*sv;
	BView		*panel;
	static char	*labels[] = {	"Note Pitch",
							"Note Velocity",
							"Program Change",
							"Controller Value" };

		// Construct the Audible feedback panel
	panel = AddPanel( "Audible Feedback" );

	int32		y;

	y = 20;
	
	for (int i = 0; i < 4; i++)
	{
		sv = new BStringView( BRect( 20, y, 100, y + 16 ), NULL, labels[ i ] );
		sv->SetAlignment( B_ALIGN_RIGHT );
		panel->AddChild( sv );

		cb[ i ][ 0 ] = new BCheckBox(	BRect( 110, y, 120, y + 10 ), NULL, "While dragging", new BMessage( New_Value ) );
		panel->AddChild( cb[ i ][ 0 ] );
		cb[ i ][ 0 ]->ResizeToPreferred();
	
		cb[ i ][ 1 ] = new BCheckBox(	BRect( 220, y, 230, y + 10 ), NULL, "While adjusting", new BMessage( New_Value ) );
		panel->AddChild( cb[ i ][ 1 ] );
		cb[ i ][ 1 ]->ResizeToPreferred();
	
		y = cb[ i ][ 1 ]->Frame().bottom + 2;	
	}

	y += 13;

	chan_cb = new BCheckBox(	BRect( 110, y, 120, y + 10 ), NULL, "Channel Change", new BMessage( New_Value ) );
	panel->AddChild( chan_cb );
	chan_cb->ResizeToPreferred();

	y = chan_cb->Frame().bottom + 15;	

	sv = new BStringView( BRect( 10, y, 100, y + 13 ), NULL, "Delay (ms)" );
	panel->AddChild( sv );
	sv->SetAlignment( B_ALIGN_RIGHT );
	
	fbDelay = new CTextSlider( BRect( 110, y, 320, y + 13 ), new BMessage( New_Value ), NULL );
	panel->AddChild( fbDelay );
//	fbDelay->SetTarget( (BLooper *)this, NULL );
	fbDelay->SetRange( 0, 1000 );
	
		// Construct the Editing options panel

	panel = AddPanel( "Editing" );

	y = 20;
	rect_cb = new BCheckBox(	BRect( 110, y, 120, y + 10 ), NULL, "Inclusive Rectangle Selection", new BMessage( New_Value ) );
	panel->AddChild( rect_cb );
	rect_cb->ResizeToPreferred();
	y = rect_cb->Frame().bottom + 2;

		// Numeric origin panel...

	prefs = gPrefs;
	SetPanelNum( gPrefs.appPrefsPanel );
	ReadPrefs();
}

void CAppPrefsWindow::ReadPrefs()
{	
	for (int i = 0; i < 4; i++)
	{
		cb[ i ][ 0 ]->SetValue( (gPrefs.feedbackDragMask   & (1<<i)) ? true : false );
		cb[ i ][ 1 ]->SetValue( (gPrefs.feedbackAdjustMask & (1<<i)) ? true : false );
	}
	chan_cb->SetValue( (gPrefs.feedbackAdjustMask & CGlobalPrefs::FB_Channel) ? true : false );
	rect_cb->SetValue( gPrefs.inclusiveSelection );
	fbDelay->SetValue( gPrefs.feedbackDelay );
}

void CAppPrefsWindow::WritePrefs()
{
	for (int i = 0; i < 4; i++)
	{
		uint32		mask = 1 << i;
		
		if (cb[ i ][ 0 ]->Value())	gPrefs.feedbackDragMask |= mask;
		else							gPrefs.feedbackDragMask &= ~mask;
			
		if (cb[ i ][ 1 ]->Value())	gPrefs.feedbackAdjustMask |= mask;
		else							gPrefs.feedbackAdjustMask &= ~mask;
	}

	if (chan_cb->Value())		gPrefs.feedbackAdjustMask |=  CGlobalPrefs::FB_Channel;
	else						gPrefs.feedbackAdjustMask &= ~CGlobalPrefs::FB_Channel;

	gPrefs.inclusiveSelection		= rect_cb->Value();
	gPrefs.feedbackDelay		= fbDelay->Value();
	gPrefs.appPrefsPanel		= GetPanelNum();
}

bool CAppPrefsWindow::QuitRequested()
{
	return true;
}

void CAppPrefsWindow::MessageReceived( BMessage *msg )
{
	switch (msg->what) {
	case Close_ID:
		PostMessage( B_QUIT_REQUESTED );
		break;
		
	case Revert_ID:
		gPrefs = prefs;
		ReadPrefs();
		break;

/*	case Select_ID:
			// When they change panels, you can no longer revert...?
		prefs = gPrefs;
		CPrefsWindow::MessageReceived( msg );
		break; */
		
	case New_Value:
		WritePrefs();
		break;
		
	default:
		CPrefsWindow::MessageReceived( msg );
		break;
	}
}

CLEditorPrefsWindow::CLEditorPrefsWindow( CWindowState &inState, CEventTrack *inTrack )
	: CPrefsWindow( inState, "Track Preferences", B_NOT_RESIZABLE | B_NOT_ZOOMABLE ),
	  CObserver( *this, inTrack )
{
	BView				*panel;
//	BStringView			*sv;
	int					y;
	BCheckBox			*showNoteStrip,
						*showVelocityAttack,
						*showVelocityRelease,
						*showPitchBendStrip,
						*showControllerStrip[ 4 ],
						*showTrackControlStrip;

		// Note strip
	panel = AddPanel( "Notes" );
	y = 20;
	showNoteStrip = new BCheckBox(	BRect( 10, y, 120, y + 10 ), NULL, "Show Note Strip", new BMessage( New_Value ) );
	panel->AddChild( showNoteStrip );
	showNoteStrip->ResizeToPreferred();
	y = showNoteStrip->Frame().bottom + 2;	

	showVelocityAttack = new BCheckBox(	BRect( 10, y, 120, y + 10 ), NULL, "Show Attack Velocity", new BMessage( New_Value ) );
	panel->AddChild( showVelocityAttack );
	showVelocityAttack->ResizeToPreferred();
	y = showVelocityAttack->Frame().bottom + 2;

	showVelocityRelease = new BCheckBox( BRect( 10, y, 120, y + 10 ), NULL, "Show Release Velocity", new BMessage( New_Value ) );
	panel->AddChild( showVelocityRelease );
	showVelocityRelease->ResizeToPreferred();

		// Aftertouch strip
	panel = AddPanel( "Aftertouch" );
		// Show Channel Aftertouch
		// Show Polyphonic Aftertouch

		// Pitchbend strip
	panel = AddPanel( "Pitchbend" );
	y = 20;
	showPitchBendStrip = new BCheckBox(	BRect( 10, y, 120, y + 10 ), NULL, "Show Pitch Bend Strip", new BMessage( New_Value ) );
	panel->AddChild( showPitchBendStrip );
	showPitchBendStrip->ResizeToPreferred();

		// Controller strips
	panel = AddPanel( "Controllers" );
	y = 20;
	
		// REM: We should have a popup menu for which strips we're talking about.

	for (int i = 0; i < 4; i++)
	{
		char			sName[ 64 ];
	
		sprintf( sName, "Show Controller Strip #%d", i + 1 );

		showControllerStrip[ i ] = new BCheckBox(	BRect( 10, y, 120, y + 10 ), NULL, sName, new BMessage( New_Value ) );
		panel->AddChild( showControllerStrip[ i ] );
		showControllerStrip[ i ]->ResizeToPreferred();
		y = showControllerStrip[ i ]->Frame().bottom + 2;

			// 1-8 Controller types
			// Controller Type
			// Color
			// Editable
	}

		// Track control strip
	panel = AddPanel( "Track Control" );
	y = 20;
	showTrackControlStrip = new BCheckBox(	BRect( 10, y, 120, y + 10 ), NULL, "Show Track Control Strip", new BMessage( New_Value ) );
	panel->AddChild( showTrackControlStrip );
	showTrackControlStrip->ResizeToPreferred();

	// Need a way to specify density of inserted events...
	// Need a way to specify type of inserted controller

#if 0
		// Add the track list.
	stripList = new BOutlineListView(
		BRect( 10.0, 18.0, 314, 202.0 ),
		NULL, B_SINGLE_SELECTION_LIST, B_FOLLOW_ALL );

	stripList->AddItem( new BStringItem( "Chromatic Strip", 0, false ) );
	stripList->AddItem( new BStringItem( "Notes", 1 ) );
	stripList->AddItem( new BStringItem( "Program Changes", 1 ) );
	stripList->AddItem( new BStringItem( "System Exclusive", 1 ) );
	stripList->AddItem( new BStringItem( "Sequencer Control", 1 ) );
	stripList->AddItem( new BStringItem( "Text Events", 1 ) );

	stripList->AddItem( new BStringItem( "Velocity Strip", 0, false ) );
	stripList->AddItem( new BStringItem( "Attack Velocity", 1 ) );
	stripList->AddItem( new BStringItem( "Release Velocity", 1 ) );

	stripList->AddItem( new BStringItem( "Aftertouch Strip", 0, false ) );
	stripList->AddItem( new BStringItem( "Channel Aftertouch", 1 ) );
	stripList->AddItem( new BStringItem( "Polyphonic Aftertouch", 1 ) );

	stripList->AddItem( new BStringItem( "Pitchbend Strip", 0, false ) );

	for (int i = 0; i < 3; i++)
	{
		char		sName[ 32 ];
		
		sprintf( sName, "Control Change Strip #%d", i + 1 );
	
		stripList->AddItem( new BStringItem( sName, 0, false ) );
		stripList->AddItem( new BStringItem( "Modulation", 1 ) );
		stripList->AddItem( new BStringItem( "Breath Controller", 1 ) );
		stripList->AddItem( new BStringItem( "Foot Controller", 1 ) );
		stripList->AddItem( new BStringItem( "Portamento Time", 1 ) );
		stripList->AddItem( new BStringItem( "Main Volume", 1 ) );
		stripList->AddItem( new BStringItem( "Balance", 1 ) );
		stripList->AddItem( new BStringItem( "Pan", 1 ) );
		stripList->AddItem( new BStringItem( "Expression Controller", 1 ) );
	}

	stripList->AddItem( new BStringItem( "Track Control Strip", 0, false ) );
	stripList->AddItem( new BStringItem( "Track Play Events", 1 ) );
	stripList->AddItem( new BStringItem( "Script Events", 1 ) );
	stripList->AddItem( new BStringItem( "Repeats", 1 ) );

	BScrollView *scv = new BScrollView(	NULL,
									stripList,
									B_FOLLOW_TOP_BOTTOM | B_FOLLOW_LEFT,
									0,
									false, true,
									B_PLAIN_BORDER );

	panel->AddChild( scv );
#endif

	// REM: Need to read the prefs here...

	SetPanelNum( gPrefs.lEditorPrefsPanel );
}

CLEditorPrefsWindow::~CLEditorPrefsWindow()
{
	gPrefs.lEditorPrefsPanel = GetPanelNum();
//	DeleteListItems( stripList );
}

	// Centers on PARENT window, but constrains to screen
CMiniDialog::CMiniDialog( int32 inWidth, int32 inHeight, BWindow *parent, const char *title )
	: BWindow( UScreenUtils::CenterOnWindow( inWidth, inHeight, parent ),
		title,
		B_TITLED_WINDOW, B_NOT_RESIZABLE | B_NOT_ZOOMABLE | B_NOT_MINIMIZABLE,
		parent->Workspaces() )
{
	BRect		r( Frame() );
	r.OffsetTo( B_ORIGIN );

	background = new BView( r, NULL, B_FOLLOW_ALL_SIDES, B_WILL_DRAW );
	
	AddChild( background );
	background->SetViewColor( 220, 220, 220 );

	background->AddChild( new BButton(	BRect( inWidth - 102, inHeight - 30, inWidth - 8, inHeight - 7 ),
						"Apply", "Apply",
						new BMessage( Apply_ID ) ) );

	background->AddChild( new BButton(	BRect( 8, inHeight - 30, 102, inHeight - 7 ),
						"Close", "Close",
						new BMessage( Close_ID ) ) );

}
