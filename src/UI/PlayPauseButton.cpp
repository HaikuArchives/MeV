/* ===================================================================== *
 * PlayPauseButton.cpp (MeV/UI)
 * ===================================================================== */

#include "PlayPauseButton.h"

// Application Kit
#include <MessageRunner.h>
// Interface Kit
#include <Window.h>

// ---------------------------------------------------------------------------
// Constants

const bigtime_t PAUSE_BLINK_PERIOD	= 600 * 1000;
const uint32 	PAUSE_BLINK			= 'ppbB';

// ---------------------------------------------------------------------------
// Constructor/Destructor

CPlayPauseButton::CPlayPauseButton(
	BRect frame,
	const char *name,
	BBitmap *normalBitmap,
	BBitmap *disabledBitmap,
	BBitmap *pressedBitmap,
	BBitmap *normalPlayingBitmap,
	BBitmap *pressedPlayingBitmap,
	BBitmap *normalPausedBitmap,
	BBitmap *pressedPausedBitmap,
	BMessage *message,
	uint32 key,
	uint32 modifiers,
	uint32 resizingMode)
	:	CTransportButton(frame, name,
						 normalBitmap, disabledBitmap, pressedBitmap,
						 message, 0, key, modifiers, 
						 resizingMode),
		m_normalPlayingBitmap(normalPlayingBitmap),
		m_pressedPlayingBitmap(pressedPlayingBitmap),
		m_normalPausedBitmap(normalPausedBitmap),
		m_pressedPausedBitmap(pressedPausedBitmap),
		m_playState(STOPPED),
		m_lastPauseBlinkTime(0),
		m_lastModeMask(0),
		m_blinkMessageRunner(NULL)
{
}

// ---------------------------------------------------------------------------
// Operations

void 
CPlayPauseButton::SetStopped()
{
	if (m_playState == STOPPED || m_playState == ABOUT_TO_PLAY)
		return;

	m_playState = STOPPED;
	Invalidate();

	if (m_blinkMessageRunner)
	{
		delete m_blinkMessageRunner;
		m_blinkMessageRunner = NULL;
	}
}

void 
CPlayPauseButton::SetPlaying()
{
	if (m_playState == PLAYING || m_playState == ABOUT_TO_PAUSE)
		return;

	m_playState = PLAYING;
	Invalidate();

	if (m_blinkMessageRunner)
	{
		delete m_blinkMessageRunner;
		m_blinkMessageRunner = NULL;
	}
}

void 
CPlayPauseButton::SetPaused()
{
	if (m_playState == ABOUT_TO_PLAY)
		return;

	// in paused m_playState blink the LED on and off
	if (m_playState == PAUSED_LED_ON || m_playState == PAUSED_LED_OFF)
	{
		if (m_playState == PAUSED_LED_ON)
			m_playState = PAUSED_LED_OFF;
		else
			m_playState = PAUSED_LED_ON;
	}
	else
	{
		m_playState = PAUSED_LED_ON;
	}
	
	if (!m_blinkMessageRunner)
	{
		BMessenger messenger(this, Window());
		m_blinkMessageRunner = new BMessageRunner(messenger,
												  new BMessage(PAUSE_BLINK),
												  PAUSE_BLINK_PERIOD);
	}
	Invalidate();
}

// ---------------------------------------------------------------------------
// CTransportButton Implementation

BBitmap *
CPlayPauseButton::MakeBitmap(
	uint32 mask)
{
	switch (mask)
	{
		case PLAYING_MASK:
			return m_normalPlayingBitmap;
		case PLAYING_MASK | PRESSED_MASK:
			return m_pressedPlayingBitmap;
		case PAUSED_MASK:
			return m_normalPausedBitmap;
		case PAUSED_MASK | PRESSED_MASK:
			return m_pressedPausedBitmap;
		default:
			return CTransportButton::MakeBitmap(mask);
	}	

	return 0;
}

void
CPlayPauseButton::MessageReceived(
	BMessage *message) {

	switch (message->what)
	{
		case PAUSE_BLINK:
		{
			if ((m_playState == PAUSED_LED_ON)
			 || (m_playState == PAUSED_LED_OFF))
				SetPaused();
			break;
		}
		default:
		{
			CTransportButton::MessageReceived(message);
		}
	}
}

uint32 
CPlayPauseButton::ModeMask() const
{
	if (!IsEnabled())
		return DISABLED_MASK;
	
	uint32 result = 0;

	if (Value())
		result = PRESSED_MASK;

	if (m_playState == PLAYING || m_playState == ABOUT_TO_PLAY)
		result |= PLAYING_MASK;
	else if (m_playState == ABOUT_TO_PAUSE || m_playState == PAUSED_LED_ON)
		result |= PAUSED_MASK;

	return result;
}

void 
CPlayPauseButton::StartPressing()
{
	if (m_playState == PLAYING)
		m_playState = ABOUT_TO_PAUSE;
	else
	 	m_playState = ABOUT_TO_PLAY;

	CTransportButton::StartPressing();
}

void 
CPlayPauseButton::MouseCancelPressing()
{
	if (m_playState == ABOUT_TO_PAUSE)
	 	m_playState = PLAYING;
	else
		m_playState = STOPPED;
	
	CTransportButton::MouseCancelPressing();
}

void 
CPlayPauseButton::DonePressing()
{
	if (m_playState == ABOUT_TO_PAUSE) {
	 	m_playState = PAUSED_LED_ON;
		m_lastPauseBlinkTime = system_time();
	}
	else if (m_playState == ABOUT_TO_PLAY)
	{
		m_playState = PLAYING;
	}

	CTransportButton::DonePressing();
}

// END - PlayPauseButton.cpp
