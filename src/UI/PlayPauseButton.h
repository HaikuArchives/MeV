/* ===================================================================== *
 * PlayPauseButton.h (MeV/UI)
 * ---------------------------------------------------------------------
 * License:
 *	Be Sample Code License
 *
 *	Copyright 1991-1999, Be Incorporated.
 *	All rights reserved.
 *
 *	Redistribution and use in source and binary forms, with or without
 *	modification, are permitted provided that the following conditions
 *	are met:
 *
 *	1. Redistributions of source code must retain the above copyright
 *	   notice, this list of conditions, and the following disclaimer.
 *
 *	2. Redistributions in binary form must reproduce the above copyright
 *	   notice, this list of conditions, and the following disclaimer in the
 *	   documentation and/or other materials provided with the distribution.
 *
 *	3. The name of the author may not be used to endorse or promote products
 *	   derived from this software without specific prior written permission.
 *
 *	THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR
 *	IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 *	OF TITLE, NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *	PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 *	DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 *	AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *	TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *	OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.    
 *
 *  Contributor(s): 
 *		Christopher Lenz (cell)
 *
 * ---------------------------------------------------------------------
 * History:
 *	07/08/2000	cell
 *		Based on Be, Inc. sample code (media_kit/MediaFile/mplay)
 * ===================================================================== */

#ifndef __C_PlayPauseButton_H__
#define __C_PlayPauseButton_H__

#include "TransportButton.h"

/**
 *		CTransportButton subclass that knows about playing and paused 
 *		states. Blinks the pause LED during paused state.
 *		@author	Christoper Lenz.   
 */
 
class CPlayPauseButton
	:	public CTransportButton
{

public:							// Constructor/Destructor

								CPlayPauseButton(
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
									uint32 key = 0,
									uint32 modifiers = 0,
									uint32 resizeFlags = B_FOLLOW_LEFT | B_FOLLOW_TOP);
	
public:							// Operations

	void						SetStopped();

	void						SetPlaying();

	void						SetPaused();

protected:						// CTransportButton Implementation
	
	virtual void				DonePressing();

	virtual BBitmap *			MakeBitmap(
									uint32 mask);

	virtual void				MessageReceived(
									BMessage *message);

	virtual uint32				ModeMask() const;

	virtual void				MouseCancelPressing();

	virtual void				StartPressing();

private:						// Constants

	enum play_button_state {
		STOPPED,
		ABOUT_TO_PLAY,
		PLAYING,
		ABOUT_TO_PAUSE,
		PAUSED_LED_ON,
		PAUSED_LED_OFF
	};

	enum {
		PLAYING_MASK = 0x4,
		PAUSED_MASK = 0x8
	};

private:						// Instance Data

	BBitmap *					m_normalPlayingBitmap;
	BBitmap *					m_pressedPlayingBitmap;
	BBitmap *					m_normalPausedBitmap;
	BBitmap *					m_pressedPausedBitmap;
	
	int32						m_playState;
	bigtime_t					m_lastPauseBlinkTime;
	uint32						m_lastModeMask;

	BMessageRunner *			m_blinkMessageRunner;
};

#endif /* __C_PlayPauseButton_H__ */
