

//channel manager view
#include "TextDisplay.h"
#include "MeVDoc.h"
#include "DestinationListView.h"
#include "MathUtils.h"
#include "MidiDeviceInfo.h"
#include "IconMenuItem.h"
#include <Message.h>
#include <Looper.h>
//Interface kit
#include <Rect.h>
#include <MenuField.h>
#include <Bitmap.h>
//debug
#include <stdio.h>
#include <Box.h>
enum EInspectorControlIDs {
	CHANNEL_CONTROL_ID	= 'chan',
	Slider1_ID			= 'sld1',
	Slider2_ID			= 'sld2',
	Slider3_ID			= 'sld3',
	EDIT_ID			= 'butt',
	NEW_ID				= 'nwid',
	DELETE_ID			= 'dtid',
	VCTM_NOTIFY			= 'ntfy',
	VCQUIT 				='vcqt'
};

CDestinationListView::CDestinationListView(
	BRect 		inFrame,
	BLooper		*thelooper,
	uint32		inResizingMode,
	uint32		inFlags )
	: BView( inFrame, "ChannelManager", inResizingMode, inFlags ),CObserver (*thelooper,NULL)
{
	//initialize
	channel = 0;
	track = NULL;
	m_destList=NULL;
	SetFontSize( 9.0 );
	BRect r;
	//interface setup
		//popup
	r.Set(2,2,132,10);
	m_destMenu=new BPopUpMenu("-");
	BMenuField *menufield;
	menufield=new BMenuField(r,"dest field","Destination",m_destMenu);
	AddChild (menufield);
			//add new, delete.
	BMenuItem *new_vc=new BMenuItem ("New",new BMessage (NEW_ID));
	m_destMenu->AddItem(new_vc);
	BSeparatorItem *sep=new BSeparatorItem ();
	m_destMenu->AddItem(sep);
	r.Set(135,2,175,10);
	m_deleteButton=new BButton(r,"editButton","Delete",new BMessage(DELETE_ID));
	AddChild(m_deleteButton);
		//edit button
	r.Set(180,2,220,10);
	m_editButton=new BButton(r,"editButton","Edit",new BMessage(EDIT_ID));
	AddChild(m_editButton);
		//port
	r.Set(2,30,40,42);
	m_port=new BStringView(r,"m_port","Port:");

	AddChild(m_port);
	r.Set(45,30,132,42);
	m_portName=new CTextDisplay(r,"m_portName");
	m_portName->SetText("no port");
	AddChild(m_portName);
		//channel
	r.Set(132,30,180,42);
	m_channel=new BStringView(r,"m_channel","Channel:");
	AddChild(m_channel);
	r.Set(180,30,200,42);
	m_channelValue=new CTextDisplay(r,"m_channelValue");
	m_channelValue->SetText("-");
	AddChild(m_channelValue);
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	
	BBox *box=new BBox(Bounds());
	AddChild(box);
	
	

}

void CDestinationListView::AttachedToWindow()
{
	m_editButton->SetTarget((BView *)this);
	m_deleteButton->SetTarget((BView *)this);
	m_destMenu->SetTargetForItems((BView *)this);
	BMenuItem *select=m_destMenu->ItemAt(0);
	select->SetMarked((BView *)true);
	
}
void CDestinationListView::OnUpdate(BMessage *message)
{
//read port,channel,color,name changes and what not.//mute refresh track.
	Update();
	SetChannel(m_selected_id);
	//we can really improve the efficency here.
}
void CDestinationListView::Update()
{
	
	int n=m_destMenu->CountItems()-1;
	while (n>=2)
	{
		delete m_destMenu->RemoveItem(n);
		n--;
	}
	
	if (track)
	{
		m_destList=track->Document().GetDestinationList ();
		SetSubject(m_destList);
		Destination *VCptr;
		int id;
		BRect icon_r;
		icon_r.Set(0,0,15,15); 
		for (m_destList->First();!m_destList->IsDone();m_destList->Next())
		{
			VCptr=m_destList->CurrentDest();
			id=m_destList->CurrentID();
			CIconMenuItem *vc_item;
			BMessage *vc_message;
			vc_message=new BMessage(CHANNEL_CONTROL_ID);
			vc_message->AddInt8("channel",id);
			BBitmap	*icon = new BBitmap( BRect(0,0,15,15), B_RGBA32 );
			uint32 *bits = (uint32 *)(icon->Bits());
			for( int32 i = 0; i < icon->BitsLength()/4; i++ )
			{
				bits[i] = int32((VCptr->fillColor.alpha << 24 )
							+ (VCptr->fillColor.red << 16 )
							+ (VCptr->fillColor.green << 8 )
							+ VCptr->fillColor.blue);
			}
				
			icon->SetBits((void *)bits,255,0,B_RGB32);
			vc_item=new CIconMenuItem(VCptr->name.String(),vc_message,icon);
			vc_item->SetTarget((BView *)this);
			m_destMenu->AddItem(vc_item);
		}
	}
}
void CDestinationListView::SetTrack( CEventTrack *inTrack )
{

	if (track != inTrack)
	{
		CRefCountObject::Release( track );
		track = inTrack;
		if (track) track->Acquire();
		if (Window())
		{
			Window()->Lock();
			Update();
			Window()->Unlock();
		}
		Update();
	}
}

		/**	Set which channel is selected. */
void CDestinationListView::SetChannel( uint8 inChannel )
{
	m_selected_id=inChannel;

	int c=0;
	for (m_destList->First();!m_destList->IsDone();m_destList->Next())
	{
		if (m_selected_id==m_destList->CurrentID())
		{
			Destination *dest=m_destList->CurrentDest();
			BMenuItem *selected=m_destMenu->ItemAt(c+2);
			selected->SetMarked(true);
			if (dest->m_producer)
			{
				m_portName->SetText(dest->m_producer->Name());
			}
			else
			{
				m_portName->SetText("no port");
			}
			BString sch;
			sch << dest->channel;
			m_channelValue->SetText(sch.String());
			return;
		}
		c++;
	}

}

void CDestinationListView::MessageReceived(BMessage *msg)
{
	switch (msg->what)
		{
			case CHANNEL_CONTROL_ID:
			{
				int8 vc_id;
			  	msg->FindInt8("channel",&vc_id);
			  	SetChannel(vc_id);
			  	m_selected_id=vc_id;
				Window()->PostMessage( msg );
			}
			break;
			case EDIT_ID:
			{
				if (!m_destMenu->ItemAt(0)->IsMarked())
				{
					if (m_modifierMap[m_selected_id]!=NULL)
					{
						m_modifierMap[m_selected_id]->Lock();
						m_modifierMap[m_selected_id]->Activate();
						m_modifierMap[m_selected_id]->Unlock();
					}
					else 
					{
						BRect r;
						r.Set(40,40,300,200);
						m_modifierMap[m_selected_id]=new CDestinationModifier(r,m_selected_id,m_destList,(BView *)this);
						m_modifierMap[m_selected_id]->Show();
					}
				}
				
			}
			break;
			case DELETE_ID:
			{	
				if (!m_destMenu->ItemAt(0)->IsMarked())
				{
					m_destList->RemoveVC(m_selected_id);
					Update();
					BMenuItem *select=m_destMenu->ItemAt(0);
					select->SetMarked(true);
					if (m_modifierMap[m_selected_id]!=NULL)
					{
						m_modifierMap[m_selected_id]->Lock();	
						m_modifierMap[m_selected_id]->Quit();
						m_modifierMap[m_selected_id]=NULL;
					}
				}
			}
			break;
			case VCQUIT:
			{
				int32 quit_id=msg->FindInt32("ID");
				m_modifierMap[quit_id]->Lock();
				m_modifierMap[quit_id]->Quit();
				m_modifierMap[quit_id]=NULL;
			}
			break;
			case NEW_ID:
			{
				BRect r;
				r.Set(40,40,300,200);
				int n=m_destList->NewDest();
				Update();
				m_modifierMap[n]=new CDestinationModifier(r,n,m_destList,(BView *)this);
				m_modifierMap[n]->Show();
				SetChannel(n);			
			}
			break;
			case Update_ID:
			case Delete_ID:
			{
				CObserver::MessageReceived(msg);
			}
			break;
			//case VCTM_NOTIFY:
			//{
			//	Update();
			//	SetChannel(m_selected_id);
			//}
			default:
				BView::MessageReceived(msg);
				break;
			}
}
