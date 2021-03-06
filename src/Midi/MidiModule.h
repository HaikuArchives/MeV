/* ===================================================================== *
 * MidiModule.h (MeV/Midi)
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
 * History:
 *	6/21/2000		dwalton
 *		Original implementation
 *		added to the repository...
 *	8/13/2000		dwalton
 *		modified to support one producer per CDestination.
 *	11/24/2000		cell
 *		modified to inherit from new CMeVModule class, renamed to 
 *		CMidiModule
 * ===================================================================== */

#ifndef __C_MidiModule_H__
#define __C_MidiModule_H__

#include "MeVModule.h"

// Standard Template Library
#include <map>
#include <string>

class BMidiConsumer;
class BMidiEndpoint;
class BMidiProducer;
class BMidiRoster;

namespace Midi
{

class CInternalSynth;

enum chunk_ids
{
	// Midi destination chunk ids
	CONNECTION_NAME_CHUNK		= 'Mcon',		// Midi consumer name
	DESTINATION_SETTINGS_CHUNK	= 'Mset'		// Midi destination settings
};

const size_t CONNECTION_NAME_LENGTH = 256;

class CMidiModule
	: 	public CMeVModule
{

public:							// Singleton Access

	static CMidiModule *		Instance();

public:							// Accessors

	BMidiEndpoint *				FindEndpoint(
									const BString &name);
		
	BMidiProducer *				GetNextProducer(
									int32 *id) const;
	BMidiProducer *				FindProducer(
									int32 id);
	BMidiProducer *				FindProducer(
									const BString &name);

	BMidiConsumer *				GetNextConsumer(
									int32 *id) const;
	BMidiConsumer *				FindConsumer(
									int32 id);
	BMidiConsumer *				FindConsumer(
									const BString &name);

	// returns the icon for a specific BMidiEndpoint
	status_t					GetIconFor(
									BMidiEndpoint *endpoint,
									icon_size which,
									BBitmap *outBitmap);

	CInternalSynth *			InternalSynth() const
								{ return m_internalSynth; }

public:							// CMeVModule Implementation

	/** Called by the document when a new destination is requested. */
	virtual CDestination *		CreateDestination(
									CMeVDoc *document,
									int32 *id = NULL,
									const char *name = NULL);

	/** Returns the generic icon for this module. */
	virtual status_t			GetIcon(
									icon_size which,
									BBitmap *outBitmap);

	virtual void				MessageReceived(
									BMessage *message);

	virtual bool				SubjectReleased(
									CObservable *subject);

	virtual void				SubjectUpdated(
									BMessage *message);

protected:						// Hidden Constructor

								CMidiModule();

	virtual						~CMidiModule();

private:						// Internal Operations

	void						_endpointRegistered(
									int32 id,
									const BString &type);

	void						_endpointUnregistered(
									int32 id,
									const BString &type);

	void						_endpointConnected(
									int32 producerID,
									int32 consumerID);

	void						_endpointDisconnected(
									int32 producerID,
									int32 consumerID);

	void						_endpointChangedName(
									int32 id,
									const BString &type,
									const BString &name);

	void						_endpointChangedLatency(
									int32 id,
									const BString &type,
									bigtime_t latency);

	void						_endpointChangedProperties(
									int32 id,
									const BString &type,
									const BMessage *properties);

private:						// Instance Data

	BMidiRoster *				m_roster;

	CInternalSynth *			m_internalSynth;

private:						// Class Data

	static CMidiModule *		s_instance;
};

};

#endif /* __C_MidiModule_H__ */
