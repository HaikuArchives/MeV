/* ===================================================================== *
 * MeVDoc.h (MeV)
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
 *		Curt Malouin (malouin)
 *
 * ---------------------------------------------------------------------
 * Purpose:
 *  MeV Document class
 * ---------------------------------------------------------------------
 * History:
 *	1997		Talin
 *		Original implementation
 *	04/08/2000	cell
 *		General cleanup in preparation for initial SourceForge checkin
 *	09/13/2000	malouin
 *		Don't add default track if any tracks were created before the doc
 *		window is first shown.
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __C_MevDoc_H__
#define __C_MevDoc_H__

#include "MeV.h"
#include "MeVApp.h"
#include "Document.h"
#include "Event.h"
#include "WindowState.h"
#include "TempoMap.h"

class CAssemblyWindow;
class CDestinationList;
class CMeVApp;
class CTrack;
class CEventTrack;
class CIFFReader;
class CIFFWriter;

class BMimeType;

class CMeVDoc
	:	public CDocument
{
	friend class CTrackDeleteUndoAction;
	friend class CTrack;
	friend class CDestination;
	friend class CDestDeleteUndoAction;
	friend class MeVTrackRef;

public:							// Constants

	enum window_types
	{
								ASSEMBLY_WINDOW = 0,

								MIX_WINDOW,

								OPERATORS_WINDOW,

								WINDOW_TYPE_COUNT
	};

	enum EDocUpdateHintBits {

		Update_Name			= (1<<0),			// Document name changed
		Update_AddTrack		= (1<<1),			// Track added
		Update_DelTrack		= (1<<2),			// Track deleted
		Update_TrackOrder	= (1<<3),			// Track order changed
		Update_Operator	= (1<<4),			// Operators changed
		Update_OperList	= (1<<5),			// list of operators changed
		Update_TempoMap	= (1<<6),			// list of operators changed
		Update_AddDest = (1<<7),
		Update_DelDest = (1<<8)
	};

	static const double			DEFAULT_TEMPO;

public:							// Constructor/Destructor

								CMeVDoc(
									CMeVApp *app);

								CMeVDoc(
									CMeVApp *app,
									entry_ref &ref);

								CMeVDoc(
									CMeVApp *app,
									entry_ref &ref,
									CIFFReader &reader);

	virtual						~CMeVDoc();
	
public:							// Accessors

	static BMimeType *			MimeType();

public:							// Operator Management

	/**	Return the number of operators. */
	int32						CountActiveOperators()
								{ return activeOperators.CountItems(); }

	/**	Return the Nth operator (Increases reference count). */
	EventOp *					ActiveOperatorAt(
									int32 index) const;
	
	/**	Return the index of this operator in the list, or negative if none. */
	int32						ActiveOperatorIndex(
									EventOp *op) const
								{ return activeOperators.IndexOf(op); }
	
	/**	Return the number of operators. */
	int32						CountOperators() const
								{ return operators.CountItems(); }

	/**	Return the Nth operator (Increases reference count). */
	EventOp *					OperatorAt(
									int32 index) const;
	
	/**	Return the index of this operator in the list, or negative if none. */
	int32						OperatorIndex(
									EventOp *op) const
								{ return operators.IndexOf(op); }

	/**	Add an operator to the document's list of operators. */
	void						AddOperator(
									EventOp *op);

	/**	Delete an operator to the document's list of operators. */
	void						RemoveOperator(
									EventOp *op);
	
	/**	Set an operator active / inactive. */
	void						SetOperatorActive(
									EventOp *op,
									bool enabled);

	/**	Does a notification to all windows viewing the operator. */
	void						NotifyOperatorChanged(
									EventOp *op);

public:							// Track Management

	/**	Change the ordering of the tracks... */
	void						ChangeTrackOrder(
									int32 oldIndex,
									int32 newIndex);

	/** Create a new track (refcount = 1) */
	CTrack *					NewTrack(
									ulong inTrackType,
									TClockType inClockType);

	long						GetUniqueTrackID();

	/**	Locate a track by it's ID, and Acquire it. (-1 for master track) */
	CTrack *					FindTrack(
									long inTrackID);

	/**	Locate a track by it's name, and Acquire it. */
	CTrack *					FindTrack(
									char *inTrackName);

	/**	Get the first track. */
	CTrack *					FindNextHigherTrackID(
									int32 inID);

	/**	For iterating through tracks in list order. */
	int32						CountTracks() const
								{ return tracks.CountItems(); }

	/**	For iterating through tracks in list order. */
	CTrack *					TrackAt(
									int32 index)
								{ return (CTrack *)tracks.ItemAt(index); }

public :						//Destination Management

	CDestination *				NewDestination(
									unsigned long type = 'MIDI');

	CDestination *				FindDestination(
									int32 id) const;
	
	int32						CountDestinations() const
								{ return m_destinations.CountItems(); }

	/**	Retrieves the next destination, starting at index.
	 *	You should start with a value of 0, it will be incremented 
	 *	inside this function to the next valid destination.
	 *	If there are no more destinations, this function returns NULL.
	 */
	CDestination *				GetNextDestination(
									int32 *index) const;

	/**	Returns the index of the destination in a non-fragmented list,
	 *	which can be different from the destination's ID. In case the
	 *	destination is not found, a negative number is returned.
	 */
	int32						IndexOf(
									const CDestination *destination) const;

	bool 						IsDefinedDest(
									int32 id) const;
	
public:							// Window Management

	/**	Show or hide the window of a particular type */
	BWindow *					ShowWindow(
									uint32 type,
									bool show = true);

	/**	Returns TRUE if a particular window type is open. */
	bool						IsWindowOpen(
									uint32 type);

	/**	Show or hide the window for a particular track */
	void						ShowWindowFor(
									CTrack *track);

public:							// Operations

	/**	Get the value of a default attribute */
	int32						GetDefaultAttribute(
									enum E_EventAttribute type) const
								{ return defaultAttributes[type]; }

	/**	Set the value of a default attribute */
	void						SetDefaultAttribute(
									enum E_EventAttribute type,
									int32 value)
								{ defaultAttributes[type] = value; }

	/**	Post an update message to all tracks. */
	void						PostUpdateAllTracks(
									CUpdateHint *hint);

	/**	Notify all observers (including possibly observers of the document
		as well) that some attributes of this document have changed.
	*/
	void						NotifyUpdate(
									int32 inHintBits,
									CObserver *source);

	/** return which of the two master tracks is selected. */
	CEventTrack *				ActiveMaster() const
								{ return m_activeMaster; }
	
	/** Set the selected master track. Call with any track, ignored if
		not a master track.
	*/
	void						SetActiveMaster(
									CEventTrack *inTrack);

	/**	Return a pointer to the tempo map. */
	const CTempoMap &			TempoMap()
								{ return tempoMap; }

	/** Get the initial tempo of this document */
	double						InitialTempo()
								{ return m_initialTempo; }

	/** Set the initial tempo of this document */
	void						SetInitialTempo(
									double inTempo)
								{ m_initialTempo = inTempo; }

	/** Export the document */
	void						Export(
									BMessage *msg);

	/**	Sets a flag indicating that the tempo map needs to be recompiled.
		This is called by the track editing code whenever a tempo change
		event is modified.
	*/
	void						InvalidateTempoMap()
								{ validTempoMap = false; }

	/**	Returns true if the tempo map is valid, i.e. it correctly represents the
		compilation of all tempo events in the master tracks.
	*/
	bool						ValidTempoMap()
								{ return validTempoMap; }

	/**	Replace the current tempo map. */
	void						ReplaceTempoMap(
									CTempoMapEntry *entries,
									int length);


public:							// CDocument Implementation

	virtual CMeVApp *			Application() const
								{ return static_cast<CMeVApp *>
										 (CDocument::Application()); }

	/** Save the document to it's current location */
	virtual void				SaveDocument();

	virtual void				ReadChunk(
									CIFFReader &reader);

	virtual void				Serialize(
									CIFFWriter &writer);

private:						// Internal Operations

	void						_addDefaultOperators();

	void						_init();

	/** Read a single track. */
	void						_readTrack(
									uint32 inTrackType,
									CIFFReader &iffReader);

	/** Read environment chunk. */
	void						_readEnvironment(
									CIFFReader &iffReader);

private:						// Instance Data

	BList						tracks;
	int32						m_newTrackID;
	
	BList						m_destinations;

	// Opers associated with doc
	BList						operators;

	// Operators in use...
	BList						activeOperators;

	// Master track (real)
	CEventTrack *				m_masterRealTrack;

	// Master track (metered)
	CEventTrack *				m_masterMeterTrack;

	// Which track is being edited
	CEventTrack *				m_activeMaster;
	

	int32						defaultAttributes[EvAttr_Count];

	// Initial tempo for document
	double						m_initialTempo;

	CTempoMap					tempoMap;

	bool						validTempoMap;

	// Master editing window -- we close this, we close
	// everything...
	CAssemblyWindow *			assemblyWindow;

	CWindowState				m_windowState[WINDOW_TYPE_COUNT];
};

#endif /* __C_MeVDoc_H__ */
