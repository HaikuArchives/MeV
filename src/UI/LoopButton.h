/* ===================================================================== *
 * LoopButton.h (MeV/UI)
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

#ifndef __C_LoopButton_H__
#define __C_LoopButton_H__

#include "TransportButton.h"

/**	CTransportButton subclass to switches loop modes between 'looping' and
 *	'not looping'.
 */
class CLoopButton
	:	public CTransportButton
{

public:							// Constructor/Destructor

								CLoopButton(
									BRect frame,
									const char *name,
									BBitmap *normalBitmap,
									BBitmap *disabledBitmap,
									BBitmap *pressedBitmap,
									BBitmap *normalLoopingBitmap,
									BBitmap *pressedLoopingBitmap,
									BMessage *message,
									uint32 key = 0,
									uint32 modifiers = 0,
									uint32 resizeFlags = B_FOLLOW_LEFT | B_FOLLOW_TOP);
	
public:							// Accessors

	bool						IsLooping() const
								{ return m_looping; }
	void						SetLooping(
									bool looping = true);

protected:						// CTransportButton Implementation
	
	virtual void				DonePressing();

	virtual BBitmap *			MakeBitmap(
									uint32 mask);

	virtual uint32				ModeMask() const;

private:						// Constants

	enum {
		LOOPING_MASK = 0x4
	};

private:						// Instance Data

	BBitmap *					m_normalLoopingBitmap;
	BBitmap *					m_pressedLoopingBitmap;

	bool						m_looping;
};

#endif /* __C_LoopButton_H__ */
