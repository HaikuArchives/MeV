/* ===================================================================== *
 * DocWindow.cpp (MeV/Framework)
 * ===================================================================== */

#include "DocWindow.h"

#include "DocApp.h"
#include "Document.h"
#include "IconMenuItem.h"
#include "ResourceUtils.h"
#include "ToolBar.h"
#include "WindowState.h"
#include "ScreenUtils.h"

// Interface Kit
#include <Alert.h>
#include <Menu.h>
#include <MenuBar.h>
#include <MenuItem.h>
#include <Screen.h>
#include <ScrollBar.h>
// Storage Kit
#include <Directory.h>
#include <Entry.h>
// Support Kit
#include <Debug.h>
#include <String.h>
#include <Locker.h>

#define D_ALLOC(x) //PRINT(x)			// Constructor/Destructor
#define D_HOOK(x) //PRINT(x)			// CAppWindow Implementation

// ---------------------------------------------------------------------------
// Class Data Initialization

CDocWindow *
CDocWindow::s_activeDocWin = NULL;

static BLocker s_activeWinLocker("DocWindow locker");

// ---------------------------------------------------------------------------
// Constructor/Destructor

CDocWindow::CDocWindow(
	BRect frame,
	CDocument *document,
	bool isMaster,
	const char *inTypeName,
	window_type type,
	uint32 flags)
	:	CAppWindow(frame, NULL, type, 0),
		m_document(document),
		m_isMaster(isMaster),
		m_toolBar(NULL),
		m_windowMenu(NULL),
		m_windowMenuStart(-1),
		m_name(inTypeName),
		m_waitingToQuit(false),
		m_zoomed(false),
		m_zooming(false)
{
	D_ALLOC(("CDocWindow::CDocWindow(rect)\n"));

	CalcWindowTitle();

	m_document->AddWindow(this);
}

CDocWindow::CDocWindow(
	CWindowState &state,
	CDocument *document,
	bool isMaster,
	const char *inTypeName,
	window_type type,
	uint32 flags)
	:	CAppWindow(state, state.Rect(), NULL, type, 0),
		m_document(document),
		m_isMaster(isMaster),
		m_toolBar(NULL),
		m_windowMenu(NULL),
		m_windowMenuStart(-1),
		m_name(inTypeName),
		m_waitingToQuit(false),
		m_zoomed(false),
		m_zooming(false)
{
	D_ALLOC(("CDocWindow::CDocWindow(state)\n"));

	CalcWindowTitle();

	m_document->AddWindow(this);
}

CDocWindow::~CDocWindow()
{
	D_ALLOC(("CDocWindow<%s>::~CDocWindow()\n", Name()));

	if (m_document)
	{
		m_document->RemoveWindow(this);
		m_document = NULL;
	}

	if (s_activeWinLocker.Lock())
	{
		if (this == s_activeDocWin)
			s_activeDocWin = NULL;
		s_activeWinLocker.Unlock();
	}
}

// ---------------------------------------------------------------------------
// Hook Functions

CDocument *
CDocWindow::Document()
{
	return m_document;
}

void
CDocWindow::GetContentSize(
	float *width,
	float *height) const
{
	BRect rect(Bounds());
	*width = rect.Width();
	*height = rect.Height();
}

// ---------------------------------------------------------------------------
// Accessors

void
CDocWindow::SetToolBar(
	CToolBar *toolBar)
{
	if (m_toolBar && (m_toolBar != toolBar))
	{
		RemoveChild(m_toolBar);
		delete m_toolBar;
		m_toolBar = NULL;
	}

	m_toolBar = toolBar;
	if (m_toolBar)
	{
		AddChild(m_toolBar);

		float minWidth, maxWidth, minHeight, maxHeight;
		GetSizeLimits(&minWidth, &maxWidth, &minHeight, &maxHeight);
		minWidth = 100.0;
		if (KeyMenuBar() != NULL)
			minHeight = KeyMenuBar()->Frame().Height();
		minHeight += m_toolBar->Frame().Height();
		minHeight += B_H_SCROLL_BAR_HEIGHT;
		SetSizeLimits(minWidth, maxWidth, minHeight, maxHeight);
	}
}

// ---------------------------------------------------------------------------
// Operations

void
CDocWindow::AcquireSelectToken()
{
	if (s_activeWinLocker.Lock())
	{
		if (this != ActiveDocWindow())
		{
			if (ActiveDocWindow())
			{
				BMessage message(SELECTED);
				message.AddBool("active", false);
				BMessenger messenger(NULL, ActiveDocWindow());
				messenger.SendMessage(&message);
			}
			s_activeDocWin = this;
	
			BMessage message(SELECTED);
			message.AddBool("active", true);
			PostMessage(&message, this);
		}
		s_activeWinLocker.Unlock();
	}
}

// ---------------------------------------------------------------------------
// CAppWindow Implementation

void
CDocWindow::FrameResized(
	float width,
	float height)
{
	D_HOOK(("CDocWindow::FrameResized()\n"));

	if (!m_zooming)
		m_zoomed = false;
	else
		m_zooming = false;
}

void
CDocWindow::MenusBeginning()
{
	D_HOOK(("CDocWindow::MenusBeginning()\n"));

	UpdateWindowMenu();

	CAppWindow::MenusBeginning();
}

void
CDocWindow::MessageReceived(
	BMessage *message)
{
	switch (message->what)
	{
		case ACTIVATE:
		{
			Activate();
			break;
		}
		case HIDE_ALL:
		{
			for (int32 i = 0; i < Document()->CountWindows(); i++)
			{
				if (!Document()->WindowAt(i)->IsMinimized())
					Document()->WindowAt(i)->Minimize(true);
			}
			if (!IsMinimized())
				Minimize(true);
			break;
		}
		case SHOW_ALL:
		{
			for (int32 i = 0; i < Document()->CountWindows(); i++)
			{
				if (Document()->WindowAt(i)->IsMinimized())
					Document()->WindowAt(i)->Minimize(false);
			}
			if (IsMinimized())
				Minimize(false);
			break;
		}
		case MOVE_TO_WORKSPACE:
		{
			int32 workspace = current_workspace();
			message->FindInt32("workspace_id", &workspace);
			for (int32 i = 0; i < Document()->CountWindows(); i++)
				Document()->WindowAt(i)->SetWorkspaces(1 << workspace);
			SetWorkspaces(1 << workspace);
			activate_workspace(workspace);
			break;
		}
		case B_CANCEL:
		{
			m_waitingToQuit = false;
			Document()->CancelSaving();
			break;
		}
		case B_SAVE_REQUESTED:
		{
			entry_ref directory;
			const char *name;
			if ((message->FindRef("directory", 0, &directory) == B_OK)
			 && (message->FindString("name", 0, &name ) == B_OK))
			{
				BDirectory dir(&directory);
				if (dir.InitCheck() != B_NO_ERROR)
					return;
				BEntry entry(&dir, name);
				if (Document()->SetEntry(&entry) == B_OK)
					Document()->Save();
			}
			if (m_waitingToQuit)
			{
				PostMessage(B_QUIT_REQUESTED);
				BMessage doneMsg(DONE_SAVING);
				doneMsg.AddPointer("Document",
								   reinterpret_cast<void *>(Document()));
				be_app->PostMessage(&doneMsg);
			}
			break;
		}
		case CDocument::NAME_CHANGED:
		{
			const char *name;
			if (message->FindString("name", &name) == B_OK)
				CalcWindowTitle(name);
			else
				CalcWindowTitle();
			break;
		}
		default:
		{
			CAppWindow::MessageReceived(message);
		}
	}
}

bool
CDocWindow::QuitRequested()
{
	D_HOOK(("CDocWindow<%s>::QuitRequested()\n", Name()));

	if (IsMasterWindow())
	{
		if (m_document->Modified())
		{
			m_waitingToQuit = true;

			char fileName[B_FILE_NAME_LENGTH];
			m_document->GetName(fileName);
	
			BString text = "Save changes to '";
			text << fileName << "' ?";
	
			BAlert *alert = new BAlert("Quit", text.String(), "Cancel",
									   "Don't Save", "Save",
									   B_WIDTH_AS_USUAL, B_OFFSET_SPACING,
									   B_WARNING_ALERT); 
			alert->SetShortcut(1, B_ESCAPE);
			int32 result = alert->Go();
			
			if (result == 0)
			{
				// Cancel
				m_waitingToQuit = false;
				return false;
			}
			else if (result == 1)
			{
				// Don't Save
				if (m_document->Application()->CountDocuments() == 1)
					be_app->PostMessage(B_QUIT_REQUESTED);
			}
			else if (result == 2)
			{
				// Save
				bool named = Document()->Named();
				Document()->Save();
				if (!named)
					return false;
			}
		}
	}

	return CAppWindow::QuitRequested();
}

bool
CDocWindow::SubjectReleased(
	CObservable *subject)
{
	D_OBSERVE(("CDocWindow<%p>::SubjectReleased()\n", this));

	if (subject == m_document)
	{
		D_OBSERVE((" -> stop observing document\n"));

		m_document->RemoveWindow(this);
		m_document = NULL;
		PostMessage(B_QUIT_REQUESTED, this);
		return true;
	}

	return CAppWindow::SubjectReleased(subject);
}

void
CDocWindow::WindowActivated(
	bool active)
{
	D_HOOK(("CDocWindow<%s>::WindowActivated()\n", Name()));

	CAppWindow::WindowActivated(active);

	if (active)
		AcquireSelectToken();
}

void
CDocWindow::Zoom(
	BPoint origin,
	float width,
	float height)
{
	D_HOOK(("CDocWindow<%s>::Zoom()\n", Name()));

	m_zooming = true;

	BScreen screen(this);
	if (!screen.Frame().Contains(Frame()))
		m_zoomed = false;

	if (!m_zoomed)
	{
		// resize to the ideal size
		m_manualSize = Bounds();
		m_zoomed = true;
		GetContentSize(&width, &height);
		BRect rect(Frame().LeftTop(),
				   Frame().LeftTop() + BPoint(width, height));
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
CDocWindow::CalcWindowTitle(
	const char *documentName,
	const char *windowName)
{
	char docName[B_FILE_NAME_LENGTH];
	if (documentName == NULL)
		m_document->GetName(docName);
	else
		strncpy(docName, documentName, B_FILE_NAME_LENGTH);

	if (windowName != NULL)
		m_name = windowName;

	BString title = docName;
	if (m_name.CountChars() > 0)
		title << ": " << m_name;

	SetTitle(title.String());
}

void
CDocWindow::UpdateWindowMenu()
{
	if (m_windowMenu != NULL)
	{
		if (m_windowMenuStart < 0)
		{
			m_windowMenuStart = m_windowMenu->CountItems();
		}
		else
		{
			BMenuItem *item;
			while ((item = m_windowMenu->RemoveItem(m_windowMenuStart)))
				delete item;
		}

		// add one submenu per document
		for (int32 i = 0; i < Document()->Application()->CountDocuments(); i++)
		{
			CDocument *doc = Document()->Application()->DocumentAt(i);
			BMenu *subMenu = new BMenu(doc->MasterWindow()->Title());

			// Add windows to document submenu
			CDocWindow *window = doc->MasterWindow();
			BMenuItem *item;
			bool hidden = false;
			bool shown = false;
			if (!window->IsMinimized())
			{
				item = new CIconMenuItem(window->Title(), new BMessage(ACTIVATE),
										 ResourceUtils::LoadImage("WindowShownIcon"));
				shown = true;
			}
			else
			{
				item = new CIconMenuItem(window->Title(), new BMessage(ACTIVATE),
										 ResourceUtils::LoadImage("WindowHiddenIcon"));
				hidden = true;
			}
			item->SetTarget(window);
			if (window == this)
				item->SetMarked(true);
			subMenu->AddItem(item);

			for (int i = 0; i < doc->CountWindows(); i++)
			{
				window = doc->WindowAt(i);
				if (!window->IsMinimized())
				{
					item = new CIconMenuItem(window->Title(), new BMessage(ACTIVATE),
											 ResourceUtils::LoadImage("WindowShownIcon"));
					shown = true;
				}
				else
				{
					item = new CIconMenuItem(window->Title(), new BMessage(ACTIVATE),
											 ResourceUtils::LoadImage("WindowHiddenIcon"));
					hidden = true;
				}
				item->SetTarget(window);
				if (window == this)
					item->SetMarked(true);

				subMenu->AddItem(item);
			}

			subMenu->AddSeparatorItem();
			subMenu->AddItem(item = new BMenuItem("Hide All",
												  new BMessage(HIDE_ALL)));
			item->SetEnabled(shown);
			item->SetTarget(doc->MasterWindow());
			subMenu->AddItem(item = new BMenuItem("Show All",
												  new BMessage(SHOW_ALL)));
			item->SetEnabled(hidden);
			item->SetTarget(doc->MasterWindow());

			BMenu *wsMenu = new BMenu("Move All To");
			for (int32 i = 0; i < count_workspaces(); i++)
			{
				BMessage *wsMsg = new BMessage(MOVE_TO_WORKSPACE);
				wsMsg->AddInt32("workspace_id", i);
				BString str = "Workspace ";
				str << i + 1;
				wsMenu->AddItem(new BMenuItem(str.String(), wsMsg));
			}
			wsMenu->SetTargetForItems(doc->MasterWindow());
			subMenu->AddItem(wsMenu);

			subMenu->AddSeparatorItem();
			subMenu->AddItem(item = new BMenuItem("Close All",
												  new BMessage(B_QUIT_REQUESTED)));
			item->SetTarget(doc->MasterWindow());

			// Add master document window
			item = new CIconMenuItem(subMenu, new BMessage(ACTIVATE),
									 ResourceUtils::LoadImage("WindowGroupIcon"));
			item->SetTarget(doc->MasterWindow());
			m_windowMenu->AddItem(item);
		}
	}
}

// END - DocWindow.cpp
