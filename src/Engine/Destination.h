/* ===================================================================== *
 * Destination.h (MeV/Engine)
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
 * Purpose:
 *  Defines virtual channel table
 * ---------------------------------------------------------------------
 * History:
 *	1997		Joe Pearce
 *		Original implementation
 *	04/08/2000	cell
 *		General cleanup in preparation for initial SourceForge checkin
 *  07/28/2000  dwalton
 *      Changed from struct to class.
 *		Added producer pointer.
 *  
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __C_Destination_H__
#define __C_Destination_H__

#include "BitSet.h"
#include "MeV.h"
#include "Observable.h"
#include "IFFWriter.h"
#include "IFFReader.h"
#include "ReconnectingMidiProducer.h"

// Midi Kit
#include <MidiProducer.h>
// Interface Kit
#include <Bitmap.h>
#include <InterfaceDefs.h>
#include <Rect.h>
//Support Kit.
#include <String.h>

class CMeVDoc;

/**	Destinations, allow routing and remapping of MIDI data
	upon playback.
	@author		Talin, Dan Walton, Christopher Lenz
	@package	Engine
 */
class CDestination
	:	public CObservable
{
	friend class CDestinationDeleteUndoAction;

public:							// Constants

	enum EDestUpdateHintBits
	{
								Update_Name = (1<<0),

								Update_Color = (1<<1),

								Update_Channel = (1<<2),

								Update_Connections = (1<<3),

								Update_Flags= (1<<4),

								Update_Latency = (1<<5)
	};

	enum destinationFlags
	{		
								// channel is muted
								muted			= (1<<0),

								// channel muted because of solo
								mutedFromSolo	= (1<<1),

								// channel is solo'd
								solo			= (1<<2),

								// channel disabled because of vanished midi port.
								disabled		= (1<<3),

								// only one channel should be in this catagory.
								deleted			= (1<<4)
	};

public:							//Constructor/Destructor

								CDestination(
									int32 id,
									CMeVDoc *document,
									const char *name);

								CDestination(
									CIFFReader &reader,
									CMeVDoc *document);

	virtual						~CDestination();

public: 		  				// Serialization

	virtual void				WriteDestination(
									CIFFWriter &writer);

public:							// Accessors
	
	virtual status_t			GetIcon(
									icon_size which,
									BBitmap *outIcon);
	int32 						GetID() const
								{ return m_id; }

	/** Returns true if not disabled or deleted. */
	bool 						IsValid () const;

	virtual void 				SetMuted(
									bool muted);
	/** Returns whether or not the destination is currently muted. If 
	 *	you provide the fromSolo argument, it is set to true when the
	 *	destination is muted because another destination is currently
	 *	soloed.
	 */
	bool						Muted(
									bool *fromSolo = NULL) const;

	virtual void 				SetSolo(
									bool solo);
	bool 						Solo() const
								{ return m_flags & solo; }
						
	virtual void 				SetName(
									const BString &name);
	const char *				Name() const
								{ return m_name.String(); }

	virtual void				SetLatency(
									bigtime_t microseconds);
	bigtime_t					Latency() const
								{ return m_latency; }

	void						SetColor(
									rgb_color color);
	rgb_color					GetFillColor() const
								{ return m_fillColor; }
	rgb_color					GetHighlightColor() const
								{ return m_highlightColor; }

	virtual void				SetDisabled(
									bool disabled);
	bool			 			Disabled()const
								{ return m_flags & disabled; }

	CMeVDoc *					Document() const
								{ return m_doc; } 
	
public:							// Midi specific functionality
								// +++ move to Midi::CMidiDestination

	void						SetConnect(
									BMidiConsumer *sink,
									bool connect);
	bool						IsConnected(
									BMidiConsumer *sink) const;
	Midi::CReconnectingMidiProducer *GetProducer() const
								{ return m_producer; }

	void 						SetChannel(
									uint8 channel);
	uint8						Channel() const
								{ return m_channel; }

public:							// Operations

	void						Delete();
	void						Undelete(
									int32 originalIndex);
	bool						Deleted() const
								{ return m_flags & deleted; }

public:							//Hook Functions

	virtual int32				Bytes()
								{ return sizeof(*this); }

private:   						// Internal Operations

	bool						_addFlag (int32 flag);

	bool						_removeFlag(int32 flag);	
	
	void 						_addIcons(
									BMessage* msg,
									BBitmap* largeIcon,
									BBitmap* miniIcon) const;

	BBitmap * 					_createIcon(
									BRect r);

	void						_updateIcons();

private:						// Instance Data

	CMeVDoc *					m_doc;

	int32						m_id;

	BString 					m_name;

	bigtime_t					m_latency;

	// various flags
	uint8						m_flags;

	rgb_color					m_fillColor;
	rgb_color					m_highlightColor;

	// +++ move these to CMidiDestination in the future

	// real midi channel
	uint8						m_channel;

	//this is the id that that this dest is connected to.
	int32						m_consumerID;

	Midi::CReconnectingMidiProducer *	m_producer;					

private:						// Class Data

	static const rgb_color		s_defaultColorTable[16];
};

class CDestinationDeleteUndoAction
	:	public UndoAction
{

public:
								CDestinationDeleteUndoAction(
									CDestination *dest);
					
								~CDestinationDeleteUndoAction();
						
public:

	
	const char *				Description() const
								{ return "Delete Destination"; }
	
	void						Redo();
	
	int32						Size()
								{ return m_dest->Bytes(); }
	
	void						Undo();
	
private:
	
	CDestination *				m_dest;
	
	int32						m_index;
};

typedef BitSet<Max_Destinations> VBitTable;

#endif /* __C_Destination_H__ */
