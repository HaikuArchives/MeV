/* ===================================================================== *
 * Player.cpp (MeV/Engine)
 * ===================================================================== */

#include "MeV.h"
#include "MeVApp.h"

#include "TimeUnits.h"
#include "Track.h"
#include "EventStack.h"
#include "PlaybackTask.h"
#include "PlaybackTaskGroup.h"
#include "Player.h"
#include "Idents.h"
#include "MidiDeviceInfo.h"
#include <stdio.h>
#include <MidiProducer.h>

#include <Debug.h>

// Gnu C Library
#include <math.h>
// Midi Kit


#define D_CONTROL(x) //PRINT (x)
#define D_SYNTH(x) //PRINT (x)
#define D_EVENT(x) //PRINT (x)
#define D_WARNING(x) PRINT (x)

const int32			maxSleep = 30;

CMIDIPlayer			thePlayer;

// ---------------------------------------------------------------------------
// The main player loop

status_t 
CMIDIPlayer::ControlThreadEntry(void *user)
{
	(static_cast<CMIDIPlayer*>(user))->ControlThread();
	return B_OK;
}

status_t 
CMIDIPlayer::StopControlThread()
{
	thread_id tid = m_thread;
	if(tid < 0)
	{
		D_WARNING(("CMIDIPlayer::StopControlThread(): thread already stopped\n"));
		return B_NOT_ALLOWED;
	}
	status_t err = write_port_etc(m_port, Command_Quit, 0, 0, B_TIMEOUT, 250000LL);
	if(err < B_OK)
	{
		D_WARNING(("CMIDIPlayer::StopControlThread(): write_port_etc(): %s\n",
			strerror(err)));
		return err;
	}
	while(wait_for_thread(tid, &err) == B_INTERRUPTED) {}
	m_thread = 0;
	return B_OK;
}


void CMIDIPlayer::ControlThread()
{
	D_CONTROL(("CMIDIPlayer::Run()\n"));

		// Initialize time vars
	m_internalTimerTick = system_time() / 1000;			// the current time value

	// +++++ convert to bigtime_t
	long nextEventTime = m_internalTimerTick;
	
	bool keepRunning = true;
	while (keepRunning) {
	
		int32		cmdSize,
					cmdCode;
		CommandArgs	cmdArgs;
		bigtime_t	wakeUp = (bigtime_t)nextEventTime * 1000 - system_time();
		
		if (wakeUp < 0) {
//			fprintf(stderr, "late: next event %Ld, now %Ld\n",
//				bigtime_t(nextEventTime) * 1000,
//				system_time());
			wakeUp = 0;
		}
		
		cmdSize = read_port_etc(m_port,
								&cmdCode,
								&cmdArgs,
								sizeof cmdArgs,
								B_TIMEOUT,
								wakeUp );

		if (cmdSize >= 0)
		{
			switch( cmdCode ) {
			case Command_Start:
				D_CONTROL(("CMIDIPlayer: Command_Start\n"));
				if (cmdArgs.document == NULL) break;
				CRefCountObject::Release( songGroup->doc );
				songGroup->doc = cmdArgs.document;
				songGroup->m_destlist = cmdArgs.document->GetDestinationList(  );
				//songGroup->m_destlist = &cmdArgs.document->GetVChannel( 0 );
				//check the above out dan				
					// Reset all channel state records...
				InitChannelStates();

				if (cmdArgs.trackID >= 0 && cmdArgs.trackID <= 1)
				{
					CTrack	*track1,
							*track2;
		
					track1 = cmdArgs.document->FindTrack( (int32)0 );
					track2 = cmdArgs.document->FindTrack( 1 );
					if (track1 != NULL && track2 != NULL)
					{	
						songGroup->Start(track1,
										track2,
										cmdArgs.locTime,
										(enum ELocateTarget)cmdArgs.locateTarget,
										cmdArgs.duration,
										(enum ESyncType)cmdArgs.syncType,
										cmdArgs.options );
					}
					else
					{
						if (track1) CRefCountObject::Release( track1 );
						if (track2) CRefCountObject::Release( track2 );
					}
				}
				else
				{
					CTrack	*track;
		
					track = cmdArgs.document->FindTrack( cmdArgs.trackID );
					if (track != NULL)
					{	
						songGroup->Start(track,
										NULL,
										cmdArgs.locTime,
										(enum ELocateTarget)cmdArgs.locateTarget,
										cmdArgs.duration,
										(enum ESyncType)cmdArgs.syncType,
										cmdArgs.options );
					}
				}
				break;

			case Command_Stop:
				D_CONTROL(("CMIDIPlayer: Command_Stop\n"));
				if (cmdArgs.document == NULL) break;
				CRefCountObject::Release( cmdArgs.document );
				songGroup->flags |= CPlaybackTaskGroup::Clock_Stopped;
				songGroup->FlushNotes();
				// REM: Notify UI of stoppage...
				break;

			case Command_Pause:
			case Command_Continue:
				break;
				
			case Command_Quit:
				D_CONTROL(("CMIDIPlayer: Command_Quit\n"));
				keepRunning = false;
				break;
			}
			
			if (cmdCode != Command_Attention) {

				BMessage		msg( Player_ChangeTransportState );

				be_app->PostMessage( &msg );
			}
		}
		
			// And then process any waiting events.
		m_internalTimerTick = system_time() / 1000;			// the current time value

		{
			LOCK_PLAYER;
			
			CPlaybackTaskGroup *group;
			
			nextEventTime = m_internalTimerTick + maxSleep;

			for (	group = (CPlaybackTaskGroup *)m_groupList.First();
					group;
					group = (CPlaybackTaskGroup *)group->Next() )
			{
				int32		nextGroupEvent;
			
				group->nextEventTime = LONG_MAX;
				group->Update( m_internalTimerTick );

				// Compute next event time as min of all track times
				nextGroupEvent = group->nextEventTime + group->origin;
				if (IsTimeGreater( nextGroupEvent, nextEventTime ))
					nextEventTime = nextGroupEvent;
			}
		}
	}
}

// ---------------------------------------------------------------------------
// initialize the player task

CMIDIPlayer::CMIDIPlayer()
{
	for (uint32 i = 0; i < Max_MidiPorts; i++)
	{
		m_ports[ i ] = NULL;
		strcpy( m_portInfo[ i ].portName, "" );
		strcpy( m_portInfo[ i ].devString, "" );
	}
	
	InitChannelStates();

//	synth = NULL;
//	synthUseCount = 0;
	
		// Default tempo is 100 bpm
//	defaultTempoPeriod = RateToPeriod( 100.0 );
	
	songGroup = NULL;
	wildGroup = NULL;
}

// ---------------------------------------------------------------------------
// clean up the player task

CMIDIPlayer::~CMIDIPlayer()
{
	StopControlThread();

	CPlaybackTaskGroup	*group;

		// Delete song contexts
	while ((group = (CPlaybackTaskGroup *)m_groupList.First())) delete group;
	songGroup = wildGroup = NULL;

		// release ports
		// REM: Commented out because code crashes...!
	for (uint32 i = 0; i < Max_MidiPorts; i++)
	{
		m_ports[ i ]->Release();
	}
	//delete synth;
}

void CMIDIPlayer::Initialize()
{
	m_thread = spawn_thread(
		&ControlThreadEntry,
		"CMIDIPlayer",
		B_REAL_TIME_PRIORITY,
		this);

	m_port = create_port( 1024, "MeVPlayer Command Port" );

		// Set up a context for playing songs
		// REM: Should pass the document pointer as parameter
	songGroup = new CPlaybackTaskGroup( NULL );
	D_CONTROL((">>>>> songGroup: %p\n", songGroup));

		// Set up a context for miscellaneous note-playing
		// REM: wildContext->setTime( m_internalTimerTick, 0 );
	wildGroup = new CPlaybackTaskGroup( NULL );
	D_CONTROL((">>>>> wildGroup: %p\n", wildGroup));

		// Set up wildContext with current time, and start it.
	wildGroup->origin = 0; // system_time() / 1000;
	wildGroup->real.time = 0;
	wildGroup->metered.time = 0;
		// set context to "running always"
	wildGroup->flags = CPlaybackTaskGroup::Clock_Continuous;

	resume_thread(m_thread);
}

port_id 
CMIDIPlayer::Port() const
{
	return m_port;
}


// ---------------------------------------------------------------------------
// locking API

bool 
CMIDIPlayer::Lock()
{
	return m_lock.Lock();
}

void 
CMIDIPlayer::Unlock()
{
	m_lock.Unlock();
}

bool 
CMIDIPlayer::IsLocked()
{
	return m_lock.IsLocked();
}



// ---------------------------------------------------------------------------
// Initialize all channel state records...

void CMIDIPlayer::InitChannelStates()
{
			// Initialize all channel states...
	for (uint32 i = 0; i < Max_MidiPorts; i++)
	{
		for (int j = 0; j < 16; j++)
		{
			memset( &m_portInfo[ i ].channelStates[ j ], 0xff, sizeof (ChannelState) );
		}
	}
}

#if 0
	// Any channel information which was modified during locating should now
	// be dumped out to the output device.

void CMIDIPlayer::DumpChannelStates()
{
			// Initialize all channel states...
	for (int i = 0; i < Max_MidiPorts; i++)
	{
		for (int j = 0; j < 16; j++)
		{
			memset( &m_portInfo[ i ].channelStates[ j ], 0xff, sizeof (ChannelState) );
		}
	}
}
#endif

// ---------------------------------------------------------------------------
// Allocate a new BMidiSynth and a BSynth if needed.

BMidiSynth *CMIDIPlayer::NewMidiSynth()
{
	D_SYNTH(("CMIDIPlayer::NewMidiSynth()\n"));
	
/*	BMidiSynth		*s;
	status_t 		err;

	synthUseCount++;
	if (synth == NULL)
	{
		synth = new BSynth;
		err = synth->LoadSynthData( B_BIG_SYNTH );

		if (err)
		{
			delete synth;
			synth = NULL;
			return NULL;
		}
	}

	s = new BMidiSynth();
		
	err = s->EnableInput( true, false );
	if (err != B_NO_ERROR) return NULL;

	for (int i = 0; i < 127; i++)
	{
		int instrument = B_ACOUSTIC_GRAND + i;
	
		s->LoadInstrument( instrument );
		s->ProgramChange( i + 1, instrument );
	}

		// For test purposes, let's create a test output port
	return s;
	*/
}

// ---------------------------------------------------------------------------
// Free the BMidiSynth and the BSynth if needed

void CMIDIPlayer::DeleteMidiSynth( BMidiSynth *inSynth )
{
	D_SYNTH(("CMIDIPlayer::DeleteMidiSynth()\n"));

	/*delete inSynth;
	synthUseCount--;
	if (synthUseCount <= 0 && synth != NULL)
	{
		delete synth;
		synth = NULL;
	}*/
}

// ---------------------------------------------------------------------------
// queueEvents -- push some events on the the linear time stack, and then
// check to see if it's time to wake up.

bool CMIDIPlayer::QueueEvents( Event *eventList, uint32 count, long startTime )
{
		// Note: Locking not needed since stack has it's own lock.

	if (wildGroup->real.stack.PushList( eventList, count, startTime ))
	{
		return true;
	}
	else return false;
}


// ---------------------------------------------------------------------------
// queue given events for immediate execution
bool 
CMIDIPlayer::QueueImmediate(Event *eventList, uint32 count)
{
	return QueueEvents( eventList, count, uint32(system_time() / 1000LL));
}



// ---------------------------------------------------------------------------
// Find the group associated with a particular document

CPlaybackTaskGroup *CMIDIPlayer::FindGroup( CMeVDoc *doc )
{
	if (doc != NULL && songGroup->doc == doc) return songGroup;
	return NULL;
}

// ---------------------------------------------------------------------------
// Send midi events to the appropriate output port

void CMIDIPlayer::SendEvent(
	const Event		&ev,
	BMidiLocalProducer	*inPort,
	uchar			inActualChannel,
	bigtime_t		inTime )
{
	BMidiLocalProducer	*port =  inPort ;
	
	ChannelState	*chState = &m_portInfo[0].channelStates[ 0];
//	ChannelState	*chState = &portInfo[0].channelStates[ inActualChannel ];
//	BMidiLocalProducer	*port = m_ports[ inPort ];
//	ChannelState	*chState = &m_portInfo[ inPort ].channelStates[ inActualChannel ];

	MIDIDeviceInfo	*mdi;
	uint8			lsbIndex;

	if (port == NULL) return;

		// Attempt to send the event.

	switch (ev.Command()) {

		// MIDI channel events:

	case EvtType_Note:							// note-on event
		D_EVENT(("SendEvent(NoteOn)\n"));
		port->SprayNoteOn(inActualChannel,
						 ev.note.pitch,
						 ev.note.attackVelocity, (bigtime_t)inTime );
		break;

	case EvtType_NoteOff:						// note-off event 
		D_EVENT(("SendEvent(NoteOff)\n"));
		port->SprayNoteOff(	inActualChannel,
						ev.note.pitch,
						ev.note.releaseVelocity,
						(bigtime_t)inTime );
		break;

	case EvtType_ChannelATouch:					// channel aftertouch
		D_EVENT(("SendEvent(ChannelATouch)\n"));
		 if (chState->channelAfterTouch != ev.aTouch.value)
		{
			port->SprayChannelPressure(
								inActualChannel,
								ev.aTouch.value,
								(bigtime_t)inTime );
			chState->channelAfterTouch = ev.aTouch.value;
		}
		break;

	case EvtType_PolyATouch:						// polyphonic aftertouch
		D_EVENT(("SendEvent(PolyATouch)\n"));
		port->SprayKeyPressure(	inActualChannel,
							ev.aTouch.pitch,
							ev.aTouch.value,
							(bigtime_t)inTime );
		break;

	case EvtType_Controller:						// controller change
		D_EVENT(("SendEvent(Controller)\n"));

			// Check if it's a 16-bit controller
		lsbIndex = controllerInfoTable[ ev.controlChange.controller ].LSBNumber;

		if (lsbIndex == ev.controlChange.controller || lsbIndex > 127)
		{
				// It's an 8-bit controller.
			if (	ev.controlChange.MSB != chState->ctlStates[ ev.controlChange.controller ])
			{
				port->SprayControlChange(	inActualChannel,
									ev.controlChange.controller,
									ev.controlChange.MSB,
									(bigtime_t)inTime );

				chState->ctlStates[ ev.controlChange.controller ] = ev.controlChange.MSB;
			}
		}
		else
		{
				// Handle the MSB first...if the MSB changed, then update LSB state as well
			if (	ev.controlChange.MSB != chState->ctlStates[ ev.controlChange.controller ]
				&& ev.controlChange.MSB < 128)
			{
				port->SprayControlChange(	inActualChannel,
									ev.controlChange.controller,
									ev.controlChange.MSB,
									(bigtime_t)inTime );

				chState->ctlStates[ ev.controlChange.controller ] = ev.controlChange.MSB;
				chState->ctlStates[ lsbIndex ] = 0;
			}

				// Now deal with the LSB.
			if (	ev.controlChange.LSB != chState->ctlStates[ lsbIndex ]
				&& ev.controlChange.LSB < 128)
			{
				port->SprayControlChange(	inActualChannel,
									lsbIndex,
									ev.controlChange.LSB,
									(bigtime_t)inTime );

				chState->ctlStates[ lsbIndex] = ev.controlChange.LSB;
			}
		}
		break;

	case EvtType_ProgramChange:					// program change
		D_EVENT(("SendEvent(ProgramChange)\n"));

			// Return the MIDI device associated with this port and channel
		//mdi = ((CMeVApp *)be_app)->LookupInstrument( inPort, inActualChannel );
		//dan 6/30/00
			// Only send bank changes if the instrument understands such...
			// And only if this event has a valid bank number
		if (mdi != NULL && mdi->SupportsProgramBanks())
		{
				// Don't send a bank change if the instrument is already
				// set to that bank
			if (	ev.programChange.bankMSB < 128
				&& ev.programChange.bankLSB < 128
				&& (chState->ctlStates[ 0 ] != ev.programChange.bankMSB
				   || chState->ctlStates[ 32 ] != ev.programChange.bankLSB) )
			{
					// Both bank bytes must always be sent...
			
					// Send a bank change MSB message
				port->SprayControlChange(	inActualChannel,
									0x00,
									ev.programChange.bankMSB,
									(bigtime_t)inTime );

					// Send a bank change LSB message
				port->SprayControlChange(	inActualChannel,
									0x20,
									ev.programChange.bankLSB,
									(bigtime_t)inTime );
	
					// Update the channel state
				chState->ctlStates[ 0 ] = ev.programChange.bankMSB;
				chState->ctlStates[ 32 ] = ev.programChange.bankLSB;
			}
		}
		
			// Always send a program change, regardless...
		port->SprayProgramChange(	inActualChannel,
							ev.programChange.program,
							(bigtime_t)inTime );

		chState->program = ev.programChange.program;
		break;

	case EvtType_PitchBend:						// pitch bend
		D_EVENT(("SendEvent(PitchBend)\n"));

			// Don't send un-needed pitch-bends
		if (chState->pitchBendState != ev.pitchBend.targetBend)
		{
			port->SprayPitchBend(		inActualChannel,
								ev.pitchBend.targetBend & 0x7f,
								ev.pitchBend.targetBend >> 7,
								(bigtime_t)inTime );
			chState->pitchBendState = ev.pitchBend.targetBend;
		}
		break;

		// MIDI system events

	case EvtType_SysEx:							// system exclusive
		D_EVENT(("SendEvent(SysEx)\n"));

		void				*data;
		int32			size;
		
		data = ev.ExtendedData();
		size = ev.ExtendedDataSize();	
		
		if (data != NULL && size > 0 )
		{
			port->SpraySystemExclusive( data, size, inTime );
		}
		break;

// Global control events

// case evtTypeStop:							// stop the sequencer
// case evtTypeGo:								// start the sequencer
// case evtTypeLocate:							// locate to "duration" time
// case evtTypeCue:							// trigger a cue point
// case evtTypeMTCCue:							// trigger an MTC cue point
// case evtTypeMuteVChannel:					// mute a vChannel
// case evtTypeMuteTrack:						// mute a track
// case evtTypeSpliceIn:						// a "splice" event for overdub
// case evtTypeSpliceOut:						// a "splice" event for overdub
// 	break;

		// Track control events
// case evtTypeSequence:						// play another track
// case evtTypeBranch:							// conditional branch to track
// case evtTypeErase:							// erase notes on channel
// case evtTypePunch:							// punch in over track
// 	break;

		// Clock control events

// case evtTypeTempo:							// change tempo event
// case evtTypeTimeSig:						// change time signature event
// 	break;

// case evtTypeVelocityContour:				// velocity contour event
// case evtTypeTranspose:						// transposition for vChannel
	}
}

void CPlayerControl::InitPlayer()
{
	thePlayer.Initialize();
}


// ---------------------------------------------------------------------------
// Audio feedback routine for user interface

	// Play some audible feedback.
void CPlayerControl::DoAudioFeedback(
	CMeVDoc					*doc,
	enum E_EventAttribute 	feedbackAttribute,	// Which attr is changing
	uint8					attributeValue,		// New attribute value
	const Event				*demoEvent )			// Prototype event
{
	Event			fbEvents[ 3 ],
					*modEvent = &fbEvents[ 0 ],
					*noteStart = &fbEvents[ 1 ],
					*noteStop = &fbEvents[ 2 ];
	
	if (doc && demoEvent)
	{
		int32		channel;
		
		channel = (feedbackAttribute == EvAttr_Channel)
				? attributeValue
				: demoEvent->GetVChannel();

		Destination	*dest = doc->GetVChannel( channel );
		
		modEvent->stack.actualPort = dest->m_producer;	
		modEvent->stack.actualChannel = dest->channel;
	}
	else
	{
		int32		channel;
		
		channel = (feedbackAttribute == EvAttr_Channel)
				? attributeValue
				: 0;

		modEvent->stack.actualPort = 0;
		modEvent->stack.actualChannel = channel;
	}

		// Set up a default list of events which plays an average note.

	modEvent->stack.start = 0;
	modEvent->stack.task = cFeedbackTask;
	modEvent->note.command = 0;

	*noteStart = *modEvent;
	*noteStop = *modEvent;

	noteStart->stack.start	= gPrefs.feedbackDelay;
	noteStop->stack.start		= gPrefs.feedbackDelay + 450;
	noteStart->note.command	= EvtType_Note;
	noteStop->note.command	= EvtType_NoteOff;
	if (demoEvent && demoEvent->Command() == EvtType_Note)
	{
		noteStart->note.pitch = noteStop->note.pitch = demoEvent->note.pitch;
		noteStart->note.attackVelocity = demoEvent->note.attackVelocity;
		noteStop->note.releaseVelocity = demoEvent->note.releaseVelocity;
	}
	else
	{
		noteStart->note.pitch = noteStop->note.pitch = 60;
		noteStart->note.attackVelocity = noteStop->note.releaseVelocity = 64;
	}

	switch (feedbackAttribute) {
	default:
// case evAttrControllerValue8:
// case evAttrControllerValue16:
// case evAttrBendValue:
// case evAttrNone:		
// case evAttrControllerNumber:
// case evAttrProgramBank:
// case evAttrVPos:
// case evAttrRepeatCount:
// case evAttrSequenceNumber:
// case evAttrTransposition:
// case evAttrCountourLevel:
// case evAttrTempoValue:
		return;

	case EvAttr_Channel:
			// REM: Change this to real channel...
// 	noteStart->stack.actualChannel
// 		= noteStop->stack.actualChannel = attributeValue;
		break;

	case EvAttr_Pitch:
		noteStart->note.pitch = noteStop->note.pitch = attributeValue;
		break;
		
	case EvAttr_AttackVelocity:
		noteStart->note.attackVelocity = attributeValue;
		//printf("player.cpp %d\n",noteStart->note.attackVelocity);
		break;

	case EvAttr_ReleaseVelocity:
		noteStart->note.releaseVelocity = attributeValue;
		break;

	case EvAttr_Program:
		modEvent->programChange.command = EvtType_ProgramChange;
		modEvent->programChange.program = attributeValue;
		modEvent->programChange.bankMSB = MIDIValueUnset;
		break;
	}

	LOCK_PLAYER;

		// Kill the previous feedback event
	thePlayer.wildGroup->FlushEvents();

		// If it's a note event, play the note.
		// If the doc is already playing, play only the feedback event, no note,
		// else play the feedback event and a note to hear it.
	if (modEvent->note.command == 0)
		thePlayer.QueueImmediate( &fbEvents[ 1 ], 2 );
	else if (IsPlaying( doc ))
		thePlayer.QueueImmediate( &fbEvents[ 0 ], 1 );
	else thePlayer.QueueImmediate( &fbEvents[ 0 ], 3 );
}

// ---------------------------------------------------------------------------
// Tell the player to start playing a song!

void CPlayerControl::PlaySong(
	CMeVDoc				*document,
	int32				trackID,
	long					locTime,
	enum ELocateTarget	locateTarget,
	int32				duration,
	enum ESyncType		syncType,
	int16				options )
{
	CommandArgs		cmd;
	
	document->Acquire();				// Make sure document doesn't go away!
	
	cmd.document		= document;
	cmd.trackID		= trackID;
	cmd.locTime		= locTime;
	cmd.locateTarget	= locateTarget;
	cmd.duration		= duration;
	cmd.syncType		= syncType;
	cmd.options		= options;
	
	write_port(	thePlayer.Port(),
				Command_Start,
				&cmd,
				sizeof cmd );
}

// ---------------------------------------------------------------------------
// Tell the player to stop playing a song!

void CPlayerControl::StopSong( CMeVDoc *document )
{
	CommandArgs		cmd;
	
	document->Acquire();				// Make sure document doesn't go away!

	cmd.document		= document;
	
	write_port( thePlayer.m_port, Command_Stop, &cmd, sizeof cmd );
}

// ---------------------------------------------------------------------------
// Determine if this document or any tracks in it are playing

bool CPlayerControl::IsPlaying( CMeVDoc *document )
{
	LOCK_PLAYER;
	CPlaybackTaskGroup *group = thePlayer.FindGroup( document );
	if (group && (group->flags & CPlaybackTaskGroup::Clock_Stopped) == false)
		return true;
	return false;
}

// ---------------------------------------------------------------------------
// If this document is playing (or paused), return the current playback state

bool CPlayerControl::GetPlaybackState( CMeVDoc *document, PlaybackState &outState )
{
	LOCK_PLAYER;
	CPlaybackTaskGroup *group = thePlayer.FindGroup( document );
	if (group)
	{
		outState.running		= !(group->flags & CPlaybackTaskGroup::Clock_Stopped);
		outState.realTime		= group->real.time;
		outState.meteredTime	= group->metered.time;
		return true;
	}
	return false;
}

// ---------------------------------------------------------------------------
// Determine the pause state for this document.

bool CPlayerControl::PauseState( CMeVDoc *document )
{
	LOCK_PLAYER;
	CPlaybackTaskGroup *group = thePlayer.FindGroup( document );
	if (group && (group->flags & CPlaybackTaskGroup::Clock_Paused) != false)
		return true;
	return false;
}

// ---------------------------------------------------------------------------
// Set the state of the pause flag for this document, if it's playing.

void CPlayerControl::SetPauseState( CMeVDoc *document, bool inPauseState )
{
	LOCK_PLAYER;
	CPlaybackTaskGroup *group = thePlayer.FindGroup( document );
	if (group)
	{
			// REM: If externally sync'd, then no effect.
		if (inPauseState)	group->flags |= CPlaybackTaskGroup::Clock_Paused;
		else					group->flags &= ~CPlaybackTaskGroup::Clock_Paused;
	}
}

// ---------------------------------------------------------------------------
// Toggle the state of the pause flag for this document, if it's playing.

void CPlayerControl::TogglePauseState( CMeVDoc *document )
{
	LOCK_PLAYER;
	CPlaybackTaskGroup *group = thePlayer.FindGroup( document );
	if (group)
	{
			// REM: If externally sync'd, then no effect.
		group->flags ^= CPlaybackTaskGroup::Clock_Paused;
	}
}

// ---------------------------------------------------------------------------
// Return the current tempo.

double CPlayerControl::Tempo( CMeVDoc *document )
{
	LOCK_PLAYER;
	CPlaybackTaskGroup *group = thePlayer.FindGroup( document );
	
	if (group) return PeriodToRate( group->CurrentTempoPeriod() );
//	return PeriodToRate( thePlayer.defaultTempoPeriod );
	if (document) return document->InitialTempo();
	return 100.0;
}
	
// ---------------------------------------------------------------------------
// Set the current tempo

void CPlayerControl::SetTempo( CMeVDoc *document, double newTempo )
{
	long		period = RateToPeriod( newTempo );
	
	LOCK_PLAYER;
	CPlaybackTaskGroup *group = thePlayer.FindGroup( document );
	if (group) group->ChangeTempo( period, group->metered.time, 0, ClockType_Metered );

//	thePlayer.defaultTempoPeriod = period;
}

// ---------------------------------------------------------------------------
// Get the port name of the Nth port.

char *CPlayerControl::PortName(uint32 inPortIndex )
{
	//maybe we should be returning the name of the producer on this index.
	if (inPortIndex < 0 || inPortIndex >= Max_MidiPorts) return NULL;
	return thePlayer.m_portInfo[ inPortIndex ].portName;
}
	
// ---------------------------------------------------------------------------
// Set the port name of the Nth port.

void CPlayerControl::SetPortName(uint32 inPortIndex, char *inPortName )
{
	//maybe we should be setting the name of the producer on this index.
	CMIDIPlayer::PortInfo		*pi;
	if (inPortIndex < 0 || inPortIndex >= Max_MidiPorts) return;
	pi = &thePlayer.m_portInfo[ inPortIndex ];
	strncpy( pi->portName, inPortName, sizeof( pi->portName ) );
	pi->portName[ sizeof( pi->portName ) - 1 ] = '\0';
}
	

// Get the port device string of the Nth port.


// ---------------------------------------------------------------------------
// Set the port device string of the Nth port.
//when called with a name, if undefined, we define.  if defined, we rename


	
// ---------------------------------------------------------------------------
// For each task that is currently playing the given track, return
// what time in the track the task is playing at. This is used to
// show the timeline on the track as the track is playing.

int32 CPlayerControl::GetPlaybackMarkerTimes(
	CTrack			*track,
	int32			*resultBuf,
	int32			bufSize )
{
	int32			count = 0;
	CPlaybackTaskGroup *group;
	CPlaybackTask	*th;

		// Lock list of contexts while we are looking through it
 	LOCK_PLAYER;

		// Search each context
	for (	group = (CPlaybackTaskGroup *)thePlayer.m_groupList.First();
			group != NULL && count < bufSize;
			group = (CPlaybackTaskGroup *)group->Next() )
	{
		if (!(group->flags &
			(CPlaybackTaskGroup::Clock_Stopped
			| CPlaybackTaskGroup::Clock_AwaitingSync
			| CPlaybackTaskGroup::Clock_Locating)))
		{
			for (	th = (CPlaybackTask *)group->tasks.First();
					th != NULL;
					th = (CPlaybackTask *)th->Next() )
			{
				if (th->track == track)
				{
					resultBuf[ count++ ] = th->CurrentTime();
					if (count >= bufSize) break;
				}
			}
		}
	}
	
	return count;
}

/*	1. How are the timestamps going to be interpreted from the input device?

	Launch task at moment's notice (example: key mapping)

	Relocate at moment's notice
		MIDI start
		MIDI continue
		Cue buttons during playback.
		Cue buttons at other times.
		MTC discontinuities

	Negative start times:
		Count-in
		MTC with offset*/
