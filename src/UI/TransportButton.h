/* ===================================================================== *
 * TransportButton.h (MeV/UI)
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

#ifndef __C_TransportButton_H__
#define __C_TransportButton_H__

// Interface Kit
#include <Control.h>

class BMessage;
class BMessageRunner;
class BBitmap;
class CBitmapStash;

/**
 *		Media buttons for transport control.
 *		CTransportButton must be installed into a window with 
 *		B_ASYNCHRONOUS_CONTROLS on.
 *		@author	Christoper Lenz.  
 */
 
class CTransportButton
	:	public BControl
{
	friend class SkipButtonKeypressFilter;
	friend class CBitmapStash;

public:							// Constructor/Destructor

								CTransportButton(
									BRect frame,
									const char *name,
									BBitmap *normalBitmap,
									BBitmap *disabledBitmap,
									BBitmap *pressedBitmap,
									BMessage *message,
									bigtime_t period = 0,
									uint32 key = 0,
									uint32 modifiers = 0,
									uint32 resizeFlags = B_FOLLOW_LEFT | B_FOLLOW_TOP);
	
	virtual						~CTransportButton();

public:							// Accessors

	void						SetStartPressingMessage(
									BMessage *message);

	void						SetPressingMessage(
									BMessage *message);

	void						SetDonePressingMessage(
									BMessage *message);

	void						SetPressingPeriod(
									bigtime_t period);

public:							// BControl Implementation

	virtual void				AttachedToWindow();

	virtual void				DetachedFromWindow();

	virtual void				Draw(
									BRect updateRect);

	virtual void				MouseDown(
									BPoint point);

	virtual	void				MouseMoved(
									BPoint point,
									uint32 transit,
									const BMessage *message);

	virtual	void				MouseUp(
									BPoint point);

	virtual void				SetEnabled(
									bool enabled = true);

	virtual	void				WindowActivated(
									bool active);

protected:						// Internal Hooks

	enum {
		DISABLED_MASK = 0x1,
		PRESSED_MASK = 0x2
	};
	
	/**	Lazy bitmap builder.	*/
	virtual BBitmap *			MakeBitmap(
									uint32 mask);
	
	/**	Mode mask corresponding to the current button state
			- determines which bitmap will be used.	*/
	virtual uint32				ModeMask() const;

	/**	Overriding class can add swapping between two pairs of bitmaps, etc.	*/
	virtual void				StartPressing();

	virtual void				MouseCancelPressing();

	virtual void				DonePressing();

private:						// Internal Operations

	void						ShortcutKeyDown();

	void						ShortcutKeyUp();
	
	void						MouseStartPressing();

	void						MouseDonePressing();

private:						// Instance Data

	CBitmapStash *				m_bitmaps;

	BBitmap	*					m_normalBitmap;
	BBitmap *					m_disabledBitmap;
	BBitmap *					m_pressedBitmap;

	BMessage *					m_startPressingMessage;
	BMessage *					m_pressingMessage;
	BMessage *					m_donePressingMessage;
	bigtime_t					m_pressingPeriod;
	
	bool						m_mouseDown;
	bool						m_keyDown;

	BMessageFilter *			m_keyPressFilter;
	BMessageRunner *			m_messageSender;
};

#endif /* __C_TransportButton_H__ */
