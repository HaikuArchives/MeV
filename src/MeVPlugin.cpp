/* ===================================================================== *
 * MeVPlugin.cpp (MeV)
 * ===================================================================== */

#include "MeVPlugin.h"
#include "MeVApp.h"
#include "MeVDoc.h"
#include "EventTrack.h"

#ifdef __POWERPC__
#pragma export on
#endif

extern char				gPlugInName[ B_FILE_NAME_LENGTH ];

MeVPlugIn::MeVPlugIn()
{
	((CMeVApp *)be_app)->plugInList.AddItem( this );
	be_app->AddHandler( this );
	SetName( gPlugInName );
}

void MeVPlugIn::AddMenuItem( char *inMenuText, int32 inWhichMenu, BMessage *inMsg )
{
	CMeVApp		&app = *(CMeVApp *)be_app;
	CDynamicMenuDef	*def = NULL;

	switch (inWhichMenu) {
	case Assembly_Menu:
		def = &app.assemWindowPlugIns;
		break;
		
	case TrackEditor_Menu:
		def = &app.trackWindowPlugIns;
		break;
		
	case Operator_Menu:
		def = &app.operWindowPlugIns;
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
			
			doc->Acquire();
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
			
			doc->Acquire();
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
			
		doc->Acquire();
		ref->data = doc;
		
		app.Unlock();
		return ref;
	}

	app.Unlock();
	return NULL;
}

void MeVPlugIn::ReleaseDocument( MeVDocHandle inHandle )
{
	CMeVDoc		*doc = (CMeVDoc *)inHandle->data;
	
	CRefCountObject::Release( doc );
	delete inHandle;
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
		CRefCountObject::Release( doc );
		doc = (CMeVDoc *)app.DocumentAt( index + 1 );
		doc->Acquire();
		data = doc;
		
		app.Unlock();
		return true;
	}

	app.Unlock();
	return false;
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
	{
		MeVTrackRef		*th = new MeVTrackRef();
		
		th->data = track;
		track->Acquire();
		track->Lock(Lock_Exclusive);
		return th;
	}
	return NULL;
}


MeVTrackHandle MeVDocRef::FindTrack( int32 inTrackID )
{
	CMeVDoc		*doc = (CMeVDoc *)data;
	StSubjectLock	myLock( *doc, Lock_Shared );
	CTrack		*track = doc->FindTrack( inTrackID );
	
	if (track != NULL)
	{
		MeVTrackRef		*th = new MeVTrackRef();
		
		th->data = track;
		track->Acquire();
		track->Lock(Lock_Exclusive);
		return th;
	}
	return NULL;
}

MeVTrackHandle MeVDocRef::FindTrack( char *inTrackName )
{
	CMeVDoc		*doc = (CMeVDoc *)data;
	StSubjectLock	myLock( *doc, Lock_Shared );
	CTrack		*track = doc->FindTrack( inTrackName );
	
	if (track != NULL)
	{
		MeVTrackRef		*th = new MeVTrackRef();
		
		th->data = track;
		track->Acquire();
		track->Lock(Lock_Exclusive);
		return th;
	}
	return NULL;
}

MeVTrackHandle MeVDocRef::FirstTrack()
{
	CMeVDoc		*doc = (CMeVDoc *)data;
	StSubjectLock	myLock( *doc, Lock_Shared );
	CTrack		*track = doc->FindNextHigherTrackID( 0 );
	
	if (track != NULL)
	{
		MeVTrackRef		*th = new MeVTrackRef();
		
		th->data = track;
		track->Acquire();
		track->Lock(Lock_Exclusive);
		return th;
	}
	return NULL;
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

	char		name[ B_FILE_NAME_LENGTH ];
		
	doc->GetName( name );

	strncpy( outName, name, inMaxChars - 1 );
	outName[ inMaxChars - 1 ] = '\0';
}

void MeVDocRef::ShowWindow()
{
	CMeVDoc		*doc = (CMeVDoc *)data;
	
	doc->ShowWindow( CMeVDoc::Assembly_Window );
}

MeVTrackRef::MeVTrackRef()
{
	data = NULL;
	undo = NULL;
}

MeVTrackRef::~MeVTrackRef()
{
	((CTrack *)data)->Unlock(/*Lock_Exclusive*/);
	CRefCountObject::Release( (CTrack *)data );
}

#if 0
	/**	Repositions this handle to point to the next track. */
bool MeVTrackRef::NextTrack()
{
}
#endif

	/**	Get the ID of this track */
int32 MeVTrackRef::GetID()
{
	CTrack		*track = (CTrack *)data;
	
	return track->GetID();
}

	/**	Get the name of this track */
void MeVTrackRef::GetName( char *name, int32 inMaxChars )
{
	CTrack		*track = (CTrack *)data;
	
	strncpy( name, track->Name(), inMaxChars );
}

	/**	Change the name of this track */
void MeVTrackRef::SetName( char *name )
{
	CTrack		*track = (CTrack *)data;
	
	track->SetName( name );
}

	/**	Start a new undo record for this track */
bool MeVTrackRef::BeginUndoAction( char *inActionLabel )
{
	CEventTrack	*track = (CEventTrack *)data;
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
		if (keep) ((CTrack *)data)->AddUndoAction( (EventListUndoAction *)undo );
		else ((EventListUndoAction *)undo)->Rollback();

		delete (EventListUndoAction *)undo;
		undo = NULL;
	}
}

		/* Merge a list of sorted events into the EventList. */
void MeVTrackRef::Merge( Event *inEventArray, long inEventCount )
{
	CEventTrack		*track = (CEventTrack *)(CTrack *)data;

	track->MergeEvents( inEventArray, inEventCount, (EventListUndoAction *)undo );
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

#ifdef __POWERPC__
#pragma export off
#endif
