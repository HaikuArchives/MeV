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
#include "Event.h"

// Standard Template Library
#include <list>
#include <map>
// Application Kit
#include <Messenger.h>
// Support Kit
#include <String.h>

#define CONTROLLER_NAME_LENGTH 128
#define NOTE_NAME_LENGTH 128
#define PROGRAM_NAME_LENGTH 128

class CMeVDoc;

class BMidiLocalProducer;

namespace Midi {

/**	Destinations, allow routing and remapping of MIDI data
	upon playback.
	@author		Talin, Dan Walton, Christopher Lenz
	@package	Engine
 */
class CMidiDestination
	:	public CDestination
{
	/** Enable Monitor views to register/unregister with the destination,
	 *	thus making multiple monitors possible.
	 */
	friend class CDestinationMonitorView;

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

	/**	Copies a string identifying a controller into outName.
	 *	outName should point to a string buffer of at least
	 *	CONTROLLER_NAME_LENGTH bytes.
	 *	@return		true if a name for the controller exists.
	 */
	bool						GetControllerName(
									unsigned char controller,
									char *outName) const;

	/**	Copies a string identifying a note number into outName.
	 *	outName should point to a string buffer of at least
	 *	NOTE_NAME_LENGTH bytes.
	 *	@return		true if a name for the note exists.
	 */
	bool						GetNoteName(
									unsigned char note,
									char *outName) const;

	/**	Copies the program name at the given bank and program numbers
	 *	into outName. outName should point to a string buffer of at least
	 *	PROGRAM_NAME_LENGTH bytes.
	 *	@return		true if the program could be identified.
	 */
	bool						GetProgramName(
									unsigned short bank,
									unsigned char program,
									char *outName) const;

	bool						IsConnectedTo(
									BMidiConsumer *consumer) const;
	BMidiConsumer *				ConnectedTo() const;

	uint8						Channel() const
								{ return m_channel; }
	void 						SetChannel(
									uint8 channel);

public:							// Operations

	void						ConnectTo(
									BMidiConsumer *consumer);
	void						ConnectTo(
									int32 consumerID);
	void						ConnectTo(
									const char *name);

	void						Disconnect();

public:							// CDestination Implementation

	virtual void				DoneLocating(
									bigtime_t when);

	virtual void				Execute(
									CEvent &event,
									bigtime_t when);

	virtual status_t			GetIcon(
									icon_size which,
									BBitmap *outIcon);

	virtual void				Interpolate(
									CEvent &ev,
									CEventStack &stack,
									long time,
									long elapsed);

	virtual CConsoleView *		MakeConfigurationView(
									BRect frame);

	virtual CConsoleView *		MakeMonitorView(
									BRect frame);

	virtual void				ReadChunk(
									CIFFReader &reader);

	virtual void				Serialize(
									CIFFWriter &writer);

	virtual void				Stack(
									CEvent &event,
									const CEventTask &task,
									CEventStack &stack,
									long duration);

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

protected:						// Monitor Management

	void						MonitorOpened(
									const BMessenger &messenger);

	void						MonitorClosed(
									const BMessenger &messenger);

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
	BString						m_consumerName;

	/** MIDI channel. */
	uint8						m_channel;

	/** Whether or not this destination supports General Midi. */
	bool						m_generalMidi;

	map<event_type, CEvent>		m_state;

	uint16						m_currentPitch;
	uint16						m_targetPitch;

	list<BMessenger>			m_monitors;
};

};

#endif /* __C_MidiDestination_H__ */
