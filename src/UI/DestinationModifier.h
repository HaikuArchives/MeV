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
 *		Christopher Lenz (cell)
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

#include "MidiManager.h"
#include "Observer.h"

// Application Kit
#include <Message.h>
// Interface Kit
#include <ColorControl.h> 
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <StringView.h>
#include <TextControl.h>
#include <View.h>
#include <Window.h>

class Destination;
class CDestinationList;

class CDestinationModifier
	:	public BWindow,
		public CObserver
{

public:							// Constants

	enum messages
	{
								NAME_CHANGED = 'dmoA',

								PORT_SELECTED,

								CHANNEL_SELECTED,

								MUTED,

								SOLOED,

								COLOR_CHANGED,

								NOTIFY,

								ADD_ID,

								WINDOW_CLOSED
	};

public:							// Constructor/Destructor

								CDestinationModifier(
									BRect frame,
									int32 id,
									CDestinationList *tm,
									BHandler *parent);

public:							// Operations

	void						Update();

public:							// BWindow Implementation

	virtual void				MenusBeginning();

	virtual void				MenusEnded();

	virtual void				MessageReceived(BMessage *msg);

	virtual bool				QuitRequested();

public:							// CObserver Implementation

	virtual void				OnUpdate(
									BMessage *message);

private:						// Internal Operations

	void						_buildUI();

	void						_populatePortsMenu();

	void						_updateStatus();

private:						// Instance Data

	//pointer to the currently selected dest.
	Destination *				m_vc;

	int32						m_id;

	CDestinationList *			m_tm;

	CMidiManager *				m_midiManager;	

	BHandler *					m_parent;

	BView *						m_background;

	BTextControl *				m_name;

	BPopUpMenu *				m_midiPorts;

	BPopUpMenu *				m_channels;

	BCheckBox *					m_mute;

	BCheckBox *					m_solo;

	BColorControl *				m_colors;
};

#endif /* __C_DestinationModifier_H__ */
