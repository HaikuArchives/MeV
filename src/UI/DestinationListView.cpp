/* ===================================================================== *
 * DestinationListView.cpp (MeV/UI)
 * ===================================================================== */

#include "DestinationListView.h"

#include "DestinationModifier.h"
#include "IconMenuItem.h"
#include "MathUtils.h"
#include "MeVDoc.h"
#include "MidiDeviceInfo.h"
#include "TextDisplay.h"

// Application Kit
#include <Looper.h>
#include <Message.h>
//Interface kit
#include <Bitmap.h>
#include <Box.h>
#include <MenuField.h>
//debug
#include <stdio.h>

enum EInspectorControlIDs {
	CHANNEL_CONTROL_ID	= 'chan',
	Slider1_ID			= 'sld1',
	Slider2_ID			= 'sld2',
	Slider3_ID			= 'sld3',
	EDIT_ID			= 'butt',
	NEW_ID				= 'nwid',
	DELETE_ID			= 'dtid',
	VCTM_NOTIFY			= 'ntfy',
};

CDestinationListView::CDestinationListView(
	BRect frame,
	BLooper *looper,
	uint32 resizingMode,
	uint32 flags )
	:	BView(frame, "DestinationListView", resizingMode, flags),
		CObserver(*looper, NULL),
		track(NULL),
		channel(0),
		m_destList(NULL)
{
	BRect rect(Bounds());
	BBox *box = new BBox(rect);
	AddChild(box);

	// add destination menu-field
	rect.InsetBy(5.0, 5.0);
	m_destMenu = new BPopUpMenu("");
	BMenuItem *item = new BMenuItem("New" B_UTF8_ELLIPSIS,
									  new BMessage(NEW_ID));
	m_destMenu->AddItem(item);
	m_destMenu->AddSeparatorItem();
	m_destfield = new BMenuField(rect, "Destination", "Destination: ", m_destMenu);
	m_destfield->SetDivider(be_plain_font->StringWidth("Destination:  "));
	m_destfield->SetEnabled(false);
	box->AddChild(m_destfield);

	font_height fh;
	be_plain_font->GetHeight(&fh);

	BRect buttonsRect(m_destfield->Frame());
	buttonsRect.left += m_destfield->Divider();
	buttonsRect.top += 2 * (fh.ascent + fh.descent);
	buttonsRect.bottom = box->Bounds().bottom - 5.0;

	// add "Delete" button
	rect = buttonsRect;
	rect.right = (rect.left + rect.right) / 2.0 - 3.0;
	m_deleteButton = new BButton(rect, "Delete", "Delete", 
								 new BMessage(DELETE_ID));
	box->AddChild(m_deleteButton);

	// add "Edit" button
	rect.OffsetBy(rect.Width() + 5.0, 0.0);
	m_editButton = new BButton(rect, "Edit", "Edit", new BMessage(EDIT_ID));
	box->AddChild(m_editButton);

	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
}

void
CDestinationListView::AttachedToWindow()
{
	m_editButton->SetTarget((BView *)this);
	m_deleteButton->SetTarget((BView *)this);
	m_destMenu->SetTargetForItems((BView *)this);
	BMenuItem *select=m_destMenu->ItemAt(0);
	select->SetMarked((BView *)true);
	
}

void
CDestinationListView::OnUpdate(BMessage *message)
{
	//read port,channel,color,name changes and what not.//mute refresh track.
	Update();
	SetChannel(m_destList->SelectedId());
	//we can really improve the efficency here.
}

void
CDestinationListView::Update()
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
			
			BBitmap	*icon = m_destList->GetIconFor(id,icon_r);
						
			vc_item=new CIconMenuItem(VCptr->name.String(),vc_message,icon);
			vc_item->SetTarget((BView *)this);
			m_destMenu->AddItem(vc_item);
		}
	}
}
void CDestinationListView::SetTrack( CEventTrack *inTrack )
{

	BView::LockLooper();
	m_destfield->SetEnabled(true);
	BView::UnlockLooper();
	if (track != inTrack)
	{
		CRefCountObject::Release( track );
		track = inTrack;
		if (track)
			track->Acquire();
		Update();
		SetChannel(m_destList->SelectedId());
	}
}

		/**	Set which channel is selected. */
void CDestinationListView::SetChannel( int inChannel )
{
	m_destList->SetSelectedId(inChannel);
	if (m_destList->SelectedId()==-1)
	{
		BMenuItem *selected=m_destMenu->ItemAt(0);
		selected->SetMarked(true);
		return;
	}
	int c=0;
	for (m_destList->First();!m_destList->IsDone();m_destList->Next())
	{
		if (m_destList->SelectedId()==m_destList->CurrentID())
		{
			BMenuItem *selected=m_destMenu->ItemAt(c+2);
			selected->SetMarked(true);
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
		  	m_destList->SetSelectedId(vc_id);
			Window()->PostMessage( msg );
		}
		break;
		case EDIT_ID:
		{
			if (!m_destMenu->ItemAt(0)->IsMarked())
			{
				if (m_modifierMap[m_destList->SelectedId()]!=NULL)
				{
					m_modifierMap[m_destList->SelectedId()]->Lock();
					m_modifierMap[m_destList->SelectedId()]->Activate();
					m_modifierMap[m_destList->SelectedId()]->Unlock();
				}
				else 
				{
					BRect r;
					r.Set(40,40,300,220);
					m_modifierMap[m_destList->SelectedId()]=new CDestinationModifier(r,m_destList->SelectedId(),m_destList,(BView *)this);
					m_modifierMap[m_destList->SelectedId()]->Show();
				}
			}
			
		}
		break;
		case DELETE_ID:
		{	
			if (!m_destMenu->ItemAt(0)->IsMarked())
			{
				m_destList->RemoveVC(m_destList->SelectedId());
				Update();
				BMenuItem *select=m_destMenu->ItemAt(0);
				select->SetMarked(true);
				if (m_modifierMap[m_destList->SelectedId()]!=NULL)
				{
					m_modifierMap[m_destList->SelectedId()]->Lock();	
					m_modifierMap[m_destList->SelectedId()]->Quit();
					m_modifierMap[m_destList->SelectedId()]=NULL;
				}
				m_destList->SetSelectedId(-1);
			}
		}
		break;
		case CDestinationModifier::WINDOW_CLOSED:
		{
			int32 destinationID = msg->FindInt32("destination_id");
			if (m_modifierMap[destinationID] != NULL)
			{
				m_modifierMap[destinationID]->Lock();
				m_modifierMap[destinationID]->Quit();
				m_modifierMap[destinationID] = NULL;
			}
			break;
		}
		case NEW_ID:
		{
			BRect r;
			r.Set(40,40,300,220);
			int n=m_destList->NewDest();
			m_modifierMap[n]=new CDestinationModifier(r,n,m_destList,(BView *)this);
			m_modifierMap[n]->Show();	
			Update();	
			SetChannel(n);	
			BMessage *msg=new BMessage(CHANNEL_CONTROL_ID);
			msg->AddInt8("channel",n);
			Window()->PostMessage( msg );
		}
		break;
		case Update_ID:
		case Delete_ID:
		{
			CObserver::MessageReceived(msg);
		}
		break;
		default:
		{
			BView::MessageReceived(msg);
			break;
		}
	}
}
