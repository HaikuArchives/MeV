/* ===================================================================== *
 * ChannelManagerView.h (MeV/User Interface)
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
 *		Dan Walton (dwalton)
 *
 * ---------------------------------------------------------------------
 * Purpose:
 *  
 * ---------------------------------------------------------------------
 * History:
 * 6/21/00 Original Implementation: Dan Walton
 * 
 * ---------------------------------------------------------------------
 * To Do:
 * When no destination is selected and one tries to add an event.  We crash.
 * ===================================================================== */
#ifndef __C_ChannelSelectorView_H__
#define __C_ChannelSelectorView_H__
#include "EventTrack.h"
#include "TextDisplay.h"
#include "View.h"
#include "IconMenuItem.h"
#include "VCTableManager.h"
#include <PopUpMenu.h>
#include <Button.h>
#include <CheckBox.h>
#include <StringView.h>
//stl
#include <map.h>

#include "VChannelModifier.h"
class CChannelManagerView : 
	public BView {

private:
	uint8			m_selected_id;
	uint8			m_default_id;
	VChannelEntry *m_vc;//selected vc.
	CEventTrack		*track;
	uint8			channel;
	CTextDisplay	*nameView;
	BPopUpMenu		*m_vcMenu;
	BButton			*m_editButton;
	BButton			*m_deleteButton;
	BStringView	    *m_channel;
	CTextDisplay	*m_channelValue;
	BStringView  	*m_port;
	CTextDisplay	*m_portName;
	BCheckBox		*m_mute;
	BCheckBox		*m_lock;
	map <int,CVChannelModifier *> m_modifierMap;
	CVCTableManager *m_vcTableM;
	
	
//	+-------------------------------+ +---------+
//	| m_vcMenu                      | | editbut. |
//	+-------------------------------+ +---------+
//	Port:		ObjektSynth
//	Channel:	1
//		[x] Mute				[x] Lock	
	virtual void AttachedToWindow();
	//update the info on selected channel;
public:
		/**	Constructor */
	CChannelManagerView(BRect 		inFrame,
						CTextDisplay	*inNameView,
							uint32		inResizingMode = B_FOLLOW_LEFT | B_FOLLOW_RIGHT,
							uint32		inFlags = B_WILL_DRAW );
							
	~CChannelManagerView() { CRefCountObject::Release( track ); }

		/**	Select which track we are looking at, so we can draw channel
			array properly.
		*/
	void SetTrack( CEventTrack *inTrack );
	void Update();
	virtual void MessageReceived(BMessage *msg);
		/**	Set which channel is selected. */
	void SetChannel( uint8 inChannel );
};
#endif /* __C_ChannelSelectorView_H__ */

