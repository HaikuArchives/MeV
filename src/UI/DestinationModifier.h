/* ===================================================================== *
 * DestinationModifier.h (MeV/UI)
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
 * A class that manages the destinations.  
 * ---------------------------------------------------------------------
 * History:
 *	6/21/2000		dwalton
 *		Original implementation.
 *	8/01/2000		dwalton
 *      Name change, many improvements.
 * ---------------------------------------------------------------------
 * To Do:
 * 
 * ===================================================================== */

#ifndef __C_VChannelModifier_H__
#define __C_VChannelModifier_H__
#include <Window.h>
#include <View.h>
#include <MenuItem.h>
#include <Button.h>
#include <TextControl.h>
#include <ColorControl.h> 
#include <StringView.h>
#include <Message.h>
#include <PopUpMenu.h>
#include "MidiManager.h"
#include "Observer.h"
#include "DestinationList.h"
class CDestinationModifier :
	public BWindow,public CObserver {
private:
	Destination *m_vc;   //pointer to the currently selected dest.
	int32 m_id;
	BHandler *m_parent;
	//int m_selected_id;
	BView *m_background;
	BPopUpMenu *m_midiPorts;
	BPopUpMenu *m_channels;
	BTextControl *m_name;
	BCheckBox *m_mute;
	BCheckBox *m_solo;
	BButton *m_done;
	BButton *m_cancel;
	BStringView *m_status;
	BColorControl *m_colors;
	CMidiManager *m_midiManager;	
	void _buildUI();
	CDestinationList *m_tm;
	void _updateStatus();
	virtual void OnUpdate(BMessage *msg);
	
virtual void AttachedToWindow();
virtual void MenusBeginning();
virtual void MenusEnded();
void Update();
public:
	CDestinationModifier(BRect frame,int32 id,CDestinationList *tm,BHandler *parent);  //new vchannel;
	virtual void MessageReceived(BMessage *msg);
	virtual bool QuitRequested();
};
#endif
