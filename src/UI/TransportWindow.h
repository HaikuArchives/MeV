/* ===================================================================== *
 * TransportWindow.h (MeV/User Interface)
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
 *  Playback controls window
 * ---------------------------------------------------------------------
 * History:
 *	1997		Talin
 *		Original implementation
 *	04/08/2000	cell
 *		General cleanup in preparation for initial SourceForge checkin
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __C_TransportWindow_H__
#define __C_TransportWindow_H__

#include "WindowState.h"
#include "Observer.h"

class CTimeEditControl;
class CTextSlider;
class CEventTrack;
class CMeVDoc;

class CTransportWindow : public CAppWindow, public CObserver {
	BControl			*playButton,
					*stopButton,
					*pauseButton,
					*recordButton,
					*loopButton,
					*punchButton;
	CTimeEditControl	*realTimeDisplay,
					*meteredTimeDisplay;
	CTextSlider		*tempoSlider;
	bool				finalTempo;
	CEventTrack		*track;
	CMeVDoc			*document;

	void MessageReceived( BMessage* theMessage );
	void SetButtons();

		/**	If the app wants us to stop looking at the track, then oblige it.
			Overridden from the CObserver class.
		*/
	void OnDeleteRequested( BMessage *inMsg );

		/**	Update inspector info when we get an observer update message.
			Overridden from the CObserver class.
		*/
	void OnUpdate( BMessage *inMsg );

public:

	CTransportWindow( BPoint pos, CWindowState &inState );
	
	void UpdateDisplay();

		/**	Handle transport functions for the current track */
	void WatchTrack( CEventTrack *inTrack );
};

#endif /* __C_TransportWindow_H__ */
