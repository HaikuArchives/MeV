/* ===================================================================== *
 * MidiDestination.h (MeV/Midi)
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
 *		Dan Walton (dwalton)
 *
 * ---------------------------------------------------------------------
 * History:
 *	1997		Joe Pearce
 *		Original implementation
 *	04/08/2000	cell
 *		General cleanup in preparation for initial SourceForge checkin
 *  07/28/2000  dwalton
 *      Changed from struct to class.
 *		Added producer pointer.
 *	11/10/2000	cell
 *		Separated MIDI functionality from CDestination class
 * ===================================================================== */

#ifndef __C_MidiDestination_H__
#define __C_MidiDestination_H__

#include "Destination.h"

#define NOTE_NAME_LENGTH 128
#define PROGRAM_NAME_LENGTH 128

class CMeVDoc;

namespace Midi {

class CReconnectingMidiProducer;

/**	Destinations, allow routing and remapping of MIDI data
	upon playback.
	@author		Talin, Dan Walton, Christopher Lenz
	@package	Engine
 */
class CMidiDestination
	:	public CDestination
{

public:							// Constants

	enum update_hints
	{
								Update_Channel 		= (1 << 4),

								Update_Connections 	= (1 << 5)
	};

public:							//Constructor/Destructor

								CMidiDestination(
									long id,
									const char *name,
									CMeVDoc *document);

								CMidiDestination(
									CMeVDoc *document);

	virtual						~CMidiDestination();

public:							// Accessors

	/**	Copies a string identifying a note number into outName.
	 *	outName should point to a string buffer of at least
	 *	NOTE_NAME_LENGTH bytes.
	 *	@return		true if a name for the note exists.
	 */
	bool						GetNoteName(
									unsigned char note,
									char *outName);

	/**	Copies the program name at the given bank and program numbers
	 *	into outName. outName should point to a string buffer of at least
	 *	PROGRAM_NAME_LENGTH bytes.
	 *	@return		true if the program could be identified.
	 */
	bool						GetProgramName(
									unsigned short bank,
									unsigned char program,
									char *outName);

	bool						IsConnectedTo(
									BMidiConsumer *consumer) const;
	BMidiConsumer *				ConnectedTo() const;

	void						ConnectTo(
									BMidiConsumer *consumer);
	void						ConnectTo(
									int32 consumerID);
	void						ConnectTo(
									const char *name);

	void						Disconnect();


	BMidiLocalProducer *		Producer() const
								{ return m_producer; }

	uint8						Channel() const
								{ return m_channel; }
	void 						SetChannel(
									uint8 channel);

public:							// CDestination Implementation

	virtual status_t			GetIcon(
									icon_size which,
									BBitmap *outIcon);

	virtual void				ReadChunk(
									CIFFReader &reader);

	virtual void				Serialize(
									CIFFWriter &writer);

protected:

	virtual void				ColorChanged(
									rgb_color color);

	virtual void				Deleted();

	virtual void				Disabled(
									bool disabled);

	virtual void				Muted(
									bool muted);

	virtual void				NameChanged(
									const char *name);

	virtual void				Soloed(
									bool solo);

	virtual void				Undeleted();

private:   						// Internal Operations

	void 						_addIcons(
									BMessage* msg,
									BBitmap* largeIcon,
									BBitmap* miniIcon) const;

	BBitmap * 					_createIcon(
									BRect r);

	void						_updateIcons();

private:						// Instance Data

	/** The associated MIDI producer. */
	BMidiLocalProducer *		m_producer;					

	/** ID of the consumer the producer has been connected to via MeV. */
	int32						m_consumerID;

	/** MIDI channel. */
	uint8						m_channel;

	/** Whether or not this destination supports General Midi. */
	bool						m_generalMidi;
};

};

#endif /* __C_MidiDestination_H__ */
