/* ===================================================================== *
 * Track.cpp (MeV/Engine)
 * ===================================================================== */

#include "Track.h"
#include "MeVDoc.h"
#include "MeVFileID.h"
#include "IFFWriter.h"
#include "IFFReader.h"
#include "TrackWindow.h"

// Support Kit
#include <Debug.h>

// Debugging Macros
#define D_ALLOC(x) //PRINT(x)		// Constructor/Destructor
#define D_HOOK(x) //PRINT(x)		// Hook Functions
#define D_ACCESS(x) //PRINT(x)		// Accessors
#define D_OPERATION(x) // PRINT(x)	// Operations
#define D_SERIALIZE(x) PRINT(x)		// Serialization

// ---------------------------------------------------------------------------
// Constructor/Destructor

CTrack::CTrack(
	CMeVDoc &inDoc,
	TClockType &cType,
	int32 inID,
	char *name)
	:	lastEventTime(0),
		logicalLength(0),
		clockType(cType),
		m_document(&inDoc),
		m_muted(false),
		m_muteFromSolo(false),
		m_solo(false),
		m_recording(false),
		m_deleted(false),
		m_window(NULL),
		m_windowSettings(NULL),
		m_openWindow(false)
{
	// REM: Calculate a unique track ID.
	if (inID < 0)
		m_trackID = Document().GetUniqueTrackID();
	else
		m_trackID = inID;

	// Initialize the signature map
	static CSignatureMap::SigChange relSigMap[] = {
		{	0, Ticks_Per_QtrNote, Ticks_Per_QtrNote * 4 },
	};
		
	static CSignatureMap::SigChange absSigMap[] = {
		{	0, 1000, 10000 },
	};
		
	strcpy(m_name, name ? name : "Untitled Track");

	// Initialize the signature map.
	sigMap.entries = (cType != ClockType_Metered ? absSigMap : relSigMap);
	sigMap.numEntries = 1;
	sigMap.clockType = cType;
}

CTrack::~CTrack()
{
	// REM: Delete the signature map.

	if (m_windowSettings)
	{
		delete m_windowSettings;
		m_windowSettings = NULL;
	}
}

// ---------------------------------------------------------------------------
// Accessors

void
CTrack::SetName(
	const char *name)
{
	strncpy(m_name, name, TRACK_NAME_LENGTH);

	// Tell everyone that the name of the track changed
	CUpdateHint hint;
	hint.AddInt32("TrackID", GetID());
	hint.AddInt32("TrackAttrs", Update_Name);
	Document().PostUpdateAllTracks(&hint);
	Document().PostUpdate(&hint, NULL);
}

void
CTrack::SetMuted(
	bool muted)
{
	m_muted = muted;
	NotifyUpdate(Update_Flags, NULL);
}

void
CTrack::SetRecording(
	bool recording)
{
	m_recording = recording;
	NotifyUpdate(Update_Flags, NULL);
}

void
CTrack::SetSolo(
	bool solo)
{
	m_solo = solo;
	NotifyUpdate(Update_Flags, NULL);
}

void
CTrack::SetWindowSettings(
	BMessage *message)
{
	m_windowSettings = message;

	// the window has been closed; reset the pointer
	m_window = NULL;
}

// ---------------------------------------------------------------------------
// Operations

void
CTrack::Delete()
{
	if (Document().tracks.RemoveItem(this))
	{
		m_deleted = true;
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
		m_deleted = false;
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
CTrack::NotifyUpdate(
	int32 hintBits,
	CObserver *source)
{
	CUpdateHint hint;
	hint.AddInt32("TrackID", GetID());
	hint.AddInt32("TrackAttrs", hintBits);
	
	PostUpdate(&hint, source);
	Document().PostUpdate(&hint, source);
}

void
CTrack::AddUpdateHintBits(
	CUpdateHint &inHint,
	int32 inBits)
{
	int32 flags;

	if (inHint.FindInt32( "TrackAttrs", 0, &flags ) == B_NO_ERROR )
	{
		flags |= inBits;
		inHint.ReplaceInt32( "TrackAttrs", 0, flags );
	}
	else
		inHint.AddInt32( "TrackAttrs", inBits );
}

// ---------------------------------------------------------------------------
// Serialization

enum ETrackFlags {
	Track_Muted = (1<<0),
	Track_Solo = (1<<1),
	Track_Recording = (1<<2)
};

void
CTrack::ReadTrackChunk(
	CIFFReader &reader)
{
	uint8 flags, clock;

	switch (reader.ChunkID())
	{
		case Track_Header_ID:
		{
			reader >> m_trackID;
			reader >> flags >> clock;
			clockType = (TClockType)clock;
			if (flags & Track_Muted)
				m_muted = true;
			if (flags & Track_Solo)
				m_solo = true;
			if (flags & Track_Recording)
				m_recording = true;
			break;
		}
		case Track_Name_ID:
		{
			reader.MustRead(m_name, MIN(reader.ChunkLength(), TRACK_NAME_LENGTH));
			break;
		}
		case CTrackWindow::FILE_CHUNK_ID:
		{
			int8 visible;
			reader >> visible;
			if (visible != 0)
				m_openWindow = true;
			BMessage *settings = new BMessage();
			CTrackWindow::ReadState(reader, settings);
			SetWindowSettings(settings);
		}
	}
}

void
CTrack::WriteTrack(
	CIFFWriter &writer)
{
	D_SERIALIZE(("CTrack<%s>::WriteTrack()\n", Name()));	

	uint8 flags = 0;
	uint8 clock = clockType;

	if (m_trackID == 0)
		// master real track doesn't have window settings!
		return;

	writer.Push(Track_Header_ID);
	writer << m_trackID;
	if (Muted())
		flags |= Track_Muted;
	if (Solo())
		flags |= Track_Solo;
	if (Recording())
		flags |= Track_Recording;
	writer << flags << clock;
	writer.Pop();

	// write track name
	writer.WriteChunk(Track_Name_ID, m_name, TRACK_NAME_LENGTH);

	// write window settings
	if (m_window)
	{
		if (m_windowSettings)
			delete m_windowSettings;
		m_windowSettings = new BMessage();
		m_window->ExportSettings(m_windowSettings);
	}
	if (m_windowSettings)
	{
		writer.Push(CTrackWindow::FILE_CHUNK_ID);
		// indicate visibility
		writer << (int8)(m_window != NULL);
		CTrackWindow::WriteState(writer, m_windowSettings);
		writer.Pop();
	}
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
