//channel manager view
#include "TextDisplay.h"
#include "MeVDoc.h"
#include "ChannelManagerView.h"
#include "MathUtils.h"
#include "MidiDeviceInfo.h"
#include "IconMenuItem.h"
#include <Message.h>
//Interface kit
#include <View.h>
#include <Window.h>
#include <Rect.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <Bitmap.h>
//debug
#include <stdio.h>

enum EInspectorControlIDs {
	CHANNEL_CONTROL_ID	= 'chan',
	Slider1_ID			= 'sld1',
	Slider2_ID			= 'sld2',
	Slider3_ID			= 'sld3',
	Button_ID			= 'butt',
	NEW_ID				= 'nwid',
	DELETE_ID			= 'dtid'
};

CChannelManagerView::CChannelManagerView(
	BRect 		inFrame,
	CTextDisplay	*inNameView,
	uint32		inResizingMode,
	uint32		inFlags )
	: BView( inFrame, "ChannelManager", inResizingMode, inFlags )
{
	//initialize
	channel = 0;
	track = NULL;
	m_vcTableM=NULL;
	nameView = inNameView;
	SetFontSize( 9.0 );
	
	BRect r;
	//interface setup
		//popup
	r.Set(2,2,132,10);
	m_vcMenu=new BPopUpMenu("vchannels");
	BMenuField *menufield;
	menufield=new BMenuField(r,"vc field","v channel",m_vcMenu);
	AddChild (menufield);
	m_vcMenu->SetTargetForItems(this);
			//add new, delete.
	BMenuItem *new_vc=new BMenuItem ("New",new BMessage (NEW_ID));
	m_vcMenu->AddItem(new_vc);
	BMenuItem *delete_vc=new BMenuItem("Delete",new BMessage (DELETE_ID));
	m_vcMenu->AddItem(delete_vc);
		//edit button
	r.Set(140,2,200,10);
	m_editButton=new BButton(r,"editButton","Edit",new BMessage(Button_ID));
	AddChild(m_editButton);
		//port
	r.Set(2,30,40,42);
	m_port=new BStringView(r,"m_port","Port:");

	AddChild(m_port);
	r.Set(45,30,132,42);
	m_portName=new CTextDisplay(r,"m_portName");
	m_portName->SetText("no name");
	AddChild(m_portName);
		//channel
	r.Set(132,30,180,42);
	m_channel=new BStringView(r,"m_channel","Channel:");
	AddChild(m_channel);
	r.Set(180,30,200,42);
	m_channelValue=new CTextDisplay(r,"m_channelValue");
	m_channelValue->SetText("-");
	AddChild(m_channelValue);
	

}



void CChannelManagerView::MenusBeginning()
{
	
}
void CChannelManagerView::Update()
{
	
	int n=m_vcMenu->CountItems()-1;
	while (n>=2)
	{
	delete m_vcMenu->RemoveItem(n);
	n--;
	}
	
	if (track)
	{
		m_vcTableM=track->Document().GetVCTableManager ();
		VChannelEntry *VCptr;
		int id;
		BRect icon_r;
		icon_r.Set(0,0,15,15); 
		for (m_vcTableM->First();!m_vcTableM->IsDone();m_vcTableM->Next())
		{
			VCptr=m_vcTableM->CurrentVC();
			id=m_vcTableM->CurrentID();
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
			vc_item=new CIconMenuItem(VCptr->name->String(),vc_message,icon);
			vc_item->SetTarget(this);
			m_vcMenu->AddItem(vc_item);
		}		
		
	}
}
void CChannelManagerView::SetTrack( CEventTrack *inTrack )
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
	}
}

		/**	Set which channel is selected. */
void CChannelManagerView::SetChannel( uint8 inChannel )
{
		VChannelEntry *VCptr;
		for (m_vcTableM->First();!m_vcTableM->IsDone();m_vcTableM->Next())
		{
			if (inChannel==m_vcTableM->CurrentID())
			{
				printf ("found %s\n",m_vcTableM->CurrentVC()->name->String());
				BMenuItem *selected=m_vcMenu->FindItem(m_vcTableM->CurrentVC()->name->String());
				selected->SetMarked(true);
			}
		}
	
}
void CChannelManagerView::MessageReceived(BMessage *msg)
{
	switch (msg->what)
		{
			case CHANNEL_CONTROL_ID:
			{
				
				int8 vc;
			  	msg->FindInt8("channel",&vc);
				channel=vc;
				BString schan;
				schan << vc;
				m_channelValue->SetText(schan.String());
				Window()->PostMessage( msg );
			}
			break;
			
			default:
				BView::MessageReceived(msg);
				break;
			}
}
