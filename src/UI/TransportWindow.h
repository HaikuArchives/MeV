/* ===================================================================== *
 * TransportWindow.h (MeV/UI)
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

#include "AppWindow.h"
#include "Observer.h"

class CEventTrack;
class CLoopButton;
class CMeVDoc;
class CPlayPauseButton;
class CTempoEditControl;
class CTimeEditControl;
class CTextSlider;
class CTransportButton;

class CTransportWindow
	:	public CAppWindow
{

public:							// Constructor/Destructor

								CTransportWindow(
									BPoint pos,
									CWindowState &state);

public:							// CAppWindow Implementation

	virtual void				MessageReceived(
									BMessage *message);

	void						SetButtons();

	/**	If the app wants us to stop looking at the track, then oblige it.
		Overridden from the CObserver class.
	*/
	virtual void				SubjectReleased(
									CObservable *subject);

	/**	Update inspector info when we get an observer update message.
		Overridden from the CObserver class.
	*/
	virtual void				SubjectUpdated(
									BMessage *message);

public:

	/**	Handle transport functions for the current track */
	void						WatchTrack(
									CEventTrack *track);

private:						// Instance Data

	CMeVDoc	*					m_document;

	CEventTrack *				m_track;

	CTransportButton *			m_skipBackButton;
	CTransportButton *			m_stopButton;
	CPlayPauseButton *			m_playPauseButton;
	CTransportButton *			m_recordButton;
	CTransportButton *			m_skipForwardButton;
	CLoopButton *				m_loopButton;

	CTempoEditControl *			m_tempoCtl;

	CTimeEditControl *			m_timeCtl;
};

#endif /* __C_TransportWindow_H__ */
