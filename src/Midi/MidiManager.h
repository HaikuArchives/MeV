/* ===================================================================== *
 * MidiManager.h (MeV/Midi)
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
 *	Some of this code has been based on Be, Inc. sample code.
 *	Copyright 1999, Be Incorporated.   All Rights Reserved.
 * ---------------------------------------------------------------------
 * Purpose:
 *	This class takes care of building new consumers and connecting...etc.
 *	Manages the external Midi environment.
 *
 * ---------------------------------------------------------------------
 * History:
 *	6/21/2000		dwalton
 *		Original implementation
 *		added to the repository...
 *	8/13/2000		dwalton
 *		modified to support one producer per CDestination.
 * ---------------------------------------------------------------------
 * To Do:
 * I imagine that this could be used for all the midi port management including
 * patch names etc.
 * ===================================================================== */

#ifndef __C_MidiManager_H__
#define __C_MidiManager_H__

#include "InternalSynth.h"
#include "Observer.h"

// Application Kit
#include <Looper.h>
// Interface Kit
#include <Bitmap.h>
#include <GraphicsDefs.h>
// Midi Kit
#include <MidiProducer.h>
#include <MidiRoster.h>
#include <MidiConsumer.h>
// Storage Kit
#include <Mime.h>
// Support Kit
#include <List.h>
#include <String.h>
// Standard Template Library
#include <map>
#include <string>

class CDestination;
class CMeVDoc;

namespace Midi
{

enum chunk_ids
{
	// Midi destination chunk ids
	CONNECTION_NAME_CHUNK		= 'Mcon',		// Midi consumer name
	DESTINATION_SETTINGS_CHUNK	= 'Mset'		// Midi destination settings
};

const size_t CONNECTION_NAME_LENGTH = 256;

class CMidiManager
	: 	public BLooper,
		public CObserver
{

public:							// Singleton Access

	static CMidiManager *		Instance();

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

	/** Returns the generic icon for this module. */
	virtual status_t			GetIcon(
									icon_size which,
									BBitmap *outBitmap);

	// returns the icon for a specific BMidiEndpoint
	status_t					GetIconFor(
									BMidiEndpoint *endpoint,
									icon_size which,
									BBitmap *outBitmap);

	CInternalSynth *			InternalSynth() const
								{ return m_internalSynth; }

public:							// Operations

	/** Called by the app when a document has been created or loaded.
		We should start observing the document at this point.
	 */
	virtual void				DocumentOpened(
									CMeVDoc *document);

public:							// BLooper Implementation

	virtual void				MessageReceived(
									BMessage *message);

public:							// CObserver Implementation

	virtual bool				Released(
									CObservable *subject);

	virtual void				Updated(
									BMessage *message);

protected:						// Hidden Constructor

								CMidiManager();

	virtual						~CMidiManager();

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

	/**	Maps MidiConsumer IDs to destinations. */
	typedef multimap<string, CDestination *>
								dest_map;
	typedef multimap<string, CDestination *>::const_iterator
								dest_iter;

	dest_map					m_destinations;

private:						// Class Data

	static CMidiManager *		s_instance;
};

};

#endif /* __C_MidiManager_H__ */
