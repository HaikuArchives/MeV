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
 *		Separated MIDI functionality into CMidiDestination subclass
 * ===================================================================== */

#ifndef __C_Destination_H__
#define __C_Destination_H__

#include "BitSet.h"
#include "MeV.h"
#include "Observable.h"
#include "Serializable.h"

// Support Kit
#include <Mime.h>

class CConsoleView;
class CMeVDoc;

class BBitmap;

#define DESTINATION_NAME_LENGTH 128

/**	Destinations, allow routing and remapping of MIDI data
	upon playback.
	@author		Talin, Dan Walton, Christopher Lenz
	@package	Engine
 */
class CDestination
	:	public CObservable,
		public CSerializable
{
	friend class CDestinationDeleteUndoAction;

public:							// Constants

	enum update_hints
	{
								Update_Name 		= (1 << 0),

								Update_Latency 		= (1 << 1),

								Update_Flags		= (1 << 2),

								Update_Color 		= (1 << 3)
	};

public:							//Constructor/Destructor

								CDestination(
									unsigned long type,
									long id,
									const char *name,
									CMeVDoc *document);

								CDestination(
									unsigned long type,
									CMeVDoc *document);

	virtual						~CDestination();

public:							// Hook Functions

	virtual status_t			GetIcon(
									icon_size which,
									BBitmap *outIcon) = 0;

	virtual CConsoleView *		MakeConfigurationView(
									BRect frame) = 0;

	virtual CConsoleView *		MakeMonitorView(
									BRect frame) = 0;

protected:

	virtual void				ColorChanged(
									rgb_color color)
								{ }

	virtual void				Deleted()
								{ }

	virtual void				Disabled(
									bool disabled)
								{ }

	virtual void				Muted(
									bool muted)
								{ }

	virtual void				NameChanged(
									const char *name)
								{ }

	virtual void				Soloed(
									bool solo)
								{ }	

	virtual void				Undeleted()
								{ }

public:							// Accessors
	
	/** Returns the type of destination. */
	unsigned long				Type() const;

	/** Returns the ID of the destination. */
	long						ID() const;

	/** Returns true if not disabled or deleted. */
	bool 						IsValid() const;

	void 						SetMuted(
									bool muted);
	/** Returns whether or not the destination is currently muted. If 
	 *	you provide the fromSolo argument, it is set to true when the
	 *	destination is muted because another destination is currently
	 *	soloed.
	 */
	bool						IsMuted(
									bool *fromSolo = NULL) const;

	void 						SetSolo(
									bool solo);
	bool 						IsSolo() const;
						
	void 						SetName(
									const char *name);
	const char *				Name() const
								{ return m_name; }

	void						SetLatency(
									bigtime_t microseconds);
	bigtime_t					Latency() const
								{ return m_latency; }

	void						SetColor(
									rgb_color color);
	rgb_color					Color() const
								{ return m_color; }

	void						SetDisabled(
									bool disabled);
	bool			 			IsDisabled() const;

	CMeVDoc *					Document() const
								{ return m_doc; } 
	
public:							// Operations

	bool						IsDeleted() const;
	void						Delete();
	void						Undelete(
									int32 originalIndex);

public:							// CSerializable Implementation

	virtual void				ReadChunk(
									CIFFReader &reader);

	virtual void				Serialize(
									CIFFWriter &writer);

private:   						// Internal Operations

	bool						_addFlag (int32 flag);

	bool						_removeFlag(int32 flag);	
	
private:						// Instance Data

	CMeVDoc *					m_doc;

	unsigned long				m_type;

	long						m_id;

	char						m_name[DESTINATION_NAME_LENGTH];

	bigtime_t					m_latency;

	uint8						m_flags;

	rgb_color					m_color;
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
								{ return sizeof(m_dest); }
	
	void						Undo();
	
private:
	
	CDestination *				m_dest;
	
	int32						m_index;
};

#endif /* __C_Destination_H__ */
