/* ===================================================================== *
 * DestinationListView.cpp (MeV/UI)
 * ===================================================================== */

#include "DestinationListView.h"

#include "DestinationModifier.h"
#include "EventTrack.h"
#include "IconMenuItem.h"
#include "MathUtils.h"
#include "MeVDoc.h"
#include "MidiDeviceInfo.h"
#include "PlayerControl.h"
#include "StdEventOps.h"
#include "TextDisplay.h"

// Application Kit
#include <Looper.h>
#include <Message.h>
// Interface kit
#include <Bitmap.h>
#include <Box.h>
#include <Button.h>
#include <MenuField.h>
#include <PopUpMenu.h>
// Storage Kit
#include <Mime.h>
// Support Kit
#include <Debug.h>

// ---------------------------------------------------------------------------
//	Constructor/Destructor

CDestinationListView::CDestinationListView(
	BRect frame,
	uint32 resizingMode,
	uint32 flags)
	:	BView(frame, "DestinationListView", resizingMode, flags),
		CObserver(),
		m_doc(NULL),
		m_track(NULL)
{
	BRect rect(Bounds());
	BBox *box = new BBox(rect);
	AddChild(box);

	// add CDestination menu-field
	rect.InsetBy(5.0, 5.0);
	m_destMenu = new BPopUpMenu("(None)");
	BMenuItem *item = new BMenuItem("New" B_UTF8_ELLIPSIS,
									  new BMessage(CREATE_DESTINATION));
	m_destMenu->AddItem(item);
	m_destMenu->AddSeparatorItem();
	m_destMenu->SetLabelFromMarked(true);
	m_destField = new BMenuField(rect, "Destination", "Destination: ", 
								 m_destMenu);
	m_destField->SetDivider(be_plain_font->StringWidth("Destination:  "));
	m_destField->SetEnabled(false);
	box->AddChild(m_destField);

	font_height fh;
	be_plain_font->GetHeight(&fh);

	BRect buttonsRect(m_destField->Frame());
	buttonsRect.left += m_destField->Divider();
	buttonsRect.top += 2 * (fh.ascent + fh.descent);
	buttonsRect.bottom = box->Bounds().bottom - 5.0;

	// add "Delete" button
	rect = buttonsRect;
	rect.right = (rect.left + rect.right) / 2.0 - 3.0;
	m_deleteButton = new BButton(rect, "Delete", "Delete", 
								 new BMessage(DELETE_DESTINATION));
	m_deleteButton->SetEnabled(false);
	box->AddChild(m_deleteButton);

	// add "Edit" button
	rect.OffsetBy(rect.Width() + 5.0, 0.0);
	m_editButton = new BButton(rect, "Edit", "Edit",
							   new BMessage(EDIT_DESTINATION));
	m_editButton->SetEnabled(false);
	box->AddChild(m_editButton);

	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
}

CDestinationListView::~CDestinationListView()
{
	if (m_doc)
	{
		m_doc->RemoveObserver(this);
		m_doc = NULL;
	}
}

// ---------------------------------------------------------------------------
//	Accessors

void
CDestinationListView::SetDocument(
	CMeVDoc *document)
{
	if (document != m_doc)
	{
		if (m_doc != NULL)
		{
			m_doc->RemoveObserver(this);

			StSubjectLock lock(*m_doc, Lock_Shared);
			CDestination *dest = NULL;
			int32 index = 0;
			while ((dest = m_doc->GetNextDestination(&index)) != NULL)
				_destinationRemoved(0);
		}

		m_doc = document;
		if (m_doc != NULL)
		{
			m_doc->AddObserver(this);

			StSubjectLock lock(*m_doc, Lock_Shared);	
			CDestination *dest = NULL;
			int32 index = 0;
			while ((dest = m_doc->GetNextDestination(&index)) != NULL)
				_destinationAdded(dest->ID());
		}
	}
}

void 
CDestinationListView::SetTrack(
	CEventTrack *track)
{
	if (m_track != track)
	{
		if (m_track != NULL)
			m_track->RemoveObserver(this);

		m_track = track;
		if (m_track)
		{
			m_track->AddObserver(this);
			if (LockLooper())
			{
				m_destField->SetEnabled(true);
				m_editButton->SetEnabled(true);
				m_deleteButton->SetEnabled(true);
				UnlockLooper();
			}

			if (m_doc != &m_track->Document())
				SetDocument(&m_track->Document());
		}
		else
		{
			if (LockLooper())
			{
				m_destField->SetEnabled(false);
				m_editButton->SetEnabled(false);
				m_deleteButton->SetEnabled(false);
				UnlockLooper();
			}

			SetDocument(NULL);
		}
	}
}

// ---------------------------------------------------------------------------
//	Operations

void
CDestinationListView::SubjectUpdated(
	BMessage *message)
{
	// destination added or removed ?
	int32 docAttrs = 0;
	if (message->FindInt32("DocAttrs", &docAttrs) == B_OK)
	{
		if (docAttrs & CMeVDoc::Update_AddDest)
		{
			int32 destID;
			if (message->FindInt32("DestID", &destID) == B_OK)
				_destinationAdded(destID);
		}
		if (docAttrs & CMeVDoc::Update_DelDest)
		{
			int32 originalIndex;
			if (message->FindInt32("original_index", &originalIndex) == B_OK)
				_destinationRemoved(originalIndex);
		}
		return;
	}

	// destination properties changed ?
	int32 destAttrs = 0;
	if (message->FindInt32("DestAttrs", &destAttrs) == B_OK)
	{
		int32 destID = -1;
		if (message->FindInt32("DestID", &destID) != B_OK)
			return;

		if ((destAttrs & CDestination::Update_Name)
		 || (destAttrs & CDestination::Update_Flags)
		 || (destAttrs & CDestination::Update_Color))
		{
			_destinationChanged(destID);
		}
		return;
	}

	// part selection changed ?
	CReadLock lock(m_track);
	if (m_track && (m_track->SelectionType() != CTrack::Select_None))
	{
		const Event *event = m_track->CurrentEvent();
		if (event->HasProperty(Event::Prop_Channel))
		{
			int32 destID = event->GetVChannel();

			CWriteLock lock(m_doc);
			m_doc->SetDefaultAttribute(EvAttr_Channel, destID);
			int32 index = m_doc->IndexOf(m_doc->FindDestination(destID));

			BMenuItem *item = m_destMenu->ItemAt(index + 2);
			if (item)
				item->SetMarked(true);
			m_destMenu->Invalidate();
			return;
		}
	}
}

// ---------------------------------------------------------------------------
//	BView Implementation

void
CDestinationListView::AttachedToWindow()
{
	m_editButton->SetTarget(this);
	m_deleteButton->SetTarget(this);
	m_destMenu->SetTargetForItems(this);
}

void
CDestinationListView::MessageReceived(
	BMessage *msg)
{
	switch (msg->what)
	{
		case DESTINATION_SELECTED:
		{
			int32 destID;
			if (msg->FindInt32("destination_id", &destID) != B_OK)
				return;

			BMenuItem *item = NULL;
			if (msg->FindPointer("source", (void **)&item) != B_OK)
				return;
			item->SetMarked(true);

			if (m_track && (destID <= Max_Destinations))
			{
				uint8 channel = static_cast<uint8>(destID);
				StSubjectLock lock(*m_doc, Lock_Exclusive);
				// Set attribute for newly created events
				m_doc->SetDefaultAttribute(EvAttr_Channel, channel);
				if (m_track->SelectionType() != CEventTrack::Select_None)
				{
					// Modify any selected events
					EventOp *op = new ChannelOp(channel);
					m_track->ModifySelectedEvents(NULL, *op,
												  "Change Destination");
					CRefCountObject::Release(op);

					// Do audio feedback, if enabled
					if (gPrefs.feedbackAdjustMask & CGlobalPrefs::FB_Channel)
					{
						CPlayerControl::DoAudioFeedback(m_doc, EvAttr_Channel,
														channel,
														m_track->CurrentEvent());
					}
				}
			}
			break;
		}
		case CREATE_DESTINATION:
		{
			StSubjectLock lock(*m_doc, Lock_Exclusive);

			CDestination *dest = Document()->NewDestination();
			int32 id = dest->ID();
			Document()->SetDefaultAttribute(EvAttr_Channel, id);
			BRect r(40, 40, 300, 220);
			m_modifierMap[id] = new CDestinationModifier(r, id, m_doc, this);
			m_modifierMap[id]->Show();

			break;
		}
		case EDIT_DESTINATION:
		{
			StSubjectLock lock(*m_doc, Lock_Shared);

			int32 id = m_doc->GetDefaultAttribute(EvAttr_Channel);
			if (m_modifierMap[id] != NULL)
			{
				m_modifierMap[id]->Lock();
				m_modifierMap[id]->Activate();
				m_modifierMap[id]->Unlock();
			}
			else 
			{
				BRect r;
				r.Set(120, 120, 260, 300);
				m_modifierMap[id] = new CDestinationModifier(r, id, m_doc,
															 this);
				m_modifierMap[id]->Show();
			}
			break;
		}
		case DELETE_DESTINATION:
		{	
			int32 id = m_doc->GetDefaultAttribute(EvAttr_Channel);
			CDestination *dest = Document()->FindDestination(id);

			CDestinationDeleteUndoAction *undoAction;
			undoAction = new CDestinationDeleteUndoAction(dest);
			Document()->AddUndoAction(undoAction);

			if (m_modifierMap[id]!=NULL)
			{
				m_modifierMap[id]->Lock();	
				m_modifierMap[id]->Quit();
				m_modifierMap[id] = NULL;
			}
			break;
		}
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
		case CObservable::UPDATED:
		{
			SubjectUpdated(msg);
			break;
		}
		default:
		{
			BView::MessageReceived(msg);
			break;
		}
	}
}

// ---------------------------------------------------------------------------
//	CObserver Implementation

bool
CDestinationListView::Released(
	CObservable *subject)
{
	D_OBSERVE(("CDestinationListView<%p>::Released()\n", this));

	bool released = false;

	if (LockLooper())
	{
		if (subject == m_doc)
		{
			SetDocument(NULL);
			released = true;
		}
		else if (subject == m_track)
		{
			SetTrack(NULL);
			released = true;
		}
		UnlockLooper();
	}

	return released;
}

void
CDestinationListView::Updated(
	BMessage *message)
{
	if (Window())
		Window()->PostMessage(message, this);
}

// ---------------------------------------------------------------------------
//	Internal Operations

void
CDestinationListView::_destinationAdded(
	int32 id)
{
	CDestination *dest = Document()->FindDestination(id);
	if (dest)
	{
		dest->AddObserver(this);

		m_doc->ReadLock();
		int32 index = m_doc->IndexOf(dest);
		int32 current = m_doc->GetDefaultAttribute(EvAttr_Channel);
		m_doc->ReadUnlock();

		BMessage *message = new BMessage(DESTINATION_SELECTED);
		dest->ReadLock();
		message->AddInt32("destination_id", dest->ID());
		BBitmap	*icon = new BBitmap(BRect(0.0, 0.0, B_MINI_ICON - 1.0,
										  B_MINI_ICON - 1.0), B_CMAP8);
		if (dest->GetIcon(B_MINI_ICON, icon) != B_OK)
		{
			delete icon;
			icon = NULL;
		}
		CIconMenuItem *item = new CIconMenuItem(dest->Name(),
												message, icon);
		dest->ReadUnlock();
		item->SetTarget(this);
		m_destMenu->AddItem(item, index + 2);
		if (id == current)
			item->SetMarked(true);
	}
}

void
CDestinationListView::_destinationChanged(
	int32 id)
{
	CDestination *dest = Document()->FindDestination(id);
	if (dest)
	{
		m_doc->ReadLock();
		int32 index = m_doc->IndexOf(dest) + 2;
		m_doc->ReadUnlock();

		CIconMenuItem *item = (CIconMenuItem *)m_destMenu->RemoveItem(index);
		bool marked = item->IsMarked();
		if (item)
			delete item;

		BMessage *message = new BMessage(DESTINATION_SELECTED);
		dest->ReadLock();
		message->AddInt32("destination_id", dest->ID());
		BBitmap	*icon = new BBitmap(BRect(0.0, 0.0, B_MINI_ICON - 1.0,
										  B_MINI_ICON - 1.0), B_CMAP8);
		if (dest->GetIcon(B_MINI_ICON, icon) != B_OK)
		{
			delete icon;
			icon = NULL;
		}
		item = new CIconMenuItem(dest->Name(), message, icon);
		dest->ReadUnlock();
		item->SetTarget(this);
		m_destMenu->AddItem(item, index);
		if (marked)
			item->SetMarked(true);
	}
}

void
CDestinationListView::_destinationRemoved(
	int32 originalIndex)
{
	BMenuItem *item = m_destMenu->RemoveItem(originalIndex + 2);
	if (item)
		delete item;

	StSubjectLock lock(*m_doc, Lock_Shared);
	int32 current = m_doc->GetDefaultAttribute(EvAttr_Channel);
	CDestination *dest = m_doc->FindDestination(current);
	if (dest && !dest->IsDeleted())
	{
		int32 index = m_doc->IndexOf(dest);
		item = m_destMenu->ItemAt(index + 2);
		if (item)
			item->SetMarked(true);
	}
	m_destField->Invalidate();
}

// END - DestinationListView.cpp
