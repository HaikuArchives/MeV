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
		m_windowMenuStart(-1),
		m_name(inTypeName)
{
	CalcWindowTitle();

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
		m_windowMenuStart(-1),
		m_name(inTypeName)
{
	CalcWindowTitle();

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
		case ACTIVATE:
		{
			Activate();
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
			
			if (result == 0)
			{
				if (m_document->Application()->CountDocuments() == 1)
					be_app->PostMessage(B_QUIT_REQUESTED);
			}
			else if (result == 1)
			{
				return false;
			}
			else if (result == 2)
			{
				m_document->Save();
				if (m_document->Application()->CountDocuments() == 1)
					be_app->PostMessage(B_QUIT_REQUESTED);
			}
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
//	if (m_windowNumber > 0)
//		title << " [" << m_windowNumber + 1 << "]";

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

		for (int i = 0; i < Document()->CountWindows(); i++)
		{
			CDocWindow *window = Document()->WindowAt(i);
			BMenuItem *item = new BMenuItem(window->Title(),
											new BMessage(ACTIVATE));
			item->SetTarget(window);
			if (window == this)
				item->SetMarked(true);
	
			m_windowMenu->AddItem(item);
		}
	}
}

// END - DocWindow.cpp
