/* ===================================================================== *
 * MeVPlugin.cpp (MeV)
 * ===================================================================== */

#include "MeVPlugin.h"
#include "MeVApp.h"
#include "MeVDoc.h"
#include "EventTrack.h"
#include "Event.h"
#include "MidiDestination.h"
#include "InternalSynth.h"
#include "MidiManager.h"

#include <support/Debug.h>

#ifdef __POWERPC__
#pragma export on
#endif

extern char				gPlugInName[ B_FILE_NAME_LENGTH ];

MeVPlugIn::MeVPlugIn()
{
	be_app->AddHandler(this);
	SetName(gPlugInName);
}

void MeVPlugIn::AddMenuItem( char *inMenuText, int32 inWhichMenu, BMessage *inMsg )
{
	CMeVApp		&app = *(CMeVApp *)be_app;
	CDynamicMenuDef	*def = NULL;

	switch (inWhichMenu) {
	case Assembly_Menu:
//		def = &app.assemWindowPlugIns;
		break;
		
	case TrackEditor_Menu:
//		def = &app.trackWindowPlugIns;
		break;
		
	case Operator_Menu:
//		def = &app.operWindowPlugIns;
		break;
		
	case Import_List:
			// Add to list of importers...
		app.importerList.AddItem( this );
		return;
		
	case Export_List:
		CMeVApp::ExportInfo	*ei;
		
			// Add to list of exporters...
		ei = new CMeVApp::ExportInfo;
		ei->msg = inMsg;
		ei->plugIn = this;
		ei->menuText = inMenuText;
		app.exporterList.AddItem( ei );
		return;
	}
	
	if (def)
	{
			// REM: We need to give that item some extra information I think...
			// but only on a per-item basis...
			
			// How does the item know which window it's coming from?
	
//		def->AddItem( inMenuText, inMsg, this );
	}
}

void MeVPlugIn::AddDefaultEventOperator( EventOp *inOper )
{
	CMeVApp		&app = *(CMeVApp *)be_app;

	app.AddDefaultOperator( inOper );
}

void MeVPlugIn::NotifyOperatorChanged( EventOp *inOper )
{
	CMeVApp		&app = *(CMeVApp *)be_app;

	app.Lock();

	for (int i = 0; i < app.CountDocuments(); i++)
	{
		CMeVDoc	*doc = (CMeVDoc *)app.DocumentAt(i);
		doc->NotifyOperatorChanged(inOper);
	}

	app.Unlock();
}

MeVDocHandle MeVPlugIn::NewDocument( bool inShowWindow )
{
	CMeVApp		&app = *(CMeVApp *)be_app;
	CMeVDoc		*doc;
	
	app.Lock();

	doc = (CMeVDoc *)app.NewDocument( inShowWindow );
	if (doc)
	{
		MeVDocRef		*ref = new MeVDocRef();
			
//		doc->Acquire();
		ref->data = doc;
			
		app.Unlock();
		return ref;
	}

	app.Unlock();
	return NULL;
}

MeVDocHandle MeVPlugIn::FindDocument( int32 inDocID )
{
	CMeVApp		&app = *(CMeVApp *)be_app;
	
	app.Lock();

	for (int i = 0; i < app.CountDocuments(); i++)
	{
		CMeVDoc	*doc = (CMeVDoc *)app.DocumentAt(i);
		
		if ((int32)doc == inDocID)
		{
			MeVDocRef		*ref = new MeVDocRef();
			
			ref->data = doc;
		
			app.Unlock();
			return ref;
		}
	}

	app.Unlock();
	return NULL;
}

MeVDocHandle MeVPlugIn::FindDocument( char *inDocName )
{
	CMeVApp		&app = *(CMeVApp *)be_app;
	
	app.Lock();

	for (int i = 0; i < app.CountDocuments(); i++)
	{
		CMeVDoc	*doc = (CMeVDoc *)app.DocumentAt( i );
		char		name[ B_FILE_NAME_LENGTH ];
		
		doc->GetName( name );
		
		if (strcmp( name, inDocName ) == 0)
		{
			MeVDocRef		*ref = new MeVDocRef();
			
			ref->data = doc;
		
			app.Unlock();
			return ref;
		}
	}

	app.Unlock();
	return NULL;
}
	
MeVDocHandle MeVPlugIn::FirstDocument()
{
	CMeVApp		&app = *(CMeVApp *)be_app;
	
	app.Lock();

	if (app.CountDocuments() > 0)
	{
		CMeVDoc	*doc = (CMeVDoc *)app.DocumentAt( 0 );
		MeVDocRef		*ref = new MeVDocRef();
			
		ref->data = doc;
		
		app.Unlock();
		return ref;
	}

	app.Unlock();
	return NULL;
}

char *MeVPlugIn::LookupErrorText( status_t error )
{
	return strerror( error );
}

MeVDocRef::MeVDocRef()
{
}

MeVDocRef::~MeVDocRef()
{
}

bool MeVDocRef::NextDocument()
{
	CMeVDoc		*doc = (CMeVDoc *)data;
	CMeVApp		&app = *(CMeVApp *)be_app;
	int32		index;
	
	app.Lock();
	index = app.IndexOf( doc );

	if (app.CountDocuments() > index + 1)
	{
		doc = (CMeVDoc *)app.DocumentAt( index + 1 );
		data = doc;
		
		app.Unlock();
		return true;
	}

	app.Unlock();
	return false;
}

bool GetNextMidiConsumer(int32* cookie, int* outConsumerID, char* outName, size_t nameLength)
{
	ASSERT(cookie);
	ASSERT(outConsumerID);
	ASSERT(outName);
	ASSERT(nameLength > 0);

	BMidiConsumer* consumer = Midi::CMidiManager::Instance()->GetNextConsumer(cookie);
	if (consumer)
	{
		*outConsumerID = consumer->ID();
		strncpy(outName, consumer->Name(), nameLength);
		outName[nameLength - 1] = '\0';
		return true;
	}
	else
	{
		return false;
	}
}

int MeVDocRef::GetInternalSynthConsumerID()
{
	BMidiConsumer* synth = Midi::CMidiManager::Instance()->InternalSynth();
	ASSERT(synth);
	return synth ? synth->ID() : -1;
}

int
MeVDocRef::NewDestination(
	const char *name,
	int consumerID,
	int channel)
{
	CMeVDoc *doc = reinterpret_cast<CMeVDoc *>(data);
	CDestination *dest = doc->NewDestination();
	dest->SetName(name);

	// +++ move this midi-specific stuff outta here
	using namespace Midi;
	((CMidiDestination *)dest)->SetChannel(channel - 1);
	BMidiConsumer *consumer = CMidiManager::Instance()->FindConsumer(consumerID);
	if (consumer)
		((CMidiDestination *)dest)->SetConnect(consumer,true);

	return dest->ID();
}

int
MeVDocRef::GetChannelForDestination(
	int destinationID)
{
	CMeVDoc *doc = reinterpret_cast<CMeVDoc *>(data);
	if (doc->IsDefinedDest(destinationID))
	{
		// +++ move this midi-specific stuff outta here
		using namespace Midi;
		CDestination *dest = doc->FindDestination(destinationID);
		return ((CMidiDestination *)dest)->Channel();
	}

	return -1;
}

void MeVDocRef::AddEventOperator( EventOp *inOper )
{
	CMeVDoc		*doc = (CMeVDoc *)data;

	doc->AddOperator( inOper );
	doc->NotifyUpdate( CMeVDoc::Update_OperList, NULL );
}

void MeVDocRef::RemoveEventOperator( EventOp *inOper )
{
	CMeVDoc		*doc = (CMeVDoc *)data;

	doc->RemoveOperator( inOper );
	doc->NotifyOperatorChanged( inOper );
}

void MeVDocRef::EnableEventOperator( EventOp *inOper, bool inEnabled )
{
	CMeVDoc		*doc = (CMeVDoc *)data;

	doc->SetOperatorActive( inOper, inEnabled );
	doc->NotifyUpdate( CMeVDoc::Update_OperList, NULL );
}

MeVTrackHandle MeVDocRef::NewEventTrack( TClockType inClockType )
{
	CMeVDoc		*doc = (CMeVDoc *)data;
	StSubjectLock	myLock( *doc, Lock_Exclusive );
	CTrack		*track = doc->NewTrack( TrackType_Event, inClockType );
	
	if (track != NULL)
		return new MeVTrackRef(data, track);
	else
		return NULL;
}


MeVTrackHandle MeVDocRef::FindTrack( int32 inTrackID )
{
	CMeVDoc		*doc = (CMeVDoc *)data;
	StSubjectLock	myLock( *doc, Lock_Shared );
	CTrack		*track = doc->FindTrack( inTrackID );
	
	if (track != NULL)
		return new MeVTrackRef(data, track);
	else
		return NULL;
}

MeVTrackHandle MeVDocRef::FindTrack( char *inTrackName )
{
	CMeVDoc		*doc = (CMeVDoc *)data;
	StSubjectLock	myLock( *doc, Lock_Shared );
	CTrack		*track = doc->FindTrack( inTrackName );
	
	return track ? new MeVTrackRef(data, track) : NULL;
}

MeVTrackHandle MeVDocRef::ActiveMasterTrack()
{
	CMeVDoc* doc = reinterpret_cast<CMeVDoc*>(data);
	StSubjectLock lock(*doc, Lock_Shared);
	CTrack* track = doc->ActiveMaster();
	
	return track ? new MeVTrackRef(data, track) : NULL;
}

MeVTrackHandle MeVDocRef::FirstTrack()
{
	CMeVDoc* doc = reinterpret_cast<CMeVDoc*>(data);
	StSubjectLock lock(*doc, Lock_Shared);
	CTrack* track = doc->FindNextHigherTrackID( 0 );
	
	return track ? new MeVTrackRef(data, track) : NULL;
}

void MeVDocRef::ReleaseTrack( MeVTrackHandle th )
{
	delete th;
}
	
int32 MeVDocRef::GetID()
{
	return (int32)data;
}

void MeVDocRef::GetName( char *outName, int32 inMaxChars )
{
	CMeVDoc		*doc = (CMeVDoc *)data;

	char		name[B_FILE_NAME_LENGTH];
		
	doc->GetName(name);

	strncpy(outName, name, inMaxChars);
	outName[inMaxChars - 1] = '\0';
}

double MeVDocRef::GetInitialTempo()
{
	CMeVDoc		*doc = (CMeVDoc *)data;
	
	return doc->InitialTempo();
}

void MeVDocRef::SetInitialTempo(double tempo)
{
	CMeVDoc		*doc = (CMeVDoc *)data;
	
	doc->SetInitialTempo(tempo);
}

void MeVDocRef::ShowWindow()
{
	CMeVDoc		*doc = (CMeVDoc *)data;
	
	doc->ShowWindow(CMeVDoc::ASSEMBLY_WINDOW);
}

MeVTrackRef::MeVTrackRef(void* doc, void* track)
	:	trackData(track), docData(doc), undo(0)
{
	CTrack* theTrack = reinterpret_cast<CTrack *>(track);
	theTrack->WriteLock();
}

MeVTrackRef::~MeVTrackRef()
{
	((CTrack *)trackData)->WriteUnlock();
}

	/**	Repositions this handle to point to the next track. */
bool MeVTrackRef::NextTrack()
{
	CTrack*  track = reinterpret_cast<CTrack*>(trackData);
	CMeVDoc* doc   = reinterpret_cast<CMeVDoc*>(docData);

	StSubjectLock docLock(*doc, Lock_Shared);
	if (!docLock.LockValid())
		return false;
	
	int32 index = doc->tracks.IndexOf(track);
	if (index >= 0 && doc->tracks.CountItems() > index + 1)
	{
		track->WriteUnlock();
		track = reinterpret_cast<CTrack*>(doc->tracks.ItemAt(index + 1));
		track->WriteLock();
		trackData = track;
		return true;
	}
	else
	{
		return false;
	}
}

	/**	Get the ID of this track */
int32 MeVTrackRef::GetID()
{
	CTrack		*track = (CTrack *)trackData;
	
	return track->GetID();
}

	/**	Get the name of this track */
void MeVTrackRef::GetName( char *name, int32 inMaxChars )
{
	CTrack		*track = (CTrack *)trackData;
	
	strncpy( name, track->Name(), inMaxChars );
}

	/**	Change the name of this track */
void MeVTrackRef::SetName( char *name )
{
	CTrack		*track = (CTrack *)trackData;
	
	track->SetName( name );
}

	/**	Get whether it is a metered or real-time track */
TClockType MeVTrackRef::GetClockType()
{
	CTrack		*track = (CTrack *)trackData;
	
	return track->ClockType();
}

MeVEventHandle MeVTrackRef::FirstEvent()
{
	CEventTrack		*track = (CEventTrack *)trackData;

	return new MeVEventRef(&track->Events());
}

void MeVTrackRef::ReleaseEventRef(MeVEventHandle event)
{
	delete event;
}

	/**	Start a new undo record for this track */
bool MeVTrackRef::BeginUndoAction( char *inActionLabel )
{
	CEventTrack	*track = (CEventTrack *)trackData;
	if (undo)
	{
		return false;
	}
	undo = new EventListUndoAction( track->Events(), *track, inActionLabel );
	
	return true;
}

	/**	End a undo record for this track */
void MeVTrackRef::EndUndoAction( bool keep )
{
	if (undo != NULL)
	{
		if (keep) ((CTrack *)trackData)->AddUndoAction( (EventListUndoAction *)undo );
		else ((EventListUndoAction *)undo)->Rollback();

		delete (EventListUndoAction *)undo;
		undo = NULL;
	}
}

		/* Merge a list of sorted events into the EventList. */
void MeVTrackRef::Merge( Event *inEventArray, long inEventCount )
{
	CEventTrack		*track = (CEventTrack *)(CTrack *)trackData;

	track->MergeEvents( inEventArray, inEventCount, (EventListUndoAction *)undo );
}

int32
MeVTrackRef::Duration() const
{
	CTrack *track = (CTrack *)trackData;
	return track->LogicalLength();
}

#if 0
	/**	Select all events on this track. */
void MeVTrackRef::SelectAll()
{
	CTrack		*track = (CTrack *)data;
	
}

	/**	Deselect all events on this track. */
void MeVTrackRef::DeselectAll()
{
	CTrack		*track = (CTrack *)data;
	
}

	/**	Delete all selected events on this track */
void MeVTrackRef::DeleteSelected()
{
	CTrack		*track = (CTrack *)data;
	
}

void MeVTrackRef::EnableEventOperator( EventOp *inOper, bool inEnabled )
{
}

void MeVTrackRef::ApplyEventOperator( EventOp *inOper, bool inSelectedOnly )
{
}

#endif

MeVEventRef::MeVEventRef(void* evList)
	: data(new EventMarker(*reinterpret_cast<EventList*>(evList)))
{
	EventMarker* marker = reinterpret_cast<EventMarker*>(data);
	marker->First();
}

MeVEventRef::~MeVEventRef()
{
	delete data;
}

bool MeVEventRef::GetEvent(Event* inEvent) const
{
	const EventMarker* marker = reinterpret_cast<const EventMarker*>(data);

	return marker->Get(inEvent) == 1;
}

const Event* MeVEventRef::EventPtr()
{
	EventMarker* marker = reinterpret_cast<EventMarker*>(data);

	return Valid() ? (const Event *)*marker : 0;
}

bool MeVEventRef::Valid()
{
	EventMarker* marker = reinterpret_cast<EventMarker*>(data);

	return !marker->IsAtEnd();
}

bool MeVEventRef::Seek(int32 inSeekCount)
{
	EventMarker* marker = reinterpret_cast<EventMarker*>(data);

	return marker->Seek(inSeekCount) != 0;
}

bool MeVEventRef::SeekToFirst()
{
	EventMarker* marker = reinterpret_cast<EventMarker*>(data);

	return marker->First() != 0;
}

#ifdef __POWERPC__
#pragma export off
#endif
