/* ===================================================================== *
 * ReconnectingMidiProducer.h (MeV/Midi)
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
 *		Dan Walton
 *
 * ---------------------------------------------------------------------
 * History:
 *	5/19/2000 dwalton
 *		Original implementation
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __C_ReconnectingMidiProducer_H__
#define __C_ReconnectingMidiProducer_H__

// Midi Kit
#include <MidiProducer.h>
// Storage Kit
#include <Mime.h>

class BBitmap;

namespace Midi
{

/**	A midiproducer that keeps track of all its connections to unregistered 
 *  midiconsumer using names.
 *	@author	Dan Walton
 */
class CReconnectingMidiProducer
	:	public BMidiLocalProducer
{

public:							// Constructor/Destructor

								CReconnectingMidiProducer(
									const char *name);

public:							// Accessors

	BBitmap *					GetSmallIcon() const;

	BBitmap *					GetLargeIcon() const;

	void						SetSmallIcon(
									BBitmap *icon);

	void						SetLargeIcon(
									BBitmap *icon);

public:							// Operations

	void						AddToConnectList(
									const char *name);

	void						RemoveFromConnectList(
									const char *name);

	bool						IsInConnectList(
									const char *name) const;

private:						// Internal Operations

	BBitmap *					_createIcon(
									const BMessage *message,
									icon_size which) const;
}; 

};

#endif /* __C_ReconnectingMidiProducer_H__ */
