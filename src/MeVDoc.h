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
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __C_MevDoc_H__
#define __C_MevDoc_H__
#include "DestinationList.h"

//debug
#include <stdio.h>

#include "MeV.h"
#include "MeVApp.h"
#include "Document.h"
#include "Destination.h"
#include "Event.h"
#include "EventOp.h"
#include "WindowState.h"
#include "TempoMap.h"
class CMeVApp;
class CTrack;
class CAssemblyWindow;
class CEventTrack;
class CIFFReader;
class CIFFWriter;

class CMeVDoc : public CDocument {
	friend class CTrackDeleteUndoAction;
	friend class CTrack;
	
	BList				tracks;
	int32				m_newTrackID;

	BList				operators;			// Opers associated with doc
	BList				activeOperators;		// Operators in use...
	CEventTrack			*masterRealTrack,	// Master track (real)
					*masterMeterTrack,	// Master track (metered)
					*activeMaster;		// Which track is being edited
	CDestinationList *m_destlist;
	int32				defaultAttributes[ EvAttr_Count ];
	double			initialTempo;		// Initial tempo for document
	CTempoMap			tempoMap;
	bool				validTempoMap;

	CMeVDoc			*_me;
	
		// Master editing window -- we close this, we close
		// everything...
	CAssemblyWindow		*assemblyWindow;
	CWindowState			vChannelWinState,
						operatorWinState,
						docPrefsWinState,
						assemblyWinState;
						
	void Init();
	void AddDefaultOperators();

public:

	enum EWindowTypes {
		VChannel_Window	= 0,
		DocPrefs_Window,
		Assembly_Window,
		Operator_Window,
	};

		// Track update hints

	enum EDocUpdateHintBits {

		Update_Name		= (1<<0),			// Document name changed
		Update_AddTrack	= (1<<1),			// Track added
		Update_DelTrack	= (1<<2),			// Track deleted
		Update_TrackOrder	= (1<<3),			// Track order changed
		Update_Operator	= (1<<4),			// Operators changed
		Update_OperList	= (1<<5),			// list of operators changed
		Update_TempoMap	= (1<<6),			// list of operators changed
	};

	// CDocument();
	CMeVDoc( CMeVApp &inApp );
	CMeVDoc( CMeVApp &inApp, entry_ref &inRef );
	~CMeVDoc();
	
		// Create a new track (refcount = 1)
	CTrack *NewTrack( ulong inTrackType, TClockType inClockType );
	
	long GetUniqueTrackID();
	
	//Destination &GetVChannel( int channel ) { return vcTable[ channel ]; }
	//
	Destination  * GetVChannel (int channel) {
		Destination *dest;
		dest=m_destlist->get(channel);
		return (dest);
	}
	CDestinationList	* GetDestinationList () {
											return (m_destlist);
											}
	
		/**	Locate a track by it's ID, and Acquire it. (-1 for master track) */
	CTrack *FindTrack( long inTrackID );
	
		/**	Locate a track by it's name, and Acquire it. */
	CTrack *FindTrack( char *inTrackName );
	
		/**	Get the first track. */
	CTrack *FindNextHigherTrackID( int32 inID );
	
		/**	For iterating through tracks in list order. */
	int32 CountTracks() { return tracks.CountItems(); }
	
		/**	For iterating through tracks in list order. */
	CTrack *TrackAt( int32 index ) { return (CTrack *)tracks.ItemAt( index ); }
	
		/**	Get the value of a default attribute */
	int32 GetDefaultAttribute( enum E_EventAttribute inAttrType )
	{
		return defaultAttributes[ (int32)inAttrType ];
	}

		/**	Set the value of a default attribute */
	void SetDefaultAttribute( enum E_EventAttribute inAttrType, int32 inValue )
	{
			// REM: We should clip this value
		defaultAttributes[ (int32)inAttrType ] = inValue;
	}
	
		/**	Post an update message to all tracks. */
	void PostUpdateAllTracks( CUpdateHint *inHint );

		/**	Show or hide the window of a particular type */
	BWindow *ShowWindow( enum EWindowTypes inType );
	
		/**	Returns TRUE if a particular window type is open. */
	bool IsWindowOpen( enum EWindowTypes inType );
	
		/**	Return the number of operators. */
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
	
		/**	Return the number of operators. */
	int32 CountActiveOperators() { return activeOperators.CountItems(); }
	
		/**	Return the Nth operator (Increases reference count). */
	EventOp *ActiveOperatorAt( int32 index )
	{
		void		*ptr = activeOperators.ItemAt( index );
		
		if (ptr) return (EventOp *)((EventOp *)ptr)->Acquire();
		return NULL;
	}
	
		/**	Return the index of this operator in the list, or negative if none. */
	int32 ActiveOperatorIndex( EventOp *op )
	{
		return activeOperators.IndexOf( op );
	}
	
		/**	Add an operator to the document's list of operators. */
	void AddOperator( EventOp *inOp );

		/**	Delete an operator to the document's list of operators. */
	void RemoveOperator( EventOp *inOp );
	
		/**	Set an operator active / inactive. */
	void SetOperatorActive( EventOp *inOp, bool enabled );

		/**	Does a notification to all windows viewing the operator. */
	void NotifyOperatorChanged( EventOp *inOp );

		/**	Notify all observers (including possibly observers of the document
			as well) that some attributes of this document have changed. */
	void NotifyUpdate( int32 inHintBits, CObserver *source );
	
		/** return which of the two master tracks is selected. */
	CEventTrack *ActiveMaster() { return activeMaster; }
	
		/** Set the selected master track. Call with any track, ignored if
			not a master track.
		*/
	void SetActiveMaster( CEventTrack *inTrack );
	
		/**	Return a pointer to the tempo map. */
	const CTempoMap &TempoMap() { return tempoMap; }
	
		/** Get the initial tempo of this document */
	double InitialTempo() { return initialTempo; }
	
		/** Set the initial tempo of this document */
	void SetInitialTempo( double inTempo ) { initialTempo = inTempo; }

		/**	Change the ordering of the tracks... */
	void ChangeTrackOrder( int32 oldIndex, int32 newIndex );

		/** Calculate the name of a virtual channel from the instrument table. */
	void VirtualChannelName( int32 inChannelIndex, char *outBuf );

		/** Save the document to it's current location */
	void SaveDocument();

	virtual CMeVApp *			Application() const
								{ return static_cast<CMeVApp *>
										 (CDocument::Application()); }

		/** Export the document */
	void Export( BMessage *msg );

		/** Read the VCTable from a MeV file. */
	void ReadVCTable( CIFFReader &reader );

		/** Write the VCTable to a MeV file. */
	void WriteVCTable( CIFFWriter &writer );

		/** Read a single track */
	void ReadTrack( long inTrackType, CIFFReader &iffReader );
	
		/**	Sets a flag indicating that the tempo map needs to be recompiled.
			This is called by the track editing code whenever a tempo change
			event is modified.
		*/
	void InvalidateTempoMap() { validTempoMap = false; }
	
		/**	Returns true if the tempo map is valid, i.e. it correctly represents the
			compilation of all tempo events in the master tracks.
		*/
	bool ValidTempoMap() { return validTempoMap; }
	
		/**	Replace the current tempo map. */
	void ReplaceTempoMap( CTempoMapEntry *entries, int length );
};

#endif /* __C_MeVDoc_H__ */
