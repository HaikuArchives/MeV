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
#define D_OPERATION(x) //PRINT(x)	// Operations
#define D_SERIALIZE(x) //PRINT(x)		// Serialization

// ---------------------------------------------------------------------------
// Constructor/Destructor

CTrack::CTrack(
	CMeVDoc &inDoc,
	TClockType &cType,
	int32 inID,
	const char *name)
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
		m_openWindow(false),
		m_sectionStart(0),
		m_sectionEnd(0)
{
	D_ALLOC(("CTrack::CTrack()\n"));

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
		
	strcpy(m_name, name ? name : "Untitled Part");

	// Initialize the signature map.
	sigMap.entries = (cType != ClockType_Metered ? absSigMap : relSigMap);
	sigMap.numEntries = 1;
	sigMap.clockType = cType;
}

CTrack::~CTrack()
{
	// REM: Delete the signature map.

	// request all observers to stop immediately
	RequestDelete();

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

	// Tell everyone that the name of the track changed
	CUpdateHint hint;
	hint.AddInt32("TrackID", GetID());
	hint.AddInt32("TrackAttrs", Update_Flags);
	Document().PostUpdateAllTracks(&hint);
	Document().PostUpdate(&hint, NULL);
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
CTrack::SetSection(
	int32 begin,
	int32 end)
{
	ASSERT(IsWriteLocked());

	if (begin > end)
	{
		int32 temp = begin;
		begin = end;
		end = temp;
	}

	int32 beginMinTime = 0, beginMaxTime = 0;
	if (begin != m_sectionStart)
	{
		if (begin > m_sectionStart)
		{
			beginMinTime = m_sectionStart;
			beginMaxTime = begin;
		}
		else
		{
			beginMinTime = begin;
			beginMaxTime = m_sectionStart;
		}
	}
	int32 endMinTime = 0, endMaxTime = 0;
	if (end != m_sectionEnd)
	{
		if (end < m_sectionEnd)
		{
			endMinTime = end;
			endMaxTime = m_sectionEnd;
		}
		else
		{
			endMinTime = m_sectionEnd;
			endMaxTime = end;
		}
	}

	int32 minTime = beginMinTime;
	if (beginMinTime == 0)
		minTime = endMinTime;
	int32 maxTime = endMaxTime;
	if (endMaxTime == 0)
		maxTime = beginMaxTime;

	m_sectionStart = begin;
	m_sectionEnd = end;
	Document().SetModified();

	// Tell everyone that the section has changed
	CUpdateHint hint;
	hint.AddInt32("TrackID", GetID());
	hint.AddInt32("TrackAttrs", Update_Section);
	hint.AddInt32("MinTime", minTime);
	hint.AddInt32("MaxTime", maxTime);
	PostUpdate(&hint, NULL);
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
//	Document().PostUpdate(&hint, source);
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

void
CTrack::ReadChunk(
	CIFFReader &reader)
{
	switch (reader.ChunkID())
	{
		case TRACK_HEADER_CHUNK:
		{
			reader >> m_trackID;

			uint8 flags;
			reader >> flags;
			if (flags & MUTED)
				m_muted = true;
			if (flags & SOLO)
				m_solo = true;
			if (flags & RECORDING)
				m_recording = true;

			uint8 clock;
			reader >> clock;
			clockType = (TClockType)clock;
			break;
		}
		case TRACK_NAME_CHUNK:
		{
			reader.MustRead(m_name, MIN(reader.ChunkLength(),
										TRACK_NAME_LENGTH));
			break;
		}
		case TRACK_SECTION_CHUNK:
		{
			reader >> m_sectionStart;
			reader >> m_sectionEnd;
			break;
		}
		case TRACK_WINDOW_CHUNK:
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
CTrack::Serialize(
	CIFFWriter &writer)
{
	D_SERIALIZE(("CTrack<%s>::Serialize()\n", Name()));	

	CReadLock lock(this);

	if (m_trackID == 0)
		// master real track doesn't have window settings!
		// +++ why not ?
		return;

	// write track header
	writer.Push(TRACK_HEADER_CHUNK);
	writer << m_trackID;

	uint8 flags = 0;
	if (Muted())
		flags |= MUTED;
	if (Solo())
		flags |= SOLO;
	if (Recording())
		flags |= RECORDING;
	writer << flags;

	uint8 clock = clockType;
	writer << clock;
	writer.Pop();

	// write track name
	writer.WriteChunk(TRACK_NAME_CHUNK, m_name,
					  MIN(strlen(m_name) + 1, TRACK_NAME_LENGTH));

	// write section markers
	writer.Push(TRACK_SECTION_CHUNK);
	writer << SectionStart();
	writer << SectionEnd();
	writer.Pop();

	// write window settings
	if (m_window)
	{
		if (m_windowSettings)
		{
			delete m_windowSettings;
			m_windowSettings = NULL;
		}
		m_windowSettings = new BMessage();
		m_window->ExportSettings(m_windowSettings);
	}
	if (m_windowSettings)
	{
		writer.Push(TRACK_WINDOW_CHUNK);
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
