/* ===================================================================== *
 * TrackListView.cpp (MeV/User Interface)
 * ===================================================================== */

#include "TrackListView.h"

#include "EventTrack.h"
#include "Idents.h"
#include "LinearWindow.h"
#include "MeVDoc.h"
#include "ScreenUtils.h"
#include "TrackListItem.h"
#include "TrackListWindow.h"

// Interface Kit
#include <MenuItem.h>
#include <PopUpMenu.h>
// Support Kit
#include <Debug.h>
#include <String.h>

// Debugging Macros
#define D_ALLOC(x) //PRINT (x)		// Constructor/Destructor
#define D_OPERATION(x) //PRINT(x)	// Operations
#define D_HOOK(x) //PRINT (x)		// BListView Implementation
#define D_MESSAGE(x) //PRINT (x)	// MessageReceived()

// ---------------------------------------------------------------------------
// Constructor/Destructor

CTrackListView::CTrackListView(
	BRect frame,
	CMeVDoc *doc)
	:	BListView(frame, "TrackListView",
				  B_SINGLE_SELECTION_LIST,
				  B_FOLLOW_ALL_SIDES,
				  B_WILL_DRAW | B_FRAME_EVENTS),
		m_doc(doc),
		m_contextMenu(NULL),
		m_reordering(false),
		m_currentReorderMark(0.0, 0.0, -1.0, -1.0),
		m_lastReorderIndex(-1),
		m_lastReorderMark(0.0, 0.0, -1.0, -1.0)
{
	D_ALLOC(("CTrackListView::CTrackListView()\n"));

}
	
CTrackListView::~CTrackListView()
{
	D_ALLOC(("CTrackListView::~CTrackListView()\n"));

}

// ---------------------------------------------------------------------------
// BListView Implementation

void
CTrackListView::AttachedToWindow()
{
	D_HOOK(("CTrackListView::AttachedToWindow()\n"));

	BuildTrackList();
}

void
CTrackListView::Draw(
	BRect updateRect)
{
	D_HOOK(("CTrackListView::Draw()\n"));

	SetHighColor(ViewColor());
	FillRect(m_lastReorderMark, B_SOLID_HIGH);

	if (m_reordering)
	{
		SetHighColor(64, 64, 64, 255);
		FillRect(m_currentReorderMark, B_SOLID_HIGH);
	}
	
	BListView::Draw(updateRect);
}

void
CTrackListView::GetPreferredSize(
	float *width,
	float *height)
{
	D_HOOK(("CTrackListView::GetPreferredSize()\n"));

	*width = 0.0;
	*height = 0.0;
	for (int32 i = 0; i < CountItems(); i++)
	{
		CTrackListItem *item = dynamic_cast<CTrackListItem *>(ItemAt(i));
		if (item)
		{
			if ((item->Width() + 6.0) > *width)
				*width = item->Width() + 6.0;
			*height += item->Height() + 1.0;
		}
	}
	*width += 15.0;
}

bool
CTrackListView::InitiateDrag(
	BPoint point,
	int32 index,
	bool wasSelected)
{
	D_HOOK(("CTrackListView::InitiateDrag()\n"));

	CTrackListItem *item = dynamic_cast<CTrackListItem *>(ItemAt(index));
	if (item)
	{
		BMessage dragMsg(MeVDragMsg_ID);
		dragMsg.AddInt32("Type", DragTrack_ID);
		dragMsg.AddInt32("TrackID", item->GetTrackID());
		dragMsg.AddInt32("Index", index);
		dragMsg.AddPointer("Document", m_doc);
		point -= ItemFrame(index).LeftTop();
		DragMessage(&dragMsg, item->GetDragBitmap(), B_OP_ALPHA, point);
		return true;
	}
	return false;
}

void
CTrackListView::KeyDown(
	const char *bytes,
	int32 numBytes)
{
	D_HOOK(("CTrackListView::KeyDown()\n"));

	switch (bytes[0])
	{
		case B_ENTER:
		{
			Window()->PostMessage(CTrackListItem::EDIT_TRACK, this);
			break;
		}
		case B_DELETE:
		{
			Window()->PostMessage(CTrackListItem::DELETE_TRACK, this);
			break;
		}
		default:
		{
			BListView::KeyDown(bytes, numBytes);
		}
	}
}

void
CTrackListView::MessageReceived(
	BMessage *message)
{
	D_MESSAGE(("CTrackListView::MessageReceived()\n"));

	switch(message->what)
	{
		case CObservable::UPDATED:
		{
			int32 trackHint, docHint;
			int32 trackID;
		
			if (message->FindInt32("TrackID", &trackID) != B_OK)
				trackID = -1;
			if (message->FindInt32("TrackAttrs", &trackHint) != B_OK)
				trackHint = 0;
			if (message->FindInt32("DocAttrs", &docHint ) != B_OK)
				docHint = 0;
		
			if (trackHint != 0 || docHint != 0)
			{
				if (trackID >= 0)
				{
					for (int i = 0; i < CountItems(); i++)
					{
						CTrackListItem *item = dynamic_cast<CTrackListItem *>
											   (ItemAt(i));
						if (item && (item->GetTrackID() == trackID))
						{
							if (trackHint & (CTrack::Update_Name | CTrack::Update_Flags))
							{
								item->Update(this, be_plain_font);
								InvalidateItem(i);
							}
						}
					}
				}
				
				if ((docHint & CMeVDoc::Update_AddTrack)
				 || (docHint & CMeVDoc::Update_DelTrack)
				 || (docHint & CMeVDoc::Update_TrackOrder))
				{
					BuildTrackList();
				}
			}
			break;
		}
		case CTrackListItem::EDIT_TRACK:
		{
			D_MESSAGE((" -> CTrackListItem::EDIT_TRACK\n"));

			int32 selection = CurrentSelection();
			if (selection >= 0)
			{
				CTrackListItem *item = (CTrackListItem *)ItemAt(selection);
				if (item)
					m_doc->ShowWindowFor(item->GetTrack());
			}
			break;
		}
		case CTrackListItem::MUTE_TRACK:
		{
			D_MESSAGE((" -> CTrackListItem::MUTE_TRACK\n"));

			int32 selection = CurrentSelection();
			if (selection >= 0)
			{
				CTrackListItem *item = (CTrackListItem *)ItemAt(selection);
				CTrack *track = item->GetTrack();
				track->SetMuted(!track->Muted());
				track->NotifyUpdate(CTrack::Update_Name, NULL);
				m_doc->SetModified();
			}
			break;
		}
		case CTrackListItem::SOLO_TRACK:
		{
			D_MESSAGE((" -> CTrackListItem::SOLO_TRACK\n"));

			int32 selection = CurrentSelection();
			if (selection >= 0)
			{
				// nyi
			}
			break;
		}
		case CTrackListItem::DELETE_TRACK:
		{
			D_MESSAGE((" -> CTrackListItem::DELETE_TRACK\n"));

			int32 selection = CurrentSelection();
			if (selection >= 0)
			{
				CTrackListItem *item = (CTrackListItem *)ItemAt(selection);
				CTrack *track = item->GetTrack();
				CTrackDeleteUndoAction *undoAction;
				undoAction = new CTrackDeleteUndoAction(track);
				m_doc->AddUndoAction(undoAction);
			}
			break;
		}
		case CTrackListItem::RENAME_TRACK:
		{
			D_MESSAGE((" -> CTrackListItem::RENAME_TRACK\n"));

			int32 index = CurrentSelection();
			if (index < 0)
				return;
			CTrackListItem *item = dynamic_cast<CTrackListItem *>(ItemAt(index));
			if (item)
				item->StartEdit(this, index, ItemFrame(index));
			break;
		}
		case CTrackListItem::TRACK_NAME_EDITED:
		{
			D_MESSAGE((" -> CTrackListItem::TRACK_NAME_EDITED\n"));

			int32 index;
			BString name;
			if (message->FindInt32("index", &index) != B_OK)
				return;
			CTrackListItem *item = dynamic_cast<CTrackListItem *>(ItemAt(index));
			if (!item)
				return;
			item->StopEdit();
			if (message->FindString("text", &name) == B_OK)
			{
				// name changed
				CTrack *track = item->GetTrack();
				CTrackRenameUndoAction *undoAction;
				undoAction = new CTrackRenameUndoAction(track, name);
				m_doc->AddUndoAction(undoAction);
			}
			MakeFocus(true);
			break;
		}
		case MeVDragMsg_ID:
		{
			D_MESSAGE((" -> MeVDragMsg_ID\n"));

			int32 type;
			if ((message->FindInt32("Type", &type) != B_OK)
			 || (type != DragTrack_ID))
				return;

			// change of track order requested
			int32 index;
			message->FindInt32("Index", &index);
			// find the target index
			BPoint point = ConvertFromScreen(message->DropPoint());
			point.y -= ItemFrame(0).Height();
			int32 dragIndex;
			if (point.y < 0.0) 
				dragIndex = -1;
			else 
			{
				dragIndex = IndexOf(point);
				if (dragIndex < 0)	
					dragIndex = CountItems() - 1;
			}
			// reorder tracks
			if (dragIndex < index)
			{
				m_doc->ChangeTrackOrder(index, dragIndex + 1);
				m_doc->SetModified();
			}
			else if (dragIndex > index)
			{
				m_doc->ChangeTrackOrder(index, dragIndex);
				m_doc->SetModified();
			}

			break;
		}
		default:
		{
			BListView::MessageReceived(message);
			break;
		}
	}
}

void
CTrackListView::MouseDown(
	BPoint point)
{
	D_HOOK(("CTrackListView::MouseDown()\n"));

	Window()->Activate();

	int32 buttons = B_PRIMARY_MOUSE_BUTTON;
	if (Window()->CurrentMessage()->FindInt32("buttons", &buttons) != B_OK)
		return;
	int32 clicks = 0;
	if (Window()->CurrentMessage()->FindInt32("clicks", &clicks) != B_OK)
		return;

	if ((buttons == B_SECONDARY_MOUSE_BUTTON) || (modifiers() & B_CONTROL_KEY))
	{
		int32 index = IndexOf(point);
		Select(index);
		ShowContextMenu(point);
	}
	else
	{
		if (clicks == 2)
		{
			BRect dblClickRect = BRect(m_lastClickPoint - BPoint(2.0, 2.0),
									   m_lastClickPoint + BPoint(2.0, 2.0));
			if ((buttons == m_lastClickButton)
			 && (dblClickRect.Contains(point)))
			{
				Window()->PostMessage(CTrackListItem::EDIT_TRACK, this);
				return;
			}
		}
		m_lastClickPoint = point;
		m_lastClickButton = buttons;
		BListView::MouseDown(point);
	}
}

void
CTrackListView::MouseMoved(
	BPoint point,
	uint32 transit,
	const BMessage *message)
{
	D_HOOK(("CTrackListView::MouseMoved()\n"));

	if (!message || (message->what != MeVDragMsg_ID)) {
		BListView::MouseMoved(point, transit, message);
		return;
	}

	int32 type;
	if ((message->FindInt32("Type", &type) != B_OK)
	 || (type != DragTrack_ID)) {
		BListView::MouseMoved(point, transit, message);
		return;
	}

	int32 index;
	message->FindInt32("Index", &index);

	// calculate target index
	point.y -= ItemFrame(0).Height();
	int32 dragIndex;
	if (point.y < 0.0)
		dragIndex = -1;
	else
	{
		dragIndex = IndexOf(point);
		if (dragIndex < 0)	
			dragIndex = CountItems() - 1;
	}

	if ((transit == B_OUTSIDE_VIEW)
	 || (transit == B_EXITED_VIEW))
	{
		// disable reordering, pointer device has left window
		if (m_reordering)
		{
			m_reordering = false;
			m_lastReorderMark = m_currentReorderMark;
			m_currentReorderMark = BRect(0.0, 0.0, -1.0, -1.0);
			Invalidate(m_lastReorderMark);
		}
	}
	else if (dragIndex != m_lastReorderIndex)
	{
		// only update if necessary
		m_lastReorderMark = m_currentReorderMark;
		if (dragIndex < 0)
		{
			// special case for moving before first item
			m_currentReorderMark = ItemFrame(0);
			m_currentReorderMark.bottom = m_currentReorderMark.top + 1.0;
		}
		else
		{
			m_currentReorderMark = ItemFrame(dragIndex);
			m_currentReorderMark.top = m_currentReorderMark.bottom;
			m_currentReorderMark.bottom += 1.0;
		}
		m_reordering = true;
		Invalidate(m_lastReorderMark);		
		Invalidate(m_currentReorderMark);
	}
	m_lastReorderIndex = dragIndex;

	BListView::MouseMoved(point, transit, message);
}

void
CTrackListView::MouseUp(
	BPoint point)
{
	D_HOOK(("CTrackListView::MouseUp()\n"));

	// disable reordering, actually changing the track order
	// will be done in MessageReceived() when drag message is
	// received
	if (m_reordering)
	{
		m_reordering = false;
		m_lastReorderMark = m_currentReorderMark;
		m_currentReorderMark = BRect(0.0, 0.0, -1.0, -1.0);
		Invalidate(m_lastReorderMark);
	}

	BListView::MouseUp(point);
}

// ---------------------------------------------------------------------------
// CObserver Implementation

bool
CTrackListView::Released(
	CObservable *subject)
{
	bool released = false;

	if (LockLooper())
	{
		if (subject == m_doc)
		{
			m_doc->RemoveObserver(this);
			m_doc = NULL;
			released = true;
		}
		UnlockLooper();
	}

	return released;
}

// ---------------------------------------------------------------------------
// Operations

void
CTrackListView::BuildTrackList()
{
	// remember selection
	int32 selectedTrackID = -1;
	CTrackListItem *selectedItem = dynamic_cast<CTrackListItem *>
								   (ItemAt(CurrentSelection()));
	if (selectedItem)
		selectedTrackID = selectedItem->GetTrackID();

	// clear list
	DeselectAll();
	while (CountItems() > 0)
	{
		CTrackListItem *item = dynamic_cast<CTrackListItem *>(RemoveItem((int32)0));
		if (item)
			delete item;
	}

	if (!m_doc)
		return;

	// re-fill
	for (int i = 0; i < m_doc->CountTracks(); i++)
	{
		CTrack *track = m_doc->TrackAt(i);
		if (!track->Deleted())
		{
			AddItem(new CTrackListItem(track));
			if (track->GetID() == selectedTrackID)
				Select(i);
		}
	}

	Invalidate();
}

void
CTrackListView::SetDocument(
	CMeVDoc *doc)
{
	D_OPERATION(("CTrackListView::SetDocument()\n"));

	if (doc != m_doc)
	{	
		if (m_doc != NULL)
			m_doc->RemoveObserver(this);

		m_doc = doc;
		if (m_doc != NULL)
			m_doc->AddObserver(this);

		BuildTrackList();
	}
}

void
CTrackListView::ShowContextMenu(
	BPoint point)
{
	D_OPERATION(("CTrackListView::ShowContextMenu()"));

	int32 index = IndexOf(point);
	CTrackListItem *item = dynamic_cast<CTrackListItem *>(ItemAt(index));
	if (!item)
		return;

	if (!m_contextMenu)
	{
		m_contextMenu = new BPopUpMenu("TrackList ContextMenu", false,
									   false, B_ITEMS_IN_COLUMN);
		m_contextMenu->SetFont(be_plain_font);

		m_contextMenu->AddItem(new BMenuItem("Edit",
											 new BMessage(CTrackListItem::EDIT_TRACK)));
		m_contextMenu->AddSeparatorItem();
		m_contextMenu->AddItem(new BMenuItem("Record" B_UTF8_ELLIPSIS,
											 new BMessage(CTrackListItem::RECORD_TRACK),
											 'R'));
		m_contextMenu->AddItem(new BMenuItem("Mute",
											 new BMessage(CTrackListItem::MUTE_TRACK),
											 'M'));
		m_contextMenu->AddItem(new BMenuItem("Solo",
											 new BMessage(CTrackListItem::SOLO_TRACK),
											 'S'));
		m_contextMenu->AddSeparatorItem();
		m_contextMenu->AddItem(new BMenuItem("Rename",
											 new BMessage(CTrackListItem::RENAME_TRACK),
											 'E'));
		m_contextMenu->AddItem(new BMenuItem("Delete",
											 new BMessage(CTrackListItem::DELETE_TRACK),
											 'T'));

		m_contextMenu->SetTargetForItems(this);
	}

	m_contextMenu->FindItem(CTrackListItem::RECORD_TRACK)->SetEnabled(false);

	if (item->GetTrack()->Muted())
		m_contextMenu->FindItem(CTrackListItem::MUTE_TRACK)->SetMarked(true);
	else
		m_contextMenu->FindItem(CTrackListItem::MUTE_TRACK)->SetMarked(false);

	m_contextMenu->FindItem(CTrackListItem::SOLO_TRACK)->SetEnabled(false);

	ConvertToScreen(&point);
	point -= BPoint(1.0, 1.0);
	m_contextMenu->Go(point, true, true, true);
}

// END - TrackListView.cpp
