/* ===================================================================== *
 * MeVDoc.cpp (MeV)
 * ===================================================================== */

#include "MeVApp.h"
#include "MeVDoc.h"
#include "EventTrack.h"
#include "VChannelWindow.h"
#include "OperatorWindow.h"
#include "AssemblyWindow.h"
#include "StdEventOps.h"
#include "MidiDeviceInfo.h"
#include "PlayerControl.h"
#include "Idents.h"
#include "MeVFileID.h"
#include "ScreenUtils.h"
#include "BeFileWriter.h"
#include "BeFileReader.h"
#include "IFFWriter.h"
#include "IFFReader.h"

// Gnu C Library
#include <stdio.h>
// Storage Kit
#include <NodeInfo.h>
// Support Kit
#include <String.h>

	// REM: Move these elsewhere, eventually

inline CAbstractWriter &operator<<( CAbstractWriter& writer, rgb_color &inColor )
{
	writer << inColor.red << inColor.green << inColor.blue << inColor.alpha;
	return writer;
}

inline CAbstractReader &operator>>( CAbstractReader& reader, rgb_color &outColor )
{
	reader >> outColor.red >> outColor.green >> outColor.blue >> outColor.alpha;
	return reader;
}

int32 ReadStr255( CAbstractReader &inReader, char *outBuffer, int32 inMaxLength );
int32 ReadStr255( CAbstractReader &inReader, char *outBuffer, int32 inMaxLength )
{
	uint8			sLength;
	int32			actual;
	
	inReader >> sLength;
	actual = sLength < inMaxLength - 1 ? sLength : inMaxLength - 1;
	inReader.MustRead( outBuffer, actual );
	outBuffer[ actual ] = 0;
	if (actual < sLength) inReader.Skip( sLength - actual );
	return actual;
}

void WriteStr255( CAbstractWriter &outWriter, char *inBuffer, int32 inLength );
void WriteStr255( CAbstractWriter &outWriter, char *inBuffer, int32 inLength )
{
	if (inLength > 255) inLength = 255;
	outWriter << (uint8)inLength;
	outWriter.MustWrite( inBuffer, inLength );
}

inline uint8 Brighten( uint8 inColor )
{
	int32		d = 255 - inColor;
	
// d = (d*d) / 400;
	d = d / 2 - 16; if (d < 0) d = 0;

	return 255 - d;
}

void CalcHighlightColor( rgb_color &in, rgb_color &out )
{
	out.red   = Brighten( in.red   );
	out.green = Brighten( in.green );
	out.blue  = Brighten( in.blue  );
	out.alpha = 0;
}

void CMeVDoc::Init()
{
	masterRealTrack = NULL;
	masterMeterTrack = NULL;

		// Initialize default attributes
	defaultAttributes[ EvAttr_Duration ]			= Ticks_Per_QtrNote;
	defaultAttributes[ EvAttr_Type ]				= EvtType_Note;
	defaultAttributes[ EvAttr_Selected ]			= 0;
	defaultAttributes[ EvAttr_Channel ]			= 0;
	defaultAttributes[ EvAttr_Pitch ]				= 60;
	defaultAttributes[ EvAttr_AttackVelocity ]		= 64;
	defaultAttributes[ EvAttr_ReleaseVelocity ]	= 64;
	defaultAttributes[ EvAttr_Program ]			= 0;
	defaultAttributes[ EvAttr_ProgramBank ]		= 0;
	defaultAttributes[ EvAttr_VPos ]				= 60;
	defaultAttributes[ EvAttr_ControllerNumber ]	= 0;
	defaultAttributes[ EvAttr_ControllerValue8 ]	= 0;
	defaultAttributes[ EvAttr_ControllerValue16 ]	= 0;
	defaultAttributes[ EvAttr_BendValue ]			= 0;	//??
	defaultAttributes[ EvAttr_InitialBend ]			= 0;	//??
	defaultAttributes[ EvAttr_UpdatePeriod ]		= 20;	//??
	defaultAttributes[ EvAttr_TempoValue ]			= 0;//??
	defaultAttributes[ EvAttr_RepeatCount ]		= 2;
	defaultAttributes[ EvAttr_SequenceNumber ]		= 2;
	defaultAttributes[ EvAttr_Transposition ]		= 0;
	defaultAttributes[ EvAttr_CountourLevel ]		= 0;
	defaultAttributes[ EvAttr_DataSize ]			= 0;
	defaultAttributes[ EvAttr_TSigBeatCount ]		= 4;
	defaultAttributes[ EvAttr_TSigBeatSize ]		= 2;

	initialTempo = 64.0;

	tempoMap.list = new CTempoMapEntry[ 2 ];
	tempoMap.count = 2;
	validTempoMap = false;
//	tempoMap.count = 1;
	
	tempoMap.list[ 0 ].SetInitialTempo( RateToPeriod( initialTempo ) );
	tempoMap.list[ 1 ].SetTempo(		tempoMap.list[ 0 ],
								RateToPeriod( 256.0 ),
								Ticks_Per_QtrNote * 4,
								Ticks_Per_QtrNote * 4,
								ClockType_Metered );
	//m_VCTM=NULL;
	
	//((CMeVApp *)be_app)->GetDefaultVCTable( vcTable );
	//This should be taken care of by the vctable manager...
	//in fact...the default should be the latest saved...or something

	//m_VCTM=new CVCTableManager ();
		
	//maybe add *** new
	// kludge code
	_me = this;
}

void CMeVDoc::AddDefaultOperators()
{
	CMeVApp			*app = (CMeVApp *)be_app;

		// Add active operarators associated with document to this list.
	for (int i = 0; i < app->CountOperators(); i++)
	{
		EventOp		*op = app->OperatorAt( i );
		bool			found = false;
	
			// Check to see if an oper with this name and
			// creator has already been added.
		for (int j = 0; j < operators.CountItems(); j++)
		{
			EventOp		*search = (EventOp *)operators.ItemAt( j );
			
			if (		strcmp( search->Name(), op->Name() ) == 0
				&&	strcmp( search->CreatorName(), op->CreatorName() ) == 0)
			{
				found = true;
				break;
			}
		}
		
		if (found) CRefCountObject::Release( op );
		else operators.AddItem( op );
	}
}

	// CDocument();
CMeVDoc::CMeVDoc( CMeVApp &inApp )
	:	CDocument( (CDocApp &)inApp ),
		m_newTrackID(2),
		vChannelWinState( BRect( 40, 40, 500, 360 ) ),
		operatorWinState( BRect( 40, 40, 480, 280 ) ),
		docPrefsWinState( BRect( 40, 40, 500, 300 ) ),
		assemblyWinState( UScreenUtils::StackOnScreen( 620, 390 ) )
{
	
	Init();

	AddDefaultOperators();

	masterRealTrack  = new CEventTrack( *this, ClockType_Real, 0, "Master Real" );
	masterMeterTrack = new CEventTrack( *this, ClockType_Metered, 1, "Master Metric" );
	activeMaster = masterMeterTrack;
	NewTrack( TrackType_Event, ClockType_Metered );
	SetValid();
}

CMeVDoc::CMeVDoc( CMeVApp &inApp, entry_ref &inRef )
	:	CDocument( (CDocApp &)inApp, inRef ),
		m_newTrackID(2),
		vChannelWinState( BRect( 40, 40, 500, 360 ) ),
		operatorWinState( BRect( 40, 40, 480, 280 ) ),
		docPrefsWinState( BRect( 40, 40, 500, 300 ) ),
		assemblyWinState( BRect( 40, 40, 620, 390 ) )
{
		// REM: We want to be LOADING now...

	Init();

	AddDefaultOperators();

	BFile		file;
	status_t	error;
	
	error = file.SetTo( &inRef, B_READ_ONLY );
	if (error != B_NO_ERROR)
	{
		char		*msg;
	
		switch (error) {
		case B_BAD_VALUE: msg = "The directory or path name you specified was invalid."; break;
		case B_ENTRY_NOT_FOUND: msg = "The file could not be found. Please check the spelling of the directory and file names."; break;
		case B_PERMISSION_DENIED: msg = "You do not have permission to read that file."; break;
		case B_NO_MEMORY: msg = "There was not enough memory to complete the operation."; break;
		default: msg = "An error has been detected of a type...never before encountered, Captain."; break;
		}
		
		CDocApp::Error( msg );
		return;
	}

		// Create reader and IFF reader.
	CBeFileReader	reader( file );
	CIFFReader	iffReader( reader );
	int32		formType;
	
	iffReader.ChunkID( 1, &formType );
	
	for (;;) {
	
		if (iffReader.NextChunk() == false) break;

		switch (iffReader.ChunkID( 0, &formType )) {
		case VCTable_ID:
			ReadVCTable( iffReader );	// Virtual channel table
			break;

		case DocTempo_ID:
			iffReader >> initialTempo;
			break;

		case Form_ID:
			switch (formType) {
			case TrackType_Event:
			case FMasterRealTrack:
			case FMasterMeteredTrack:
				ReadTrack( formType, iffReader );
				break;
			}
		}
	}

	if (masterRealTrack == NULL)
		masterRealTrack  = new CEventTrack( *this, ClockType_Real, 0, "Master Real" );

	if (masterMeterTrack == NULL)
		masterMeterTrack = new CEventTrack( *this, ClockType_Metered, 1, "Master Metric" );

	activeMaster = masterMeterTrack;
	SetValid();
}

CMeVDoc::~CMeVDoc()
{
		// Delete the master track
	masterRealTrack->RequestDelete();
	masterMeterTrack->RequestDelete();
	CRefCountObject::Release( masterRealTrack );
	CRefCountObject::Release( masterMeterTrack );
	
		// Delete all tracks
	for (int i = 0; i < tracks.CountItems(); i++)
	{
// 	delete tracks.ItemAt( i );
		CTrack		*track = (CTrack *)tracks.ItemAt( i );
			
		track->RequestDelete();
		CRefCountObject::Release( track );
	}

	for (int i = 0; i < operators.CountItems(); i++)
	{
		EventOp		*op = (EventOp *)operators.ItemAt( i );

		CRefCountObject::Release( op );
	}

	for (int i = 0; i < activeOperators.CountItems(); i++)
	{
		EventOp		*op = (EventOp *)activeOperators.ItemAt( i );

		CRefCountObject::Release( op );
	}
}

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

	// Create a new track (refcount = 1)
CTrack *CMeVDoc::NewTrack( ulong inTrackType, TClockType inClockType )
{
	switch (inTrackType) {
	case TrackType_Event:
		CEventTrack		*etk;

		etk = new CEventTrack( *this, inClockType, m_newTrackID++, NULL );
		if (etk)
		{
			BString name = etk->Name();
			name << " " << etk->GetID() - 1;
			etk->SetName(name.String());
			tracks.AddItem( etk );
			return etk;
		}
		break;
	}
	return NULL;
}

	// Locate a track by it's ID. (0,1 for master track)
CTrack *CMeVDoc::FindTrack( long inTrackID )
{
	if (inTrackID == 0)		return (CTrack *)masterRealTrack->Acquire();
	else if (inTrackID == 1)	return (CTrack *)masterMeterTrack->Acquire();
	for (int i = 0; i < tracks.CountItems(); i++)
	{
		CTrack		*track = (CTrack *)tracks.ItemAt( i );
		
		if (track->Deleted()) continue;

		if (inTrackID == track->GetID()) return (CTrack *)track->Acquire();
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
		
		if (strcmp( track->Name(), inTrackName ) == 0)
			return (CTrack *)track->Acquire();
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
	masterRealTrack->PostUpdate( inHint );
	masterMeterTrack->PostUpdate( inHint );

	for (int i = 0; i < tracks.CountItems(); i++)
	{
		CTrack		*track = (CTrack *)tracks.ItemAt( i );
		
		track->PostUpdate( inHint );
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

	/**	Show or hide the window of a particular type */
BWindow *CMeVDoc::ShowWindow( enum EWindowTypes inType )
{
	CWindowState		*state;
	
	switch (inType) {
	case VChannel_Window: state = &vChannelWinState; break;
	case DocPrefs_Window: state = &docPrefsWinState; break;
	case Assembly_Window: state = &assemblyWinState; break;
	case Operator_Window: state = &operatorWinState; break;
	default: return NULL;
	}
	
	if (state->Activate() == false)
	{
		BWindow		*bw;
	
		switch (inType) {
		case VChannel_Window:
			//bw = new CVChannelWindow( *state, *this );
			//bw->Show();
			break;

		case DocPrefs_Window:
			break;

		case Assembly_Window:
			bw = new CAssemblyWindow(UScreenUtils::StackOnScreen(540, 300),
									 this);
			bw->Show();
			break;

		case Operator_Window:
			bw = new COperatorWindow( *state, *this );
			bw->Show();
			break;
		}
	}
	
	return state->Window();
}
	
	/**	Returns TRUE if a particular window type is open. */
bool CMeVDoc::IsWindowOpen( enum EWindowTypes inType )
{
	CWindowState		*state;
	
	switch (inType) {
	case VChannel_Window: state = &vChannelWinState; break;
	case DocPrefs_Window: state = &docPrefsWinState; break;
	case Assembly_Window: state = &assemblyWinState; break;
	case Operator_Window: state = &operatorWinState; break;
	default: return false;
	}
	
	return state->IsOpen();
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
			masterRealTrack->CompileOperators();
			masterMeterTrack->CompileOperators();
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
			masterRealTrack->CompileOperators();
			masterMeterTrack->CompileOperators();
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
	
	if (activeMaster != inTrack)
	{
		activeMaster->DeselectAll( NULL );
		activeMaster = inTrack;
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

void CMeVDoc::VirtualChannelName( int32 inChannelIndex, char *outBuf )
{
	/*
	VChannelEntry		*vc = &vcTable[ inChannelIndex ];
	MIDIDeviceInfo	*mdi = ((CMeVApp *)be_app)->LookupInstrument( vc->port, vc->channel - 1 );
	char				*name;
		
	if (mdi != NULL)	name = mdi->name;
	else
	{
		name = CPlayerControl::PortName( vc->port );
		if (name == NULL)
		{
			strcpy( outBuf, "--" );
			return;
		}
	}
*/		
	sprintf( outBuf, "%s: %d", "NONO",9 );


}

void CMeVDoc::SaveDocument()
{
		// Make a backup of the doc file.
	char		docName[ B_FILE_NAME_LENGTH ];

		// REM: Do we need to ask for over-write check?
	
	if (GetName( docName ))
	{
		BEntry	backup( DocLocation() );

			// Make sure there's enough room for the letters '.bak'
		docName[ B_FILE_NAME_LENGTH - 5 ] = '\0';
		strcat( docName, ".bak" );
		
			// This might fail if doc has never been created. So what?
		backup.Rename( docName );
	}

	BFile		file( &DocLocation(), B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE );
	
	//REM: Lock the doc. (read-only if possible)
	
		// Create reader and IFF reader.
	CBeFileWriter	writer( file );
	CIFFWriter	iffWriter( writer );
	
	iffWriter.Push( Form_ID );
	iffWriter << (long)MeV_ID;
	
	WriteVCTable( iffWriter );
	
	iffWriter.Push( DocTempo_ID );
	iffWriter << initialTempo;
	iffWriter.Pop();

		// Write master real track
	iffWriter.Push( Form_ID );
	iffWriter << (long)FMasterRealTrack;
	masterRealTrack->WriteTrack( iffWriter );
	iffWriter.Pop();

		// Write master metered track
	iffWriter.Push( Form_ID);
	iffWriter << (long)FMasterMeteredTrack;
	masterMeterTrack->WriteTrack( iffWriter );
	iffWriter.Pop();

	for (int i = 0; i < tracks.CountItems(); i++)
	{
		CTrack		*track = (CTrack *)tracks.ItemAt( i );
	
		if (track->Deleted()) continue;

			// Push a seperate FORM per track	
		iffWriter.Push( Form_ID );
		iffWriter << (long)track->TrackType();
		track->WriteTrack( iffWriter );
		iffWriter.Pop();
	}
	
	SetModified( false );
	
	BNode	node( &DocLocation() );
	if (node.InitCheck() == B_NO_ERROR)
	{
		BNodeInfo	ni( &node );
		
		if (ni.InitCheck() == B_NO_ERROR)
		{
			ni.SetType( "audio/x-vnd.SylvanTechnicalArts-MeV" );
			ni.SetPreferredApp( "application/x-vnd.SylvanTechnicalArts-MeV" );
		}
	}
}

void CMeVDoc::Export( BMessage *msg )
{
	BFilePanel	*fp = ((CMeVApp *)be_app)->GetExportPanel( &be_app_messenger );
	if (fp != NULL)
	{
		BMessage	exportMsg( 'expt' );
		BMessage	pluginMsg;
		void		*pluginPtr;
		
		msg->FindPointer( "plugin", &pluginPtr );
		msg->FindMessage( "msg", &pluginMsg );

		exportMsg.AddPointer( "Document", &this->_me );
		exportMsg.AddPointer( "plugin", pluginPtr );
		exportMsg.AddMessage( "msg", &pluginMsg );

		fp->SetMessage( &exportMsg );
		fp->SetRefFilter( NULL ); // exportFilter????
		fp->Show();
	}
}

void CMeVDoc::ReadVCTable( CIFFReader &reader )
{
	int32		i = 0;

	while (reader.BytesAvailable() > 0 )
	{
		VChannelEntry	*vc = m_VCTM[ i ];
//		ReadStr255( reader, vc.name, sizeof vc.name );
		//dan 7/17/00 reader >> vc.port >> vc.channel >> vc.flags >> vc.velocityContour >> vc.initialTranspose;
		reader >> vc->channel >> vc->flags >> vc->velocityContour >> vc->initialTranspose;
		reader >> vc->fillColor;
		//CalcHighlightColor( vc->fillColor, vc->highlightColor );
		i++;		
	}
}

void CMeVDoc::WriteVCTable( CIFFWriter &writer )
{
	writer.Push( VCTable_ID );
	for (int i = 0; i < Max_VChannels; i++)
	{
		VChannelEntry	*vc = m_VCTM[ i ];
	
//		WriteStr255( writer, vc.name, strlen( vc.name ) );
		//dan 7/17/00writer << vc.port << vc.channel << vc.flags << vc.velocityContour << vc.initialTranspose;
		writer << vc->channel << vc->flags << vc->velocityContour << vc->initialTranspose;
		writer << vc->fillColor;
	}
	writer.Pop();
}

void CMeVDoc::ReadTrack( long inTrackType, CIFFReader &reader )
{
	CTrack		*track;

	if (inTrackType == FMasterRealTrack)
	{
		masterRealTrack  = new CEventTrack( *this, ClockType_Real, 0, "Master Real" );
		track = masterRealTrack;
	}
	else if (inTrackType == FMasterMeteredTrack)
	{
		masterMeterTrack = new CEventTrack( *this, ClockType_Metered, 1, "Master Metric" );
		track = masterMeterTrack;
	}
	else
	{
		track = NewTrack( TrackType_Event, ClockType_Metered );
	}

	if (track == NULL) return;
	
	reader.Push();

	for (;;) {
		if (reader.NextChunk() == false) break;
		track->ReadTrackChunk( reader );
	}
	
	reader.Pop();
	
	if (inTrackType == TrackType_Event)
		((CEventTrack *)track)->SummarizeSelection();
}
