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

// Interface Kit
#include <Alert.h>
#include <Menu.h>
#include <MenuItem.h>
// Storage Kit
#include <Directory.h>
#include <Entry.h>
// Support Kit
#include <Debug.h>
#include <String.h>

// ---------------------------------------------------------------------------
// Class Data Initialization

CDocWindow *
CDocWindow::s_activeDocWin = NULL;

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
		m_waitingToQuit(false)
{
	CalcWindowTitle();

	m_document->Acquire();
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
		m_waitingToQuit(false)
{
	CalcWindowTitle();

	m_document->Acquire();
	m_document->AddWindow(this);
}

CDocWindow::~CDocWindow()
{
	if (m_document)
	{
		m_document->RemoveWindow(this);
		CRefCountObject::Release(m_document);
		m_document = NULL;
	}

	if (be_app->Lock())
	{
		if (this == s_activeDocWin)
			s_activeDocWin = NULL;
		be_app->Unlock();
	}
}

// ---------------------------------------------------------------------------
// Accessors

CDocument *
CDocWindow::Document()
{
	m_document->Acquire();
	return m_document;
}

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
		AddChild(m_toolBar);
}

// ---------------------------------------------------------------------------
// Operations

void
CDocWindow::AcquireSelectToken()
{
	if (be_app->Lock())
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
		be_app->Unlock();
	}
}

// ---------------------------------------------------------------------------
// CAppWindow Implementation

void
CDocWindow::MenusBeginning()
{
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

void
CDocWindow::SubjectReleased(
	CObservable *subject)
{
	D_OBSERVE(("CDocWindow<%p>::SubjectReleased()\n", this));

	if (subject == m_document)
	{
		D_OBSERVE((" -> stop observing document\n"));

		m_document->RemoveObserver(this);
		CRefCountObject::Release(m_document);
		m_document = NULL;
		PostMessage(B_QUIT_REQUESTED, this);
	}
}

void
CDocWindow::WindowActivated(
	bool active)
{
	CAppWindow::WindowActivated(active);

	if (active)
		AcquireSelectToken();
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
