/* ===================================================================== *
 * TransportWindow.cpp (MeV/UI)
 * ===================================================================== */

#include "TransportWindow.h"

#include "BorderButton.h"
#include "BorderView.h"
#include "EventTrack.h"
#include "Idents.h"
#include "MeVApp.h"
#include "MeVDoc.h"
#include "PlayerControl.h"
#include "ResourceUtils.h"
#include "StdButton.h"
#include "TempoEditControl.h"
#include "TimeEditControl.h"

// Gnu C Library
#include <stdio.h>
// Interface Kit
#include <StringView.h>

enum {
	Btn_Height	= 17,
	Btn_Height2	= 24,
	Thin_Width	= 23,
	Thick_Width	= 46,
};

CTransportWindow::CTransportWindow(
	BPoint pos,
	CWindowState &state)
	: CAppWindow(state, BRect(pos.x, pos.y,
							  pos.x + Transport_Width,
							  pos.y + Transport_Height),
					"Transport",
					B_FLOATING_WINDOW,
					B_WILL_ACCEPT_FIRST_CLICK | B_NOT_RESIZABLE | B_NOT_ZOOMABLE,
					B_CURRENT_WORKSPACE),
	CObserver(*this, NULL),
	m_document(NULL),
	m_track(NULL)
{
	BView *bg;
	BRect rect(Bounds());
	float x, y;

	rect.right++;
	bg = new CBorderView(rect, NULL, B_FOLLOW_ALL_SIDES, B_WILL_DRAW, 0,
						 CBorderView::BEVEL_BORDER);
	AddChild(bg);

	x = 5.0;
	y = 5.0;
	BControl *bb;

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
		BRect( x, y, x + Thick_Width, y + Btn_Height),
		"Play", ResourceUtils::LoadImage("Play"), new BMessage(MENU_PLAY));
	bg->AddChild( bb );
	m_playButton = bb;
	x += Thick_Width;

	bb = new CBorderButton(
		BRect( x, y, x + Thin_Width, y + Btn_Height ),
		"ToEnd", ResourceUtils::LoadImage("End"), new BMessage(MENU_LOCATE_END));
	bg->AddChild( bb );
	x += Thin_Width;

	bb = new CPushOnButton(
		BRect( x, y, x + Thick_Width, y + Btn_Height ),
		"Stop", ResourceUtils::LoadImage("Stop"), new BMessage(MENU_STOP));
	bg->AddChild( bb );
	m_stopButton = bb;
	x += Thick_Width + 3;

	bb = new CToggleButton(
		BRect( x, y, x + Thick_Width, y + Btn_Height ),
		"Pause", ResourceUtils::LoadImage("Pause"), new BMessage(MENU_PAUSE));
	bg->AddChild( bb );
	m_pauseButton = bb;
	x += Thick_Width;

	// create time edit control
	rect = Bounds();
	rect.left += 5.0;
	rect.top = m_playButton->Frame().bottom + 5.0;
	rect.right = rect.left + 86;
	rect.bottom = rect.top + 24;
	m_timeCtl = new CTimeEditControl(rect, new BMessage('strt'));
	m_timeCtl->SetEditable(false);

	// create tempo edit control
	rect.OffsetBy(rect.Width(), 0.0);
	BMessage *message = new BMessage(CTempoEditControl::TEMPO_CHANGED);
	m_tempoCtl = new CTempoEditControl(rect, message);

	// find preferred sizes
	float timeWidth, timeHeight;
	m_timeCtl->GetPreferredSize(&timeWidth, &timeHeight);
	float tempoWidth, tempoHeight;
	m_tempoCtl->GetPreferredSize(&tempoWidth, &tempoHeight);
	float maxWidth, maxHeight;
	maxWidth = timeWidth > tempoWidth ? timeWidth : tempoWidth;
	maxHeight = timeHeight > tempoHeight ? timeHeight : tempoHeight;
	m_timeCtl->ResizeTo(maxWidth, maxHeight);
	m_tempoCtl->ResizeTo(maxWidth, maxHeight);
	bg->AddChild(m_timeCtl);
	bg->AddChild(m_tempoCtl);
	if ((2 * maxWidth + 15.0) < m_pauseButton->Frame().right)
	{
		m_tempoCtl->MoveTo(m_pauseButton->Frame().right - maxWidth,
						   m_tempoCtl->Frame().top);
	}

	// resize window to preferred
	maxWidth = m_pauseButton->Frame().right;
	if (m_tempoCtl->Frame().right > maxWidth)
		maxWidth = m_tempoCtl->Frame().right;
	maxWidth += 5.0;
	maxHeight = m_tempoCtl->Frame().bottom + 5.0;
	ResizeTo(maxWidth, maxHeight);

	SetButtons();
}

void
CTransportWindow::SetButtons()
{
	if (m_document)
	{
		PlaybackState pbState;
		bool pbStateValid = CPlayerControl::GetPlaybackState(m_document,
															 pbState);
		bool isPlaying = pbStateValid & pbState.running;

		m_playButton->SetEnabled(true);
		m_playButton->SetValue(isPlaying);
		m_stopButton->SetEnabled(true);
		m_stopButton->SetValue(!isPlaying);
		m_pauseButton->SetEnabled(true);
		m_pauseButton->SetValue(CPlayerControl::PauseState(m_document));

		m_timeCtl->SetDocument(m_document);
		if (isPlaying)
			m_timeCtl->Started();
		else
			m_timeCtl->Stopped();

		m_tempoCtl->SetDocument(m_document);
		if (isPlaying)
			m_tempoCtl->SetTempo(CPlayerControl::Tempo(m_document));
		else
			m_tempoCtl->SetTempo(m_document->InitialTempo());
	}
	else
	{
		m_playButton->SetEnabled(false);
		m_playButton->SetValue(false);
		m_stopButton->SetEnabled(false);
		m_stopButton->SetValue(true);
		m_pauseButton->SetEnabled(false);
		m_pauseButton->SetValue(false);

		m_timeCtl->Stopped();
		m_timeCtl->SetDocument(NULL);

		m_tempoCtl->SetDocument(NULL);
		m_tempoCtl->SetTempo(CPlayerControl::Tempo(NULL));
	}
}

void
CTransportWindow::MessageReceived(
	BMessage *message)
{
	if (m_track == NULL)
		return;

	switch(message->what)
	{
		case MENU_PLAY_FROM_START:
		{
			// REM: Add ability to play individual track...
			CPlayerControl::PlaySong(m_document, 0, 0,
									 LocateTarget_Real, -1,
									 SyncType_SongInternal, 0);
			break;
		}
		case MENU_PLAY:
		{
			PlaybackState pbState;
			if ((CPlayerControl::GetPlaybackState(m_document, pbState))
			 && pbState.running)
			{
				if (CPlayerControl::PauseState(m_document))
					CPlayerControl::SetPauseState(m_document, false);
				else
					CPlayerControl::SetPauseState(m_document, true);
			}
			else
			{
				// REM: Add ability to play individual track...
				if (dynamic_cast<CMeVApp *>(be_app)->GetLoopFlag())
				{
					CPlayerControl::PlaySong(m_document, 0, 0, LocateTarget_Real,
											 -1, SyncType_SongInternal,
											 PB_Loop);
				}
				else
				{
					CPlayerControl::PlaySong(m_document, 0, 0, LocateTarget_Real,
											 -1, SyncType_SongInternal, 0);
				}
			}
			break;
		}
		case MENU_PAUSE:
		{
			CPlayerControl::SetPauseState(m_document, m_pauseButton->Value());
			break;
		}
		case MENU_LOCATE_END:
		{
			SetButtons();
			break;
		}
		case MENU_STOP:
		{
			CPlayerControl::StopSong(m_document);
			break;
		}
		case CTempoEditControl::TEMPO_CHANGED:
		{
			double tempo = m_tempoCtl->Tempo();
			if (m_document)
				m_document->SetInitialTempo(tempo);
			CPlayerControl::SetTempo(m_document, tempo);
			break;
		}	
		case Player_ChangeTransportState:
		{
			SetButtons();
			break;
		}
		case Player_ChangeTempo:
		{
			m_tempoCtl->SetTempo(CPlayerControl::Tempo(m_document));
			break;
		}
		case Update_ID:
		case Delete_ID:
		{
			CObserver::MessageReceived(message);
			break;
		}
		default:
		{
			CAppWindow::MessageReceived(message);
		}
	}
}

void
CTransportWindow::WatchTrack(
	CEventTrack *track)
{
	if (track != m_track)
	{
		CMeVDoc *oldDoc = m_document;
		m_track = track;
		Lock();

		m_document = track ? &track->Document() : NULL;
		if (m_document != oldDoc)
			SetButtons();

		Unlock();
		SetSubject(m_track);
	}
}

void
CTransportWindow::OnDeleteRequested(
	BMessage *message)
{
	WatchTrack(NULL);
}

void
CTransportWindow::OnUpdate(
	BMessage *message)
{
	SetButtons();
}

// END - TransportWindow.cpp
