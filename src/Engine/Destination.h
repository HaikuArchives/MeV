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
#include "MeV.h"
#include "Observable.h"
#include "BitSet.h"
#include <MidiProducer.h>
#include "ReconnectingMidiProducer.h"
//Support Kit.
#include <String.h>
// Interface Kit
#include <InterfaceDefs.h>
#include "IFFWriter.h"
#include "IFFReader.h"
#include "Observer.h"
#include <Rect.h>
#include <Bitmap.h>
class CMeVDoc;
/* ============================================================================ *
                                Destination

	Destinations, allow routing and remapping of MIDI data
	upon playback. There are up to 128 channels supported.
 * ============================================================================ */

class CDestination
{

		friend class CMeVDoc; //only for saving
		friend class CDestinationDeleteUndoAction;
public :

				//Constants
		enum EDestUpdateHintBits {
		Update_Name = (1<<0),
		Update_Color = (1<<1),
		Update_Channel = (1<<2),
		Update_Connections = (1<<3),
		Update_Flags= (1<<4),   //mute solo ect.
		Update_Latency = (1<<5)
		};

		enum destinationFlags {		
		muted			= (1<<0),				// channel is muted
		mutedFromSolo	= (1<<1),				// channel muted because of solo
		solo			= (1<<2),				// channel is solo'd
		disabled		= (1<<3),				// channel disabled because
												// of vanished midi port.
		deleted			= (1<<4)				// only one channel should be in this catagory.
	};
		
	
public:					//Constructor/Destructor

	CDestination(int32 id,CMeVDoc &inDoc,char *name,bool notify);
														//the notify is a tempary soloution
														//to a Segfault in the initial observer on load
	~CDestination();

public:   				// Serialization
	
	virtual void WriteDestination (CIFFWriter &writer);

public:					// Accessors
	
	bool 				IsValid () const;   //if not disabled or deleted.
	
	void 				SetMuted (bool muted);
	bool				Muted () const;
						
	bool 				MutedFromSolo () const;
						
	void 				SetSolo (bool solo);
	bool 				Solo () const;
						
	void 				SetName (const char *name);	
	const char *		Name() const
						{return m_name.String();}

	void				SetLatency(int32 microseconds);
	int32				Latency(uint8 clockType);
						
	
	int32 				GetID() const
						{return m_id;}
						
	void				SetColor (rgb_color color);
	rgb_color			GetFillColor();
	rgb_color			GetHighlightColor();
						
	void 				SetChannel (uint8 channel);
	uint8				Channel () const
						{return m_channel;}
	
	void SetConnect (BMidiConsumer *sink, bool connect);
	bool IsConnected (BMidiConsumer *sink) const;
	CReconnectingMidiProducer * GetProducer() const
	{ return m_producer; }
	
	void SetDisable (bool disable);
	
	void Delete ();
	
	void Undelete(int32 originalIndex);
	
	
	bool			 Deleted() const;

	bool 			 Disabled() const;
	CMeVDoc & Document()
			{ return *m_doc; } 
	
public:							//Hook Functions
	virtual int32				Bytes()
								{ return sizeof *this; }

public:							//debug function
	void 						PrintSelf();
private:
	bool _addFlag (int32 flag);
	bool _removeFlag(int32 flag);	
	
	
private:   	
	void 					_addIcons(BMessage* msg, BBitmap* largeIcon, BBitmap* miniIcon) const;
	BBitmap * 				CreateIcon (BRect r);
	static const rgb_color m_defaultColorTable[ 16 ] ;
	
	
	int32				m_id;
	uint8				m_channel,				// real midi channel
						m_flags;					// various flags
	bigtime_t			m_latency;
	int32				m_consumer_id;			//this is the id that that this dest
												//is connected to.

	CMeVDoc 			*m_doc;
	BString 			m_name;					// in the future we may not need this.
	CReconnectingMidiProducer *m_producer;					
	int8				m_transpose,				// transposition for channel
						m_initialTranspose;		// initial transposition value

	rgb_color			m_fillColor,				// fill color
						m_highlightColor;			// hightlight color
};


class CDestinationDeleteUndoAction
	:	public UndoAction
{

public:
						CDestinationDeleteUndoAction(
							CDestination *dest);
					
						~CDestinationDeleteUndoAction();
						
public:

	
	const char *		Description() const
						{ return "Delete Destination"; }
	
	void				Redo();
	
	int32				Size()
						{return m_dest->Bytes();}
	
	void				Undo();
	
private:
	
	CDestination *		m_dest;
	
	int32				m_index;
};
typedef BitSet<Max_Destinations> VBitTable;
#endif /* __C_Destination_H__ */