/* ===================================================================== *
 * Destination.cpp (MeV/Engine)
 * ===================================================================== */

#include "Destination.h"

#include "EventTask.h"
#include "MeVDoc.h"
#include "MeVFileID.h"

// Support Kit
#include <Debug.h>

#define D_ALLOC(x) //PRINT(x)		// Constructor/Destructor
#define D_HOOK(x) //PRINT(x)		// Hook Functions
#define D_ACCESS(x) //PRINT(x)		// Accessors
#define D_OPERATION(x) //PRINT(x)	// Operations
#define D_SERIALIZE(x) //PRINT(x)	// Serialization

// ---------------------------------------------------------------------------
// Constants Initialization

const rgb_color DEFAULT_COLORS[16] = 
{
	{ 255,  64,  64, 255 },
	{  64, 128,  64, 255 },
	{ 164,  32, 164, 255 },
	{ 164, 164,  32, 255 },			
	{   0, 255,  64, 255 },			
	{   0, 255,   0, 255 },			
	{   0, 160, 160, 255 },			
	{   0, 255, 160, 255 },			
	{  64, 255, 255, 255 },
	{  47, 130, 255, 255 },			
	{ 128, 128, 255, 255 },			
	{ 200,   0, 255, 255 },			
	{ 255,   0, 255, 255 },		
	{ 255, 128, 255, 255 },	
	{ 192, 192, 192, 255 },
	{ 128, 128,   0, 255 }
};

enum flags
{		
	MUTED			= (1<<0),

	MUTED_FROM_SOLO	= (1<<1),

	SOLO			= (1<<2),

	DISABLED		= (1<<3),

	DELETED			= (1<<4)
};

// ---------------------------------------------------------------------------
// Constructor/Destructor

CDestination::CDestination(
	unsigned long type,
	long id,
	const char *name,
	CMeVDoc *document)
	:	m_doc(document),
		m_type(type),
		m_id(id),
		m_latency(0),
		m_flags(0),
		m_color(DEFAULT_COLORS[id % 15])
{
	D_ALLOC(("CDestination::CDestination()\n"));

	CWriteLock lock(this);
	snprintf(m_name, DESTINATION_NAME_LENGTH, "%s %ld", name, id + 1);
}

CDestination::CDestination(
	unsigned long type,
	CMeVDoc *document)
	:	m_doc(document),
		m_type('MIDI'),
		m_id(0),
		m_latency(0),
		m_flags(0)
{
	D_ALLOC(("CDestination::CDestination(deserialize)\n"));
}

CDestination::~CDestination()
{
	D_ALLOC(("CDestination::~CDestination()\n"));

	// request all observers to stop immediately
	RequestDelete();
}

// ---------------------------------------------------------------------------
// Hook Functions

void
CDestination::Stack(
	CEvent &event,
	const CEventTask &task,
	CEventStack &stack,
	long duration)
{
	D_HOOK(("CDestination::Stack(%s, %ld)\n",
			event.NameText(), duration));

	event.stack.destination = this;

	// this does not seem to work, disable for release
//	event.stack.start -= (Latency() / 1000);
}

// ---------------------------------------------------------------------------
// Accessors

unsigned long
CDestination::Type() const
{
	D_ACCESS(("CDestination::Type()\n"));

	return m_type;
}

long
CDestination::ID() const
{
	D_ACCESS(("CDestination::ID()\n"));

	return m_id;
}

bool
CDestination::IsValid() const
{
	D_ACCESS(("CDestination::IsValid()\n"));
	ASSERT(IsReadLocked());

	return ((m_flags & DISABLED) || (m_flags & DELETED));
}

bool
CDestination::IsMuted(
	bool *fromSolo) const
{
	D_ACCESS(("CDestination::IsMuted()\n"));
	ASSERT(IsReadLocked());

	if (fromSolo) {
		*fromSolo = m_flags & MUTED_FROM_SOLO;
	}

	return m_flags & MUTED;
}

void
CDestination::SetMuted(
	bool muted)
{
	D_ACCESS(("CDestination::SetMuted(%s)\n", muted ? "true" : "false"));
	ASSERT(IsWriteLocked());

	bool changed = false;
	if (muted)
		changed = _addFlag(MUTED);
	else
		changed = _removeFlag(MUTED);

	if (changed)
	{
		Muted(muted);
		Document()->SetModified();

		CUpdateHint hint;
		hint.AddInt32("DestID", m_id);
		hint.AddInt32("DestAttrs", Update_Flags);
		PostUpdate(&hint);
	}
}

bool
CDestination::IsSolo() const
{
	D_ACCESS(("CDestination::IsSolo()\n"));
	ASSERT(IsReadLocked());

	return m_flags & SOLO;
}

void
CDestination::SetSolo(
	bool solo)
{
	D_ACCESS(("CDestination::SetSolo(%s)\n", solo ? "true" : "false"));
	ASSERT(IsWriteLocked());

	bool changed = false;
	if (solo)
		changed = _addFlag(SOLO);
	else
		changed = _removeFlag(SOLO);

	if (changed)
	{
		Soloed(solo);
		Document()->SetModified();

		CUpdateHint hint;
		hint.AddInt32("DestID", m_id);
		hint.AddInt32("DestAttrs", Update_Flags);
		PostUpdate(&hint);
	}
}

void
CDestination::SetName(
	const char *name)
{
	D_ACCESS(("CDestination::SetName(%s)\n", name));
	ASSERT(IsWriteLocked());

	if (strcmp(m_name, name) != 0)
	{
		strncpy(m_name, name, DESTINATION_NAME_LENGTH);

		NameChanged(name);
		Document()->SetModified();
		
		CUpdateHint hint;
		hint.AddInt32("DestID", m_id);
		hint.AddInt32("DestAttrs", Update_Name);
		PostUpdate(&hint);
	}
}

void
CDestination::SetLatency(
	bigtime_t latency)
{
	D_ACCESS(("CDestination::SetLatency(%Ld)\n", latency));
	ASSERT(IsWriteLocked());

	if (latency != m_latency)
	{
		m_latency = latency;
		Document()->SetModified();
	
		CUpdateHint hint;
		hint.AddInt32("DestID", m_id);
		hint.AddInt32("DestAttrs", Update_Latency);
		PostUpdate(&hint);
	}
}

void
CDestination::SetColor(
	rgb_color color)
{
	D_ACCESS(("CDestination::SetColor(%d, %d, %d, %d)\n",
			  color.red, color.green, color.blue, color.alpha));
	ASSERT(IsWriteLocked());

	if ((color.red != m_color.red)
	 || (color.green != m_color.green)
	 || (color.blue != m_color.blue))
	{
		m_color = color;
		
		ColorChanged(color);
		Document()->SetModified();
	
		CUpdateHint hint;
		hint.AddInt32("DestID", m_id);
		hint.AddInt32("DestAttrs", Update_Color);
		PostUpdate(&hint);
	}
}

bool
CDestination::IsDisabled() const
{
	D_ACCESS(("CDestination::IsDisabled()\n"));
	ASSERT(IsReadLocked());

	return m_flags & DISABLED;
}

void
CDestination::SetDisabled(
	bool disabled)
{
	bool changed = false;
	if (disabled)
		changed = _addFlag(DISABLED);
	else
		changed = _removeFlag(DISABLED);

	if (changed)
	{
		Disabled(disabled);

		CUpdateHint hint;
		hint.AddInt32("DestID", m_id);
		hint.AddInt32("DestAttrs",Update_Flags);
		PostUpdate(&hint);
	}
}

// ---------------------------------------------------------------------------
// Operations

bool
CDestination::IsDeleted() const
{
	return m_flags & DELETED;
}

void
CDestination::Delete ()
{
	CWriteLock lock(Document());
	int32 originalIndex = Document()->IndexOf(this);
	if (_addFlag(DELETED))
	{
		int32 index = 0;
		CDestination *first = Document()->GetNextDestination(&index);
		if (first)
			Document()->SetDefaultAttribute(EvAttr_Channel, first->m_id);

		Document()->SetModified();
		
		CUpdateHint hint;
		hint.AddInt32("DestID", m_id);
		hint.AddInt32("DocAttrs", CMeVDoc::Update_DelDest);
		hint.AddInt32("original_index", originalIndex);
		Document()->PostUpdate(&hint);
	}
}

void 
CDestination::Undelete(
	int32 originalIndex)
{
	if (_removeFlag(DELETED))
	{
		Document()->SetModified();
		
		CUpdateHint hint;
		hint.AddInt32("DestID", m_id);
		hint.AddInt32("DocAttrs", CMeVDoc::Update_AddDest);
		Document()->PostUpdate(&hint);
	}
}

// ---------------------------------------------------------------------------
// CSerializable Implementation

void
CDestination::ReadChunk(
	CIFFReader &reader)
{
	ASSERT(IsWriteLocked());

	switch (reader.ChunkID())
	{
		case DESTINATION_HEADER_CHUNK:
		{
			reader >> m_id;
			reader >> m_latency;
			reader >> m_flags;
			reader >> m_color;
			ColorChanged(m_color);
			break;
		}
		case DESTINATION_NAME_CHUNK:
		{
			reader.MustRead(m_name, MIN((size_t)reader.ChunkLength(),
										DESTINATION_NAME_LENGTH));
			NameChanged(m_name);
			break;
		}
		default:
		{
			CSerializable::ReadChunk(reader);
		}
	}
}

void
CDestination::Serialize(
	CIFFWriter &writer)
{
	ASSERT(IsReadLocked());

	writer.Push(DESTINATION_HEADER_CHUNK);
	writer << m_id;
	writer << m_latency;
	writer << m_flags;
	writer << m_color;
	writer.Pop();

	writer.WriteChunk(DESTINATION_NAME_CHUNK, m_name,
					  MIN(strlen(m_name) + 1, DESTINATION_NAME_LENGTH));
}

// ---------------------------------------------------------------------------
// Internal Operations

bool 
CDestination::_addFlag(
	int32 flag)
{
	if (!(m_flags & flag))
	{
		m_flags ^= flag;
		return true;
	}

	return false;
}

bool 
CDestination::_removeFlag(
	int32 flag)
{
	if (m_flags & flag)
	{
		m_flags ^= flag;
		return true;
	}

	return false;
}

// ---------------------------------------------------------------------------
// CDestinationDeleteUndoAction: Constructor/Destructor

CDestinationDeleteUndoAction::CDestinationDeleteUndoAction(
	CDestination *dest)
	:	m_dest(dest),
		m_index(-1)
{
	D_ALLOC(("CDestinationDeleteUndoAction::CDestinationDeleteUndoAction()"));
	m_dest->Delete();
}

CDestinationDeleteUndoAction::~CDestinationDeleteUndoAction()
{
	D_ALLOC(("CDestinationDeleteUndoAction::~CDestinationDeleteUndoAction()"));
}

// ---------------------------------------------------------------------------
// CDestinationDeleteUndoAction: CUndoAction Implementation

void
CDestinationDeleteUndoAction::Redo()
{
	D_HOOK(("CDestinationDeleteUndoAction::Redo()\n"));
	m_dest->Delete();
}

void
CDestinationDeleteUndoAction::Undo()
{
	D_HOOK(("CDestinationDeleteUndoAction::Undo()\n"));
	m_dest->Undelete(m_index);
}

// END - Destination.cpp
