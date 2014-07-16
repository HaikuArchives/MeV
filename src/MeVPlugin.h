/* ===================================================================== *
 * MeVPlugin.h (MeV)
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
 *  MeV plugin and plugin manager class.
 * ---------------------------------------------------------------------
 * History:
 *	1997		Talin
 *		Original implementation
 *	04/08/2000	cell
 *		General cleanup in preparation for initial SourceForge checkin
 *	09/13/2000	malouin
 *		Expanded MeVTrackRef interface, implemented part of MeVEventRef
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __C_MeVPlugin_H__
#define __C_MeVPlugin_H__

#include "MeVSpec.h"
#include "Event.h"
#include "EventOp.h"

#include <sys/stat.h>
// Application Kit
#include <app/Handler.h>
// Interface Kit
#include <interface/GraphicsDefs.h>
// Kernel Kit
#include <kernel/OS.h>
// Storage Kit
#include <Node.h>

// ---------------------------------------------------------------------
//	Forward declarations

class MeVSpec MeVPlugIn;
class MeVSpec MeVDocRef;
class MeVSpec MeVTrackRef;
class MeVSpec MeVEventRef;

typedef MeVDocRef	*MeVDocHandle;
typedef MeVTrackRef	*MeVTrackHandle;
typedef MeVEventRef	*MeVEventHandle;

// ---------------------------------------------------------------------
//	Constants for adding plugins to various editor window menus.

const int32			Assembly_Menu = 'mAsm',
					TrackEditor_Menu = 'mTke',
					Operator_Menu = 'mOpr';

// ---------------------------------------------------------------------
//	Constants for adding plugins to list of imported and exported formats.

const int32			Import_List = 'impt',
					Export_List = 'expt';

/** ---------------------------------------------------------------------
	MeVPlugin is the base class for most plug-ins. It functions as the
	primary interface between MeV and the plug-in functions. The host
	services provided by the application are accessed primarily through this
	classes members.
 */

#ifdef __POWERPC__
#pragma export on
#endif

class MeVPlugIn : public BHandler {
public:

		/**	Constructor */
	MeVPlugIn();

		/**	Hook function, which should be implemented by the plug-in, to
			display "about box" style information.
		*/
	virtual char *AboutText() = 0;
	
		/**	Call this function to add a menu item to various application
			menus. The message inMsg will be sent to the plug-in when the
			menu is invoked.
			
			Note: In the case of an import or export filter, the method
			OnImport() or OnExport() will be called instead, with additional
			parameters.
		*/
	void AddMenuItem( char *inMenuText, int32 inWhichMenu, BMessage *inMsg );

		/**	Adds an event operator to the global list of default operators.
			Any newly created documents will have this list of operators
			added to the operator list for that document. Documents which
			are loaded will also have each of these operaters added,
			except in cases where the document already has an operator
			with the same name and the same plug-in name.
		*/
	void AddDefaultEventOperator( EventOp *inOper );
	
		/*	Notify the application that we made some changes to the operator,
			and that it should update any UIs that are looking at the operator's
			state.
		*/
	void NotifyOperatorChanged( EventOp *inOper );

		/**	Called by app when the user wishes to call up a UI to edit an operator
			created and maintained by this plug-in. The "queryOnly" flag indicates
			that we don't want to edit the operator, we just want to know if we can.
		*/
	virtual bool EditOperator( EventOp *inOp, bool queryOnly ) { return false; }

		/**	Called by app to see if this plug-in recognizes the file type
			listed. Only called for plug-ins which have registered themselves
			as importers.
			
			<p>Return a non-negative integer if you recognize the file type,
			or -1 if you can't. The code will be passed back to your
			OnImport function.
		*/
	virtual int32 DetectFileType(	const entry_ref	*ref,
							BNode			*node,
							struct stat		*st,
							const char		*filetype )
	{ return -1; }

		/**	OnImport is called when the plug-in is invoked as an import filter.
			Off course, in order for that to occur, you'd have to have called
			AddMenuItem( <name>, Import_List, <msg> ) to register this plugin as
			an importer. (Note: In the current version, the name and message
			are ignored -- auto-detection is used instead. Later, we may want
			a method to disable auto-detection, so that specific loaders can
			be loaded. This also means that the inMsg parameter to this function
			is also NULL)
		*/
	virtual void OnImport( BMessage *inMsg, entry_ref *ref, int32 inFileType ) {}

		/**	OnExport is called when the plug-in is invoked as an export filter.
			Off course, in order for that to occur, you'd have to have called
			AddMenuItem( <name>, Export_List, <msg> ) to register this plugin as
			an importer. The document id is the ID of the document to
			be exported.
		*/
	virtual void OnExport( BMessage *inMsg, int32 inDocID, entry_ref *ref ) {}

	/**	Function to create a new document. */
	MeVDocHandle				NewDocument(
									const char *name,
									bool showWindow);
								
		/**	Function to gain access to a MeV document. You should
			release the handle when you are done with it. If the document
			was not found, it returns NULL.
		*/
	MeVDocHandle FindDocument( int32 inDocID );

		/**	Another function to find a documentm, this time by name.
			Be sure and release the handle when finished.
		*/
	MeVDocHandle FindDocument( char *inDocName );
	
		/**	Returns a handle to the first track. */
	MeVDocHandle FirstDocument();

		/** Return text of error */
	char *LookupErrorText( status_t error );
};

#ifdef __POWERPC__
#pragma export off
#endif

/** ---------------------------------------------------------------------
	MeVDocRef is a reference to a MeV Document.
 */
 
#ifdef __POWERPC__
#pragma export on
#endif

class MeVDocRef {
	friend class MeVPlugIn;

	void			*data;
	
	MeVDocRef();
	~MeVDocRef();

public:
		/**	Repositions this handle to point to the next document. */
	bool NextDocument();

		/**	Function to create a new MeV track. You should
			release the handle when you are done with it. If the track
			could not be created, it returns NULL.
		*/
	MeVTrackHandle NewEventTrack( TClockType inClockType );

		/**	Function to gain access to a MeV track. You should
			release the handle when you are done with it. If the track
			was not found, it returns NULL.
		*/
	MeVTrackHandle FindTrack( int32 inTrackID );

		/**	Function to gain access to a MeV track. You should
			release the handle when you are done with it. If the track
			was not found, it returns NULL.
			Note that the master track has an ID of 0.
		*/
	MeVTrackHandle FindTrack( char *inTrackName );
	
		/**	Returns a handle to the active master track. */
	MeVTrackHandle ActiveMasterTrack();

		/**	Returns a handle to the first track. */
	MeVTrackHandle FirstTrack();

		/**	Function to release a track handle */
	void ReleaseTrack( MeVTrackHandle );
	
		/**	Get the ID of this document */
	int32 GetID();

		/**	Get the name of this document */
	void GetName( char *name, int32 inMaxChars );
	
		/**	Get the initial tempo of this document */
	double GetInitialTempo();
	
		/**	Set the initial tempo of this document */
	void SetInitialTempo(double tempo);
	
		/**	Iterate through available MIDI consumers.  cookie should be 0 for
		 *	the initial call.  Returns false if there are no more consumers.
		 *	nameLength is the size of the outName buffer allocated by the caller
		 *	(includes terminating NULL).
		 */
	bool GetNextMidiConsumer(int32* cookie, int* outConsumerID, char* outName, size_t nameLength);
	
		/**	Return the ID of the internal synth. */
	int GetInternalSynthConsumerID();
	
		/**	Add a new destination.  Returns the destination ID.
		 *  Channels are 1-16.
		 */
	int NewDestination(const char* name, int consumerID, int channel);
	
		/**	Return the MIDI channel for a destination, negative if destination does not exist.
		 *  Channels are 1-16.
		 */
	int GetChannelForDestination(int destinationID);

		/**	Call this function to add an event operator to
			the document's list of available operators.
		*/
	void AddEventOperator( EventOp *inOper );

		/**	Call this function to remove an event operator to
			the document's list of available operators.
		*/
	void RemoveEventOperator( EventOp *inOper );

		/**	Call this function to enable or disable an event operator
			on all tracks. (This corresponds to the "listen all"
			checkmark in the operators window.)
		*/
	void EnableEventOperator( EventOp *inOper, bool inEnabled );
	
		/** Call this to open the document's assembly window. */
	void ShowWindow();
};

#ifdef __POWERPC__
#pragma export off
#endif

	/**	Track ID of master track */
const int32		MasterTrack_ID = 0;
 
/** ---------------------------------------------------------------------
	MeVTrackRef is a reference to a MeV Document.
 */
 
	//	REM: This is an observer of the track! (that's where "release" comes in)
	//	(It can even handle events!)
#ifdef __POWERPC__
#pragma export on
#endif

class MeVTrackRef {
	friend class MeVDocRef;

	void *trackData;	// track
	void *docData;		// owner (document)
	void *undo;
	
	MeVTrackRef(void* doc, void* track);
	~MeVTrackRef();

public:

		/**	Repositions this handle to point to the next track. */
	bool NextTrack();

		/**	Get the ID of this track */
	int32 GetID();

		/**	Get the name of this track */
	void GetName( char *name, int32 inMaxChars );

		/**	Change the name of this track */
	void SetName( char *name );
	
		/**	Get whether it is a real-time or metered track */
	TClockType GetClockType();

		/**	Select all events on this track. */
	void SelectAll();

		/**	Deselect all events on this track. */
	void DeselectAll();

		/**	Delete all selected events on this track */
	void DeleteSelected();
	
		/**	Call this function to enable or disable an event operator
			for this track. (This corresponds to the "listen"
			checkmark in the operators window.)
		*/
	void EnableEventOperator( EventOp *inOper, bool inEnabled );

		/**	Call this function to permanently modify events
			in the track using the supplied operator
		*/
	void ApplyEventOperator( EventOp *inOper, bool inSelectedOnly );

		/**	Start a new undo record for this track */
	bool BeginUndoAction( char *inActionLabel );

		/**	End a undo record for this track */
	void EndUndoAction( bool keep );

		/* Merge a list of sorted events into the EventList. */
	void Merge( CEvent *inEventArray, long inEventCount );

		/**	Returns a handle to the first event in track. */
	MeVEventHandle FirstEvent();

		/**	Function to release a track handle */
	void ReleaseEventRef( MeVEventHandle );
	
	int32 Duration() const;

#if 0
		/**	Returns the number of selected events. */
	int CountSelected();

		/**	Extract all selected events from a sequence and place into linear buffer.
			Linear buffer is allocated by this function, you must delete it.
		*/
	long ExtractSelected( EventPtr &result, EventListUndoAction *inAction );

		/**	Copy all selected events from a sequence and place into linear buffer.
			Linear buffer is allocated by this function, you must delete it.
		*/
	long CopySelected( EventPtr &result );

		/** Insert a block of time */
	void InsertTime( long startTime, long offset, EventListUndoAction *inAction );
	
		/** Delete a block of time. */
	void DeleteTime( long startTime, long offset, EventListUndoAction *inAction );
	
		/** Returns the count of the number of events in this track */
	int32 CountItems();

		/**	Returns the start time of the earliest selected item. */
	long MinSelectTime();
	
		/**	Returns the maximum time within the current selection. */
	long MaxSelectTime();
	
		/**	Undo the undoable action, if any. */
	bool Undo();

		/**	Redo the redoable action, if any. */
	bool Redo();
	
		/**	Apply an operator to the selected events. */
	void ModifySelectedEvents(
		CEventEditor			*inEditor,			// Active editor window, or NULL
		EventOp				&op,				// Operation to apply
		const char			*inActionLabel );	// Menu label for undo

		/**	Apply an operator to the selected events. */
	void ModifyAllEvents(
		CEventEditor			*inEditor,			// Active editor window, or NULL
		EventOp				&op,				// Operation to apply
		const char			*inActionLabel );	// Menu label for undo

		/**	Copy selected events, and apply the operator to the copy. */
	void CopySelectedEvents(
		CEventEditor			*inEditor,			// Active editor window, or NULL
		EventOp				&op,				// Operation to apply
		const char			*inActionLabel );	// Menu label for undo

		/**	Create a new event, and do all appropriate updates, notifications, etc. */
	void CreateEvent(
		CEventEditor			*inEditor,
		CEvent				&newEv,
		const char			*inActionLabel );

		/**	Return the current selection type. */
//	virtual enum CTrack::E_SelectionTypes SelectionType();

//	const CEvent *CurrentEvent() { return currentEvent.Peek( 0 ); }

		/**	Return the current gridsnap size. */
	long TimeGridSize();
	
		/**	Return true if gridsnap is enabled for this item. */
	bool GridSnapEnabled();
	
		/**	Enable or disable gridsnap. */
	void EnableGridSnap( bool inEnabled );
	
		/**	Set the track grid size. */
	void SetTimeGridSize( int32 inGrid );
	
		/**	Get start of playback section. */
	int32 SectionStart();
	
		/**	Get end of playback section. */
	int32 SectionEnd();
	
		/**	Set the playback section. */
	void SetSection( int32 start, int32 end );

#if 0
		/**	Overrides AddUndoAction from CObservable to deal with
			master track issues. */
	void AddUndoAction( UndoAction *inAction );

		/**	Write the track to the MeV file. */
	void WriteTrack( CIFFWriter &writer );
	
		/** Read one chunk from the MeV file. */
	void ReadTrackChunk( CIFFReader &reader );
#endif
	
			// ---------- Channel locking
			
		/** Test if a channel is locked... */
	bool IsChannelLocked( int32 inChannel );

		/** Lock or unlock a channel */
	void LockChannel( int32 inChannel, bool inLocked = true );
	
		/** TRUE if this event is "locked" (but only if the event even has a channel.) */
	bool IsChannelLocked( const CEvent &ev );
	
		/** TRUE if any event in this track is using this channel. */
	bool IsChannelUsed( int32 inChannel ) { return usedChannels[ inChannel ] ? true : false; }

#endif

/*	BeginTransaction( char *inActionName );
	EndTransaction();
*/

/*	void Mute( bool inMute );
	bool Muted() const;
	long LastEventTime() { return lastEventTime; }
	SelectionType?? */
	
};

class MeVEventRef {
private:
	friend class MeVTrackRef;

	void			*data;				// event marker

		/**	pass address of an event list (copied)
		 */
	MeVEventRef(void* eventList);

public:

	~MeVEventRef();

		/**	Returns a copy of the curent event. Returns false if at end
			of track.
		*/
	bool GetEvent( CEvent *inEvent ) const;

		/**	Returns a pointer to the current event. Do not modify this
			directly, otherwise the undo information will not be recorded.
			Returns NULL if not pointing to a real event.
		*/
	const CEvent *EventPtr();

		/**	Return true if pointing at a real event. Returns false if the
			track has no events, or the reference is pointing beyond the
			last event in the track.
		*/
	bool Valid();
	
#if 0
		/**	Modifies the current event, replacing it with the given event.
		*/
	bool Modify( const CEvent *inEv );

		/**	Delete the current event. All changes are discarded, and the
			marker is positioned to the event after the deleted one.
		*/
	void Delete();
#endif

		//	Navigation functions
		
		/**	Seek forward or backwards a given number of events. Positive numbers
			seek forward, negative numbers seek backwards.
		*/
	bool Seek( int32 inSeekCount );
	
#if 0
		/**	Position the reference to the first event which has a start time
			that is greater than or equal to the given time.
			@return false if inStartTime is greater than the start time of any event.
		*/
	bool SeekToTime( int32 inStartTime );
	
		/**	Position the reference to the first selected event in the track.
			@return true if there was in fact a selected event.
		*/
	bool SeekToFirstSelected();
	
		/**	Position the reference to the next selected event after the
			current one.
			@return true if there was a next selected event.
		*/
	bool SeekToNextSelected ();
	
		/**	Position the reference to the first event which overlaps
			the given range of time, even if the event starts before
			or ends after that range.
			@return false if there are no such events, true otherwise.
		*/
	bool SeekToFirstInRange( int32 inStartTime, int32 inEndTime );
	
		/**	Position the reference to the next event after the current
			one which overlaps the given range of time, even if the event
			starts before or ends after that range.
			@return false if there are no such events, true otherwise.
		*/
	bool SeekToNextInRange ( int32 inStartTime, int32 inEndTime );
#endif

		/**	Position the reference to the first event in the track.
			@return true if the sequence is non-empty.
		*/
	bool SeekToFirst();

#if 0
		/**	Position the reference to the last event in the track.
			@return true if the sequence is non-empty.
		*/
	bool SeekToLast();

		/**	Position the reference just after the last event in the track.
			@return true if the sequence is non-empty.
		*/
	bool SeekToEnd();

		/**	Returns true if the reference is positioned to the first
			event in the track.
		*/
	bool IsAtStart();
	
		/**	Returns true if the reference is positioned just beyond the
			last event in the track.
		*/
	bool IsAtEnd();
#endif
};

#ifdef __POWERPC__
#pragma export off
#endif

#ifdef __POWERPC__
#pragma export on
#endif

extern "C" {
#ifdef __INTEL__
 #if !defined( MEV_SHARED_LIBRARY )
  MeVPlugIn __declspec(dllexport) *CreatePlugin();
 #endif
#else
 extern MeVPlugIn *CreatePlugin();
#endif
}

#ifdef __POWERPC__
#pragma export off
#endif

#endif /* __C_MeVPlugin_H__ */
