/* ===================================================================== *
 * MidiDestination.cpp (MeV/Midi)
 * ===================================================================== */

#include "MidiDestination.h"

#include "DestinationConfigView.h"
#include "DestinationMonitorView.h"
#include "Event.h"
#include "EventTask.h"
#include "GeneralMidi.h"
#include "InternalSynth.h"
#include "MeVDoc.h"
#include "MidiModule.h"
#include "PlaybackTaskGroup.h"

// Interface Kit
#include <Rect.h>
#include <Bitmap.h>
#include <View.h>
// Midi Kit
#include <MidiConsumer.h>
#include <MidiProducer.h>
// Support Kit
#include <Debug.h>

#define D_ALLOC(x) //PRINT(x)		// Constructor/Destructor
#define D_HOOK(x) //PRINT(x)		// Hook Functions
#define D_ACCESS(x) //PRINT(x)		// Accessors
#define D_OPERATION(x) //PRINT(x)	// Operations
#define D_SERIALIZE(x) //PRINT(x)	// Serialization
#define D_MONITOR(x) //PRINT(x)		// Monitor Management
#define D_INTERNAL(x) //PRINT(x)	// Internal Operations

using namespace Midi;

// ---------------------------------------------------------------------------
// Constants

const char *NOTE_NAMES[] =
{ "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

// ---------------------------------------------------------------------------
// Constructor/Destructor

CMidiDestination::CMidiDestination(
	long id,
	const char *name,
	CMeVDoc *document)
	:	CDestination('MIDI', id, name, document),
		m_producer(NULL),
		m_channel(0),
		m_generalMidi(false),
		m_currentPitch(0xff)
{
	D_ALLOC(("CMidiDestination::CMidiDestination()\n"));

	CWriteLock lock(this);

	BString producerName = "MeV: ";
	producerName << Name();
	m_producer = new BMidiLocalProducer(producerName.String());
	_updateIcons();
	m_producer->Register();
}

CMidiDestination::CMidiDestination(
	CMeVDoc *document)
	:	CDestination('MIDI', document),
		m_producer(NULL),
		m_channel(0),
		m_generalMidi(false),
		m_currentPitch(0xff)
{
	D_ALLOC(("CMidiDestination::CMidiDestination(deserialize)\n"));

	CWriteLock lock(this);

	m_producer = new BMidiLocalProducer("");
	_updateIcons();
	m_producer->Register();
}

CMidiDestination::~CMidiDestination()
{
	D_ALLOC(("CMidiDestination::~CMidiDestination()\n"));

	m_producer->Release();
}

// ---------------------------------------------------------------------------
// Accessors

bool
CMidiDestination::GetControllerName(
	unsigned char controller,
	char *outName) const
{
	if (m_generalMidi)
	{
		const char *name = GeneralMidi::GetControllerNameFor(controller);
		if (name)
		{
			strncpy(outName, name, CONTROLLER_NAME_LENGTH);
			return true;
		}
	}

	return false;
}

bool
CMidiDestination::GetNoteName(
	unsigned char note,
	char *outName) const
{
	if (m_generalMidi && (m_channel == GeneralMidi::DRUM_KIT_CHANNEL))
	{
		strncpy(outName, GeneralMidi::GetDrumSoundNameFor(note),
				NOTE_NAME_LENGTH);
		return true;
	}
	else
	{
		unsigned char octave = note / 12;
		unsigned char key = note % 12;
		snprintf(outName, NOTE_NAME_LENGTH, "%s%d (%d)",
				 NOTE_NAMES[key], octave - 2, note);
		return true;
	}

	return false;
}

bool
CMidiDestination::GetProgramName(
	unsigned short bank,
	unsigned char program,
	char *outName) const
{
	if (m_generalMidi)
	{
		if (m_channel == GeneralMidi::DRUM_KIT_CHANNEL)
			snprintf(outName, PROGRAM_NAME_LENGTH, "Drum Kit");
		else
			strncpy(outName, GeneralMidi::GetProgramNameFor(program),
					PROGRAM_NAME_LENGTH);
		return true;
	}

	return false;
}

BMidiConsumer *
CMidiDestination::ConnectedTo() const
{
	D_ACCESS(("CMidiDestination::ConnectedTo()\n"));

	return (BMidiConsumer *)m_producer->Connections()->ItemAt(0);
}

bool
CMidiDestination::IsConnectedTo(
	BMidiConsumer *consumer) const
{
	D_ACCESS(("CMidiDestination::IsConnectedTo()\n"));
	ASSERT(consumer != NULL);

	if ((m_consumerName == consumer->Name())
	 && (m_producer->IsConnected(consumer)))
		return true;

	return false;
}

void
CMidiDestination::SetChannel(
	uint8 channel)
{
	if (channel != m_channel)
	{	
		m_channel = channel;
		Document()->SetModified();

		CUpdateHint hint;
		hint.AddInt32("DestID", ID());
		hint.AddInt32("DestAttrs", Update_Channel);
		PostUpdate(&hint);
	}
}

// ---------------------------------------------------------------------------
// Operations

void
CMidiDestination::ConnectTo(
	BMidiConsumer *consumer)
{
	D_OPERATION(("CMidiDestination::ConnectTo(consumer)\n"));
	ASSERT(consumer != NULL);

	Disconnect();

	BMessage props;
	if (consumer->GetProperties(&props) == B_OK)
	{
		if (props.HasBool("mev:internal_synth"))
			// init internal synth
			((Midi::CInternalSynth *)consumer)->Init();
		if (props.HasBool("mev:general_midi"))
			m_generalMidi = true;
	}

	m_consumerName = consumer->Name();
	status_t error = m_producer->Connect(consumer);
	if (error)
		D_OPERATION((" -> error connecting to %s: %s\n",
					 consumer->Name(), strerror(error)));
	SetLatency(consumer->Latency());
}

void
CMidiDestination::ConnectTo(
	int32 id)
{
	D_OPERATION(("CMidiDestination::ConnectTo(id)\n"));

	BMidiConsumer *consumer = CMidiModule::Instance()->FindConsumer(id);
	if (consumer != NULL)
		ConnectTo(consumer);
}

void
CMidiDestination::ConnectTo(
	const char *name)
{
	D_OPERATION(("CMidiDestination::ConnectTo(name)\n"));
	ASSERT(name != NULL);

	Disconnect();

	m_consumerName = name;
	BMidiConsumer *consumer = CMidiModule::Instance()->FindConsumer(name);
	if (consumer != NULL)
		ConnectTo(consumer);
}

void
CMidiDestination::Disconnect()
{
	D_OPERATION(("CMidiDestination::Disconnect()\n"));

	CMidiModule *mm = CMidiModule::Instance();
	BMidiConsumer *consumer = mm->FindConsumer(m_consumerName);
	if ((consumer != NULL) && m_producer->IsConnected(consumer))
	{
		m_producer->Disconnect(consumer);
		m_generalMidi = false;
		SetLatency(0LL);
	}
}

// ---------------------------------------------------------------------------
// CDestination Implementation

void
CMidiDestination::DoneLocating(
	bigtime_t when)
{
	D_HOOK(("CMidiDestination::DoneLocating(%Ld)\n",
			when));

	// we use this hook to flush the various cached states
	map<event_type, CEvent>::iterator i;
	for (i = m_state.begin(); i != m_state.end(); ++i)
		Execute(i->second, when);
	m_state.clear();
}

void
CMidiDestination::Execute(
	CEvent &event,
	bigtime_t time)
{
	D_HOOK(("CMidiDestination::Execute(%s, %Ld)\n",
			event.NameText(), time));

	// Attempt to send the event.
	switch (event.Command())
	{
		case EvtType_Note:
		{
			m_producer->SprayNoteOn(m_channel, event.note.pitch,
									event.note.attackVelocity, time);
			if (!m_monitors.empty())
			{
				BMessage message(CDestinationMonitorView::NOTE_ON);
				message.AddInt8("mev:note", event.note.pitch);
				message.AddInt8("mev:velocity", event.note.attackVelocity);
				list<BMessenger>::iterator i;
				for (i = m_monitors.begin(); i != m_monitors.end(); ++i)
					i->SendMessage(&message);
			}
			break;
		}
		case EvtType_NoteOff:
		{
			m_producer->SprayNoteOff(m_channel, event.note.pitch,
									 event.note.releaseVelocity, time);
			if (!m_monitors.empty())
			{
				BMessage message(CDestinationMonitorView::NOTE_OFF);
				message.AddInt8("mev:note", event.note.pitch);
				message.AddInt8("mev:velocity", event.note.releaseVelocity);
				list<BMessenger>::iterator i;
				for (i = m_monitors.begin(); i != m_monitors.end(); ++i)
					i->SendMessage(&message);
			}
			break;
		}
		case EvtType_ChannelATouch:
		{
			m_producer->SprayChannelPressure(m_channel, event.aTouch.value,
											 time);
			break;
		}
		case EvtType_PolyATouch:
		{
			m_producer->SprayKeyPressure(m_channel, event.aTouch.pitch,
										 event.aTouch.value, time);
			break;
		}
		case EvtType_Controller:
		{
			// Check if it's a 16-bit controller
			uint8 lsbIndex = controllerInfoTable[event.controlChange.controller].LSBNumber;
			if ((lsbIndex == event.controlChange.controller)
			 || (lsbIndex > 127))
			{
				// It's an 8-bit controller.
				m_producer->SprayControlChange(m_channel, event.controlChange.controller,
											   event.controlChange.MSB, time);
				if (!m_monitors.empty())
				{
					BMessage message(CDestinationMonitorView::CONTROL_CHANGE);
					message.AddInt8("mev:control", event.controlChange.controller);
					message.AddInt8("mev:value", event.controlChange.MSB);
					list<BMessenger>::iterator i;
					for (i = m_monitors.begin(); i != m_monitors.end(); ++i)
						i->SendMessage(&message);
				}
			}
			else
			{
				// Handle the MSB first...if the MSB changed, then update LSB state as well
				if (event.controlChange.MSB < 128)
					m_producer->SprayControlChange(m_channel,
												   event.controlChange.controller,
												   event.controlChange.MSB, time);

				// Now deal with the LSB.
				if (event.controlChange.LSB < 128)
					m_producer->SprayControlChange(m_channel, lsbIndex,
												   event.controlChange.LSB, time);
				if (!m_monitors.empty())
				{
					BMessage message(CDestinationMonitorView::CONTROL_CHANGE);
					message.AddInt8("mev:control", event.controlChange.controller);
					message.AddInt16("mev:value", event.controlChange.MSB * 128
												  + event.controlChange.LSB);
					list<BMessenger>::iterator i;
					for (i = m_monitors.begin(); i != m_monitors.end(); ++i)
						i->SendMessage(&message);
				}
			}
			break;
		}
		case EvtType_ProgramChange:
		{
			m_producer->SprayProgramChange(m_channel,
										   event.programChange.program,
										   time);
			if (!m_monitors.empty())
			{
				BMessage message(CDestinationMonitorView::PROGRAM_CHANGE);
				message.AddInt16("mev:bank", event.programChange.bankMSB * 128 +
											 event.programChange.bankLSB);
				message.AddInt8("mev:program", event.programChange.program);
				list<BMessenger>::iterator i;
				for (i = m_monitors.begin(); i != m_monitors.end(); ++i)
					i->SendMessage(&message);
			}
			break;
		}	
		case EvtType_StartInterpolate:
		{	
			if (event.startInterpolate.interpolationType == Interpolation_PitchBend)
			{
				m_currentPitch = event.startInterpolate.startValue;
				m_targetPitch = event.startInterpolate.targetValue;
				m_producer->SprayPitchBend(m_channel,
										   m_currentPitch & 0x7f,
										   m_currentPitch >> 7,
										   time);
				if (!m_monitors.empty())
				{
					BMessage message(CDestinationMonitorView::PITCH_BEND);
					message.AddInt16("mev:pitch", m_currentPitch - 8192);
					list<BMessenger>::iterator i;
					for (i = m_monitors.begin(); i != m_monitors.end(); ++i)
						i->SendMessage(&message);
				}
			}
			break;
		}
		case EvtType_PitchBend:
		{
			m_producer->SprayPitchBend(m_channel,
									   event.pitchBend.targetBend & 0x7f,
									   event.pitchBend.targetBend >> 7,
									   time);
			if (!m_monitors.empty())
			{
				BMessage message(CDestinationMonitorView::PITCH_BEND);
				message.AddInt16("mev:pitch", event.pitchBend.targetBend - 8192);
				list<BMessenger>::iterator i;
				for (i = m_monitors.begin(); i != m_monitors.end(); ++i)
					i->SendMessage(&message);
			}
			break;
		}
		case EvtType_SysEx:
		{
			void *data = event.ExtendedData();
			int32 size = event.ExtendedDataSize();	
			if ((data != NULL) && (size > 0))
				m_producer->SpraySystemExclusive(data, size, time);
			break;
		}
	}
}

status_t
CMidiDestination::GetIcon(
	icon_size which,
	BBitmap *outBitmap)
{
	ASSERT(outBitmap != NULL);

	BMessage props;
	if ((m_producer == NULL) || (m_producer->GetProperties(&props) != B_OK))
		return B_NAME_NOT_FOUND;

	const void *data;
	ssize_t size;
	status_t error;
	switch (which)
	{
		case B_LARGE_ICON:
		{
			error = props.FindData("be:large_icon", 'ICON', &data, &size);
			if (error)
				return error;
			break;
		}
		case B_MINI_ICON:
		{
			error = props.FindData("be:mini_icon", 'MICN', &data, &size);
			if (error)
				return error;
			break;
		}
	}
	memcpy(outBitmap->Bits(), data, size);

	return B_OK;
}

void
CMidiDestination::Interpolate(
	CEvent &event,
	CEventStack &stack,
	long time,
	long elapsed)
{
	D_HOOK(("CMidiDestination::Interpolate(%s, %ld, %ld)\n",
			event.NameText(), time, elapsed));

	// Here's the amount by which the value would have changed in that time.
	int32 delta = ((int32)m_targetPitch - (int32)m_currentPitch)
				  * elapsed / (int32)event.interpolate.duration;

	// If pitch bend has never been set, then center it.
	if (m_currentPitch > 0x3fff)
		m_currentPitch = 0x2000;

	// Calculate the current state
	int32 newValue = (int32)m_currentPitch + (int32)delta;

	// Now, we're going to modify the interpolation event so that it
	// is shorter, and later... and we're going to push that back onto
	// the execution stack so that it gets sent back to us later.
	event.interpolate.start += elapsed;
	event.interpolate.duration -= elapsed;

	// If there's more left to do in this interpolation, then push the event
	// on the stack for the next iteration
	if ((unsigned long)elapsed < event.interpolate.duration)
		stack.Push(event);
	else
		newValue = m_targetPitch;

	// Now, we need to construct a pitch bend event...
	if (newValue != m_currentPitch)
	{
		event.pitchBend.command = EvtType_PitchBend;
		event.pitchBend.targetBend = newValue;
		Execute(event, time);
		m_currentPitch = newValue;
	}
}

CConsoleView *
CMidiDestination::MakeConfigurationView(
	BRect rect)
{
	return new CDestinationConfigView(rect, this);
}

CConsoleView *
CMidiDestination::MakeMonitorView(
	BRect rect)
{
	return new CDestinationMonitorView(rect, this);
}

void
CMidiDestination::ReadChunk(
	CIFFReader &reader)
{
	D_SERIALIZE(("CMidiDestination::ReadChunk()\n"));
	ASSERT(IsWriteLocked());

	switch (reader.ChunkID())
	{
		case CONNECTION_NAME_CHUNK:
		{
			D_SERIALIZE((" -> CONNECTION_NAME_CHUNK\n"));
			char buffer[Midi::CONNECTION_NAME_LENGTH];
			reader.MustRead(buffer, MIN((size_t)reader.ChunkLength(),
										Midi::CONNECTION_NAME_LENGTH));
			D_SERIALIZE((" -> buffer = %s\n", buffer));
			if (strlen(buffer) > 0)
				ConnectTo(buffer);
			break;
		}
		case DESTINATION_SETTINGS_CHUNK:
		{
			D_SERIALIZE((" -> DESTINATION_SETTINGS_CHUNK\n"));
			reader >> m_channel;
			D_SERIALIZE((" -> channel: %d\n", m_channel + 1));
			break;
		}
		default:
		{
			D_SERIALIZE((" -> pass on to CDestination\n"));
			CDestination::ReadChunk(reader);
		}
	}
}

void
CMidiDestination::Serialize(
	CIFFWriter &writer)
{
	ASSERT(IsReadLocked());

	CDestination::Serialize(writer);

	BMidiConsumer *consumer = (BMidiConsumer *)m_producer->Connections()->ItemAt(0);
	if (consumer)
	{
		const char *consumerName = consumer->Name();
		writer.WriteChunk(Midi::CONNECTION_NAME_CHUNK, consumerName,
						  MIN(strlen(consumerName) + 1,
						  	  Midi::CONNECTION_NAME_LENGTH));
	}

	writer.Push(Midi::DESTINATION_SETTINGS_CHUNK);
	writer << m_channel;
	writer.Pop();
}

void
CMidiDestination::Stack(
	CEvent &event,
	const CEventTask &task,
	CEventStack &stack,
	long duration)
{
	D_HOOK(("CMidiDestination::Stack(%s, %ld)\n",
			event.NameText(), duration));

	if (IsMuted())
		return;

	CDestination::Stack(event, task, stack, duration);

	switch (event.Command())
	{
		case EvtType_Note:
		{
			// Ignore the note event if locating
			if (task.IsLocating())
				break;
			
			// REM: Here we would do the VU meter code...
			
			// +++ move this to CEventTrack::FilterEvent ?
			event.note.pitch += task.Transposition();

			// If pitch went out of bounds, then don't play the note.
			// +++++ CLIPPING WOULD BE MUCH NICER!
			if (event.note.pitch & 0x80)
				break;

			// If there was room on the stack to push the note-off, then
			// play the note-on
			event.stack.start += duration;
			event.stack.command = EvtType_NoteOff;
			if (stack.Push(event))
			{
				event.stack.start -= duration;
				event.stack.command	= EvtType_Note;
				stack.Push(event);
			}
			break;
		}
		case EvtType_PitchBend:
		{
			// If locating, update channel state table but don't stack the event
			if (task.IsLocating())
			{
				m_state[EvtType_PitchBend] = event;
				break;
			}

			if ((duration > 0) && (event.pitchBend.updatePeriod > 0))
			{
				// Make a copy of the event
				CEvent copy(event);

				// Push a "start interpolating" event
				copy.startInterpolate.command = EvtType_StartInterpolate;
				copy.startInterpolate.interpolationType = Interpolation_PitchBend;
				copy.startInterpolate.startValue = event.pitchBend.startBend;
				copy.startInterpolate.targetValue = event.pitchBend.targetBend;
				stack.Push(copy);

				// Push an "interpolation" event
				copy.interpolate.command = EvtType_Interpolate;
				copy.interpolate.interpolationType = Interpolation_PitchBend;
				copy.interpolate.duration = duration;
				copy.interpolate.timeStep = event.pitchBend.updatePeriod;
				copy.interpolate.start += event.pitchBend.updatePeriod;
				stack.Push(copy);
			}
			else
			{
				// Just push an ordinary pitch bend event...
				stack.Push(event);
			}
			break;
		}
		case EvtType_ProgramChange:
		{
			// If locating, update channel state table but don't stack the event
			if (task.IsLocating())
				m_state[EvtType_ProgramChange] = event;
			else
				stack.Push(event);
			break;
		}
		case EvtType_ChannelATouch:
		{
			// If locating, update channel state table but don't stack the event
			if (task.IsLocating())
				m_state[EvtType_ChannelATouch] = event;
			else
				stack.Push(event);
			break;
		}
		case EvtType_Controller:
		{
			// REM: Data entry controls should probably be passed through, since they
			// can't be summarized in a simple way.
			// (Actually, the ideal would be to aggregate data entry -- but that's a job for
			// another time and another event type.)

			// If locating, update channel state table but don't stack the event
			if (task.IsLocating())
				m_state[EvtType_Controller] = event;
			else
				stack.Push(event);
			break;
		}
		case EvtType_PolyATouch:
		{
			// Ignore the event if locating
			if (task.IsLocating())
				break;
			stack.Push(event);
			break;
		}
		case EvtType_SysEx:
		{
			stack.Push(event);
			break;
		}
	}
}

void
CMidiDestination::ColorChanged(
	rgb_color color)
{
	_updateIcons();
}

void
CMidiDestination::Deleted()
{
	// +++ disconnect
}

void
CMidiDestination::Disabled(
	bool disabled)
{
	_updateIcons();
}

void
CMidiDestination::Muted(
	bool muted)
{
	_updateIcons();
}

void
CMidiDestination::NameChanged(
	const char *name)
{
	BString producerName = "MeV: ";
	producerName << name;
	m_producer->SetName(producerName.String());
}

void
CMidiDestination::Soloed(
	bool solo)
{
	_updateIcons();
}

void
CMidiDestination::Undeleted()
{
	// +++ reconnect
}

// ---------------------------------------------------------------------------
// Monitor Management

void
CMidiDestination::MonitorOpened(
	const BMessenger &messenger)
{
	m_monitors.push_back(messenger);
}

void
CMidiDestination::MonitorClosed(
	const BMessenger &messenger)
{
	list<BMessenger>::iterator i;
	for (i = m_monitors.begin(); i != m_monitors.end(); ++i)
	{
		if (*i == messenger)
		{
			m_monitors.erase(i);
			return;
		}
	}
}

// ---------------------------------------------------------------------------
// Internal Operations

void
CMidiDestination::_addIcons(
	BMessage *message,
	BBitmap *largeIcon,
	BBitmap *miniIcon) const
{
	if (!message->HasData("be:large_icon", 'ICON'))
		message->AddData("be:large_icon", 'ICON', largeIcon->Bits(),
						 largeIcon->BitsLength());
	else
		message->ReplaceData("be:large_icon", 'ICON', largeIcon->Bits(),
							 largeIcon->BitsLength());
	if (!message->HasData("be:mini_icon", 'MICN'))
		message->AddData("be:mini_icon", 'MICN', miniIcon->Bits(),
						 miniIcon->BitsLength());
	else
		message->ReplaceData("be:mini_icon", 'MICN', miniIcon->Bits(),
							 miniIcon->BitsLength());
}

BBitmap *
CMidiDestination::_createIcon(
	BRect r)
{
	BBitmap	*icon = new BBitmap(r, B_CMAP8,true);
	BView *iconView = new BView(r, "", B_FOLLOW_RIGHT, B_FRAME_EVENTS);
	icon->Lock();
	icon->AddChild(iconView);
	iconView->SetHighColor(B_TRANSPARENT_COLOR);
	iconView->FillRect(r, B_SOLID_HIGH);
	iconView->SetHighColor(Color());
	if (IsDisabled() || IsMuted())
	{
		rgb_color contrast;
		contrast.red = (Color().red + 128) % 255;
		contrast.green = (Color().green + 128) % 255;
		contrast.blue = (Color().blue + 128) % 255;
		iconView->SetLowColor(contrast);
		pattern mixedColors = { { 0xf0, 0xf0, 0xf0, 0xf0,
								  0x0f, 0x0f, 0x0f, 0x0f } };
		iconView->FillEllipse(r, mixedColors);
	}
	else
	{
		iconView->FillEllipse(r);
	}
	iconView->Sync();
	iconView->RemoveSelf();
	delete iconView;
	icon->Unlock();
	return icon;
}

void
CMidiDestination::_updateIcons()
{
	BRect sr, lr;
	sr.Set(0.0, 0.0, 15.0, 15.0);
	lr.Set(0.0, 0.0, 31.0, 31.0);
	BMessage msg;
	m_producer->GetProperties(&msg);
	BBitmap *lg = _createIcon(lr);
	BBitmap *sm = _createIcon(sr);
	_addIcons(&msg, lg, sm);
	delete lg;
	delete sm;
	m_producer->SetProperties(&msg);
}

// END - MidiDestination.cpp
