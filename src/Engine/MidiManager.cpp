#include "MidiManager.h"
#include <MidiProducer.h>
#include <Messenger.h>
#include <Debug.h>
CMidiManager* CMidiManager::m_instance=0;
CMidiManager* CMidiManager::Instance()
{
	if (m_instance==0)
	{
		m_instance=new CMidiManager();
	}
	return m_instance;
}
	
CMidiManager::CMidiManager() : BHandler("MidiHandler")
{
	BMidiRoster* m_roster = BMidiRoster::MidiRoster();
	if (! m_roster) {
		PRINT(("Couldn't get MIDI roster\n"));
		be_app->PostMessage(B_QUIT_REQUESTED);
		return;
	}
	
	BMessenger msgr(this);
	m_roster->StartWatching(&msgr);
}
void
CMidiManager::MessageReceived(BMessage *msg)
{
	switch (msg->what)
	{
		case B_MIDI_EVENT:
			_handleMidiEvent(msg);
			break;
		default:
			BHandler::MessageReceived(msg);
			break;
	}
}
void CMidiManager::_handleMidiEvent(BMessage *msg)
{
	SET_DEBUG_ENABLED(true);
	
	int32 op;
	if (msg->FindInt32("be:op", &op) != B_OK) {
		PRINT(("PatchView::HandleMidiEvent: \"op\" field not found\n"));
		return;
	}

	switch (op) {
	case B_MIDI_REGISTERED:
		{
			int32 id;
			if (msg->FindInt32("be:id", &id) != B_OK) {
				PRINT(("PatchView::HandleMidiEvent: \"be:id\" field not found in B_MIDI_REGISTERED event\n"));
				break;
			}
			
			const char* type;
			if (msg->FindString("be:type", &type) != B_OK) {
				PRINT(("PatchView::HandleMidiEvent: \"be:type\" field not found in B_MIDI_REGISTERED event\n"));
				break;
			}
			
			PRINT(("MIDI Roster Event B_MIDI_REGISTERED: id=%ld, type=%s\n", id, type));
			if (! strcmp(type, "producer")) {
				_addProducer(id);
			} else if (! strcmp(type, "consumer")) {
				_addConsumer(id);
			}
		}
		break;
	case B_MIDI_UNREGISTERED:
		{
			int32 id;
			if (msg->FindInt32("be:id", &id) != B_OK) {
				PRINT(("PatchView::HandleMidiEvent: \"be:id\" field not found in B_MIDI_UNREGISTERED\n"));
				break;
			}
			
			const char* type;
			if (msg->FindString("be:type", &type) != B_OK) {
				PRINT(("PatchView::HandleMidiEvent: \"be:type\" field not found in B_MIDI_UNREGISTERED\n"));
				break;
			}
			
			PRINT(("MIDI Roster Event B_MIDI_UNREGISTERED: id=%ld, type=%s\n", id, type));
			if (! strcmp(type, "producer")) {
				_removeProducer(id);
			} else if (! strcmp(type, "consumer")) {
				_removeConsumer(id);
			}
		}
		break;
	case B_MIDI_CHANGED_PROPERTIES:
		{
			int32 id;
			if (msg->FindInt32("be:id", &id) != B_OK) {
				PRINT(("PatchView::HandleMidiEvent: \"be:id\" field not found in B_MIDI_CHANGED_PROPERTIES\n"));
				break;
			}
			
			const char* type;
			if (msg->FindString("be:type", &type) != B_OK) {
				PRINT(("PatchView::HandleMidiEvent: \"be:type\" field not found in B_MIDI_CHANGED_PROPERTIES\n"));
				break;
			}
			
			BMessage props;
			if (msg->FindMessage("be:properties", &props) != B_OK) {
				PRINT(("PatchView::HandleMidiEvent: \"be:properties\" field not found in B_MIDI_CHANGED_PROPERTIES\n"));
				break;
			}
			
			PRINT(("MIDI Roster Event B_MIDI_CHANGED_PROPERTIES: id=%ld, type=%s\n", id, type));
			if (! strcmp(type, "producer")) {
			//	_updateProducerProps(id, &props);
			} else if (! strcmp(type, "consumer")) {
			//	_updateConsumerProps(id, &props);
			}
			
		}
		break;
	case B_MIDI_CHANGED_NAME:
	case B_MIDI_CHANGED_LATENCY:
	case B_MIDI_CONNECTED:
	case B_MIDI_DISCONNECTED:
		//we don't care about these right now.
	break;
	default:
		PRINT(("PatchView::HandleMidiEvent: unknown opcode %ld\n", op));
		break;
	}
}

void CMidiManager::_addConsumer(int32 id)
{
//build new producer, add it to the producer list,
//connect it to the thing
//give it same name.
	BMidiConsumer *theConsumer=m_roster->FindConsumer(id); 
	BMidiLocalProducer *aproducer=new BMidiLocalProducer("h");//theConsumer->Name());
	aproducer->Connect(theConsumer);
	m_midiProducers.AddItem(aproducer);

}
void CMidiManager::_removeConsumer(int32 id)
{
//go though entire list and remove the producer that is connected.
	BMidiConsumer *theConsumer=m_roster->FindConsumer(id); 
	int c=m_midiProducers.CountItems();
	while (c>=0)
	{
		if (((BMidiLocalProducer *)m_midiProducers.ItemAt(c))->IsConnected(theConsumer))
		{
			((BMidiLocalProducer *)m_midiProducers.RemoveItem(c))->Release();
		}
		c--;
	}
}

void CMidiManager::_addProducer(int32 id)
{
}
void CMidiManager::_removeProducer(int32 id)
{
}

		
		