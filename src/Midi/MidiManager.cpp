/* ===================================================================== *
 * MidiManager.cpp (MeV/Midi)
 * ===================================================================== */

#include "MidiManager.h"
#include "PortNameMap.h"
#include "list.h"
#include <MidiProducer.h>
#include <Messenger.h>
#include <Debug.h>
#include <stdio.h>
enum EDestinationModifierControlID {
	NOTIFY='ntfy'
	};
CMidiManager* CMidiManager::m_instance=0;
CMidiManager* CMidiManager::Instance()
{
	if (m_instance==0)
	{	
		m_instance=new CMidiManager();
		m_instance->Run(); 			
	}
	return m_instance;
}

CMidiManager::CMidiManager() : BLooper("MidiHandler"),CObservableSubject()
{
	BMidiRoster* m_roster = BMidiRoster::MidiRoster();
	if (! m_roster) {
		PRINT(("Couldn't get MIDI roster\n"));
		be_app->PostMessage(B_QUIT_REQUESTED);
		return;
	}
	BMessenger msgr(this);
	m_roster->StartWatching(&msgr);
	m_notifier=NULL;
	m_portNameMap=new CPortNameMap();
	m_isynth_source=NULL;
	m_isynth_sink=NULL;
}


BMidiLocalProducer * CMidiManager::GetProducer (BString *name)
{
	int c=m_midiProducers.CountItems()-1;
	while (c>=0)
	{
		BMidiLocalProducer *aproducer=((BMidiLocalProducer *)m_midiProducers.ItemAt(c));
		if ((name->Compare(aproducer->Name()))==0)
		{
			return aproducer;
		}
		c++;
	}
	return NULL; //think about the null producer option.
}
BMidiLocalProducer * CMidiManager::GetProducer (int32 id)
{
	if (m_roster->FindProducer(id,true)==m_isynth_source)
	{
		return (InternalSynth());
	}
	else
	{
		return (BMidiLocalProducer *)(m_roster->FindProducer(id,true));
	}
}
void CMidiManager::FirstProducer()
{
	m_pos=0;
}
void CMidiManager::NextProducer()
{
	m_pos++;
}
bool CMidiManager::IsLastProducer()
{
	if (m_pos>(m_midiProducers.CountItems()-1))
	{
		return true;
	}
	else
	{
		return false;
	}
}
BString * CMidiManager::CurrentProducerName()
{
 BString *portname = new BString(((BMidiProducer *)m_midiProducers.ItemAt(m_pos))->Name());
 return (m_portNameMap->Map(portname));
}
int32 CMidiManager::CurrentProducerID()
{

 		return (((BMidiProducer *)m_midiProducers.ItemAt(m_pos))->ID());

}
void
CMidiManager::MessageReceived(BMessage *msg)
{
	switch (msg->what)
	{
		case B_MIDI_EVENT:
		{
			_handleMidiEvent(msg);
		}
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
BMidiLocalProducer * CMidiManager::InternalSynth()
{
	if (m_isynth_sink!=NULL)
	{
		return (m_isynth_source);
	}
	else
	{
		m_isynth_sink=new CInternalSynth("Internal Synth");
		m_isynth_source->Connect(m_isynth_sink);
		return (m_isynth_source);
	}
}

void CMidiManager::AddInternalSynth()
{
	if (!m_isynth_source)
	{
		m_isynth_source=new BMidiLocalProducer("Internal Synth");
		BMidiLocalConsumer *isynth_monitor=new BMidiLocalConsumer("Internal Synth Monitor"); //this is a temp way to keep the 
																							//i synth from having zero connections
																							//and be deleted
		m_isynth_source->Connect(isynth_monitor);
		m_midiProducers.AddItem(m_isynth_source);
	}
}
void CMidiManager::_addConsumer(int32 id)
{
//build new producer, add it to the producer list,
//connect it to the thing
//give it same name.  map the name?
	BMidiConsumer *theConsumer=m_roster->FindConsumer(id); 
	BMidiLocalProducer *aproducer=new BMidiLocalProducer(theConsumer->Name());//theConsumer->Name());
	aproducer->Connect(theConsumer);
	m_midiProducers.AddItem(aproducer);
	
	
	int32 op;
	op=B_MIDI_REGISTERED;
	CUpdateHint hint;
	hint.AddInt32("midiop",op);
	hint.AddString("name",aproducer->Name());
	CObservableSubject::PostUpdate(&hint,NULL);
}
void CMidiManager::_removeConsumer(int32 id)
{
//go though entire list and remove the producer that is connected.
	int c=m_midiProducers.CountItems()-1;
	while (c>=0)
	{
		BMidiLocalProducer *aproducer=((BMidiLocalProducer *)m_midiProducers.ItemAt(c));
		if ((aproducer->Connections()->CountItems()==0))
		{
			BMidiLocalProducer *removing=((BMidiLocalProducer *)m_midiProducers.RemoveItem(c));
			
			
			int32 op;
			op=B_MIDI_UNREGISTERED;
			CUpdateHint hint;
			hint.AddInt32("midiop",op);
			hint.AddString("name",removing->Name());
			CObservableSubject::PostUpdate(&hint,NULL);
			
			//PRINT( ("%s connected, releasing\n",removing->Name()) );
			removing->Release();
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
void CMidiManager::Die()
{
m_isynth_sink->Release();

}


		
		