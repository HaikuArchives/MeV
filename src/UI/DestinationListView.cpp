/* ===================================================================== *
 * DestinationListView.cpp (MeV/UI)
 * ===================================================================== */

#include "DestinationListView.h"

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
#include <MenuField.h>
#include <PopUpMenu.h>
// Storage Kit
#include <Mime.h>
// Support Kit
#include <Debug.h>

// Debugging Macros
#define D_ALLOC(x) //PRINT(x)		// Constructor/Destructor
#define D_HOOK(x) //PRINT(x)		// CAppWindow Implementation
#define D_INTERNAL(x) //PRINT(x)	// Internal Operations

// ---------------------------------------------------------------------------
//	Constructor/Destructor

CDestinationListView::CDestinationListView(
	BRect frame,
	uint32 resizingMode,
	uint32 flags)
	:	BView(frame, "", resizingMode, flags),
		CObserver(),
		m_doc(NULL),
		m_track(NULL)
{
	D_ALLOC(("CDestinationListView::CDestinationListView()\n"));

	BRect rect(Bounds());
	BBox *box = new BBox(rect);
	AddChild(box);

	// add CDestination menu-field
	rect.InsetBy(5.0, 5.0);
	m_destMenu = new BPopUpMenu("(None)");
	m_destMenu->SetLabelFromMarked(true);
	m_destField = new BMenuField(rect, "Destination", "Destination: ", 
								 m_destMenu);
	m_destField->SetDivider(be_plain_font->StringWidth("Destination:  "));
	m_destField->SetEnabled(false);
	box->AddChild(m_destField);

	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
}

CDestinationListView::~CDestinationListView()
{
	D_ALLOC(("CDestinationListView::~CDestinationListView()\n"));

	if (m_doc != NULL)
	{
		CWriteLock lock(m_doc);
		CDestination *dest = NULL;
		int32 index = 0;
		while ((dest = m_doc->GetNextDestination(&index)) != NULL)
			dest->RemoveObserver(this);
	
		m_doc->RemoveObserver(this);
		m_doc = NULL;
	}

	if (m_track != NULL)
	{
		m_track->RemoveObserver(this);
		m_track = NULL;
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

			// clear the destination menu
			BMenuItem *item;
			while ((item = m_destMenu->RemoveItem((int32)0)) != NULL)
				delete item;
			m_destField->Invalidate();
		}

		m_doc = document;
		if (m_doc != NULL)
		{
			m_doc->AddObserver(this);

			CReadLock lock(m_doc);
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
		if (m_track != NULL)
		{
			m_track->AddObserver(this);
			m_destField->SetEnabled(true);

			if (m_doc != &m_track->Document())
				SetDocument(&m_track->Document());
		}
		else
		{
			m_destField->SetEnabled(false);
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
		const CEvent *event = m_track->CurrentEvent();
		if (event->HasProperty(CEvent::Prop_Channel))
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
				CWriteLock lock(m_doc);
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
		m_destMenu->AddItem(item, index);
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
		int32 index = m_doc->IndexOf(dest);
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
	BMenuItem *item = m_destMenu->RemoveItem(originalIndex);
	if (item)
		delete item;

	CReadLock lock(m_doc);
	int32 current = m_doc->GetDefaultAttribute(EvAttr_Channel);
	CDestination *dest = m_doc->FindDestination(current);
	if (dest && !dest->IsDeleted())
	{
		int32 index = m_doc->IndexOf(dest);
		item = m_destMenu->ItemAt(index);
		if (item)
			item->SetMarked(true);
	}
	m_destField->Invalidate();
}

// END - DestinationListView.cpp
