/* ===================================================================== *
 * TransportWindow.cpp (MeV/UI)
 * ===================================================================== */

#include "TransportWindow.h"

#include "BorderView.h"
#include "EventTrack.h"
#include "Idents.h"
#include "LoopButton.h"
#include "MeVApp.h"
#include "MeVDoc.h"
#include "PlayerControl.h"
#include "PlayPauseButton.h"
#include "ResourceUtils.h"
#include "StdButton.h"
#include "TempoEditControl.h"
#include "TimeEditControl.h"
#include "TransportButton.h"

// Gnu C Library
#include <stdio.h>
// Interface Kit
#include <Bitmap.h>
#include <StringView.h>
// Support Kit
#include <Autolock.h>
#include <Debug.h>

// Debugging Macros
#define D_ALLOC(x) PRINT(x)		// Constructor/Destructor

// ---------------------------------------------------------------------------
// Constructor/Destructor

CTransportWindow::CTransportWindow(
	BPoint pos,
	CWindowState &state)
	: CAppWindow(state, BRect(pos.x, pos.y,
							  pos.x + Transport_Width,
							  pos.y + Transport_Height),
					"Transport",
					B_FLOATING_WINDOW,
					B_WILL_ACCEPT_FIRST_CLICK | B_AVOID_FOCUS
					| B_NOT_RESIZABLE | B_NOT_ZOOMABLE
					| B_ASYNCHRONOUS_CONTROLS,
					B_CURRENT_WORKSPACE),
	m_document(NULL),
	m_track(NULL)
{
	D_ALLOC(("CTransportWindow::CTransportWindow()\n"));

	BRect rect(Bounds());
	BBitmap *bitmap;

	rect.right++;
	rect.bottom++;
	BView *bg = new CBorderView(rect, NULL, B_FOLLOW_ALL_SIDES, B_WILL_DRAW, 
						 		false, NULL, CBorderView::BEVEL_BORDER);
	AddChild(bg);

	// add transport buttons
	bitmap = ResourceUtils::LoadImage("SkipBack");
	rect.SetLeftTop(BPoint(5.0, 5.0));
	rect.SetRightBottom(rect.LeftTop() + bitmap->Bounds().RightBottom());
	m_skipBackButton = new CTransportButton(rect, "Skip Backward",
						 					bitmap,
						 					ResourceUtils::LoadImage("SkipBack_Disabled"),
						 					ResourceUtils::LoadImage("SkipBack_Pressed"),
											new BMessage(MENU_LOCATE_START));
	bg->AddChild(m_skipBackButton);

	bitmap = ResourceUtils::LoadImage("Stop");
	rect.SetLeftTop(rect.RightTop() + BPoint(5.0, 0.0));
	rect.SetRightBottom(rect.LeftTop() + bitmap->Bounds().RightBottom());
	m_stopButton = new CTransportButton(rect, "Stop",
				 						bitmap,
						 				ResourceUtils::LoadImage("Stop_Disabled"),
						 				ResourceUtils::LoadImage("Stop_Pressed"),
										new BMessage(MENU_STOP));
	bg->AddChild(m_stopButton);

	bitmap = ResourceUtils::LoadImage("PlayPause");
	rect.SetLeftTop(rect.RightTop() + BPoint(5.0, 0.0));
	rect.SetRightBottom(rect.LeftTop() + bitmap->Bounds().RightBottom());
	m_playPauseButton = new CPlayPauseButton(rect, "Play/Pause",
						 					 bitmap,
						 					 ResourceUtils::LoadImage("PlayPause_Disabled"),
						 					 ResourceUtils::LoadImage("PlayPause_Pressed"),
											 ResourceUtils::LoadImage("PlayPause_Playing"),
						 					 ResourceUtils::LoadImage("PlayPause_Pressed"),
											 ResourceUtils::LoadImage("PlayPause_Paused"),
						 					 ResourceUtils::LoadImage("PlayPause_Pressed"),
											 new BMessage(MENU_PLAY));
	bg->AddChild(m_playPauseButton);

	bitmap = ResourceUtils::LoadImage("Record");
	rect.SetLeftTop(rect.RightTop() + BPoint(5.0, 0.0));
	rect.SetRightBottom(rect.LeftTop() + bitmap->Bounds().RightBottom());
	m_recordButton = new CTransportButton(rect, "Record",
				 						  bitmap,
						 				  ResourceUtils::LoadImage("Record_Disabled"),
						 				  ResourceUtils::LoadImage("Record_Pressed"),
										  new BMessage(MENU_STOP));
	bg->AddChild(m_recordButton);
	m_recordButton->SetEnabled(false);

	bitmap = ResourceUtils::LoadImage("SkipForward");
	rect.SetLeftTop(rect.RightTop() + BPoint(5.0, 0.0));
	rect.SetRightBottom(rect.LeftTop() + bitmap->Bounds().RightBottom());
	m_skipForwardButton = new CTransportButton(rect, "Skip Forward",
						 					   bitmap,
						 					   ResourceUtils::LoadImage("SkipForward_Disabled"),
						 					   ResourceUtils::LoadImage("SkipForward_Pressed"),
											   new BMessage(MENU_LOCATE_END));
	m_skipForwardButton->SetEnabled(false);
	bg->AddChild(m_skipForwardButton);

	bitmap = ResourceUtils::LoadImage("LoopOff");
	rect.left = rect.right + 5.0;
	rect.top = rect.top + rect.Height() / 2.0 - bitmap->Bounds().bottom / 2.0;
	rect.right = rect.left + bitmap->Bounds().right;
	rect.bottom = rect.top + bitmap->Bounds().bottom;
	m_loopButton = new CLoopButton(rect, "Loop",
						 		   bitmap,
						 		   ResourceUtils::LoadImage("LoopOff_Disabled"),
						 		   ResourceUtils::LoadImage("LoopOff_Pressed"),
						 		   ResourceUtils::LoadImage("LoopOn"),
						 		   ResourceUtils::LoadImage("LoopOn_Pressed"),
								   new BMessage('loop'));
	bg->AddChild(m_loopButton);

	// create time edit control
	rect = Bounds();
	rect.left += 5.0;
	rect.top = m_playPauseButton->Frame().bottom + 5.0;
	rect.right = rect.left + 86;
	rect.bottom = rect.top + 24;
	m_timeCtl = new CTimeEditControl(rect, new BMessage('strt'));
	m_timeCtl->SetEditable(false);

	// create tempo edit control
	rect.OffsetBy(rect.Width(), 0.0);
	m_tempoCtl = new CTempoEditControl(rect,
									   new BMessage(CTempoEditControl::TEMPO_CHANGED));

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
	if ((2 * maxWidth + 15.0) < m_loopButton->Frame().right)
	{
		m_tempoCtl->MoveTo(m_loopButton->Frame().right - maxWidth,
						   m_tempoCtl->Frame().top);
	}

	// resize window to preferred
	maxWidth = m_loopButton->Frame().right;
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

		m_skipBackButton->SetEnabled(true);
		m_playPauseButton->SetEnabled(true);
		if (isPlaying)
		{
			if (CPlayerControl::PauseState(m_document))
				m_playPauseButton->SetPaused();
			else
				m_playPauseButton->SetPlaying();
		}
		else
		{
			m_playPauseButton->SetStopped();
			m_timeCtl->SetTime(0L, 0);
		}

		m_stopButton->SetEnabled(true);

		m_loopButton->SetEnabled(true);
		if (dynamic_cast<CMeVApp *>(be_app)->GetLoopFlag())
			m_loopButton->SetLooping(true);
		else
			m_loopButton->SetLooping(false);

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
		m_skipBackButton->SetEnabled(false);
		m_playPauseButton->SetEnabled(false);
		m_stopButton->SetEnabled(false);
		m_loopButton->SetEnabled(false);

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
		case MENU_LOCATE_START:
		{
			PlaybackState	pbState;
			if ((CPlayerControl::GetPlaybackState(m_document, pbState))
			 && pbState.running)
			{
				bool paused = CPlayerControl::PauseState(m_document);
				// REM: Add ability to play individual track...
				CPlayerControl::PlaySong(m_document, 0, 0, LocateTarget_Real, -1, SyncType_SongInternal, 0);
				if (paused)
					CPlayerControl::SetPauseState(m_document, true);
			}
			SetButtons();
			break;
		}
		case MENU_PLAY:
		{
			PlaybackState	pbState;
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
			SetButtons();
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
			m_timeCtl->SetTime(0L, 0);
			SetButtons();
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
		case 'loop':
		{
			((CMeVApp *)be_app)->SetLoopFlag(m_loopButton->IsLooping());
			SetButtons();
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
	BAutolock lock(this);

	if (track != m_track)
	{
		if (m_track)
			m_track->RemoveObserver(this);

		CMeVDoc *oldDoc = m_document;
		m_track = track;

		m_document = track ? &track->Document() : NULL;
		if (m_document != oldDoc)
		{
			if (oldDoc != NULL)
				oldDoc->RemoveObserver(this);
			SetButtons();
		}

		m_track->AddObserver(this);
	}
}

void
CTransportWindow::SubjectReleased(
	CObservable *subject)
{
	D_OBSERVE(("CTransportWindow<%p>::SubjectReleased()\n", this));

	if (subject == m_track)
		WatchTrack(NULL);
}

void
CTransportWindow::SubjectUpdated(
	BMessage *message)
{
	SetButtons();
}

// END - TransportWindow.cpp
