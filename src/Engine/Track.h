/* ===================================================================== *
 * Track.h (MeV/Engine)
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
 *  Abstract base class for tracks
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

#ifndef __C_Track_H__
#define __C_Track_H__

#include "MeV.h"
#include "Observer.h"
#include "SignatureMap.h"

// Support Kit
#include <String.h>

class CMeVDoc;
class CIFFReader;
class CIFFWriter;

class CTrack
	:	public CObservableSubject
{
	friend class CTrackDeleteUndoAction;

private:						// Instance Data

	short			trackID;				// id number of this track
	char			m_name[64];				// name of the track
	CMeVDoc			&document;				// pointer to parent document
	
	bool			muted;						// track is muted
	bool			muteFromSolo;				// track is muted because of solo
	bool			solo;						// track is soloed
	bool			recording;					// track is recording
	bool			deleted;						// track hidden because deleted

protected:
		// Time of the last event on the track
	long				lastEventTime,			// duration of track in favored units
					logicalLength;			// Logical play-length of track
	TClockType		clockType;				// Track's clock type
	CSignatureMap	sigMap;					// Temporal structure

public:

		// Types of selection for each track
	enum E_SelectionTypes {
			// These describe the state of the current selection
		Select_None = 0,					// nothing is selected
		Select_Single,						// a single event is selected
		Select_Subset,						// use select bits for each item

			// These describe how the selection got that way
// 	Select_RealRange,					// selection by real time
// 	Select_MeterRange,					// selection by metered time
// 	Select_Vertex,						// vertex/vertices of a countour
	};

public:
		// Track update hints

	enum ETrackUpdateHintBits {

		Update_Name		= (1<<0),			// Track name changed
		Update_Duration	= (1<<1),			// Track duration changed
		Update_Flags		= (1<<2),			// Muted / recording flag
		Update_SigMap	= (1<<3),			// Signature map changed
		Update_Operators	= (1<<4),			// Operator list changed
		Update_Section	= (1<<5),			// Update section markers
		Update_Summary	= (1<<6),			// Track summary info changed...
		Update_TempoMap	= (1<<7)			// Tempo map changed
	};

		// ---------- Constructors

		/** New track constructor */
	CTrack( CMeVDoc &inDoc, TClockType &cType, int32 inID = -1, char *inName = NULL );
	
		/** Deserialization constructor */
	CTrack( CMeVDoc &inDoc, CIFFReader &inReader );

		/** Destructor */
	~CTrack();
	
		// ---------- Loading and saving

		/** Write track data to MeV file. */
	virtual void WriteTrack( CIFFWriter &writer );

		/** Read track data from MeV file. */
	virtual void ReadTrackChunk( CIFFReader &reader );

		// ---------- Getters

		/**	Return pointer to track name */
	const char *Name() const { return m_name; }
	short GetID() const { return trackID; }
	bool Muted() { return muted; }
	bool MutedFromSolo() { return muteFromSolo; }
	bool Solo() { return solo; }
	bool Recording() { return recording; }
	long LastEventTime() { return lastEventTime; }
	int32 LogicalLength() { return logicalLength; }
	CMeVDoc &Document() { return document; }
	CSignatureMap &SigMap() { return sigMap; }
	virtual enum E_SelectionTypes SelectionType() = 0;
	TClockType ClockType() { return clockType; }
	virtual int32 Bytes() { return sizeof *this; }
	bool Deleted() { return deleted; }
	virtual int32 TrackType() const = 0;
	
		// ---------- Setters

	void SetName( const char *name );
	void SetMuted( bool inMute ) { muted = inMute; NotifyUpdate( Update_Flags, NULL ); }
	void SetRecording( bool inRecord ) { recording = inRecord; NotifyUpdate( Update_Flags, NULL ); }
// void SetSolo( bool inSolo );

		// ---------- Operations

	/** Delete this track from the document. */
	void						Delete();

	/** Undelete a previously deleted track.
		@param originalIndex The Index at which the track was before deletion
	 */
	void						Undelete(
									int32 originalIndex);

	virtual void SelectAll() = 0;
	virtual void DeleteSelection() = 0;

		/**	Notify all observers (including possibly observers of the document
			as well) that some attributes of this track have changed. */
	void NotifyUpdate( int32 inHintBits, CObserver *source );
	
		/**	Add update hint bits to an update message. */
	static void AddUpdateHintBits( CUpdateHint &inHint, int32 inBits );
};

// ---------------------------------------------------------------------------
// UndoAction for deleting a track

class CTrackDeleteUndoAction
	:	public UndoAction
{

public:							// Constructor/Destructor

								CTrackDeleteUndoAction(
									CTrack *track);

								~CTrackDeleteUndoAction();

public:							// UndoAction Implementation

	const char *				Description() const
								{ return "Delete Track"; }

	void						Redo();

	int32						Size()
								{ return m_track->Bytes(); }

	void						Undo();

private:						// Instance Data

	CTrack *					m_track;

	int32						m_index;
};

// ---------------------------------------------------------------------------
// UndoAction for renaming a track

class CTrackRenameUndoAction
	:	public UndoAction
{

public:							// Constructor/Destructor

								CTrackRenameUndoAction(
									CTrack *track,
									BString newName);

								~CTrackRenameUndoAction();

public:							// UndoAction Implementation

	const char *				Description() const
								{ return "Rename Track"; }

	void						Redo();

	int32						Size()
								{ return m_name.Length(); }

	void						Undo();

private:						// Instance Data

	CTrack *					m_track;

	BString						m_name;
};

#endif /* __C_Track_H__ */
