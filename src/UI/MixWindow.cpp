/* ===================================================================== *
 * MixWindow.cpp (MeV/UI)
 * ===================================================================== */

#include "MixWindow.h"

#include "ConsoleContainerView.h"
#include "Destination.h"
#include "DestinationView.h"
#include "Idents.h"
#include "MeVDoc.h"
#include "PlayerControl.h"
#include "QuickKeyMenuItem.h"

// Interface Kit
#include <MenuBar.h>
#include <MenuItem.h>
#include <View.h>
// Support Kit
#include <Debug.h>

// Debugging Macros
#define D_ALLOC(x) //PRINT (x)		// Constructor/Destructor
#define D_HOOK(x) //PRINT(x)		// CDocWindow Implementation
#define D_MESSAGE(x) //PRINT (x)	// MessageReceived()

// ---------------------------------------------------------------------------
// Constructor/Destructor

CMixWindow::CMixWindow(
	CWindowState &state,
	CMeVDoc *document,
	bool hasSettings)
	:	CDocWindow(state, document, false, "Mix",
				   B_DOCUMENT_WINDOW, B_ASYNCHRONOUS_CONTROLS),
		m_containerView(NULL),
		m_consoleOffset(0.0)
{
	D_ALLOC(("CMixWindow::CMixWindow(%s)\n",
			 hasSettings ? "hasSettings == true" : "hasSettings == false"));

	_addMenuBar();

	BRect rect(Bounds());
	rect.top = KeyMenuBar()->Frame().bottom + 1.0;
	m_containerView = new CConsoleContainerView(rect, "Mix");
	AddChild(m_containerView);

	int32 index = 0;
	CDestination *destination = NULL;
	StSubjectLock lock(*Document(), Lock_Shared);
	while ((destination = Document()->GetNextDestination(&index)) != NULL)
		_destinationAdded(destination);
	m_containerView->Pack();
}

CMixWindow::~CMixWindow()
{
	D_ALLOC(("CMixWindow::CMixWindow()\n"));
}

// ---------------------------------------------------------------------------
// CDocWindow Implementation

void
CMixWindow::GetContentSize(
	float *width,
	float *height) const
{
	D_HOOK(("CMixWindow::GetContentSize()\n"));

	m_containerView->GetPreferredSize(width, height);
	*height += KeyMenuBar()->Frame().Height() + 1.0;
}

void
CMixWindow::MenusBeginning()
{
	D_HOOK(("CMixWindow::MenusBeginning()\n"));

	BMenuItem *item = NULL;
	BString itemLabel;
	const char *description;

	// Set up Undo menu
	item = KeyMenuBar()->FindItem(MENU_UNDO);
	description = NULL;
	if (Document()->CanUndo())
	{
		itemLabel = "Undo";
		description = Document()->UndoDescription();
		if (description)
		{
			itemLabel << ": " << description;
		}
		if (item)
		{
			item->SetLabel(itemLabel.String());
			item->SetEnabled(true);
		}
	}
	else if (item)
	{
		item->SetLabel("Undo");
		item->SetEnabled(false);
	}

	// Set up Redo menu
	item = KeyMenuBar()->FindItem(MENU_REDO);
	description = NULL;
	if (Document()->CanRedo())
	{
		itemLabel = "Redo";
		description = Document()->RedoDescription();
		if (description)
		{
			itemLabel << ": " << description;
		}
		if (item)
		{
			item->SetLabel(itemLabel.String());
			item->SetEnabled(true);
		}
	}
	else if (item)
	{
		item->SetLabel("Redo");
		item->SetEnabled(false);
	}
	
	// Set up Window menu
	CMeVApp *app = Document()->Application();
	item = KeyMenuBar()->FindItem(MENU_PARTS_WINDOW);
	if (item)
		item->SetMarked(app->TrackList());
	item = KeyMenuBar()->FindItem(MENU_MIX_WINDOW);
	if (item)
		item->SetMarked(Document()->IsWindowOpen(CMeVDoc::MIX_WINDOW));
	item = KeyMenuBar()->FindItem(MENU_INSPECTOR);
	if (item)
		item->SetMarked(app->Inspector());
	item = KeyMenuBar()->FindItem(MENU_GRIDWINDOW);
	if (item)
		item->SetMarked(app->GridWindow());
	item = KeyMenuBar()->FindItem(MENU_TRANSPORT);
	if (item)
		item->SetMarked(app->TransportWindow());

	CDocWindow::MenusBeginning();
}

void
CMixWindow::MessageReceived(
	BMessage* message)
{
	switch (message->what)
	{
		case NEW_DESTINATION:
		{
			Document()->NewDestination();
			break;
		}
		case MENU_UNDO:
		{
			Document()->Undo();
			break;
		}
		case MENU_REDO:
		{
			Document()->Redo();
			break;
		}
		case MENU_CLEAR:
		{
			long index = 0;
			CConsoleView *slot;
			while ((slot = m_containerView->GetNextSelected(&index)) != NULL)
			{
				CDestinationView *ds = dynamic_cast<CDestinationView *>(slot);
				if (ds != NULL)
				{
					CDestinationDeleteUndoAction *undoAction;
					undoAction = new CDestinationDeleteUndoAction(ds->Destination());
					Document()->AddUndoAction(undoAction);
				}
			}
			break;
		}
		case MENU_PLAY:
		{
			if (CPlayerControl::IsPlaying(Document()))
			{
				CPlayerControl::StopSong(Document());
				break;
			}
			CMeVApp *app = Document()->Application();
			CPlayerControl::PlaySong(Document(), 0, 0, LocateTarget_Real, -1,
									 SyncType_SongInternal,
									 (app->GetLoopFlag() ? PB_Loop : 0));
			break;
		}
		case MENU_PAUSE:
		{
			CPlayerControl::TogglePauseState(Document());
			break;
		}
		case MENU_PARTS_WINDOW:
		{
			bool show = (Document()->Application()->TrackList() == NULL);
			Document()->Application()->ShowTrackList(show);
			// In case window was deactivated
			Activate();
			break;
		}
		case MENU_MIX_WINDOW:
		{
			bool visible = Document()->IsWindowOpen(CMeVDoc::MIX_WINDOW);
			Document()->ShowWindow(CMeVDoc::MIX_WINDOW, !visible);
			break;
		}
		case MENU_INSPECTOR:
		{
			bool show = (Document()->Application()->Inspector() == NULL);
			Document()->Application()->ShowInspector(show);
			// In case window was deactivated
			Activate();
			break;
		}
		case MENU_GRIDWINDOW:
		{
			bool show = (Document()->Application()->GridWindow() == NULL);
			Document()->Application()->ShowGridWindow(show);
			// In case window was deactivated
			Activate();
			break;
		}
		case MENU_TRANSPORT:
		{
			bool show = (Document()->Application()->TransportWindow() == NULL);
			Document()->Application()->ShowTransportWindow(show);
			// In case window was deactivated
			Activate();
			break;
		}
		case B_KEY_DOWN:
		{
			int32 key;
			message->FindInt32("raw_char", &key);
			int32 modifiers;
			message->FindInt32("modifiers", &modifiers);

			if ((modifiers & B_COMMAND_KEY)
			 || (!CQuickKeyMenuItem::TriggerShortcutMenu(KeyMenuBar(), key)))
				CDocWindow::MessageReceived(message);
			break;
		}
		case B_SELECT_ALL:
		{
			// select all slots
			m_containerView->SelectAll();
		}
		default:
		{
			CDocWindow::MessageReceived(message);
			break;
		}
	}
}

void
CMixWindow::SubjectUpdated(
	BMessage *message)
{
	D_OBSERVE(("CMixWindow<%p>::SubjectUpdated()\n", this));

	int32 docAttrs;
	if (message->FindInt32("DocAttrs", &docAttrs) != B_OK)
		return;

	if (docAttrs & CMeVDoc::Update_AddDest)
	{
		int32 destID;
		if (message->FindInt32("DestID", &destID) != B_OK)
			return;

		CDestination *destination = Document()->FindDestination(destID);
		if (destination)
			_destinationAdded(destination);
		m_containerView->Pack();
	}
	if (docAttrs & CMeVDoc::Update_DelDest)
	{
		int32 destID;
		if (message->FindInt32("DestID", &destID) != B_OK)
			return;

		_destinationRemoved(destID);
		m_containerView->Pack();
	}

	CDocWindow::SubjectUpdated(message);
}

// ---------------------------------------------------------------------------
// Internal Operations

void
CMixWindow::_addMenuBar()
{
	BMenu *menu, *subMenu;
	BMenuItem *item;

	BMenuBar *menuBar = new BMenuBar(Bounds(), "General");

	// Create the 'File' menu
	menu = new BMenu("File");
	subMenu = new BMenu("New");
	BMessage *message = new BMessage(NEW_DESTINATION);
	message->AddInt32("type", 'MIDI');
	subMenu->AddItem(new BMenuItem("MIDI Destination", message));
	menu->AddItem(subMenu);
	menu->AddSeparatorItem();
	menu->AddItem(item = new BMenuItem("Import Environment " B_UTF8_ELLIPSIS,
									   NULL));
	item->SetEnabled(false);
	menu->AddItem(item = new BMenuItem("Export Environment " B_UTF8_ELLIPSIS,
									   NULL));
	item->SetEnabled(false);
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem("Close", new BMessage(B_QUIT_REQUESTED)));
	menuBar->AddItem(menu);

	// Create the edit menu
	menu = new BMenu("Edit");
	menu->AddItem(new BMenuItem("Undo", new BMessage(MENU_UNDO), 'Z'));
	menu->AddItem(new BMenuItem("Redo", new BMessage(MENU_REDO), 'Z',
								B_SHIFT_KEY));
	menu->AddSeparatorItem();
	menu->AddItem(item = new BMenuItem("Cut", new BMessage(B_CUT), 'X'));
	item->SetEnabled(false);
	menu->AddItem(item = new BMenuItem("Copy", new BMessage(B_COPY), 'C'));
	item->SetEnabled(false);
	menu->AddItem(item = new BMenuItem("Paste", new BMessage(B_PASTE), 'V'));
	item->SetEnabled(false);
	menu->AddItem(new CQuickKeyMenuItem("Clear", new BMessage(MENU_CLEAR), B_DELETE, "Del"));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem("Select All", new BMessage(B_SELECT_ALL), 'A'));
	menuBar->AddItem(menu);

	// Create the 'Play' menu
	menu = new BMenu("Play");
	menu->AddItem(new CQuickKeyMenuItem("Pause", new BMessage(MENU_PAUSE), B_SPACE, "Space"));
	menu->AddSeparatorItem();
	menu->AddItem(new CQuickKeyMenuItem("Start", new BMessage(MENU_PLAY), B_ENTER, "Enter"));
	menuBar->AddItem(menu);
	
	// Create the 'Window' menu
	menu = new BMenu("Window");
	menu->AddItem(new BMenuItem("Parts",
								new BMessage(MENU_PARTS_WINDOW), 'P'));
	menu->AddItem(new BMenuItem("Mix",
								new BMessage(MENU_MIX_WINDOW), 'M'));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem("Inspector",
								new BMessage(MENU_INSPECTOR), 'I'));
	menu->AddItem(new BMenuItem("Grid",
								new BMessage(MENU_GRIDWINDOW), 'G'));
	menu->AddItem(new BMenuItem("Transport",
								new BMessage(MENU_TRANSPORT), 'T'));
	menu->AddSeparatorItem();
	SetWindowMenu(menu);
	menuBar->AddItem(menu);

	// Add the menus
	AddChild(menuBar);
}

void
CMixWindow::_destinationAdded(
	CDestination *destination)
{
	BRect rect(Bounds());
	rect.OffsetBy(m_consoleOffset, 0.0);
	rect.right = rect.left + 150.0;
	CDestinationView *view = new CDestinationView(rect, destination);
	view->ResizeToPreferred();
	rect = view->Frame();

	// get the index where the destination should be inserted
	Document()->ReadLock();
	long index = Document()->IndexOf(destination);
	Document()->ReadUnlock();

	m_containerView->AddSlot(view, index);

	// add the destination to the map
	m_destinations[destination->ID()] = view;
}

void
CMixWindow::_destinationRemoved(
	int32 destinationID)
{
	CDestinationView *view = m_destinations[destinationID];
	if (view)
	{
		m_containerView->RemoveSlot(view);
		delete view;

		// remove the destination from the map
		m_destinations.erase(destinationID);
	}
}

// END - MixWindow.cpp
