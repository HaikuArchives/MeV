/* ===================================================================== *
 * InspectorWindow.h (MeV/User Interface)
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
 *		Separated ChannelSelector View.
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __C_InspectorWindow_H__
#define __C_InspectorWindow_H__

#include "TextDisplay.h"
#include "WindowState.h"
#include "Observer.h"
#include "EventEditor.h"
#include "EventTrack.h"
#include "MeVApp.h"
#include "DestinationListView.h"
//#include "ChannelSelectorView.h"

class CTextDisplay;
class CMeVDoc;
class CTextSlider;

class CInspectorWindow :
	public CAppWindow,
	public CObserver
{

public:						// Constants

	static const BRect		DEFAULT_DIMENSIONS;

public:						// Constructor/Destructor

							CInspectorWindow(
								BPoint position,
								CWindowState &state);

public:						// CObserver Implementation

	virtual void			MessageReceived(
								BMessage *message);

	// If the app wants us to stop looking at the track, then oblige it.
	virtual void			OnDeleteRequested(
								BMessage *message);

	// Update inspector info when we get an observer update message.
	virtual void			OnUpdate(
								BMessage *message);

public:						// Operations

	// Inspect the current event of the track
	void					WatchTrack(
								CEventTrack *track);

	void					Clear();

private:					// Instance Data
	virtual void            MenusBeginning();
	CMeVDoc					*m_doc;
	CEventTrack				*m_track;

	CTextDisplay			*m_eventTypeView;
	CTextDisplay			*m_channelNameView;
	//CChannelSelectorView	*m_channelControl;
	CDestinationListView	   *m_channelControl;
	BStringView				*m_vLabel[3];
	CTextSlider				*m_vSlider[3];			// Three sliders
	int32					m_baseValue[3];
	int32					m_previousValue;
	E_EventAttribute		m_editedAttr[3];
};

#endif /* __C_InspectorWindow_H__ */
