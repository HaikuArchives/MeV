/* ===================================================================== *
 * TrackListView.cpp (MeV/User Interface)
 * ===================================================================== */

#include "TrackListView.h"
#include "MeVDoc.h"
#include "Idents.h"
// Engine
#include "EventTrack.h"
// Framework
#include "ScreenUtils.h"
// UI
#include "LinearWindow.h"
#include "TrackListItem.h"

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
	:	BListView(frame, "SequenceList",
				  B_SINGLE_SELECTION_LIST,
				  B_FOLLOW_ALL_SIDES,
				  B_WILL_DRAW | B_FRAME_EVENTS),
		m_doc(doc),
		m_contextMenu(NULL)
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

	Window()->AddShortcut('E', B_COMMAND_KEY,
						  new BMessage(CTrackListItem::EDIT_TRACK), this);
	Window()->AddShortcut('E', B_COMMAND_KEY | B_SHIFT_KEY,
						  new BMessage(CTrackListItem::RENAME_TRACK), this);
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
CTrackListView::MessageReceived(
	BMessage *message)
{
	D_MESSAGE(("CTrackListView::MessageReceived()\n"));

	switch(message->what)
	{
		case CTrackListItem::EDIT_TRACK:
		{
			D_MESSAGE((" -> CTrackListItem::EDIT_TRACK\n"));

			int32 selection = CurrentSelection();
			if (selection >= 0)
			{
				CTrackListItem *item = (CTrackListItem *)ItemAt(selection);
				CLinearWindow *window;
				window = new CLinearWindow(UScreenUtils::StackOnScreen(540, 370),
										   *m_doc, (CEventTrack *)item->GetTrack());
				window->Show();
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
				track->AddUndoAction(undoAction);
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
				track->SetName(name.String());
				track->NotifyUpdate(CTrack::Update_Name, NULL);
				m_doc->SetModified();
			}
			MakeFocus(true);
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
	D_MESSAGE(("CTrackListView::MouseDown()\n"));

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

// ---------------------------------------------------------------------------
// Operations

void
CTrackListView::BuildTrackList()
{
	// remember selection
	int32 sel = CurrentSelection();

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
		}
	}

	Select(sel);
	Invalidate();
}

void
CTrackListView::SetDocument(
	CMeVDoc *doc)
{
	D_OPERATION(("CTrackListView::SetDocument()\n"));

	if (doc != m_doc)
	{	
		m_doc = doc;
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
											 new BMessage(CTrackListItem::EDIT_TRACK),
											 'E'));
		m_contextMenu->AddSeparatorItem();
		m_contextMenu->AddItem(new BMenuItem("Mute",
											 new BMessage(CTrackListItem::MUTE_TRACK)));
		m_contextMenu->AddItem(new BMenuItem("Solo",
											 new BMessage(CTrackListItem::SOLO_TRACK)));
		m_contextMenu->AddSeparatorItem();
		m_contextMenu->AddItem(new BMenuItem("Rename",
											 new BMessage(CTrackListItem::RENAME_TRACK),
											 'E', B_SHIFT_KEY));
		m_contextMenu->AddItem(new BMenuItem("Delete",
											 new BMessage(CTrackListItem::DELETE_TRACK),
											 'T'));

		m_contextMenu->SetTargetForItems(this);
	}

	if (item->GetTrack()->Muted())
		m_contextMenu->FindItem("Mute")->SetMarked(true);
	else
		m_contextMenu->FindItem("Mute")->SetMarked(false);

	m_contextMenu->FindItem("Solo")->SetEnabled(false);

	ConvertToScreen(&point);
	point -= BPoint(1.0, 1.0);
	m_contextMenu->Go(point, true, true, true);
}

// END - TrackListView.cpp
