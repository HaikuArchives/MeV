/* ===================================================================== *
 * ChannelSelectorView.h (MeV/User Interface)
 * ---------------------------------------------------------------------
 * License:
 *  The contents of this file are subject to the Mozilla Public
 *  License Version 1.1 (the "License"); you may not use this file
 *  except in compliance with the License. You may obtain a copy of
 *  the License at http://www.mozilla.org/MPL/
 *
 *  Software distributed under the License is distributed on an "AS
 *  IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 *  implied. See the License for the specific language governing
 *  rights and limitations under the License.
 *
 *  The Original Code is MeV (Musical Environment) code.
 *
 *  The Initial Developer of the Original Code is Sylvan Technical 
 *  Arts. Portions created by Sylvan are Copyright (C) 1997 Sylvan 
 *  Technical Arts. All Rights Reserved.
 *
 *  Contributor(s): 
 *		Christopher Lenz (cell)
 *
 * ---------------------------------------------------------------------
 * Purpose:
 *  
 * ---------------------------------------------------------------------
 * History:
 *	1997		Talin
 *		Original implementation
 *	04/08/2000	cell
 *		General cleanup in preparation for initial SourceForge checkin
 *  05/10/2000  dwalton
 *		Separated from Inspector Window.
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */
#ifndef __C_ChannelSelectorView_H__
#define __C_ChannelSelectorView_H__
#include "EventTrack.h"
#include "TextDisplay.h"

class CChannelSelectorView : 
	public BControl {

private:

	CEventTrack		*track;
	uint8			channel;
	CTextDisplay		*nameView;

		/**	Invalidate the rectangle surrounding a particular channel */
	void InvalidateChannel( int32 inChannel );
	void MouseDown( BPoint point );
	void Draw( BRect r );
	void AttachedToWindow() { SetViewColor( B_TRANSPARENT_32_BIT ); }

public: 
		/**	Constructor */
	CChannelSelectorView(		BRect 		inFrame,
							BMessage		*inMessage,
							CTextDisplay	*inNameView,
							uint32		inResizingMode = B_FOLLOW_LEFT | B_FOLLOW_RIGHT,
							uint32		inFlags = B_WILL_DRAW );
							
	~CChannelSelectorView() { CRefCountObject::Release( track ); }

		/**	Select which track we are looking at, so we can draw channel
			array properly.
		*/
	void SetTrack( CEventTrack *inTrack )
	{
		if (track != inTrack)
		{
			CRefCountObject::Release( track );
			track = inTrack;
			if (track) track->Acquire();
			if (Window())
			{
				Window()->Lock();
				Invalidate();
				Window()->Unlock();
			}
		}
	}
	
		/**	Set which channel is selected. */
	void SetChannel( uint8 inChannel );
};
#endif /* __C_ChannelSelectorView_H__ */

