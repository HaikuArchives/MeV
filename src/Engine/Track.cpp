/* ===================================================================== *
 * Track.cpp (MeV/Engine)
 * ===================================================================== */

#include "Track.h"
#include "MeVDoc.h"
#include "MeVFileID.h"
#include "IFFWriter.h"
#include "IFFReader.h"

// Support Kit
#include <Debug.h>

// Debugging Macros
#define D_ALLOC(x) //PRINT(x)		// Constructor/Destructor
#define D_HOOK(x) //PRINT(x)		// Hook Functions
#define D_ACCESS(x) //PRINT(x)		// Accessors
#define D_OPERATION(x) // PRINT(x)	// Operations

// ---------------------------------------------------------------------------
// Constructor/Destructor

CTrack::CTrack(
	CMeVDoc &inDoc,
	TClockType &cType,
	int32 inID,
	char *name)
	:	document(inDoc),
		muted(false),
		muteFromSolo(false),
		solo(false),
		recording(false),
		deleted(false),
		lastEventTime(0),
		logicalLength(0),
		clockType(cType)
{
		// REM: Calculate a unique track ID.
	if (inID < 0) trackID = document.GetUniqueTrackID();
	else trackID = inID;

		// Initialize the signature map
	static CSignatureMap::SigChange relSigMap[] = {
		{	0, Ticks_Per_QtrNote, Ticks_Per_QtrNote * 4 },
	};
		
	static CSignatureMap::SigChange absSigMap[] = {
		{	0, 1000, 10000 },
	};
		
	strcpy(m_name, name ? name : "Untitled Track");

		// Initialize the signature map.
	sigMap.entries		= (cType != ClockType_Metered ? absSigMap : relSigMap);
	sigMap.numEntries	= 1;
	sigMap.clockType		= cType;
}

CTrack::~CTrack()
{
	// REM: Delete the signature map.
}

// ---------------------------------------------------------------------------
// Operations

void
CTrack::Delete()
{
	if (Document().tracks.RemoveItem(this))
	{
		deleted = true;
		Document().SetModified();

		// Tell everyone that the track has been deleted
		CUpdateHint hint;
		hint.AddInt32("TrackID", GetID());
		hint.AddInt32("TrackAttrs", Update_Name);
		Document().PostUpdateAllTracks(&hint);
		Document().NotifyUpdate(CMeVDoc::Update_DelTrack, NULL);
	}
}

void
CTrack::Undelete(
	int32 originalIndex)
{
	if (Document().tracks.AddItem(this, originalIndex))
	{
		deleted = false;
		Document().SetModified();

		// Tell everyone that the track has been re-added
		CUpdateHint hint;
		hint.AddInt32("TrackID", GetID());
		hint.AddInt32("TrackAttrs", Update_Name);
		Document().PostUpdateAllTracks(&hint);
		Document().NotifyUpdate(CMeVDoc::Update_AddTrack, NULL);
	}
}

void
CTrack::SetName(
	const char *name)
{
	strncpy(m_name, name, 64);

	// Tell everyone that the name of the track changed
	CUpdateHint hint;
	hint.AddInt32("TrackID", GetID());
	hint.AddInt32("TrackAttrs", Update_Name);
	document.PostUpdateAllTracks(&hint);
	document.PostUpdate(&hint, NULL);
}

void
CTrack::NotifyUpdate(
	int32 hintBits,
	CObserver *source)
{
	CUpdateHint hint;
	hint.AddInt32("TrackID", GetID());
	hint.AddInt32("TrackAttrs", hintBits);
	
	PostUpdate(&hint, source);
	document.PostUpdate(&hint, source);
}

	/**	Add update hint bits to an update message. */
void CTrack::AddUpdateHintBits( CUpdateHint &inHint, int32 inBits )
{
	int32			flags;

	if (inHint.FindInt32( "TrackAttrs", 0, &flags ) == B_NO_ERROR )
	{
		flags |= inBits;
		inHint.ReplaceInt32( "TrackAttrs", 0, flags );
	}
	else inHint.AddInt32( "TrackAttrs", inBits );
}

enum ETrackFlags {
	Track_Muted = (1<<0),
	Track_Solo = (1<<1),
	Track_Recording = (1<<2)
};

void CTrack::ReadTrackChunk( CIFFReader &reader )
{
	uint8		flags, clock;

	switch (reader.ChunkID()) {
	case Track_Header_ID:
		reader >> trackID;
		reader >> flags >> clock;
		clockType = (TClockType)clock;
		if (flags & Track_Muted)		muted = true;
		if (flags & Track_Solo)		solo = true;
		if (flags & Track_Recording)	recording = true;
		break;
		
	case Track_Name_ID:
		reader.MustRead(m_name, MIN(reader.ChunkLength(), sizeof(m_name)));
		m_name[sizeof(m_name - 1)] = '\0';
		break;
	}
}

void CTrack::WriteTrack( CIFFWriter &writer )
{
	uint8		flags = 0, clock = clockType;
	
	if (trackID == 0 || trackID == 1) return;

	writer.Push( Track_Header_ID );
	writer << trackID;
	if (muted)	flags |= Track_Muted;
	if (solo)		flags |= Track_Solo;
	if (recording)	flags |= Track_Recording;
	writer << flags << clock;
	writer.Pop();

	writer.WriteChunk(Track_Name_ID, m_name, strlen(m_name) + 1);
}

// ---------------------------------------------------------------------------
// CTrackDeleteUndoAction: Constructor/Destructor

CTrackDeleteUndoAction::CTrackDeleteUndoAction(
	CTrack *track)
	:	m_track(track),
		m_index(-1)
{
	D_ALLOC(("CTrackDeleteUndoAction::CTrackDeleteUndoAction()\n"));

	for (m_index = 0; m_index < m_track->Document().CountTracks(); m_index++)
		if (m_track->Document().TrackAt(m_index) == m_track)
			break;

	m_track->Delete();
}

CTrackDeleteUndoAction::~CTrackDeleteUndoAction()
{
	D_ALLOC(("CTrackDeleteUndoAction::CTrackDeleteUndoAction()\n"));
}

// ---------------------------------------------------------------------------
// CTrackDeleteUndoAction: UndoAction Implementation

void
CTrackDeleteUndoAction::Redo()
{
	D_HOOK(("CTrackDeleteUndoAction::Redo()\n"));

	m_track->Delete();
}

void
CTrackDeleteUndoAction::Undo()
{
	D_HOOK(("CTrackDeleteUndoAction::Undo()\n"));

	m_track->Undelete(m_index);
}

// ---------------------------------------------------------------------------
// CTrackRenameUndoAction: Constructor/Destructor

CTrackRenameUndoAction::CTrackRenameUndoAction(
	CTrack *track,
	BString newName)
	:	m_track(track)
{
	D_ALLOC(("CTrackRenameUndoAction::CTrackRenameUndoAction()\n"));

	m_name = m_track->Name();
	m_track->SetName(newName.String());

	m_track->Document().SetModified();
}

CTrackRenameUndoAction::~CTrackRenameUndoAction()
{
	D_ALLOC(("CTrackRenameUndoAction::~CTrackRenameUndoAction()\n"));

}

// ---------------------------------------------------------------------------
// CTrackRenameUndoAction: UndoAction Implementation

void
CTrackRenameUndoAction::Redo()
{
	D_HOOK(("CTrackRenameUndoAction::Redo()\n"));

	BString oldName = m_track->Name();
	m_track->SetName(m_name.String());
	m_name = oldName;
	m_track->Document().SetModified();
}

void
CTrackRenameUndoAction::Undo()
{
	D_HOOK(("CTrackRenameUndoAction::Undo()\n"));

	BString oldName = m_track->Name();
	m_track->SetName(m_name.String());
	m_name = oldName;
	m_track->Document().SetModified();
}

// END - Track.cpp
