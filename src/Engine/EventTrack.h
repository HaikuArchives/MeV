/* ===================================================================== *
 * EventTrack.h (MeV/Engine)
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
 *  A track that contains events
 * ---------------------------------------------------------------------
 * History:
 *	1997		Talin
 *		Original implementation
 *	04/08/2000	cell
 *		General cleanup in preparation for initial SourceForge checkin
 *	09/13/2000	malouin
 *		Added friend declaration for MeVTrackRef
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __C_EventTrack_H__
#define __C_EventTrack_H__

#include "Track.h"
#include "EventList.h"
#include "EventOp.h"
#include "Destination.h"

// Standard Template Library
#include <map>

class CMeVDoc;
class CEventEditor;
class EventOp;

const ulong			TrackType_Event	= 'eTrk';

const int			Max_Track_Filters = 8;

/**
 *	A CTrack that contains events. Since many track types have event 
 *	lists, we'll define a subclass for that.
 *	@author	Talin, Christopher Lenz, Curt Malouin
 *	@todo	Move UI functionality (references to CEventEditor and 
 *			CEventRenderer) out of this class.
 */
class CEventTrack : 
	public CTrack
{
	friend class MeVTrackRef;

public:							// Constructor/Destructor

								CEventTrack(
									CMeVDoc &doc,
									TClockType clockType,
									int32 id,
									const char *name);

public:							// Accessors

	/** Returns the count of the number of events in this track */
	int32						CountEvents() const
								{ return events.TotalItems(); }

	long						MinSelectTime() const
								{ return m_minSelectTime; }
	long						MaxSelectTime() const
								{ return m_maxSelectTime; }
	
	const CEvent *				CurrentEvent()
								{ return m_currentEvent.Peek(0); }

	/** Gain access to the event list. */
	EventList &					Events()
								{ return events; }
								
	uint32						TrackType() const
								{ return TrackType_Event; }

	/**	Returns the destinations in use by this part. Start calling this
	 *	function with a pointer to an integer set to zero. The part has to
	 *	be read-locked when you call this!
	 */
	CDestination *				GetNextUsedDestination(
									long *index) const;

public:							// Operations

	/** Compute summary information for currently selected events. */
	void						SummarizeSelection();
	
	/**	Indicates that a signature event has been moved. */
	void						InvalidateSigMap()
								{ m_validSigMap = false; }
	
	/**	Returns true if the signature map has not been invalidated */
	bool						ValidSigMap()
								{ return m_validSigMap; }
	
	/**	Indicates that a tempo event has been moved.
	 *	(only applies to master tracks)
	 */
	void						InvalidateTempoMap();
	
	/**	This is used to synthesize a selection time when new events are being
	 *	added.
	 */
	void						SetSelectTime(
									long minTime,
									long maxTime);
								
	void						SetCurrentEvent(
									EventMarker &marker);
	
	/**	Undo the undoable action, if any. */
	bool						Undo()
								{
									CWriteLock lock(this);
									DeselectAll(NULL, false);
									if (CTrack::Undo())
									{
										SummarizeSelection();
										return true;
									}
									return false;
								}

	/**	Redo the redoable action, if any. */
	bool						Redo()
								{
									CWriteLock lock(this);
									DeselectAll(NULL, false);
									if (CTrack::Redo())
									{
										SummarizeSelection();
										return true;
									}
									return false;
								}
	
		/**	Select all events. */
	void SelectAll(	CEventEditor *inEditor );		// Active editor window, or NULL
	
		/**	Select all events. */
	void SelectAll( void ) { SelectAll( NULL ); }	// Active editor window, or NULL

		/**	Deselect all events. */
	void DeselectAll(	CEventEditor *inEditor,	// Active editor window, or NULL
						bool inDoUpdate = true );

	// delete a specific event
	void DeleteEvent(const CEvent *which);

		/**	Delete all selected events. */
	void DeleteSelection();

		/**	Apply an operator to the selected events. */
	void ModifySelectedEvents(
		CEventEditor			*inEditor,			// Active editor window, or NULL
		EventOp				&op,					// Operation to apply
		const char			*inActionLabel,		// Menu label for undo
		int32				inAggregateAction = 0 );	// Aggregate w/ previous action

		/**	Copy selected events, and apply the operator to the copy. */
	void CopySelectedEvents(
		CEventEditor			*inEditor,			// Active editor window, or NULL
		EventOp				&op,					// Operation to apply
		const char			*inActionLabel );	// Menu label for undo

		/**	Create a new event, and do all appropriate updates, notifications, etc. */
	void CreateEvent(
		CEventEditor			*inEditor,
		CEvent				&newEv,
		const char			*inActionLabel );

		/** Merge-in an event stream. */
	void MergeEvents(
		CEvent				*inEvents,
		int32				eventCount,
		EventListUndoAction	*undoAction);

	/**	Return the current selection type. */
	enum CTrack::E_SelectionTypes SelectionType()
	{
		if (m_selectionCount == 0)
			return Select_None;
		if (m_selectionCount == 1)
			return Select_Single;
		return Select_Subset;
	}

		// Filter an event through the filters assigned to this track
	void FilterEvent( CEvent &ioEv );
	
		// Compile list of operators.
	void CompileOperators();

		/**	Return the number of operators associated with this track. */
	int32 CountOperators() { return operators.CountItems(); }
	
		/**	Return the Nth operator (Increases reference count). */
	EventOp *OperatorAt( int32 index )
	{
		void		*ptr = operators.ItemAt( index );
		
		if (ptr) return (EventOp *)((EventOp *)ptr)->Acquire();
		return NULL;
	}
	
		/**	Return the index of this operator in the list, or negative if none. */
	int32 OperatorIndex( EventOp *op )
	{
		return operators.IndexOf( op );
	}

		/**	Set an operator active / inactive. */
	void SetOperatorActive( EventOp *inOp, bool enabled );
	
	/**	Returns the number of selected events. */
	int32						SelectionCount() const
								{ return m_selectionCount; }

	/**	Return the current gridsnap size. */
	long						TimeGridSize() const
								{ return m_timeGridSize; }
	
	/**	Return true if gridsnap is enabled for this item. */
	bool						GridSnapEnabled() const
								{ return m_gridSnapEnabled; }

	/**	Enable or disable gridsnap. */
	void						EnableGridSnap(
									bool enabled)
								{ m_gridSnapEnabled = enabled; }

	/**	Set the track grid size. */
	void						SetTimeGridSize(
									int32 grid)
								{ m_timeGridSize = grid; }
	
		/**	Overrides AddUndoAction from CObservable to deal with
			master track issues. */
	void AddUndoAction( UndoAction *inAction );

public:							// CSerializable Implementation

	/** Read one chunk from the MeV file. */
	virtual void				ReadChunk(
									CIFFReader &reader);
	
	/**	Write the track to the MeV file. */
	virtual void				Serialize(
									CIFFWriter &writer);

private:						// Internal Operations

	void						_eventAdded(
									const CEvent *ev);

	void						_eventRemoved(
									const CEvent *ev);

	void						_initUsedDestinations();

	int32						Bytes()
								{ return sizeof *this + CountEvents() * sizeof(CEvent); }

	void						RecalcSigMap();

	CEventTrack *				Sibling();

private:						// Instance Data

	/** List of events for this part. */
	EventList					events;

	/** For single-event selection. */
	EventMarker					m_currentEvent;

	/** Number of selected events. */
	long						m_selectionCount;

	/** Start time of selection. */
	long						m_minSelectTime;

	/** End time of selection. */
	long						m_maxSelectTime;
					
	/** Gridsnap size for time. */
	long						m_timeGridSize;

	/** Grid snap is enabled. */
	bool						m_gridSnapEnabled;

	/** Previous aggregate undo action.
	 *	Aggregate actions are when multiple indpendent events are
	 *	aggregated into a single UNDO. (arrow-key editing, for example) */
	UndoAction *				m_prevAggregateUndo;

	/** Previous aggregate action code. */
	int32						m_prevAggregateAction;

	bool						m_validSigMap;

	/** Operators specific to this track. */
	BList						operators;

	EventOp *					m_filters[Max_Track_Filters];
	int32						m_filterCount;

	/**	A map of the destinations used by this part. The key is a 
	 *	pointer to the destination, while the value contains the 
	 *	number of events using that destination. */
	typedef map<CDestination *, unsigned long> used_destinations_map;
	used_destinations_map		m_destinations;
};

/**	A hint which takes it's parameters from the current selection. */
class CEventSelectionUpdateHint
	:	public CUpdateHint
{

public:							// Constructor/Destructor

								CEventSelectionUpdateHint(
									const CEventTrack &track,
									bool selChangeOnly = false);

};

/**	A hint which takes it's parameters from a single event. */
class CEventUpdateHint
	:	public CUpdateHint
{

public:							// Constructor/Destructor

								CEventUpdateHint(
									const CEventTrack &track,
									const CEvent &event);

};

#endif /* __C_EventTrack_H__ */
