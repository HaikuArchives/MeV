/* ===================================================================== *
 * TransportWindow.cpp (MeV/User Interface)
 * ===================================================================== */

#include "TransportWindow.h"
#include "EventTrack.h"
#include "Idents.h"
#include "MeVApp.h"
#include "BorderView.h"
#include "BorderButton.h"
#include "TextSlider.h"
#include "StdButton.h"
#include "Junk.h"
#include "PlayerControl.h"
#include "ResourceUtils.h"

// Gnu C Library
#include <stdio.h>
// Interface Kit
#include <StringView.h>

// =============================================================================

class CTempoTextHook : public CTextSlider::CTextHook {
public:
		/**	Return the width in pixels of the largest possible knob text. */
	int32 Largest( BView *inView, int32 inMin, int32 inMax )
	{
		return static_cast<int32>(inView->StringWidth("000.0"));
	}

		/**	Format the text for the text slider knob */
	void FormatText( char *outText, int32 inValue, int32 inMaxLen )
	{
		sprintf( outText, "%3ld.%1.1ld", inValue / 10, inValue % 10 );
	}
};

static CTempoTextHook		ttHook;

// =============================================================================

class CTransportPulseView : public BView {
public:
	CTransportPulseView()
		: BView( BRect( -1, -1, -1, -1 ), "Pulse", 0, B_PULSE_NEEDED ) {}
	
	void Pulse()
	{
		CTransportWindow		*w;
	
		if ((w = dynamic_cast<CTransportWindow *>(Window())))
		{
			w->UpdateDisplay();
		}
	}
};

enum {
	Btn_Height	= 17,
	Btn_Height2	= 24,
	Thin_Width	= 23,
	Thick_Width	= 52,
};

CTransportWindow::CTransportWindow( BPoint pos, CWindowState &inState )
	: CAppWindow(	inState,
					BRect(	pos.x, pos.y,
							pos.x + Transport_Width, pos.y + Transport_Height ),
					"Transport",
					B_FLOATING_WINDOW,
					B_WILL_ACCEPT_FIRST_CLICK | B_NOT_RESIZABLE | B_NOT_ZOOMABLE,
					B_CURRENT_WORKSPACE ),
	CObserver( *this, NULL )
{
	BView		*bg;
	BRect		rect( Frame() );
	CTransportPulseView *tv;
	float		x, y, w;
	
	track = NULL;
	document = NULL;
	
	finalTempo = true;
	
	rect.OffsetTo( B_ORIGIN );
	rect.right++;
	bg = new CBorderView( rect, NULL, B_FOLLOW_ALL_SIDES, B_WILL_DRAW, 0, CBorderView::BEVEL_BORDER);
	AddChild( bg );

	x = 17;
	y = 2.0;
	BControl	*bb;
	
	bb = new CBorderButton(
		BRect( x, y, x + Thin_Width, y + Btn_Height ),
		"ToStart", ResourceUtils::LoadImage("Begin"), new BMessage( MENU_LOCATE_START ) );
	bg->AddChild( bb );
	x += Thin_Width;

	bb = new CBorderButton(
		BRect( x, y, x + Thick_Width, y + Btn_Height ),
		"Play FromStart", ResourceUtils::LoadImage("PlayBeg"), new BMessage( MENU_PLAY_FROM_START ) );
	bg->AddChild( bb );
	x += Thick_Width;

	bb = new CPushOnButton(
		BRect( x, y, x + Thick_Width, y + Btn_Height ),
		"Play", ResourceUtils::LoadImage("Play"), new BMessage( MENU_PLAY ) );
	bg->AddChild( bb );
	playButton = bb;
	x += Thick_Width;
	
	bb = new CBorderButton(
		BRect( x, y, x + Thin_Width, y + Btn_Height ),
		"ToEnd", ResourceUtils::LoadImage("End"), new BMessage( MENU_LOCATE_END ) );
	bg->AddChild( bb );
	x += Thin_Width;

	bb = new CPushOnButton(
		BRect( x, y, x + Thick_Width, y + Btn_Height ),
		"Stop", ResourceUtils::LoadImage("Stop"), new BMessage( MENU_STOP ) );
	bg->AddChild( bb );
	stopButton = bb;
	x += Thick_Width + 3;
	
	bb = new CToggleButton(
		BRect( x, y, x + Thick_Width, y + Btn_Height ),
		"Pause", ResourceUtils::LoadImage("Pause"), new BMessage( MENU_PAUSE ) );
	bg->AddChild( bb );
	pauseButton = bb;
	x += Thick_Width;
	
	w = x;
	// Second line of controls
	
	x = 17;
	y += Btn_Height + 3;

	bb = new CToggleButton(
		BRect( x, y, x + 44, y + Btn_Height2 ), "Record", 
		ResourceUtils::LoadImage("Rec"), NULL );
	bg->AddChild( bb );
	x += 44 + 3;

	loopButton = new CToggleButton(
		BRect( x, y, x + 32, y + Btn_Height2 ), "Loop",
		ResourceUtils::LoadImage("LoopOff"), new BMessage( 'loop' ) );
	bg->AddChild( loopButton );
	loopButton->SetValue( ((CMeVApp *)be_app)->GetLoopFlag() );
	x += 32 + 3;

//	bb = new CToggleButton(
//		BRect( x, y, x + 28, y + Btn_Height2 ), "Punch", NULL, NULL );
//	bg->AddChild( bb );
//	x += 28 + 3;
	
	// Also overlay...

	meteredTimeDisplay = new CTimeEditControl( BRect( x, y, x + 86, y + Btn_Height2 ), new BMessage( 'strt' ) );
	bg->AddChild( meteredTimeDisplay );
	x += 86 + 3;

	realTimeDisplay = new CTimeEditControl( BRect( x, y, x + 86, y + Btn_Height2 ), new BMessage( 'strt' ) );
	bg->AddChild( realTimeDisplay );
	realTimeDisplay->SetClockType( ClockType_Real );

	x = 17;
	y += Btn_Height2 + 2;
	
	BStringView		*label;
	label = new BStringView( BRect( x, y, x + 36, y + 13 ), NULL, "Tempo:" );
	bg->AddChild( label );
	label->SetAlignment( B_ALIGN_RIGHT );
	x += 36 + 2;

	tempoSlider = new CTextSlider( BRect( x, y, w, y + 13 ), new BMessage( 'temp' ), NULL );
	tempoSlider->SetTextHook( &ttHook );
	tempoSlider->SetRange( 100, 5000 );
	bg->AddChild( tempoSlider );
	tempoSlider->SetEnabled( true );
	
	tv = new CTransportPulseView();
	bg->AddChild( tv );

	SetButtons();
	SetPulseRate( 32000 );
}

void CTransportWindow::SetButtons()
{
	if (document)
	{
		PlaybackState	pbState;
		bool			pbStateValid,
					isPlaying;
		
		pbStateValid	= CPlayerControl::GetPlaybackState( document, pbState );
		isPlaying		= pbStateValid & pbState.running;
		
		playButton->SetEnabled( true );
		playButton->SetValue( isPlaying );

		stopButton->SetEnabled( true );
		stopButton->SetValue( !isPlaying );

		pauseButton->SetEnabled( true );
		pauseButton->SetValue( CPlayerControl::PauseState( document ) );

		if (finalTempo)
		{
			tempoSlider->SetEnabled( true );
			if (isPlaying)
				tempoSlider->SetValue( (int32)(CPlayerControl::Tempo( document ) * 10.0) );
			else tempoSlider->SetValue( document->InitialTempo() * 10.0 );
		}
	}
	else
	{
		playButton->SetEnabled( false );
		playButton->SetValue( false );

		stopButton->SetEnabled( false );
		stopButton->SetValue( true );

		pauseButton->SetEnabled( false );
		pauseButton->SetValue( false );

		if (finalTempo)
		{
			tempoSlider->SetEnabled( false );
			tempoSlider->SetValue( (int32)(CPlayerControl::Tempo( NULL ) * 10.0) );
		}
	}
}

void CTransportWindow::UpdateDisplay()
{
	PlaybackState	pbState;

	if (document && CPlayerControl::GetPlaybackState( document, pbState ))
	{
		realTimeDisplay->SetValue( pbState.realTime );
		meteredTimeDisplay->SetValue( pbState.meteredTime );
	}
}

void CTransportWindow::MessageReceived( BMessage* msg )
{
	if (track == NULL) return;

	switch(msg->what) {
	case MENU_LOCATE_START:
		break;

	case MENU_PLAY:
			// REM: Add ability to play individual track...
		CPlayerControl::PlaySong( document, 0, 0, LocateTarget_Continue, -1, SyncType_SongInternal, 0 );
		break;

	case MENU_PLAY_FROM_START:
#if 0
			// Start playing a song.
		CPlayerControl::PlaySong(	(CMeVDoc *)&document,
								Track()->GetID(), 0, LocateTarget_Real, -1,
								SyncType_SongInternal, (app.GetLoopFlag() ? PB_Loop : 0) );
		break;
#endif
		CPlayerControl::PlaySong( document, 0, 0, LocateTarget_Real, -1, SyncType_SongInternal, 0 );
		break;

	case MENU_LOCATE_END:
		break;

	case MENU_STOP:
		CPlayerControl::StopSong( document );
		break;

	case MENU_PAUSE:
		CPlayerControl::SetPauseState( document, pauseButton->Value() );
		break;
		
	case 'temp':
		double		tempo;
		
		tempo = (double)(tempoSlider->Value()) / 10.0;
		if (document) document->SetInitialTempo( tempo );
		CPlayerControl::SetTempo( document, tempo );
		finalTempo = msg->HasBool( "final" );
		break;
		
	case 'loop':
		((CMeVApp *)be_app)->SetLoopFlag(  loopButton->Value() != false );
		break;

	case Update_ID:
	case Delete_ID:
		CObserver::MessageReceived( msg );
		break;

	default:
		CAppWindow::MessageReceived( msg );
		break;
	}

	SetButtons();
}

void CTransportWindow::WatchTrack( CEventTrack *inTrack )
{
	if (track != inTrack)
	{
		CMeVDoc		*oldDoc = document;

		track = inTrack;
		Lock();

		document = track ? &track->Document() : NULL;
		if (document != oldDoc) 	SetButtons();

		Unlock();
		SetSubject( track );
	}
}

	// We want to delete the subject, please stop observing...
	// Note that observers are free to ignore this message, in which
	// case the object will not be deleted.
void CTransportWindow::OnDeleteRequested( BMessage *inMsg )
{
	WatchTrack( NULL );
}

		// Update message from another observer
void CTransportWindow::OnUpdate( BMessage *inMsg )
{
#if 0
	if (track == NULL)
	{
// 	Clear();
	}
#endif
}
