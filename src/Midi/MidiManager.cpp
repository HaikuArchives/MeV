/* ===================================================================== *
 * MidiManager.cpp (MeV/Midi)
 * ===================================================================== */
#include "MidiManager.h"
#include "PortNameMap.h"
#include "list.h"

#include "ReconnectingMidiProducer.h"
#include <MidiConsumer.h>
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
		PRINT(("Couldn't get MIDI m_roster\n"));
		be_app->PostMessage(B_QUIT_REQUESTED);
		return;
	}
	BMessenger msgr(this);
	m_roster->StartWatching(&msgr);
	m_portNameMap=new CPortNameMap();
	m_internalSynth=NULL;
	AddInternalSynth();
}
BMidiConsumer * CMidiManager::NextConsumer(int32 *id)
{
	return (m_roster->NextConsumer(id));
}
BMidiConsumer * CMidiManager::FindConsumer (int32 id)
{
	return (m_roster->FindConsumer(id));
}
BMidiEndpoint* CMidiManager::FindEndpoint(const char* name)
{
	int32 id=0;
	BMidiEndpoint* object = m_roster->NextEndpoint(&id);
	while (object) {
		if (! strcmp(object->Name(), name))
			break;

		// Don't forget to decrement the refcount
		// from NextEndpoint!			
		object->Release();
		object = m_roster->NextEndpoint(&id);
	}

	return object;
}

BMidiProducer* CMidiManager::FindProducer(const char* name)
{
	int32 id=0;
	BMidiProducer* object = m_roster->NextProducer(&id);
	while (object) {
		if (! strcmp(object->Name(), name))
			break;
			
		// Don't forget to decrement the refcount
		// from NextProducer!			
		object->Release();
		object = m_roster->NextProducer(&id);
	}

	return object;
}

BMidiConsumer* CMidiManager::FindConsumer(const char* name)
{
	BString isname="Internal Synth";
	if (isname.Compare(name)==0)
	{
		return InternalSynth();
	}
	int32 id=0;
	BMidiConsumer* object = m_roster->NextConsumer(&id);
	while (object) {
		if (! strcmp(object->Name(), name))
			break;
			
		// Don't forget to decrement the refcount
		// from NextConsumer!			
		object->Release();
		object = m_roster->NextConsumer(&id);
	}

	return object;
}

BBitmap * CMidiManager::ConsumerIcon(int32 id,icon_size which)
{
	BMessage props;
	BMidiEndpoint *endpoint;
	endpoint=m_roster->FindEndpoint (id);
	endpoint->GetProperties(&props);
	return (_createIcon(&props,B_MINI_ICON));
}

CInternalSynth * CMidiManager::InternalSynth()
{
return (m_internalSynth);
}

void CMidiManager::AddInternalSynth()
{
	if (m_internalSynth==NULL)
	{
		m_internalSynth=new CInternalSynth("Internal Synth");
		m_internalSynth->Register();
		PRINT(("adding %s\n",m_internalSynth->Name()));
	}
}
void CMidiManager::_addConsumer(int32 id)
{
	/*int32 pid;
	CReconnectingMidiProducer *prod;
	BMidiConsumer *con=m_roster->FindConsumer(id);
	while (prod=(CReconnectingMidiProducer *)m_roster->NextProducer(&pid))
	{
		if (prod->IsInConnectList(con->Name()))
		{
			prod->Connect(con);
			prod->RemoveFromConnectList(con->Name());
		}
	}
	
	int32 op;
	op=B_MIDI_REGISTERED;
	CUpdateHint hint;
	hint.AddInt32("midiop",op);
	hint.AddString("name",con->Name());
	CObservableSubject::PostUpdate(&hint,NULL);
*/
}
void CMidiManager::_removeConsumer(int32 id)
{
	/*
	int32 pid;
	CReconnectingMidiProducer *prod;
	BMidiConsumer *con=m_roster->FindConsumer(id);
	while (prod=(CReconnectingMidiProducer *)m_roster->NextProducer(&pid))
	{
		if (prod->IsConnected(con))
		{
			printf ("adding %s to %s\n",prod->Name(),con->Name());
			prod->AddToConnectList(con->Name());
		}
	}
	int32 op;
	op=B_MIDI_UNREGISTERED;
	CUpdateHint hint;
	hint.AddInt32("midiop",op);
	hint.AddString("name",con->Name());
	CObservableSubject::PostUpdate(&hint,NULL);*/
}
void CMidiManager::_addProducer(int32 id)
{
}
void CMidiManager::_removeProducer(int32 id)
{
}
void CMidiManager::Die()
{

}

void CMidiManager::AddIcons(BMessage* msg, BBitmap* largeIcon, BBitmap* miniIcon) const
{
	if (! msg->HasData("be:large_icon", 'ICON')) {
		msg->AddData("be:large_icon", 'ICON', largeIcon->Bits(),
			largeIcon->BitsLength());
	} else {
		msg->ReplaceData("be:large_icon", 'ICON', largeIcon->Bits(),
			largeIcon->BitsLength());
	}
	if (! msg->HasData("be:mini_icon", 'MICN')) {
		msg->AddData("be:mini_icon", 'MICN', miniIcon->Bits(),
			miniIcon->BitsLength());
	} else {
		msg->ReplaceData("be:mini_icon", 'MICN', miniIcon->Bits(),
			miniIcon->BitsLength());
	}
}
BBitmap* CMidiManager::_createIcon(const BMessage* msg, icon_size which)
{
	float iconSize;
	uint32 iconType;
	const char* iconName;

	if (which == B_LARGE_ICON) {
		iconSize = 32;
		iconType = 'ICON';
		iconName = "be:large_icon";
	} else if (which == B_MINI_ICON) {
		iconSize = 16;
		iconType = 'MICN';
		iconName = "be:mini_icon";
	} else {
		return NULL;
	}
							
	const void* data;
	ssize_t size;
	BBitmap* bitmap = NULL;

	if (msg->FindData(iconName, iconType, &data, &size) == B_OK)
	{
		BRect r(0, 0, iconSize-1, iconSize-1);
		bitmap = new BBitmap(r, B_CMAP8);
		ASSERT((bitmap->BitsLength() == size));
		memcpy(bitmap->Bits(), data, size);
	}
	else if (iconName=="be:mini_icon")
	{
	iconName="be:small_icon";
		if (msg->FindData(iconName, iconType, &data, &size) == B_OK)
		{
			BRect r(0, 0, iconSize-1, iconSize-1);
			bitmap = new BBitmap(r, B_CMAP8);
			ASSERT((bitmap->BitsLength() == size));
			memcpy(bitmap->Bits(), data, size);
		}
	}
	return bitmap;
}

void CMidiManager::_copyIcon(const BMessage* smsg,BMessage* dmsg)
{
	float iconSize;
	uint32 iconType;
	const char* iconName;
	const void* data;
	const void* data2;
	ssize_t size,size2;

	iconSize = 32;
	iconType = 'ICON';
	iconName = "be:large_icon";
	smsg->FindData(iconName, iconType, &data, &size);
	dmsg->AddData (iconName, iconType, data, size);
	iconSize = 16;
	iconType = 'MICN';
	iconName = "be:mini_icon";				
	smsg->FindData(iconName, iconType, &data, &size);
	dmsg->AddData(iconName,iconType,data, size);
	
	iconName = "be:small_icon"; //sometimes its mini, other times its small.
	smsg->FindData(iconName,iconType, &data, &size);
	if (size>0)
	{
		dmsg->AddData("be:mini_icon",iconType,data, size);
	}
}
void CMidiManager::_disconnect(int32 prod,int32 con)
{
	
	BMidiProducer *producer = m_roster->FindProducer(prod);
	BMidiConsumer *consumer = m_roster->FindConsumer(con);	
	//printf ("disonnected %s %s\n",producer->Name(),consumer->Name());

}
void CMidiManager::_connect(int32 prod,int32 con)
{

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
		PRINT(("MidiManager::HandleMidiEvent: \"op\" field not found\n"));
		return;
	}

	
	switch (op) {
	case B_MIDI_REGISTERED:
		{
			int32 id;
			if (msg->FindInt32("be:id", &id) != B_OK) {
				PRINT(("MidiManager::HandleMidiEvent: \"be:id\" field not found in B_MIDI_REGISTERED event\n"));
				break;
			}
			
			const char* type;
			if (msg->FindString("be:type", &type) != B_OK) {
				PRINT(("MidiManager::HandleMidiEvent: \"be:type\" field not found in B_MIDI_REGISTERED event\n"));
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
				PRINT(("MidiManager::HandleMidiEvent: \"be:id\" field not found in B_MIDI_UNREGISTERED\n"));
				break;
			}
			
			const char* type;
			if (msg->FindString("be:type", &type) != B_OK) {
				PRINT(("MidiManager::HandleMidiEvent: \"be:type\" field not found in B_MIDI_UNREGISTERED\n"));
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
				PRINT(("MidiManager::HandleMidiEvent: \"be:id\" field not found in B_MIDI_CHANGED_PROPERTIES\n"));
				break;
			}
			
			const char* type;
			if (msg->FindString("be:type", &type) != B_OK) {
				PRINT(("MidiManager::HandleMidiEvent: \"be:type\" field not found in B_MIDI_CHANGED_PROPERTIES\n"));
				break;
			}
			
			BMessage props;
			if (msg->FindMessage("be:properties", &props) != B_OK) {
				PRINT(("MidiManager::HandleMidiEvent: \"be:properties\" field not found in B_MIDI_CHANGED_PROPERTIES\n"));
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
		{
			int32 prod;
			if (msg->FindInt32("be:producer", &prod) != B_OK) {
				PRINT(("PatchView::HandleMidiEvent: \"be:producer\" field not found in B_MIDI_CONNECTED\n"));
				break;
			}
			
			int32 cons;
			if (msg->FindInt32("be:consumer", &cons) != B_OK) {
				PRINT(("PatchView::HandleMidiEvent: \"be:consumer\" field not found in B_MIDI_CONNECTED\n"));
				break;
			}
			PRINT(("MIDI Roster Event B_MIDI_CONNECTED: producer=%ld, consumer=%ld\n", prod, cons));
			_connect(prod, cons);
		}
		break;
	case B_MIDI_DISCONNECTED:
		{
			int32 prod;
			if (msg->FindInt32("be:producer", &prod) != B_OK) {
				PRINT(("PatchView::HandleMidiEvent: \"be:producer\" field not found in B_MIDI_DISCONNECTED\n"));
				break;
			}
			
			int32 cons;
			if (msg->FindInt32("be:consumer", &cons) != B_OK) {
				PRINT(("PatchView::HandleMidiEvent: \"be:consumer\" field not found in B_MIDI_DISCONNECTED\n"));
				break;
			}
			PRINT(("MIDI Roster Event B_MIDI_DISCONNECTED: producer=%ld, consumer=%ld\n", prod, cons));
			_disconnect(prod, cons);
		}
		break;
	
		//we don't care about these right now.
	break;
	default:
		PRINT(("MidiManager::HandleMidiEvent: unknown opcode %ld\n", op));
		break;
	}
}


