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
#include "Observable.h"
#include "Serializable.h"
#include "SignatureMap.h"

// Support Kit
#include <String.h>

class CMeVDoc;
class CTrackWindow;

#define TRACK_NAME_LENGTH 128

class CTrack
	:	public CObservable,
		public CSerializable
{
	friend class CMeVDoc;
	friend class CTrackDeleteUndoAction;

public:							// Constants

	/** Types of selection for each track */
	enum E_SelectionTypes {
		// These describe the state of the current selection
		Select_None = 0,					// nothing is selected
		Select_Single,						// a single event is selected
		Select_Subset,						// use select bits for each item
	};

	enum update_hints
	{
		/** Track name changed. */
		Update_Name			= (1 << 0),

		/** Track duration changed. */
		Update_Duration		= (1 << 1),

		/** Muted/Recording flag. */
		Update_Flags		= (1 << 2),

		/** Signature map changed. */
		Update_SigMap		= (1 << 3),

		/** Operator list changed. */
		Update_Operators	= (1 << 4),

		/** Update section markers. */
		Update_Section		= (1 << 5),

		/** Track summary info changed. */
		Update_Summary		= (1 << 6),

		/** Tempo map changed. */
		Update_TempoMap		= (1 << 7),

		/** Part starts using a destination, ie an event assigned to a 
		 *	destination not yet in use by this track has been added.
		 */
		Update_AddDest		= (1 << 8),

		/** Part stops using a destination. This usually means that the
		 *	last event using that destination has been deleted from the 
		 * 	part.
		 */
		Update_DelDest		= (1 << 9)
	};

	enum flags {
		MUTED 			= (1 << 0),
		SOLO			= (1 << 1),
		RECORDING		= (1 << 2)
	};
	
public:							// Constructor/Destructor

	/** New track constructor */
								CTrack(
									CMeVDoc &doc,
									TClockType &cType,
									int32 inID = -1,
									char *inName = NULL);

	/** Destructor */
								~CTrack();
	
public:							// Hook Functions

	virtual int32				Bytes()
								{ return sizeof *this; }

	virtual void				DeleteSelection() = 0;

	virtual void				SelectAll() = 0;

	virtual enum E_SelectionTypes SelectionType() = 0;

	virtual uint32				TrackType() const = 0;
	
public:							// CSerializable Implementation

	/** Read track data from MeV file. */
	virtual void				ReadChunk(
									CIFFReader &reader);

	/** Write track data to MeV file. */
	virtual void				Serialize(
									CIFFWriter &writer);

public:							// Accessors

	TClockType					ClockType() const
								{ return clockType; }

	bool						Deleted() const
								{ return m_deleted; }

	short						GetID() const
								{ return m_trackID; }

	const char *				Name() const
								{ return m_name; }
	void						SetName(
									const char *name);

	bool						Muted() const
								{ return m_muted; }
	void						SetMuted(
									bool muted);

	bool						MutedFromSolo() const
								{ return m_muteFromSolo; }

	bool						Solo() const
								{ return m_solo; }
	void						SetSolo(
									bool solo);

	bool						Recording() const
								{ return m_recording; }
	void						SetRecording(
									bool recording);

	long						LastEventTime() const
								{ return lastEventTime; }

	int32						LogicalLength() const
								{ return logicalLength; }

	CMeVDoc &					Document()
								{ return *m_document; }

	CSignatureMap &				SigMap()
								{ return sigMap; }

	/**	Get start of playback section. */
	int32						SectionStart() const
								{ return m_sectionStart; }

	/**	Get end of playback section. */
	int32						SectionEnd()
								{ return m_sectionEnd; }

	/**	Set the playback section. */
	void						SetSection(
									int32 start,
									int32 end);

	/** Remember the associated TrackWindows' state */
	void						SetWindowSettings(
									BMessage *message);
	BMessage *					GetWindowSettings() const
								{ return m_windowSettings; }

	CTrackWindow *				Window() const
								{ return m_window; }

public:							// Operations

	/**	Add update hint bits to an update message. */
	static void					AddUpdateHintBits(
									CUpdateHint &inHint,
									int32 inBits);

	/** Delete this track from the document. */
	void						Delete();

	/** Undelete a previously deleted track.
		@param originalIndex The Index at which the track was before deletion
	 */
	void						Undelete(
									int32 originalIndex);

	/**	Notify all observers (including possibly observers of the document
		as well) that some attributes of this track have changed.
	*/
	void						NotifyUpdate(
									int32 inHintBits,
									CObserver *source);

protected:						// Instance Data

	// Time of the last event on the track
	long						lastEventTime;

	// duration of track in favored units
	// Logical play-length of track
	long						logicalLength;
	
	// Track's clock type
	TClockType					clockType;
	
	// Temporal structure
	CSignatureMap				sigMap;

private:

	/** pointer to parent document */
	CMeVDoc *					m_document;

	/** id number of this track */
	short						m_trackID;

	/** name of the track */
	char						m_name[TRACK_NAME_LENGTH];

	/** track is muted */
	bool						m_muted;

	/** track is muted because of solo */
	bool						m_muteFromSolo;

	/** track is soloed */
	bool						m_solo;

	/** track is recording */
	bool						m_recording;

	/** track hidden because deleted */
	bool						m_deleted;

	/** a pointer to the associated window */
	CTrackWindow *				m_window;

	/** a message containing the settings of the associated TrackWindow */
	BMessage *					m_windowSettings;

	/** indicates that the associated window should be opened after loading
	 the track. */
	bool						m_openWindow;

	/** Start of playback section. */
	int32						m_sectionStart;

	/** End of playback section. */
	int32						m_sectionEnd;
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
								{ return "Delete Part"; }

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
								{ return "Rename Part"; }

	void						Redo();

	int32						Size()
								{ return m_name.Length(); }

	void						Undo();

private:						// Instance Data

	CTrack *					m_track;

	BString						m_name;
};

#endif /* __C_Track_H__ */
