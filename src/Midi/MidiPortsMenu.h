/* ===================================================================== *
 * MidiPortsMenu.h (MeV/Midi)
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
 * History:
 *	12/01/2000	cell
 *		Original implementation
 * ===================================================================== */

#ifndef __C_MidiPortsMenu_H__
#define __C_MidiPortsMenu_H__

// Application Kit
#include <Messenger.h>
// Interface Kit
#include <PopUpMenu.h>

namespace Midi
{

class CMidiDestination;

class CMidiPortsMenu
	:	public BPopUpMenu
{

public:						// Constants

	enum messages
	{
							CONSUMER_SELECTED = 'mcmA',

							PRODUCER_SELECTED
	};

public:						// Constructor/Destructor

							CMidiPortsMenu(
								CMidiDestination *destination);

	virtual					~CMidiPortsMenu();

public:						// Operations

	void					SetTarget(
								BMessenger target)
							{ m_target = target; }

public:						// BPopUpMenu Implementation

	virtual void			AttachedToWindow();

private:					// Internal Operations

	void					_update();

private:					// Instance Data

	CMidiDestination *		m_destination;

	BMessenger				m_target;
};

};

#endif /* __C_MidiPortsMenu_H__ */
