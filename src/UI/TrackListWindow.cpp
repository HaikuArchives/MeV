/* ===================================================================== *
 * TrackListWindow.cpp (MeV/UI)
 * ===================================================================== */

#include "TrackListWindow.h"

#include "TrackListItem.h"
#include "TrackListView.h"
#include "QuickKeyMenuItem.h"
#include "MeVDoc.h"
#include "Idents.h"
#include "EventTrack.h"
#include "ScreenUtils.h"

// Interface Kit
#include <Menu.h>
#include <MenuBar.h>
#include <MenuItem.h>
#include <Screen.h>
#include <ScrollBar.h>
// Support Kit
#include <Autolock.h>
#include <Debug.h>
#include <String.h>

// Debugging Macros
#define D_ALLOC(x) //PRINT(x)		// Constructor/Destructor
#define D_OPERATION(x) //PRINT(x)	// Operations
#define D_HOOK(x) //PRINT(x)		// BListView Implementation
#define D_MESSAGE(x) //PRINT(x)		// MessageReceived()

// ---------------------------------------------------------------------------
// Class Data Initialization

const BRect
CTrackListWindow::DEFAULT_DIMENSIONS(0.0, 0.0, 150.0, 150.0);

// ---------------------------------------------------------------------------
// Constructor/Destructor

CTrackListWindow::CTrackListWindow(
	BPoint position,
	CWindowState &state)
	:	CAppWindow(state,
				   BRect(position.x, position.y,
						 position.x + DEFAULT_DIMENSIONS.Width(),
						 position.y + DEFAULT_DIMENSIONS.Height()),
				   "Parts", B_FLOATING_WINDOW,
				   B_WILL_ACCEPT_FIRST_CLICK | B_ASYNCHRONOUS_CONTROLS,
				   B_CURRENT_WORKSPACE),
		m_doc(NULL),
		m_zoomed(false),
		m_zooming(false)
{
	D_ALLOC(("CTrackListWindow::CTrackListWindow()\n"));

	BRect rect;

	_addMenuBar();

	rect = Bounds();
	rect.top = KeyMenuBar()->Frame().bottom + 1.0;
	rect.right -= B_V_SCROLL_BAR_WIDTH;
	m_listView = new CTrackListView(rect, m_doc);
	AddChild(m_listView);

	rect = Bounds();
	rect.top = KeyMenuBar()->Frame().bottom;
	rect.right += 1.0;
	rect.left = rect.right - B_V_SCROLL_BAR_WIDTH;
	rect.bottom += 1.0;
	BScrollBar *scrollBar = new BScrollBar(rect, "", m_listView,
										   0.0, 0.0, B_VERTICAL);
	scrollBar->SetRange(0.0, 0.0);
	AddChild(scrollBar);

	// adjust mix/max size limits
	float minWidth, maxWidth, minHeight, maxHeight;
	GetSizeLimits(&minWidth, &maxWidth, &minHeight, &maxHeight);
	minHeight = KeyMenuBar()->Frame().Height() + 6 * B_H_SCROLL_BAR_HEIGHT;
	SetSizeLimits(minWidth, maxWidth, minHeight, maxHeight);
}

CTrackListWindow::~CTrackListWindow()
{
	D_ALLOC(("CTrackListWindow::~CTrackListWindow()\n"));

}

// ---------------------------------------------------------------------------
// CAppWindow Implementation

void
CTrackListWindow::FrameResized(
	float width,
	float height)
{
	D_HOOK(("CTrackListWindow::FrameResized()\n"));

	if (!m_zooming)
	{
		m_zoomed = false;
	}
}

void
CTrackListWindow::MenusBeginning()
{
	D_HOOK(("CTrackListWindow::MenusBeginning()\n"));

	if (!m_doc)
	{
		KeyMenuBar()->SetEnabled(false);
		return;
	}
	else
		KeyMenuBar()->SetEnabled(true);	

	BMenuItem *item;
	BString itemLabel;
	const char *description = NULL;

	// Set up Undo menu
	item = KeyMenuBar()->FindItem(MENU_UNDO);
	if (m_doc->CanUndo())
	{
		itemLabel = "Undo";
		description = m_doc->UndoDescription();
		if (description)
			itemLabel << ": " << description;
		item->SetLabel(itemLabel.String());
		item->SetEnabled(true);
	}
	else
	{
		item->SetLabel("Undo");
		item->SetEnabled(false);
	}

	// Set up Redo menu
	item = KeyMenuBar()->FindItem(MENU_REDO);
	description = NULL;
	if (m_doc->CanRedo())
	{
		itemLabel = "Redo";
		description = m_doc->RedoDescription();
		if (description)
			itemLabel << ": " << description;
		item->SetLabel(itemLabel.String());
		item->SetEnabled(true);
	}
	else
	{
		item->SetLabel("Redo");
		item->SetEnabled(false);
	}

	CTrack *track = NULL;
	int32 sel;
	if ((sel = m_listView->CurrentSelection()) >= 0)
	{
		track = dynamic_cast<CTrackListItem *>
				(m_listView->ItemAt(sel))->GetTrack();
	}
	item = KeyMenuBar()->FindItem(CTrackListItem::EDIT_TRACK);
	if (item)
	{
		item->SetEnabled(track);
		item->SetTarget(m_listView);
	}
	item = KeyMenuBar()->FindItem(CTrackListItem::RECORD_TRACK);
	if (item)
	{
		item->SetEnabled(false); // nyi
		item->SetTarget(m_listView);
	}
	item = KeyMenuBar()->FindItem(CTrackListItem::MUTE_TRACK);
	if (item)
	{
		item->SetEnabled(track);
		if (track)
			item->SetMarked(track->Muted());
		item->SetTarget(m_listView);
	}
	item = KeyMenuBar()->FindItem(CTrackListItem::SOLO_TRACK);
	if (item)
	{
		item->SetEnabled(false); // nyi
		if (track)
			item->SetMarked(track->Solo());
		item->SetTarget(m_listView);
	}
	item = KeyMenuBar()->FindItem(CTrackListItem::RENAME_TRACK);
	if (item)
	{
		item->SetEnabled(track);
		item->SetTarget(m_listView);
	}
	item = KeyMenuBar()->FindItem(CTrackListItem::DELETE_TRACK);
	if (item)
	{
		item->SetEnabled(track);
		item->SetTarget(m_listView);
	}
}

void
CTrackListWindow::MessageReceived(
	BMessage *message)
{
	D_MESSAGE(("CTrackListWindow::MessageReceived()\n"));

	switch(message->what)
	{
		case MENU_UNDO:
		{
			D_MESSAGE((" -> MENU_UNDO\n"));
			m_doc->Undo();
			break;
		}
		case MENU_REDO:
		{
			D_MESSAGE((" -> MENU_REDO\n"));

			m_doc->Redo();
			break;
		}
		case MENU_CREATE_METERED_TRACK:
		{
			D_MESSAGE((" -> MENU_CREATE_METERED_TRACK\n"));

			m_doc->NewTrack(TrackType_Event, ClockType_Metered);
			m_doc->NotifyUpdate(CMeVDoc::Update_AddTrack, NULL);
			m_doc->SetModified();
			break;
		}
		case CMeVApp::WATCH_TRACK:
		{
			CEventTrack *track = NULL;
			if (message->FindPointer("mev:track", (void **)&track) != B_OK)
				return;
			WatchTrack(track);
			break;
		}
		default:
		{
			CAppWindow::MessageReceived(message);
			break;
		}
	}
}

bool
CTrackListWindow::SubjectReleased(
	CObservable *subject)
{
	if (subject == m_doc)
	{
		m_doc->RemoveObserver(this);
		m_doc = NULL;
		return true;
	}

	return CAppWindow::SubjectReleased(subject);
}

void
CTrackListWindow::Zoom(
	BPoint origin,
	float width,
	float height)
{
	D_HOOK(("CTrackListWindow::Zoom()\n"));

	m_zooming = true;

	BScreen screen(this);
	if (!screen.Frame().Contains(Frame()))
		m_zoomed = false;

	if (!m_zoomed)
	{
		// resize to the ideal size
		m_manualSize = Bounds();
		m_listView->GetPreferredSize(&width, &height);
		m_zoomed = true;
		BRect rect(Frame().LeftTop(),
				   Frame().LeftTop() + BPoint(width + B_V_SCROLL_BAR_WIDTH,
				   							  height + KeyMenuBar()->Frame().Height()));
		rect = UScreenUtils::ConstrainToScreen(rect);
		MoveTo(rect.LeftTop());
		ResizeTo(rect.Width(), rect.Height());
	}
	else
	{
		// resize to the most recent manual size
		ResizeTo(m_manualSize.Width(), m_manualSize.Height());
		m_zoomed = false;
	}
	
}

// ---------------------------------------------------------------------------
// Internal Operations

void
CTrackListWindow::_addMenuBar()
{
	D_OPERATION(("CTrackListWindow::_addMenuBar()\n"));

	BMenuBar *menuBar;
	BMenu *menu;
	BMenuItem *item;

	menuBar = new BMenuBar(Bounds(), "KeyMenuBar");
	menuBar->SetFont(be_plain_font);

	// Create the 'File' menu
	menu = new BMenu("File");
	menu->SetFont(be_plain_font);
	menu->AddItem(item = new BMenuItem("New Part",
									   new BMessage(MENU_CREATE_METERED_TRACK),
									   'N'));
	menu->AddSeparatorItem();
	menu->AddItem(item = new BMenuItem("Close", new BMessage(B_QUIT_REQUESTED), 'W'));
	menuBar->AddItem(menu);

	// Create the 'Edit' menu
	menu = new BMenu("Edit");
	menu->SetFont(be_plain_font);
	menu->AddItem(item = new BMenuItem("Undo",
									   new BMessage(MENU_UNDO), 'Z'));
	menu->AddItem(item = new BMenuItem("Redo",
									   new BMessage(MENU_REDO), 'Z', B_SHIFT_KEY));
	menu->AddSeparatorItem();
	menu->AddItem(item = new CQuickKeyMenuItem("Edit",
									   new BMessage(CTrackListItem::EDIT_TRACK),
									   B_ENTER, "Enter"));
	menu->AddSeparatorItem();
	menu->AddItem(item = new BMenuItem("Record" B_UTF8_ELLIPSIS,
									   new BMessage(CTrackListItem::RECORD_TRACK),
									   'R'));
	menu->AddItem(item = new BMenuItem("Mute",
									   new BMessage(CTrackListItem::MUTE_TRACK),
									   'M'));
	menu->AddItem(item = new BMenuItem("Solo",
									   new BMessage(CTrackListItem::SOLO_TRACK),
									   'S'));
	menu->AddSeparatorItem();
	menu->AddItem(item = new BMenuItem("Rename",
									   new BMessage(CTrackListItem::RENAME_TRACK),
									   'E'));
	menu->AddItem(item = new BMenuItem("Delete",
									   new BMessage(CTrackListItem::DELETE_TRACK),
									   'T'));
	menuBar->AddItem(menu);

	AddChild(menuBar);
}

void
CTrackListWindow::WatchTrack(
	CEventTrack *track)
{
	D_OPERATION(("CTrackListWindow::WatchTrack()\n"));

	if (track != NULL)
	{
		CMeVDoc *doc = &(track->Document());
		if (doc != m_doc)
		{
			if (m_doc != NULL)
				m_doc->RemoveObserver(this);

			m_doc = doc;
			if (m_doc != NULL)		
				m_doc->AddObserver(this);
		}
	}
	else
	{
		if (m_doc != NULL)
			m_doc->RemoveObserver(this);
	}

	m_listView->SetDocument(m_doc);
}

// END - TrackListWindow.cpp
