#include "VChannelModifier.h"
#include "VChannel.h"
#include <Rect.h>
#include <MenuField.h>
#include "MidiManager.h"
#include "ChannelManagerView.h"
#include <stdio.h>
enum EVChannelModifierControlID {
	NAME_ID='name',
	PORT_SELECT='ptsl',
	CHANNEL_SELECT='cslt',
	NOTIFY='ntfy',
	ADD_ID='add',
	MOD_ID='mod',
	VCQUIT='vcqt',
	MUTE_ID='cmut'
	};
	
CVChannelModifier::CVChannelModifier(BRect frame,int32 id,CVCTableManager *tm,BHandler *parent) : 
	BWindow(frame,"Destination Modifier",B_TITLED_WINDOW,B_NOT_ZOOMABLE | B_NOT_RESIZABLE),
	CObserver(*this,tm)
{
	m_tm=tm;
	m_midiManager=CMidiManager::Instance();
	m_vc=tm->get(id);
	m_id=id;
	_buildUI();
	Update();
	m_parent=parent;
}



void
CVChannelModifier::_buildUI()
{
	BRect r;
	m_background=new BView (Bounds(),"bk",B_FOLLOW_LEFT,B_WILL_DRAW);
	m_background->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	AddChild(m_background);
	BPoint st(0,0);
	m_colors=new BColorControl(st,B_CELLS_16x16,1,"dest color",new BMessage(MOD_ID));
	m_colors->SetValue(m_vc->fillColor);
	m_background->AddChild(m_colors);
	r.Set(100,5,260,20);
	m_name=new BTextControl(r,"name","Name: ","",new BMessage (NAME_ID));
	m_name->SetText(m_vc->name.String());
	m_name->SetDivider(36);
	BString title;
	title << "Destination: ";
	title << m_vc->name.String();
	SetTitle(title.String());
	m_background->AddChild(m_name);
	//fill midi port pop up.
	m_midiPorts=new BPopUpMenu ("Port");
	m_channels=new BPopUpMenu ("Channel");
	int c;
	for (c=1;c<=16;c++)
	{
		BMessage *msg=new BMessage (CHANNEL_SELECT);
		msg->AddInt8("value",c);
		BString cname;
		cname << c;
		BMenuItem *item=new BMenuItem(cname.String(),msg);
		m_channels->AddItem(item);
	}
	(m_channels->ItemAt((m_vc->channel-1)))->SetMarked(true);
	BMenuField *port;
	BMenuField *channel;
	r.Set(10,110,150,130);
	port=new BMenuField(r,"port","Port:",m_midiPorts);
	port->SetDivider(28);
	m_background->AddChild(port);	

	r.Set(130,110,200,130);
	channel=new BMenuField(r,"channel","Channel:",m_channels);
	channel->SetDivider(56);
	m_background->AddChild(channel);
	r.Set(10,140,100,160);
	
	m_mute=new BCheckBox(r,"mutebox","Mute",new BMessage (MUTE_ID));
	m_background->AddChild(m_mute);
}
void CVChannelModifier::AttachedToWindow()
{
	
	m_done->SetTarget((BView *)this);
	m_name->SetTarget((BView *)this);
	m_channels->SetTargetForItems((BView *)this);
	m_midiPorts->SetTargetForItems((BView *)this);	
	m_colors->SetTarget((BView *)this);
	m_mute->SetTarget((BView *)this);
}

void
CVChannelModifier::Update()
{
	int c=m_midiPorts->CountItems();
	while (c>=0)
	{
		delete (m_midiPorts->RemoveItem(c--));
	}
	for (m_midiManager->FirstProducer();!m_midiManager->IsLastProducer();m_midiManager->NextProducer())
	{
		BMessage *msg=new BMessage (PORT_SELECT);
		msg->AddInt32("pid",m_midiManager->CurrentProducerID());
		BMenuItem *item=new BMenuItem(m_midiManager->CurrentProducerName()->String(),
									msg);
		item->SetTarget((BView *)this);
		m_midiPorts->AddItem(item);
		if (m_vc->m_producer)
		{
			if (m_vc->m_producer->ID()==m_midiManager->CurrentProducerID())  
			{
				item->SetMarked(true);
			}
		}	
	}
}
void CVChannelModifier::OnUpdate(BMessage *msg)
{
//i don't really know why this would happen.
}
void
CVChannelModifier::MenusBeginning()
{
	Update();
}
bool 
CVChannelModifier::QuitRequested()
{
	BMessage *msg = new BMessage (VCQUIT);
	msg->AddInt32("ID",m_id);
	BMessenger *amsgr=new BMessenger(m_parent);
	amsgr->SendMessage(msg);
	return 0;
}

void
CVChannelModifier::MessageReceived(BMessage *msg)
{
	switch (msg->what)
	{
	case MOD_ID:
	{
		m_tm->SetColorFor(m_id, m_colors->ValueAsColor());
		
		//((CChannelManagerView *) m_parent)->track->RefreshChannel(m_id);
	}
	break;
	case CHANNEL_SELECT:
	{
		m_tm->SetChannelFor(m_id,msg->FindInt8("value"));	
	}
	break;
	case PORT_SELECT:
	{
		m_tm->SetPortFor(m_id,m_midiManager->GetProducer(msg->FindInt32("pid")));
	}
	break;
	case NAME_ID:
	{
		//m_vc->name.SetTo(m_name->Text());
		BString name;
		name << m_name->Text();
		m_tm->SetNameFor(m_id,name);
		BString title;
		title << "Destination: ";
		title << name;
		SetTitle(title.String());
	}
	break;
	case MUTE_ID:
	{
		if (m_mute->Value()==B_CONTROL_ON)
		{
			m_tm->SetMuteFor (m_id, true);
		}
		else
		{
			m_tm->SetMuteFor (m_id,false);
		}
	}
	break;
	default:
		BWindow::MessageReceived(msg);
	break;
	}
}