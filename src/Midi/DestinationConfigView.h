/* ===================================================================== *
 * DestinationConfigView.h (MeV/Midi)
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
 *	10/13/2000	cell
 *		Original implementation
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __C_MidiConfigView_H__
#define __C_MidiConfigView_H__

#include "ConsoleView.h"

// STL
#include <map>

class BPopUpMenu;
class BTextControl;

namespace Midi
{

class CMidiDestination;

class CDestinationConfigView
	:	public CConsoleView
{

public:						// Constants

	enum messages
	{
							PORT_SELECTED = 'msvA',

							CHANNEL_SELECTED,
	};

public:						// Constructor/Destructor

							CDestinationConfigView(
								BRect frame,
								CMidiDestination *destination);

	virtual					~CDestinationConfigView();

public:						// Accessors

	CMidiDestination *		Destination() const
							{ return m_destination; }

public:						// CConsoleView Implementation

	virtual void			AttachedToWindow();

	virtual void			GetPreferredSize(
								float *width,
								float *height);

	virtual void			MessageReceived(
								BMessage *message);

	virtual bool			SubjectReleased(
								CObservable *subject);

	virtual void			SubjectUpdated(
								BMessage *message);

private:					// Internal Operations

	void					_updatePortMenu();

private:					// Instance Data

	CMidiDestination *		m_destination;

	BPopUpMenu *			m_channelMenu;
	BPopUpMenu *			m_portMenu;
};

};

#endif /* __C_DestinationConfigView_H__ */
