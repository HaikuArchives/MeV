/* ===================================================================== *
 * Player.cpp (MeV/Engine)
 * ===================================================================== */

#include "MeV.h"
#include "MeVApp.h"

#include "TimeUnits.h"
#include "Track.h"
#include "EventStack.h"
#include "PlaybackThread.h"
#include "PlaybackThreadTeam.h"
#include "Player.h"
#include "Idents.h"
#include "MidiDeviceInfo.h"

// Gnu C Library
#include <math.h>
// Midi Kit
#include <MidiPort.h>
#include <MidiSynth.h>

const int32			maxSleep = 30;

CMIDIPlayer			thePlayer;

// ---------------------------------------------------------------------------
// The main player loop

void CMIDIPlayer::Run()
{
		// Initialize time vars
	internalTimerTick = system_time() / 1000;			// the current time value
	nextEventTime = internalTimerTick;
	
	while (KeepRunning()) {
	
		int32		cmdSize,
					cmdCode;
		CommandArgs	cmdArgs;
		bigtime_t	wakeUp = (bigtime_t)nextEventTime * 1000 - system_time();
		
		if (wakeUp < 0) wakeUp = 0;
		
		// Calculate the REAL time of the next event.
		// We need to always calculate the REAL time of the next METERED event.
		// But -- never wait longer than, say, 20 ms.
		
		cmdSize = read_port_etc(	cmdPort,
								&cmdCode,
								&cmdArgs,
								sizeof cmdArgs,
								B_TIMEOUT,
								wakeUp );

		if (cmdSize >= 4)
		{
			switch( cmdCode ) {
			case Command_Start:
				if (cmdArgs.document == NULL) break;
				CRefCountObject::Release( songTeam->doc );
				songTeam->doc = cmdArgs.document;
				songTeam->vChannelTable = &cmdArgs.document->GetVChannel( 0 );
				
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
						songTeam->Start(	track1,
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
						songTeam->Start(	track,
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
				if (cmdArgs.document == NULL) break;
				CRefCountObject::Release( cmdArgs.document );
				songTeam->flags |= CPlaybackThreadTeam::Clock_Stopped;
				songTeam->FlushNotes();
				// REM: Notify UI of stoppage...
				break;

			case Command_Pause:
			case Command_Continue:
				break;
				
			case Command_Quit:
				break;
			}
			
			if (cmdCode != Command_Attention) {

				BMessage		msg( Player_ChangeTransportState );

				be_app->PostMessage( &msg );
			}
		}
		
			// And then process any waiting events.
		internalTimerTick = system_time() / 1000;			// the current time value

		{
			LOCK_PLAYER;
			
			CPlaybackThreadTeam *team;
			
			nextEventTime = internalTimerTick + maxSleep;

			for (	team = (CPlaybackThreadTeam *)teamList.First();
					team;
					team = (CPlaybackThreadTeam *)team->Next() )
			{
				int32		nextTeamEvent;
			
				team->nextEventTime = LONG_MAX;
				team->Update( internalTimerTick );

					// Compute next event time as min of all track times
				nextTeamEvent = team->nextEventTime + team->origin;
				if (IsTimeGreater( nextTeamEvent, nextEventTime ))
					nextEventTime = nextTeamEvent;
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
		ports[ i ] = NULL;
		strcpy( portInfo[ i ].portName, "<no device>" );
		strcpy( portInfo[ i ].devString, "" );
	}
	
	InitChannelStates();

	synth = NULL;
	synthUseCount = 0;
	
		// Default tempo is 100 bpm
//	defaultTempoPeriod = RateToPeriod( 100.0 );
	
	songTeam = NULL;
	wildTeam = NULL;
}

// ---------------------------------------------------------------------------
// clean up the player task

CMIDIPlayer::~CMIDIPlayer()
{
	Stop();

	CPlaybackThreadTeam	*team;

		// Delete song contexts
	while ((team = (CPlaybackThreadTeam *)teamList.First())) delete team;
	songTeam = wildTeam = NULL;

		// Delete ports
		// REM: Commented out because code crashes...!
	for (uint32 i = 0; i < Max_MidiPorts; i++)
	{
		delete ports[ i ];
	}
	delete synth;
}

void CMIDIPlayer::Initialize()
{
	cmdPort = create_port( 1024, "MeVPlayer Command Port" );

		// Set up a context for playing songs
		// REM: Should pass the document pointer as parameter
	songTeam = new CPlaybackThreadTeam( NULL );

		// Set up a context for miscellaneous note-playing
		// REM: wildContext->setTime( internalTimerTick, 0 );
	wildTeam = new CPlaybackThreadTeam( NULL );

		// Set up wildContext with current time, and start it.
	wildTeam->origin = 0; // system_time() / 1000;
	wildTeam->real.time = 0;
	wildTeam->metered.time = 0;
		// set context to "running always"
	wildTeam->flags = CPlaybackThreadTeam::Clock_Continuous;

	Start();
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
			memset( &portInfo[ i ].channelStates[ j ], 0xff, sizeof (ChannelState) );
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
			memset( &portInfo[ i ].channelStates[ j ], 0xff, sizeof (ChannelState) );
		}
	}
}
#endif

// ---------------------------------------------------------------------------
// Allocate a new BMidiSynth and a BSynth if needed.

BMidiSynth *CMIDIPlayer::NewMidiSynth()
{
	BMidiSynth		*s;
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
}

// ---------------------------------------------------------------------------
// Free the BMidiSynth and the BSynth if needed

void CMIDIPlayer::DeleteMidiSynth( BMidiSynth *inSynth )
{
	delete inSynth;
	synthUseCount--;
	if (synthUseCount <= 0 && synth != NULL)
	{
		delete synth;
		synth = NULL;
	}
}

// ---------------------------------------------------------------------------
// queueEvents -- push some events on the the linear time stack, and then
// check to see if it's time to wake up.

bool CMIDIPlayer::QueueEvents( Event *eventList, uint32 count, long startTime )
{
		// Note: Locking not needed since stack has it's own lock.

	if (wildTeam->real.stack.PushList( eventList, count, startTime ))
	{
		return true;
	}
	else return false;
}

// ---------------------------------------------------------------------------
// Find the team associated with a particular document

CPlaybackThreadTeam *CMIDIPlayer::FindTeam( CMeVDoc *doc )
{
	if (doc != NULL && songTeam->doc == doc) return songTeam;
	return NULL;
}

// ---------------------------------------------------------------------------
// Send midi events to the appropriate output port

void CMIDIPlayer::SendEvent(
	const Event		&ev,
	uint8			inPort,
	uint8			inActualChannel,
	uint32			inTime )
{
	BMidi			*port = ports[ inPort ];
	ChannelState		*chState = &portInfo[ inPort ].channelStates[ inActualChannel ];
	MIDIDeviceInfo	*mdi;
	uint8			lsbIndex;

	if (port == NULL) return;

		// Attempt to send the event.

	switch (ev.Command()) {

		// MIDI channel events:

	case EvtType_Note:							// note-on event
		port->NoteOn(	inActualChannel,
						ev.note.pitch,
						ev.note.attackVelocity,
						inTime );
		break;

	case EvtType_NoteOff:						// note-off event 
		port->NoteOff(	inActualChannel,
						ev.note.pitch,
						ev.note.releaseVelocity,
						inTime );
		break;

	case EvtType_ChannelATouch:					// channel aftertouch
		if (chState->channelAfterTouch != ev.aTouch.value)
		{
			port->ChannelPressure(
								inActualChannel,
								ev.aTouch.value,
								inTime );
			chState->channelAfterTouch = ev.aTouch.value;
		}
		break;

	case EvtType_PolyATouch:						// polyphonic aftertouch
		port->KeyPressure(	inActualChannel,
							ev.aTouch.pitch,
							ev.aTouch.value,
							inTime );
		break;

	case EvtType_Controller:						// controller change

			// Check if it's a 16-bit controller
		lsbIndex = controllerInfoTable[ ev.controlChange.controller ].LSBNumber;

		if (lsbIndex == ev.controlChange.controller || lsbIndex > 127)
		{
				// It's an 8-bit controller.
			if (	ev.controlChange.MSB != chState->ctlStates[ ev.controlChange.controller ])
			{
				port->ControlChange(	inActualChannel,
									ev.controlChange.controller,
									ev.controlChange.MSB,
									inTime );

				chState->ctlStates[ ev.controlChange.controller ] = ev.controlChange.MSB;
			}
		}
		else
		{
				// Handle the MSB first...if the MSB changed, then update LSB state as well
			if (	ev.controlChange.MSB != chState->ctlStates[ ev.controlChange.controller ]
				&& ev.controlChange.MSB < 128)
			{
				port->ControlChange(	inActualChannel,
									ev.controlChange.controller,
									ev.controlChange.MSB,
									inTime );

				chState->ctlStates[ ev.controlChange.controller ] = ev.controlChange.MSB;
				chState->ctlStates[ lsbIndex ] = 0;
			}

				// Now deal with the LSB.
			if (	ev.controlChange.LSB != chState->ctlStates[ lsbIndex ]
				&& ev.controlChange.LSB < 128)
			{
				port->ControlChange(	inActualChannel,
									lsbIndex,
									ev.controlChange.LSB,
									inTime );

				chState->ctlStates[ lsbIndex] = ev.controlChange.LSB;
			}
		}
		break;

	case EvtType_ProgramChange:					// program change

			// Return the MIDI device associated with this port and channel
		mdi = ((CMeVApp *)be_app)->LookupInstrument( inPort, inActualChannel );

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
				port->ControlChange(	inActualChannel,
									0x00,
									ev.programChange.bankMSB,
									inTime );

					// Send a bank change LSB message
				port->ControlChange(	inActualChannel,
									0x20,
									ev.programChange.bankLSB,
									inTime );
	
					// Update the channel state
				chState->ctlStates[ 0 ] = ev.programChange.bankMSB;
				chState->ctlStates[ 32 ] = ev.programChange.bankLSB;
			}
		}
		
			// Always send a program change, regardless...
		port->ProgramChange(	inActualChannel,
							ev.programChange.program,
							inTime );

		chState->program = ev.programChange.program;
		break;

	case EvtType_PitchBend:						// pitch bend

			// Don't send un-needed pitch-bends
		if (chState->pitchBendState != ev.pitchBend.targetBend)
		{
			port->PitchBend(		inActualChannel,
								ev.pitchBend.targetBend & 0x7f,
								ev.pitchBend.targetBend >> 7,
								inTime );
			chState->pitchBendState = ev.pitchBend.targetBend;
		}
		break;

		// MIDI system events

	case EvtType_SysEx:							// system exclusive

		void				*data;
		int32			size;
		
		data = ev.ExtendedData();
		size = ev.ExtendedDataSize();	
		
		if (data != NULL && size > 0 )
		{
			port->SystemExclusive( data, size, inTime );
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

		VChannelEntry	&vc = doc->GetVChannel( channel );

		modEvent->stack.actualPort = vc.port;	
		modEvent->stack.actualChannel = vc.channel;
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
	modEvent->stack.thread = cFeedbackThread;
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
	thePlayer.wildTeam->FlushEvents();

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
	
	write_port(	thePlayer.cmdPort,
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
	
	write_port( thePlayer.cmdPort, Command_Stop, &cmd, sizeof cmd );
}

// ---------------------------------------------------------------------------
// Determine if this document or any tracks in it are playing

bool CPlayerControl::IsPlaying( CMeVDoc *document )
{
	LOCK_PLAYER;
	CPlaybackThreadTeam *team = thePlayer.FindTeam( document );
	if (team && (team->flags & CPlaybackThreadTeam::Clock_Stopped) == false)
		return true;
	return false;
}

// ---------------------------------------------------------------------------
// If this document is playing (or paused), return the current playback state

bool CPlayerControl::GetPlaybackState( CMeVDoc *document, PlaybackState &outState )
{
	LOCK_PLAYER;
	CPlaybackThreadTeam *team = thePlayer.FindTeam( document );
	if (team)
	{
		outState.running		= !(team->flags & CPlaybackThreadTeam::Clock_Stopped);
		outState.realTime		= team->real.time;
		outState.meteredTime	= team->metered.time;
		return true;
	}
	return false;
}

// ---------------------------------------------------------------------------
// Determine the pause state for this document.

bool CPlayerControl::PauseState( CMeVDoc *document )
{
	LOCK_PLAYER;
	CPlaybackThreadTeam *team = thePlayer.FindTeam( document );
	if (team && (team->flags & CPlaybackThreadTeam::Clock_Paused) != false)
		return true;
	return false;
}

// ---------------------------------------------------------------------------
// Set the state of the pause flag for this document, if it's playing.

void CPlayerControl::SetPauseState( CMeVDoc *document, bool inPauseState )
{
	LOCK_PLAYER;
	CPlaybackThreadTeam *team = thePlayer.FindTeam( document );
	if (team)
	{
			// REM: If externally sync'd, then no effect.
		if (inPauseState)	team->flags |= CPlaybackThreadTeam::Clock_Paused;
		else					team->flags &= ~CPlaybackThreadTeam::Clock_Paused;
	}
}

// ---------------------------------------------------------------------------
// Toggle the state of the pause flag for this document, if it's playing.

void CPlayerControl::TogglePauseState( CMeVDoc *document )
{
	LOCK_PLAYER;
	CPlaybackThreadTeam *team = thePlayer.FindTeam( document );
	if (team)
	{
			// REM: If externally sync'd, then no effect.
		team->flags ^= CPlaybackThreadTeam::Clock_Paused;
	}
}

// ---------------------------------------------------------------------------
// Return the current tempo.

double CPlayerControl::Tempo( CMeVDoc *document )
{
	LOCK_PLAYER;
	CPlaybackThreadTeam *team = thePlayer.FindTeam( document );
	
	if (team) return PeriodToRate( team->CurrentTempoPeriod() );
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
	CPlaybackThreadTeam *team = thePlayer.FindTeam( document );
	if (team) team->ChangeTempo( period, team->metered.time, 0, ClockType_Metered );

//	thePlayer.defaultTempoPeriod = period;
}

// ---------------------------------------------------------------------------
// Get the port name of the Nth port.

char *CPlayerControl::PortName(uint32 inPortIndex )
{
	if (inPortIndex < 0 || inPortIndex >= Max_MidiPorts) return NULL;
	return thePlayer.portInfo[ inPortIndex ].portName;
}
	
// ---------------------------------------------------------------------------
// Set the port name of the Nth port.

void CPlayerControl::SetPortName(uint32 inPortIndex, char *inPortName )
{
	CMIDIPlayer::PortInfo		*pi;

	if (inPortIndex < 0 || inPortIndex >= Max_MidiPorts) return;
	pi = &thePlayer.portInfo[ inPortIndex ];
	strncpy( pi->portName, inPortName, sizeof( pi->portName ) );
	pi->portName[ sizeof( pi->portName ) - 1 ] = '\0';
}
	
// ---------------------------------------------------------------------------
// Get the port device string of the Nth port.

char *CPlayerControl::PortDevice(uint32 inPortIndex )
{
	if (inPortIndex < 0 || inPortIndex >= Max_MidiPorts) return NULL;
	return thePlayer.portInfo[ inPortIndex ].devString;
}
	
// ---------------------------------------------------------------------------
// Set the port device string of the Nth port.

bool CPlayerControl::SetPortDevice(uint32 inPortIndex, char *inDeviceString )
{
	CMIDIPlayer::PortInfo		*pi;
	BMidi					*oldPort,
							*newPort;

	if (inPortIndex < 0 || inPortIndex >= Max_MidiPorts) return false;

	oldPort = thePlayer.ports[ inPortIndex ];

	if (	inDeviceString == NULL
		|| strlen( inDeviceString ) == 0
		|| strcmp( inDeviceString, "none" ) == 0)
	{
			// Deleting a port...
		delete oldPort;
		thePlayer.ports[ inPortIndex ] = NULL;
		pi = &thePlayer.portInfo[ inPortIndex ];
		pi->devString[ 0 ] = '\0';
		return true;
	}
	else if (strcmp( inDeviceString, "synth" ) == 0)
	{
		if (dynamic_cast<BMidiSynth *>(oldPort))
		{
			return true;			// Nothing to do
		}
		
		newPort = thePlayer.NewMidiSynth();
	}
	else
	{
		BMidiPort	*p = new BMidiPort;
	
		if (p->Open( inDeviceString ) != B_NO_ERROR)
		{
			delete p;
			newPort = NULL;
		}
		else
		{
			newPort = p;
		}
	}
	
	if (newPort != NULL)
	{
		delete oldPort;
		thePlayer.ports[ inPortIndex ] = newPort;

		pi = &thePlayer.portInfo[ inPortIndex ];
		strncpy( pi->devString, inDeviceString, sizeof( pi->devString ) );
		pi->devString[ sizeof( pi->devString ) - 1 ] = '\0';
		return true;
	}
	return false;
}
	
// ---------------------------------------------------------------------------
// For each thread that is currently playing the given track, return
// what time in the track the thread is playing at. This is used to
// show the timeline on the track as the track is playing.

int32 CPlayerControl::GetPlaybackMarkerTimes(
	CTrack			*track,
	int32			*resultBuf,
	int32			bufSize )
{
	int32			count = 0;
	CPlaybackThreadTeam *team;
	CPlaybackThread	*th;

		// Lock list of contexts while we are looking through it
 	LOCK_PLAYER;

		// Search each context
	for (	team = (CPlaybackThreadTeam *)thePlayer.teamList.First();
			team != NULL && count < bufSize;
			team = (CPlaybackThreadTeam *)team->Next() )
	{
		if (!(team->flags &
			(CPlaybackThreadTeam::Clock_Stopped
			| CPlaybackThreadTeam::Clock_AwaitingSync
			| CPlaybackThreadTeam::Clock_Locating)))
		{
			for (	th = (CPlaybackThread *)team->threads.First();
					th != NULL;
					th = (CPlaybackThread *)th->Next() )
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

	Launch thread at moment's notice (example: key mapping)

	Relocate at moment's notice
		MIDI start
		MIDI continue
		Cue buttons during playback.
		Cue buttons at other times.
		MTC discontinuities

	Negative start times:
		Count-in
		MTC with offset*/
