/* ===================================================================== *
 * DocWindow.cpp (MeV/Framework)
 * ===================================================================== */

#include "DocWindow.h"

#include "DocApp.h"
#include "Document.h"
#include "ToolBar.h"

// Interface Kit
#include <Alert.h>
#include <Menu.h>
#include <MenuItem.h>
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
	const char *inTypeName,
	window_type type,
	uint32 flags)
	:	CAppWindow(frame, NULL, type, 0),
		m_document(document),
		m_toolBar(NULL),
		m_windowMenu(NULL),
		m_windowMenuStart(-1)
{
	CalcWindowTitle(inTypeName);

	m_document->Acquire();
	m_document->AddWindow(this);
}

CDocWindow::CDocWindow(
	CWindowState &state,
	CDocument *document,
	const char *inTypeName,
	window_type type,
	uint32 flags)
	:	CAppWindow(state, state.Rect(), NULL, type, 0),
		m_document(document),
		m_toolBar(NULL),
		m_windowMenu(NULL),
		m_windowMenuStart(-1)
{
	CalcWindowTitle(inTypeName);

	m_document->Acquire();
	m_document->AddWindow(this);
}

CDocWindow::~CDocWindow()
{
	m_document->RemoveWindow(this);
	CRefCountObject::Release(m_document);
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
CDocWindow::WindowActivated(
	bool active)
{
	CAppWindow::WindowActivated(active);

	if (active)
		AcquireSelectToken();
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
		case Activate_ID:
		{
			Activate();
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
	if (m_document->CountWindows() == 1)
	{
		if (m_document->Modified())
		{
			long result;
	
			char fileName[B_FILE_NAME_LENGTH];
			m_document->GetName(fileName);
	
			BString text = "Save changes to '";
			text << fileName << "' ?";
	
			BAlert *alert = new BAlert("Quit", text.String(), "Don't Save",
									   "Cancel", "Save",
									   B_WIDTH_AS_USUAL, B_OFFSET_SPACING,
									   B_WARNING_ALERT); 
			alert->SetShortcut(1, B_ESCAPE);
			result = alert->Go();
			
			if (result == 1)
				return false;
			if (result == 2)
				m_document->Save();
		}
	}

	return CAppWindow::QuitRequested();
}

void
CDocWindow::AcquireSelectToken()
{
	if (be_app->Lock())
	{
		if (this != ActiveDocWindow())
		{
			if (ActiveDocWindow())
			{
				BMessage message(Select_ID);
				message.AddBool("active", false);
				BMessenger messenger(NULL, ActiveDocWindow());
				messenger.SendMessage(&message);
			}
			s_activeDocWin = this;
	
			BMessage message(Select_ID);
			message.AddBool("active", true);
			PostMessage(&message, this);
		}
		be_app->Unlock();
	}
}

// ---------------------------------------------------------------------------
// Internal Operations

void
CDocWindow::CalcWindowTitle(
	const char *inTypeName)
{
	char name[B_FILE_NAME_LENGTH + 32];
	m_document->GetName(name);

	if (inTypeName)
	{
		strncat(name, ": ", sizeof(name));
		strncat(name, inTypeName, sizeof(name));
		m_windowNumber = -1;
		SetTitle(name);
	}
	else
	{
		m_windowNumber = 0;
		SetTitle(name);
	}
}

void
CDocWindow::RecalcWindowTitle()
{
	char fileName[B_FILE_NAME_LENGTH];
	m_document->GetName(fileName);

	if (m_windowNumber > 0)
	{
		BString title = fileName;
		title << ":" << m_windowNumber;
		SetTitle(title.String());
	}
	else
	{
		SetTitle(fileName);
	}
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

		for (int i = 0; i < Document()->CountWindows(); i++)
		{
			CDocWindow *window = Document()->WindowAt(i);
			BMenuItem *item = new BMenuItem(window->Title(),
											new BMessage(Activate_ID));
			item->SetTarget(window);
			if (window == this)
				item->SetMarked(true);
	
			m_windowMenu->AddItem(item);
		}
	}
}

// END - DocWindow.cpp
