/* ===================================================================== *
 * MeVDoc.cpp (MeV)
 * ===================================================================== */

#include "MeVDoc.h"

#include "AssemblyWindow.h"
#include "BeFileWriter.h"
#include "BeFileReader.h"
#include "EventOp.h"
#include "EventTrack.h"
#include "Idents.h"
#include "IFFWriter.h"
#include "IFFReader.h"
#include "LinearWindow.h"
#include "MeVDocIconBits.h"
#include "MeVFileID.h"
#include "MeVModule.h"
#include "MixWindow.h"
#include "OperatorWindow.h"
#include "PlayerControl.h"
#include "ScreenUtils.h"
#include "StdEventOps.h"
#include "TrackWindow.h"

// Gnu C Library
#include <stdio.h>
// Interface Kit
#include <Bitmap.h>
// Storage Kit
#include <FilePanel.h>
#include <Mime.h>
#include <NodeInfo.h>
// Support Kit
#include <Debug.h>
#include <String.h>

// Debugging Macros
#define D_ALLOC(x) //PRINT(x)			// Constructor/Destructor
#define D_SERIALIZE(x) //PRINT(x)		// Serialization
#define D_WINDOW(x) //PRINT(x)			// Window Management

// ---------------------------------------------------------------------------
// Constants Initialization

const double
CMeVDoc::DEFAULT_TEMPO = 90.0;

// ---------------------------------------------------------------------------
// Constructor/Destructor

CMeVDoc::CMeVDoc(
	CMeVApp *app)
	:	CDocument(app),
		m_newTrackID(2),
		m_initialTempo(DEFAULT_TEMPO)
{
	_init();
	_addDefaultOperators();
	m_windowState[ASSEMBLY_WINDOW] = BRect(40, 40, 480, 280);
	m_windowState[MIX_WINDOW] = BRect(80, 80, 520, 320);
	m_windowState[OPERATORS_WINDOW] = UScreenUtils::StackOnScreen(620, 390);

	m_masterRealTrack = new CEventTrack(*this, ClockType_Real, 0,
										"Master Real");
	m_masterMeterTrack = new CEventTrack(*this, ClockType_Metered, 1,
										 "Master Metric");
	m_activeMaster = m_masterMeterTrack;

	SetValid(); 

	NewDestination();
}

CMeVDoc::CMeVDoc(
	CMeVApp *app,
	entry_ref &ref,
	CIFFReader &reader)
	:	CDocument(app, ref),
		m_newTrackID(2),
		m_initialTempo(DEFAULT_TEMPO)
{
	CWriteLock lock(this);

	_init();
	_addDefaultOperators();
	m_windowState[ASSEMBLY_WINDOW] = BRect(40, 40, 480, 280);
	m_windowState[MIX_WINDOW] = BRect(80, 80, 420, 220);
	m_windowState[OPERATORS_WINDOW] = UScreenUtils::StackOnScreen(620, 390);

	while (reader.NextChunk())
		ReadChunk(reader);

	if (m_masterRealTrack == NULL)
		m_masterRealTrack  = new CEventTrack(*this, ClockType_Real, 0,
											 "Master Real");

	if (m_masterMeterTrack == NULL)
		m_masterMeterTrack = new CEventTrack(*this, ClockType_Metered, 1,
											 "Master Metric");

	m_activeMaster = m_masterMeterTrack;

	for (int32 i = 0; i < CountTracks(); i++)
		if (TrackAt(i)->m_openWindow)
			ShowWindowFor(TrackAt(i));

	SetValid();
}

CMeVDoc::~CMeVDoc()
{
	D_ALLOC(("CMeVDoc::~CMeVDoc()\n"));

	// first stop the player if it's playing this document
	if (CPlayerControl::IsPlaying(this))
		CPlayerControl::StopSong(this);

	D_ALLOC((" -> delete master real track\n"));
	delete m_masterRealTrack;

	D_ALLOC((" -> delete master metered track\n"));
	delete m_masterMeterTrack;

	D_ALLOC((" -> delete tracks\n"));
	for (int i = 0; i < tracks.CountItems(); i++)
		delete (CTrack *)tracks.RemoveItem(i);

	D_ALLOC((" -> delete destinations\n"));
	for (int i = 0; i < m_destinations.CountItems(); i ++)
		delete (CDestination *)m_destinations.RemoveItem(i);

	for (int i = 0; i < operators.CountItems(); i++)
	{
		EventOp *op = (EventOp *)operators.ItemAt(i);
		CRefCountObject::Release(op);
	}

	for (int i = 0; i < activeOperators.CountItems(); i++)
	{
		EventOp *op = (EventOp *)activeOperators.ItemAt(i);
		CRefCountObject::Release(op);
	}
}

// ---------------------------------------------------------------------------
// Accessors


BMimeType *
CMeVDoc::MimeType()
{
	BMimeType *type = new BMimeType("application/x-vnd.BeUnited.MeV-Document");
	type->SetShortDescription("MeV Document");
	type->SetLongDescription("MeV Document");
	type->SetPreferredApp("application/x-vnd.BeUnited.MeV");

	BBitmap *miniIcon = new BBitmap(BRect(0.0, 0.0, B_MINI_ICON - 1.0,
										   B_MINI_ICON - 1.0),
									 B_CMAP8); 
	miniIcon->SetBits(SMALL_ICON_BITS, 256, 0, B_CMAP8);
	type->SetIcon(miniIcon, B_MINI_ICON);

	BBitmap *largeIcon = new BBitmap(BRect(0.0, 0.0, B_LARGE_ICON - 1.0,
										   B_LARGE_ICON - 1.0),
									 B_CMAP8);
	largeIcon->SetBits(LARGE_ICON_BITS, 1024, 0, B_CMAP8);
	type->SetIcon(largeIcon, B_LARGE_ICON);

	return type;
}

// ---------------------------------------------------------------------------
// Operator Management

EventOp *
CMeVDoc::ActiveOperatorAt(
	int32 index) const
{
	EventOp *op = static_cast<EventOp *>(activeOperators.ItemAt(index));
	if (op)
		op->Acquire();

	return op;
}
	
EventOp *
CMeVDoc::OperatorAt(
	int32 index) const
{
	EventOp *op = static_cast<EventOp *>(operators.ItemAt(index));
	if (op)
		op->Acquire();

	return op;
}
	
// ---------------------------------------------------------------------------
// Window Management

BWindow *
CMeVDoc::ShowWindow(
	uint32 which,
	bool show)
{
	D_WINDOW(("CMeVDoc::ShowWindow(%ld, %s)\n",
			  which, show ? "true" : "false"));

	if (which >= WINDOW_TYPE_COUNT)
		return NULL;

	if (show && (m_windowState[which].Activate() == false))
	{
		CAppWindow *window;
		switch (which)
		{
			case ASSEMBLY_WINDOW:
			{
				ShowWindowFor(m_masterMeterTrack);
				break;
			}
			case MIX_WINDOW:
			{
				window = new CMixWindow(m_windowState[which], this);
				window->Show();
				m_windowState[which].WindowOpened(window);
				break;
			}
			case OPERATORS_WINDOW:
			{
				window = new COperatorWindow(m_windowState[which], *this);
				window->Show();
				m_windowState[which].WindowOpened(window);
				break;
			}
		}
		return m_windowState[which].Window();
	}
	else if (!show)
	{
		m_windowState[which].Close();
		return NULL;
	}
}

bool
CMeVDoc::IsWindowOpen(
	uint32 which)
{
	if (which > WINDOW_TYPE_COUNT)
		return false;

	return m_windowState[which].IsOpen();
}

void
CMeVDoc::ShowWindowFor(
	CTrack *track)
{
	CTrackWindow *window = track->Window();

	if (window == NULL)
	{
		// Create default track if no tracks have been added already
		if (CountTracks() == 0)
			NewTrack(TrackType_Event, ClockType_Metered);

		CEventTrack *eventTrack = dynamic_cast<CEventTrack *>(track);
		BMessage *settings = eventTrack->GetWindowSettings();

		if (track->GetID() < 2)
		{
			window = new CAssemblyWindow(UScreenUtils::StackOnScreen(540, 370),
										 this, (bool)settings);
		}
		else if (track->TrackType() == TrackType_Event)
		{
			window = new CLinearWindow(UScreenUtils::StackOnScreen(540, 370),
									   this, eventTrack, (bool)settings);
		}
		track->m_window = window;
		if (settings)
			window->ImportSettings(settings);
	}

	if (window->Lock())
	{
		if (window->IsHidden())
			window->Show();
		else
			window->Activate();
		window->Unlock();
	}
}

// ---------------------------------------------------------------------------
// Track Operations

long CMeVDoc::GetUniqueTrackID()
{
	long			trialID = 2,
				prevID = -1;

		// Loop until we can run through the entire list
		// without a collision.
	while (trialID != prevID)
	{
		prevID = trialID;

			// If we collide, then increase the ID by one
		for (int i = 0; i < tracks.CountItems(); i++)
		{
			CTrack		*track = (CTrack *)tracks.ItemAt( i );
			
			if (trialID == track->GetID()) trialID++;
		}
	}

	return trialID;
}

CTrack *
CMeVDoc::NewTrack(
	ulong type,
	TClockType clockType)
{
	switch (type)
	{
		case TrackType_Event:
		{
			CEventTrack *track = new CEventTrack(*this, clockType,
												 m_newTrackID++, NULL);
			if (track)
			{
				BString name = track->Name();
				name << " " << track->GetID() - 1;
				track->SetName(name.String());
				tracks.AddItem(track);
				return track;
			}
			break;
		}
	}

	return NULL;
}

	// Locate a track by it's ID. (0,1 for master track)
CTrack *CMeVDoc::FindTrack( long inTrackID )
{
	if (inTrackID == 0)
		return (CTrack *)m_masterRealTrack;
	else if (inTrackID == 1)
		return (CTrack *)m_masterMeterTrack;

	for (int i = 0; i < tracks.CountItems(); i++)
	{
		CTrack *track = (CTrack *)tracks.ItemAt(i);
		if (track->Deleted())
			continue;
		if (inTrackID == track->GetID())
			 return track;
	}
	return NULL;
}

	// Locate a track by it's name, and Acquire it.
CTrack *CMeVDoc::FindTrack( char *inTrackName )
{
	for (int i = 0; i < tracks.CountItems(); i++)
	{
		CTrack		*track = (CTrack *)tracks.ItemAt( i );
		
		if (track->Deleted()) continue;
		
		if (strcmp(track->Name(), inTrackName) == 0)
			return track;
	}
	return NULL;
}

	// Get the first track with an ID greater than the given one.
CTrack *CMeVDoc::FindNextHigherTrackID( int32 inID )
{
	CTrack		*bestTrack = NULL;
	int32		bestID = LONG_MAX;
	
	for (int i = 0; i < tracks.CountItems(); i++)
	{
		CTrack		*track = (CTrack *)tracks.ItemAt( i );
		
		if (track->Deleted()) continue;

		if (		track->GetID() < bestID
			&&	track->GetID() > inID)
		{
			bestTrack = track;
			bestID = track->GetID();
		}
	}
	return bestTrack;
}

void CMeVDoc::PostUpdateAllTracks( CUpdateHint *inHint )
{
	m_masterRealTrack->PostUpdate( inHint );
	m_masterMeterTrack->PostUpdate( inHint );
	for (int i = 0; i < tracks.CountItems(); i++)
	{
		CTrack		*track = (CTrack *)tracks.ItemAt( i );
		
		track->PostUpdate( inHint );
	}
}

// ---------------------------------------------------------------------------
// Destination Management

CDestination *
CMeVDoc::FindDestination(
	int32 id) const
{
	return (CDestination *)m_destinations.ItemAt(id);
}

CDestination *
CMeVDoc::GetNextDestination(
	int32 *index) const
{
	ASSERT(IsReadLocked());

	CDestination *dest = FindDestination(*index);
	while ((dest != NULL) && dest->IsDeleted())
	{
		(*index)++;
		dest = FindDestination(*index);
	}

	if (dest != NULL)
	{
		int32 count = m_destinations.CountItems();
		do
		{
			(*index)++;
			CDestination *next = FindDestination(*index);
			if (next && !next->IsDeleted())
				break;
		} while (*index < count);
	}

	return dest;
}

int32
CMeVDoc::IndexOf(
	const CDestination *destination) const
{
	ASSERT(IsReadLocked());

	int32 index = -1;
	int32 destID = 0;
	CDestination *dest = NULL;
	do {	
		index++;
	} while ((dest = GetNextDestination(&destID)) != destination);

	return index;
}

bool
CMeVDoc::IsDefinedDest(
	int32 id) const
{
	return (m_destinations.ItemAt(id) != NULL);
}

CDestination *
CMeVDoc::NewDestination(
	unsigned long type)
{
	CMeVModule *module = Application()->ModuleFor(type);
	int32 id = m_destinations.CountItems();
	CDestination *dest = module->CreateDestination(this, &id,
												   "Untitled Destination");
	m_destinations.AddItem(dest);

	CUpdateHint hint;
	hint.AddInt32("DocAttrs", Update_AddDest);
	hint.AddInt32("DestID", dest->ID());
	PostUpdate(&hint);

	return dest;
}

int32 
CMeVDoc::MaxDestinationLatency (uint8 clockType)
{
	if (clockType==ClockType_Real)
		return m_maxDestLatency;
	else if (clockType==ClockType_Metered)
		return (TempoMap().ConvertRealToMetered(m_maxDestLatency));

	return 0;
}

void
CMeVDoc::SetDestinationLatency(
	int32 id,
	int32 microseconds)
{
	StSubjectLock lock(*this, Lock_Shared);	

	// go though entire destination list and find the one with the 
	// highest latency.
	CDestination *dest;
	int32 index = 0;
	while ((dest = GetNextDestination(&index)) != NULL)
	{
		if (m_maxDestLatency < dest->Latency())
			m_maxDestLatency = dest->Latency();
	}
}

void CMeVDoc::ReplaceTempoMap( CTempoMapEntry *entries, int length )
{
		// REM: Should be exclusively locked when this occurs

	delete tempoMap.list;
	tempoMap.list = entries;
	tempoMap.count = length;
	validTempoMap = true;
	
	// REM: Deal with any playing tracks -- tell them about map change.
}

	/**	Add an operator to the document's list of operators. */
void CMeVDoc::AddOperator( EventOp *inOp )
{
	StSubjectLock	myLock( *this, Lock_Exclusive );

	if (!operators.HasItem( inOp ))
	{
		inOp->Acquire();
		operators.AddItem( inOp );
	}
}

	/**	Delete an operator to the document's list of operators. */
void CMeVDoc::RemoveOperator( EventOp *inOp )
{
	StSubjectLock	myLock( *this, Lock_Exclusive );

	if (operators.HasItem( inOp ))
	{
		SetOperatorActive( inOp, false );
		operators.RemoveItem( inOp );
		CRefCountObject::Release( inOp );
	}
}

	/**	Set an operator active / inactive. */
void CMeVDoc::SetOperatorActive( EventOp *inOp, bool enabled )
{
	StSubjectLock	myLock( *this, Lock_Exclusive );

	if (enabled)
	{
		if (!activeOperators.HasItem( inOp ))
		{
				// Add to list of active operators
			inOp->Acquire();
			activeOperators.AddItem( inOp );

				// Recompile all operator lists for all tracks
			for (int i = 0; i < tracks.CountItems(); i++)
			{
				CTrack *track = (CTrack *)tracks.ItemAt(i);
				CEventTrack	*eTrack = dynamic_cast<CEventTrack *>(track);
				if (eTrack)
				{
					eTrack->CompileOperators();
				}
			}
			m_masterRealTrack->CompileOperators();
			m_masterMeterTrack->CompileOperators();
		}
	}
	else
	{
		if (activeOperators.HasItem( inOp ))
		{
				// Remove from list of active operators
			activeOperators.RemoveItem( inOp );
			CRefCountObject::Release( inOp );

				// Recompile all operator lists for all tracks
			for (int i = 0; i < tracks.CountItems(); i++)
			{
				CTrack		*track = (CTrack *)tracks.ItemAt( i );
				CEventTrack	*eTrack = dynamic_cast<CEventTrack *>(track);
				
				if (eTrack) eTrack->CompileOperators();
			}
			m_masterRealTrack->CompileOperators();
			m_masterMeterTrack->CompileOperators();
		}
	}
}

	/**	Does a notification to all windows viewing the operator. */
void CMeVDoc::NotifyOperatorChanged( EventOp *inOp )
{
		// If this document can see this operator
	if (	operators.IndexOf( inOp ) >= 0
		|| ((CMeVApp *)be_app)->IndexOfOperator( inOp ) >= 0)
	{
		CUpdateHint		hint;

			// Then inform all interested parties that there is an update.
		hint.AddInt32( "DocAttrs", Update_Operator );
		hint.AddPointer( "Op", inOp );
	
		PostUpdate( &hint, NULL );
	}
}

	/**	Notify all observers (including possibly observers of the document
		as well) that some attributes of this doc have changed. */
void CMeVDoc::NotifyUpdate( int32 inHintBits, CObserver *source )
{
	CUpdateHint		hint;
	
	hint.AddInt32( "DocAttrs", inHintBits );
	
	PostUpdate( &hint, source );
}

void CMeVDoc::SetActiveMaster( CEventTrack *inTrack )
{
	if (inTrack->GetID() >= 2) return;
	
	if (m_activeMaster != inTrack)
	{
		m_activeMaster->DeselectAll( NULL );
		m_activeMaster = inTrack;
	}
}

void
CMeVDoc::ChangeTrackOrder(
	int32 oldIndex,
	int32 newIndex)
{
	tracks.MoveItem(oldIndex, newIndex);
	NotifyUpdate(Update_TrackOrder, NULL);
}

// ---------------------------------------------------------------------------
// CDocument Implementation

void
CMeVDoc::SaveDocument()
{
	D_SERIALIZE(("CMeVDoc::SaveDocument()\n"));

	StSubjectLock lock(*this, Lock_Shared);

	BFile file(&DocLocation(), B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
	CBeFileWriter writer(file);
	CIFFWriter iffWriter(writer);
	Serialize(iffWriter);
	SetModified(false);
	
	BNode node(&DocLocation());
	if (node.InitCheck() == B_NO_ERROR)
	{
		BNodeInfo nodeInfo(&node);
		if (nodeInfo.InitCheck() == B_NO_ERROR)
		{
			nodeInfo.SetType("application/x-vnd.BeUnited.MeV-Document");
			nodeInfo.SetPreferredApp("application/x-vnd.BeUnited.MeV");
		}
	}
}

void
CMeVDoc::ReadChunk(
	CIFFReader &reader)
{
	D_SERIALIZE(("CMeVDoc::ReadChunk()\n"));
	ASSERT(IsWriteLocked());

	int32 formType;
	switch (reader.ChunkID(0, &formType))
	{
		case DOC_HEADER_CHUNK:
		{
			reader >> m_initialTempo;
			break;
		}
		case ENVIRONMENT_CHUNK:
		{
			_readEnvironment(reader);
			break;
		}
		case Form_ID:
		{
			_readTrack(formType, reader);
			break;
		}
	}
}

void
CMeVDoc::Serialize(
	CIFFWriter &writer)
{
	D_SERIALIZE(("CMeVDoc::Serialize()\n"));
	ASSERT(IsReadLocked());

	writer.Push(Form_ID);
	writer << (long)MeV_ID;

	// write header chunk
	writer.Push(DOC_HEADER_CHUNK);
	writer << InitialTempo();
	writer.Pop();

	// write environment chunk
	writer.Push(ENVIRONMENT_CHUNK);
	CDestination *dest = NULL;
	int32 index = 0;
	while ((dest = GetNextDestination(&index)) != NULL)
	{
		writer.Push(DESTINATION_CHUNK);
		writer << dest->Type();
		CReadLock lock(dest);
		dest->Serialize(writer);
		writer.Pop();
	}
	writer.Pop();

	// Write master real track
	writer.Push(Form_ID);
	writer << (long)FMasterRealTrack;
	m_masterRealTrack->Serialize(writer);
	writer.Pop();

	// Write master metered track
	writer.Push(Form_ID);
	writer << (long)FMasterMeteredTrack;
	m_masterMeterTrack->Serialize(writer);
	writer.Pop();

	for (int i = 0; i < CountTracks(); i++)
	{
		CTrack *track = TrackAt(i);
		if (track->Deleted())
			continue;

		// Push a seperate FORM per track	
		writer.Push(Form_ID);
		writer << (long)track->TrackType();
		track->Serialize(writer);
		writer.Pop();
	}
}

void
CMeVDoc::Export(
	BMessage *msg)
{
	BFilePanel *filePanel = Application()->GetExportPanel(&be_app_messenger);
	if (filePanel != NULL)
	{
		BMessage exportMsg(CMeVApp::EXPORT_REQUESTED);
		BMessage pluginMsg;
		void *pluginPtr;

		msg->FindPointer("plugin", &pluginPtr);
		msg->FindMessage("msg", &pluginMsg);

		exportMsg.AddPointer("Document", this);
		exportMsg.AddPointer("plugin", pluginPtr);
		exportMsg.AddMessage("msg", &pluginMsg);

		filePanel->SetMessage(&exportMsg);
		filePanel->Show();
	}
}

// ---------------------------------------------------------------------------
// Internal Operations

void
CMeVDoc::_readTrack(
	uint32 type,
	CIFFReader &reader)
{
	D_SERIALIZE(("CMeVDoc::_readEnvironment()\n"));

	CTrack *track;
	if (type == FMasterRealTrack)
	{
		m_masterRealTrack = new CEventTrack(*this, ClockType_Real, 0,
											"Master Real");
		track = m_masterRealTrack;
	}
	else if (type == FMasterMeteredTrack)
	{
		m_masterMeterTrack = new CEventTrack(*this, ClockType_Metered, 1,
											 "Master Metric");
		track = m_masterMeterTrack;
	}
	else
	{
		track = NewTrack(TrackType_Event, ClockType_Metered);
	}

	if (track == NULL)
		return;
	
	reader.Push();
	while (reader.NextChunk())
		track->ReadChunk(reader);
	reader.Pop();

	if (type == TrackType_Event)
		((CEventTrack *)track)->SummarizeSelection();
}

void
CMeVDoc::_readEnvironment(
	CIFFReader &reader)
{
	D_SERIALIZE(("CMeVDoc::_readEnvironment()\n"));

	reader.Push();
	while (reader.NextChunk())
	{	
		int32 formType;
		switch (reader.ChunkID(0, &formType))
		{
			case DESTINATION_CHUNK:
			{
				D_SERIALIZE((" -> DESTINATION_CHUNK\n"));

				unsigned long type;
				reader >> type;
				CMeVModule *module = Application()->ModuleFor(type);
				CDestination *dest = module->CreateDestination(this);
				CWriteLock lock(dest);
				reader.Push();
				while (reader.NextChunk())
					dest->ReadChunk(reader);
				reader.Pop();
				m_destinations.AddItem(dest, dest->ID());
				break;
			}
			case SOURCE_CHUNK:
			{
				D_SERIALIZE((" -> SOURCE_CHUNK\n"));

				uint32 type;
				reader >> type;			
				break;
			}
		}
	}
	reader.Pop();
}

// ---------------------------------------------------------------------------
// Internal Operations

void
CMeVDoc::_addDefaultOperators()
{
	// Add active operarators associated with document to this list.
	for (int32 i = 0; i < Application()->CountOperators(); i++)
	{
		EventOp *op = Application()->OperatorAt(i);
		bool found = false;

		// Check to see if an oper with this name and
		// creator has already been added.
		for (int32 j = 0; j < operators.CountItems(); j++)
		{
			EventOp *search = (EventOp *)operators.ItemAt(j);

			if ((strcmp(search->Name(), op->Name()) == 0)
			 && (strcmp(search->CreatorName(), op->CreatorName()) == 0))
			{
				found = true;
				break;
			}
		}
		
		if (found)
			CRefCountObject::Release(op);
		else
			operators.AddItem(op);
	}
}

void
CMeVDoc::_init()
{
	m_masterRealTrack = NULL;
	m_masterMeterTrack = NULL;

	// Initialize default attributes
	defaultAttributes[EvAttr_Duration] = Ticks_Per_QtrNote;
	defaultAttributes[EvAttr_Type] = EvtType_Note;
	defaultAttributes[EvAttr_Selected] = 0;
	defaultAttributes[EvAttr_Channel] = 0;
	defaultAttributes[EvAttr_Pitch]	= 60;
	defaultAttributes[EvAttr_AttackVelocity] = 64;
	defaultAttributes[EvAttr_ReleaseVelocity] = 64;
	defaultAttributes[EvAttr_Program] = 0;
	defaultAttributes[EvAttr_ProgramBank] = 0;
	defaultAttributes[EvAttr_VPos] = 60;
	defaultAttributes[EvAttr_ControllerNumber] = 0;
	defaultAttributes[EvAttr_ControllerValue8] = 0;
	defaultAttributes[EvAttr_ControllerValue16] = 0;
	defaultAttributes[EvAttr_BendValue] = 0;
	defaultAttributes[EvAttr_InitialBend] = 0;
	defaultAttributes[EvAttr_UpdatePeriod] = 20;
	defaultAttributes[EvAttr_TempoValue] = 0;
	defaultAttributes[EvAttr_RepeatCount] = 2;
	defaultAttributes[EvAttr_SequenceNumber] = 2;
	defaultAttributes[EvAttr_Transposition] = 0;
	defaultAttributes[EvAttr_CountourLevel] = 0;
	defaultAttributes[EvAttr_DataSize] = 0;
	defaultAttributes[EvAttr_TSigBeatCount] = 4;
	defaultAttributes[EvAttr_TSigBeatSize] = 2;

	tempoMap.list = new CTempoMapEntry[2];
	tempoMap.count = 2;
	validTempoMap = false;
	
	tempoMap.list[0].SetInitialTempo(RateToPeriod(InitialTempo()));
	tempoMap.list[1].SetTempo(tempoMap.list[0], RateToPeriod(256.0),
							  Ticks_Per_QtrNote * 4, Ticks_Per_QtrNote * 4,
							  ClockType_Metered);


    m_newDestID=0; //hemm remember me.
    m_maxDestLatency=0;
}

// END - MeVDoc.cpp
