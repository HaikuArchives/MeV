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

CTrack::CTrack( CMeVDoc &inDoc, TClockType &cType, int32 inID, char *inName  )
	: document( inDoc ),
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
		
	strcpy( name, inName ? inName : "Untitled Track" );

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

void CTrack::SetName( const char *inName )
{
	strncpy( name, inName, sizeof name - 1 );
	name[ sizeof name - 1 ] = '\0';
	
		// Tell everyone that the name of the track changed
	NotifyUpdate( Update_Name, NULL );
}

	/**	Notify all observers (including possibly observers of the document
		as well) that some attributes of this track have changed. */
void CTrack::NotifyUpdate( int32 inHintBits, CObserver *source )
{
	CUpdateHint		hint;
	
	hint.AddInt32( "TrackID", GetID() );
	hint.AddInt32( "TrackAttrs", inHintBits );
	
	PostUpdate( &hint, source );
	document.PostUpdate( &hint, source );
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
		reader.MustRead( name, MIN( reader.ChunkLength(), sizeof name ) );
		name[ sizeof name - 1 ] = '\0';
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

	writer.WriteChunk( Track_Name_ID, name, strlen( name ) + 1 );
}

// ---------------------------------------------------------------------------
// CTrackDeleteUndoAction: Constructor/Destructor

CTrackDeleteUndoAction::CTrackDeleteUndoAction(
	CTrack *track)
	:	m_track(track)
{
	D_ALLOC(("CTrackDeleteUndoAction::CTrackDeleteUndoAction()\n"));

	for (m_index = 0; m_index < m_track->Document().CountTracks(); m_index++)
		if (m_track->Document().TrackAt(m_index) == m_track)
			break;

	if (m_track->Document().tracks.RemoveItem(m_track))
	{
		m_track->deleted = true;
		m_track->Document().SetModified();
		m_track->Document().NotifyUpdate( CMeVDoc::Update_DelTrack, NULL );
	}
}

CTrackDeleteUndoAction::~CTrackDeleteUndoAction()
{
	D_ALLOC(("CTrackDeleteUndoAction::CTrackDeleteUndoAction()\n"));

	CRefCountObject::Release(m_track);
}

// ---------------------------------------------------------------------------
// CTrackDeleteUndoAction: UndoAction Implementation

void
CTrackDeleteUndoAction::Redo()
{
	D_HOOK(("CTrackDeleteUndoAction::Redo()\n"));

	if (m_track->Document().tracks.RemoveItem(m_track))
	{
		m_track->deleted = true;
		m_track->Document().SetModified();
		m_track->Document().NotifyUpdate(CMeVDoc::Update_DelTrack, NULL);
	}
	CRefCountObject::Release(m_track);
}

void
CTrackDeleteUndoAction::Undo()
{
	D_HOOK(("CTrackDeleteUndoAction::Undo()\n"));

	m_track->Acquire();
	if (m_track->Document().tracks.AddItem(m_track, m_index))
	{
		m_track->deleted = false;
		m_track->Document().SetModified();
		m_track->Document().NotifyUpdate(CMeVDoc::Update_AddTrack, NULL);
	}
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
	m_track->NotifyUpdate(CTrack::Update_Name, NULL);
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
	m_track->NotifyUpdate(CTrack::Update_Name, NULL);
}

void
CTrackRenameUndoAction::Undo()
{
	D_HOOK(("CTrackRenameUndoAction::Undo()\n"));

	BString oldName = m_track->Name();
	m_track->SetName(m_name.String());
	m_name = oldName;
	m_track->Document().SetModified();
	m_track->NotifyUpdate(CTrack::Update_Name, NULL);
}

// END - Track.cpp
