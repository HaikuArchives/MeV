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
#include <Debug.h>

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
		CObserver(*looper,NULL),
		track(NULL),
		channel(0)
	
{
	BRect rect(Bounds());
	BBox *box = new BBox(rect);
	AddChild(box);

	// add CDestination menu-field
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
	//message->PrintToStream();
	if (message->FindInt32("DestAttrs")==B_OK)
	{
		switch (message->FindInt32 ("DocAttrs"))
		{
			case CMeVDoc::Update_AddDest:
			case CMeVDoc::Update_DelDest:
			{
			Update();
			SetChannel(Document()->SelectedDestination());
			}
		}
	}
	if (message->FindInt32 ("DestAttrs"))
	{
		Update();
		SetChannel(Document()->SelectedDestination());
	}
		
	switch (message->what)
	{
		case CHANNEL_CONTROL_ID:
		{
			SetChannel(Document()->SelectedDestination());
			break;
		}
	}
	
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
		//test
		CDestination *dest;
		//SetSubject(m_destList);
		int id=-1;
		BRect icon_r;
		icon_r.Set(0,0,15,15); 
		//this isn't working zzz--
		while (dest=(Document()->FindNextHigherDestinationID(id)))
		{
			if (dest)
			{
			id=dest->GetID();
			CIconMenuItem *vc_item;
			BMessage *vc_message;
			vc_message=new BMessage(CHANNEL_CONTROL_ID);
			vc_message->AddInt8("channel",id);
			
			BBitmap	*icon = dest->GetProducer()->GetSmallIcon();
						
			vc_item=new CIconMenuItem(dest->Name(),vc_message,icon);
			vc_item->SetTarget((BView *)this);
			m_destMenu->AddItem(vc_item);
			}
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
		
		if (m_doc!=&track->Document())
		{
			SetDocument (&track->Document());
			SetSubject (&track->Document());
			Update();
		}
		SetChannel(Document()->SelectedDestination());
		//zzz
	}
}

		/**	Set which channel is selected. */
void CDestinationListView::SetChannel( int inChannel )
{
	Document()->SetSelectedDestination(inChannel);
	if (Document()->SelectedDestination()==-1)
	{
		BMenuItem *selected=m_destMenu->ItemAt(0);
		selected->SetMarked(true);
		return;
	}
	int c=0;
	int id=-1;
	CDestination *dest;
	while (dest=(Document()->FindNextHigherDestinationID(id)))
	{	
		id=dest->GetID();
		if (Document()->SelectedDestination()==dest->GetID())
		{
			BMenuItem *selected=m_destMenu->ItemAt(c+2);
			if (selected)
			{
				selected->SetMarked(true);
			}
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
		  	Document()->SetSelectedDestination(vc_id);
			Window()->PostMessage( msg );
		}
		break;
		case EDIT_ID:
		{
			if (!m_destMenu->ItemAt(0)->IsMarked())
			{
				if (m_modifierMap[Document()->SelectedDestination()]!=NULL)
				{
					m_modifierMap[Document()->SelectedDestination()]->Lock();
					m_modifierMap[Document()->SelectedDestination()]->Activate();
					m_modifierMap[Document()->SelectedDestination()]->Unlock();
				}
				else 
				{
					BRect r;
					r.Set(40,40,300,220);
					m_modifierMap[Document()->SelectedDestination()]=new CDestinationModifier(r,Document()->SelectedDestination(),m_doc,(BView *)this);
					m_modifierMap[Document()->SelectedDestination()]->Show();
				}
			}
			
		}
		break;
		case DELETE_ID:
		{	
			if (!m_destMenu->ItemAt(0)->IsMarked())
			{
				CDestination *dest=Document()->FindDestination(Document()->SelectedDestination());
				CDestinationDeleteUndoAction *undoAction;
				undoAction=new CDestinationDeleteUndoAction(dest);
				Document()->AddUndoAction(undoAction);
				if (m_modifierMap[Document()->SelectedDestination()]!=NULL)
				{
					m_modifierMap[Document()->SelectedDestination()]->Lock();	
					m_modifierMap[Document()->SelectedDestination()]->Quit();
					m_modifierMap[Document()->SelectedDestination()]=NULL;
				}
				Document()->SetSelectedDestination(-1);
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
			int n;
			BRect r;
			r.Set(40,40,300,220);
			CDestination *dest=Document()->NewDestination();
			n=dest->GetID();
			Document()->SetSelectedDestination(n);
			BMessage *msg=new BMessage(CHANNEL_CONTROL_ID);
			msg->AddInt8("channel",n);
			Window()->PostMessage( msg );
			
			m_modifierMap[n]=new CDestinationModifier(r,n,m_doc,(BView *)this);
			m_modifierMap[n]->Show();	
			
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
