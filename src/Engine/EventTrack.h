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

const ulong			TrackType_Event	= 'eTrk';

class CMeVDoc;
class CEventEditor;
class EventOp;

const int			Max_Track_Filters = 8;

	// Since many track types have event lists, we'll define a subclass for that.

class CEventTrack : 
	public CTrack
{
	friend class MeVTrackRef;

private:
	EventList		events;					// List of events for this class
	EventMarker		currentEvent;			// for single-event selection
	long				selectionCount;			// number of selected events

	long				minSelectTime,			// start time of selection
					maxSelectTime;			// end time of selection
					
	long				sectionStart,			// Start of playback section
						sectionEnd;				// End of playback section.

	long				timeGridSize;			// gridsnap size for time
	bool				gridSnapEnabled;			// grid snap is enabled.

		// "Aggregate" actions are when multiple indpendent events are
		// aggregated into a single UNDO. (arrow-key editing, for example)
	UndoAction		*prevAggregateUndo;		// previous aggregate undo action
	int32			prevAggregateAction;		// previous aggregate action code
					
	BList			operators;				// Operators specific to this track

	EventOp			*filters[ Max_Track_Filters ];
	int32			filterCount;
	
	bool				validSigMap;
	
	VBitTable		usedChannels,
					selectedChannels,
					lockedChannels;
	
	void RecalcSigMap();

		// For operations on master tracks... 
	CEventTrack *Sibling();
	int32 Bytes() { return sizeof *this + events.TotalItems() * sizeof(Event); }

public:
	CEventTrack( CMeVDoc &inDoc, TClockType cType, int32 inID, char *inName );

	void SummarizeSelection();
	
		/** Returns the count of the number of events in this track */
	int32 CountItems() const { return events.TotalItems(); }

	uint32 TrackType() const { return TrackType_Event; }

	long MinSelectTime() const { return minSelectTime; }
	long MaxSelectTime() const { return maxSelectTime; }
	
		/**	Indicates that a signature event has been moved. */
	void InvalidateSigMap() { validSigMap = false; }
	
		/**	Returns true if the signature map has not been invalidated */
	bool ValidSigMap() { return validSigMap; }
	
		/**	Indicates that a tempo event has been moved.
			(only applies to master tracks)
		*/
	void InvalidateTempoMap();
	
		/**	This is used to synthesize a selection time when new events are being
			added.
		*/
	void SetSelectTime( long minTime, long maxTime )
	{
		minSelectTime = minTime;
		maxSelectTime = maxTime;
	}
	
	void SetCurrentEvent( EventMarker &inMarker )
	{
		currentEvent = inMarker;
	}
	
		/**	Undo the undoable action, if any. */
	bool Undo()
	{
		StSubjectLock	myLock( *this );

		DeselectAll( NULL, false );
		if (CTrack::Undo())
		{
			SummarizeSelection();
			return true;
		}
		return false;
	}

		/**	Redo the redoable action, if any. */
	bool Redo()
	{
		StSubjectLock	myLock( *this );

		DeselectAll( NULL, false );
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
	void DeleteEvent(const Event *which);

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
		Event				&newEv,
		const char			*inActionLabel );

		/** Merge-in an event stream. */
	void MergeEvents(
		Event				*inEvents,
		int32				eventCount,
		EventListUndoAction	*undoAction);

		/**	Return the current selection type. */
	virtual enum CTrack::E_SelectionTypes SelectionType()
	{
		if (selectionCount == 0) return Select_None;
		if (selectionCount == 1) return Select_Single;
		return Select_Subset;
	}

	const Event *CurrentEvent() { return currentEvent.Peek( 0 ); }

#if 0
		// Set a particular attribute of all selected events to a given value
	long SetEventAttribute( uint16 attrType, long newValue, long orgVal );
#endif

		// Error-checking functions
	void Validate();

		// Gain access to the event list. (Throws exception if not locked).
	EventList		&Events() { return events; }
	
		// Filter an event through the filters assigned to this track
	void FilterEvent( Event &ioEv );
	
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
	int32 SelectionCount() { return selectionCount; }

		/**	Return the current gridsnap size. */
	long TimeGridSize() { return timeGridSize; }
	
		/**	Return true if gridsnap is enabled for this item. */
	bool GridSnapEnabled() { return gridSnapEnabled; }
	
		/**	Enable or disable gridsnap. */
	void EnableGridSnap( bool inEnabled ) { gridSnapEnabled = inEnabled; }
	
		/**	Set the track grid size. */
	void SetTimeGridSize( int32 inGrid ) { timeGridSize = inGrid; }
	
		/**	Get start of playback section. */
	int32 SectionStart() { return sectionStart; }
	
		/**	Get end of playback section. */
	int32 SectionEnd() { return sectionEnd; }
	
		/**	Set the playback section. */
	void SetSection( int32 start, int32 end )
	{
		sectionStart	= start < end ? start : end;
		sectionEnd	= start > end ? start : end;
	}

		/**	Overrides AddUndoAction from CObservableSubject to deal with
			master track issues. */
	void AddUndoAction( UndoAction *inAction );

		/**	Write the track to the MeV file. */
	void WriteTrack( CIFFWriter &writer );
	
		/** Read one chunk from the MeV file. */
	void ReadTrackChunk( CIFFReader &reader );
	
	/** Test if a channel is locked... */
	bool						IsChannelLocked(
									int32 channel) const
								{ return lockedChannels[channel]; }

	/** Lock or unlock a channel */
	void						LockChannel(
									int32 channel,
									bool locked = true);

	/** TRUE if this event is "locked" (but only if the event even 
		has a channel.)
	*/
	bool						IsChannelLocked(
									const Event &ev) const;

	/** TRUE if any event in this track is using this channel. */
	bool						IsChannelUsed(
									int32 channel) const
								{ return usedChannels[channel]; }
};

	/**	A hint which takes it's parameters from the current selection. */
class CEventSelectionUpdateHint : public CUpdateHint {
public:
	CEventSelectionUpdateHint( const CEventTrack &inTrack, bool inSelChangeOnly = false );
};

	/**	A hint which takes it's parameters from a single event. */
class CEventUpdateHint : public CUpdateHint {
public:
	CEventUpdateHint( const CEventTrack &inTrack, const Event &inEvent );
};

#endif /* __C_EventTrack_H__ */