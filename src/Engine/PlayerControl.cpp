/* ===================================================================== *
 * PlayerControl.cpp (MeV/Engine)
 * ===================================================================== */

#include "PlayerControl.h"

#include "MeVDoc.h"
#include "MidiDestination.h"
#include "PlaybackTask.h"
#include "Player.h"

// ---------------------------------------------------------------------------
// Operations

void
CPlayerControl::DoAudioFeedback(
	CMeVDoc *doc,
	enum E_EventAttribute feedbackAttribute,
	uint8 attributeValue,
	const Event *demoEvent)
{
	using namespace Midi;

	Event fbEvents[3];
	Event *modEvent = &fbEvents[0];
	Event *noteStart = &fbEvents[1];
	Event *noteStop = &fbEvents[2];

	if (doc && demoEvent)
	{
		int32 channel = (feedbackAttribute == EvAttr_Channel)
						? attributeValue
						: demoEvent->GetVChannel();
		CMidiDestination *dest = (CMidiDestination *)doc->FindDestination(channel);
		modEvent->stack.actualPort = dest->Producer();	
		modEvent->stack.actualChannel = dest->Channel();
	}
	else
	{
		int32 channel = (feedbackAttribute == EvAttr_Channel)
						? attributeValue
						: 0;
		modEvent->stack.actualPort = NULL;
		modEvent->stack.actualChannel = channel;
	}

	// Set up a default list of events which plays an average note.
	modEvent->stack.start = 0;
	modEvent->stack.task = MAX_FEEDBACK_TASKS;
	modEvent->note.command = 0;

	*noteStart = *modEvent;
	*noteStop = *modEvent;

	noteStart->stack.start = gPrefs.feedbackDelay;
	noteStop->stack.start = gPrefs.feedbackDelay + 450;
	noteStart->note.command = EvtType_Note;
	noteStop->note.command = EvtType_NoteOff;
	if (demoEvent && (demoEvent->Command() == EvtType_Note))
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

	switch (feedbackAttribute)
	{
		case EvAttr_Channel:
		{
		 	noteStart->stack.actualChannel = noteStop->stack.actualChannel = attributeValue;
			break;
		}
		case EvAttr_Pitch:
		{
			noteStart->note.pitch = noteStop->note.pitch = attributeValue;
			break;
		}	
		case EvAttr_AttackVelocity:
		{
			noteStart->note.attackVelocity = attributeValue;
			break;
		}
		case EvAttr_ReleaseVelocity:
		{
			noteStart->note.releaseVelocity = attributeValue;
			break;
		}
		case EvAttr_Program:
		{
			modEvent->programChange.command = EvtType_ProgramChange;
			modEvent->programChange.program = attributeValue;
			modEvent->programChange.bankMSB = MIDIValueUnset;
			break;
		}
		default:
		{
			return;
		}
	}

	StPlayerLock lock;

	// Kill the previous feedback event
	thePlayer.wildGroup->FlushEvents();

	// If it's a note event, play the note.
	// If the doc is already playing, play only the feedback event, no note,
	// else play the feedback event and a note to hear it.
	if (modEvent->note.command == 0)
		thePlayer.QueueImmediate(&fbEvents[1], 2);
	else if (IsPlaying(doc))
		thePlayer.QueueImmediate(&fbEvents[0], 1);
	else
		thePlayer.QueueImmediate(&fbEvents[0], 3);
}

void
CPlayerControl::InitPlayer()
{
	thePlayer.Initialize();
}

void
CPlayerControl::PlaySong(
	CMeVDoc *document,
	int32 trackID,
	long locTime,
	enum ELocateTarget locateTarget,
	int32 duration,
	enum ESyncType syncType,
	int16 options)
{
	CommandArgs args;
	args.document = document;
	args.trackID = trackID;
	args.locTime = locTime;
	args.locateTarget = locateTarget;
	args.duration = duration;
	args.syncType = syncType;
	args.options = options;
	write_port(thePlayer.Port(), Command_Start, &args, sizeof(args));
}

void
CPlayerControl::StopSong(
	CMeVDoc *document)
{
	CommandArgs args;
	args.document = document;
	write_port(thePlayer.m_port, Command_Stop, &args, sizeof(args));
}

bool
CPlayerControl::IsPlaying(
	CMeVDoc *document)
{
	StPlayerLock lock;

	CPlaybackTaskGroup *group = thePlayer.FindGroup(document);
	if (group
	 && ((group->flags & CPlaybackTaskGroup::Clock_Stopped) == false))
		return true;

	return false;
}

bool
CPlayerControl::GetPlaybackState(
	CMeVDoc *document,
	PlaybackState &outState)
{
	StPlayerLock lock;

	CPlaybackTaskGroup *group = thePlayer.FindGroup(document);
	if (group)
	{
		outState.running = !(group->flags & CPlaybackTaskGroup::Clock_Stopped);
		outState.realTime = group->real.time;
		outState.meteredTime = group->metered.time;
		return true;
	}

	return false;
}

bool
CPlayerControl::PauseState(
	CMeVDoc *document)
{
	StPlayerLock lock;

	CPlaybackTaskGroup *group = thePlayer.FindGroup(document);
	if (group
	 && ((group->flags & CPlaybackTaskGroup::Clock_Paused) != false))
		return true;

	return false;
}

void
CPlayerControl::SetPauseState(
	CMeVDoc *document,
	bool state)
{
	StPlayerLock lock;

	CPlaybackTaskGroup *group = thePlayer.FindGroup(document);
	if (group)
	{
		// REM: If externally sync'd, then no effect.
		if (state)
			group->flags |= CPlaybackTaskGroup::Clock_Paused;
		else
			group->flags &= ~CPlaybackTaskGroup::Clock_Paused;
	}
}

void
CPlayerControl::TogglePauseState(
	CMeVDoc *document)
{
	StPlayerLock lock;

	CPlaybackTaskGroup *group = thePlayer.FindGroup(document);
	if (group)
	{
		// REM: If externally sync'd, then no effect.
		group->flags ^= CPlaybackTaskGroup::Clock_Paused;
	}
}

double
CPlayerControl::Tempo(
	CMeVDoc *document)
{
	StPlayerLock lock;

	CPlaybackTaskGroup *group = thePlayer.FindGroup(document);

	if (group)
		return PeriodToRate(group->CurrentTempoPeriod());
	if (document)
		return document->InitialTempo();

	return CMeVDoc::DEFAULT_TEMPO;
}
	
void
CPlayerControl::SetTempo(
	CMeVDoc *document,
	double tempo)
{
	long period = RateToPeriod(tempo);

	StPlayerLock lock;

	CPlaybackTaskGroup *group = thePlayer.FindGroup(document);
	if (group)
		group->ChangeTempo(period, group->metered.time, 0, ClockType_Metered);
}

char *
CPlayerControl::PortName(
	uint32 index)
{
	//maybe we should be returning the name of the producer on this index.
	if ((index < 0) || (index >= Max_MidiPorts))
		return NULL;
	return thePlayer.m_portInfo[index].portName;
}
	
void
CPlayerControl::SetPortName(
	uint32 index,
	char *name)
{
	//maybe we should be setting the name of the producer on this index.
	CPlayer::PortInfo *pi;
	if ((index < 0) || (index >= Max_MidiPorts))
		return;
	pi = &thePlayer.m_portInfo[index];
	strncpy(pi->portName, name, sizeof(pi->portName));
	pi->portName[sizeof(pi->portName) - 1] = '\0';
}
	
int32
CPlayerControl::GetPlaybackMarkerTimes(
	CTrack *track,
	int32 *resultBuf,
	int32 bufSize)
{
	int32 count = 0;
	CPlaybackTaskGroup *group;
	CPlaybackTask *th;

	// Lock list of contexts while we are looking through it
	StPlayerLock lock;

	// Search each context
	for (group = (CPlaybackTaskGroup *)thePlayer.m_groupList.First();
		 group != NULL && count < bufSize;
		 group = (CPlaybackTaskGroup *)group->Next())
	{
		if (!(group->flags &
			 (CPlaybackTaskGroup::Clock_Stopped
			| CPlaybackTaskGroup::Clock_AwaitingSync
			| CPlaybackTaskGroup::Clock_Locating)))
		{
			for (th = (CPlaybackTask *)group->tasks.First();
				 th != NULL;
				 th = (CPlaybackTask *)th->Next())
			{
				if (th->track == track)
				{
					resultBuf[count++] = th->CurrentTime();
					if (count >= bufSize)
						break;
				}
			}
		}
	}

	return count;
}

// END - PlayerControl.cpp
