/* ===================================================================== *
 * Document.cpp (MeV/Framework)
 * ===================================================================== */

#include "DocApp.h"
#include "DocWindow.h"

// Gnu C Library
#include <stdio.h>
// Interface Kit
#include <Menu.h>
#include <MenuItem.h>
// Storage Kit
#include <FilePanel.h>
// Support Kit
#include <Debug.h>

// Debugging Macros
#define D_ALLOC(x) //PRINT(x)			// Constructor/Destructor
#define D_WINDOW(x) //PRINT(x)			// Window Management

// ---------------------------------------------------------------------------
//	Class Data Initialization

int32
CDocument::s_newDocCount = 0;

// ---------------------------------------------------------------------------
//	Constructor/Destructor

CDocument::CDocument(
	CDocApp *app,
	const char *name)
	:	m_app(app),
		m_modified(false),
		m_named(false),
		m_valid(false),
		m_savePanel(NULL),
		m_saving(false)
{
	D_ALLOC(("CDocument::CDocument()\n"));

	BString entryName("./");

	m_app->AddDocument(this);
	
	//	Get a unique name
	if (name == NULL)
	{
		if (++s_newDocCount == 1)
			entryName << "Untitled";
		else
			entryName << "Untitled-" << s_newDocCount;
	}
	else
	{
		entryName << name;
	}

	//	Um, where should the default directory be?
	//	A preferences item?
	m_entry.SetTo(entryName.String());
}

CDocument::CDocument(
	CDocApp *app,
	entry_ref &ref)
	:	m_entry(&ref),
		m_app(app),
		m_modified(false),
		m_named(true),
		m_valid(false),
		m_savePanel(NULL),
		m_saving(false)
{
	D_ALLOC(("CDocument::CDocument()\n"));

	m_app->AddDocument(this);
}

CDocument::~CDocument()
{
	D_ALLOC(("CDocument::~CDocument()\n"));

	if (m_savePanel)
		delete m_savePanel;
	RequestDelete();
	m_app->RemoveDocument(this);
}

// ---------------------------------------------------------------------------
// Hook Functions

BFilePanel *
CDocument::CreateSavePanel()
{
	BMessage message(B_SAVE_REQUESTED);
	message.AddPointer("Document", this);

	// Create a new save panel
	BMessenger messenger(NULL, MasterWindow());
	BFilePanel *panel = new BFilePanel(B_SAVE_PANEL, &messenger,
									   NULL, B_FILE_NODE, false, &message);

	// Set the save panel to point to the directory from where we loaded.
	BEntry entry;
	if (m_entry.GetParent(&entry) == B_NO_ERROR)
	{
		panel->SetPanelDirectory(&entry);
	}

	return panel;
}

// ---------------------------------------------------------------------------
// Accessors

status_t
CDocument::GetName(
	char *name) const
{
	return m_entry.GetName(name);
}

status_t
CDocument::GetEntry(
	BEntry *entry) const
{
	if (m_entry.InitCheck() == B_OK)
	{
		*entry = m_entry;
		return B_OK;
	}

	return m_entry.InitCheck();
}

status_t
CDocument::SetEntry(
	const BEntry *entry)
{
	m_entry = *entry;
	status_t error = m_entry.InitCheck();
	if (error)
	{
		m_named = false;
		return error;
	}

	SetNamed();
	return B_OK;
}

void
CDocument::SetNamed()
{
	char name[B_FILE_NAME_LENGTH];
	if (m_entry.GetName(name) != B_OK)
		return;

	BMessage message(NAME_CHANGED);
	message.AddString("name", name);

	// send update to master window
	BMessenger messenger(NULL, MasterWindow());
	messenger.SendMessage(&message);

	// send updates to the other windows
	for (int32 i = 0; i < CountWindows(); i++)
	{
		BMessenger messenger(NULL, WindowAt(i));
		messenger.SendMessage(&message);
	}

	m_named = true;
}

// ---------------------------------------------------------------------------
// Operations

void
CDocument::AddWindow(
	CDocWindow *window)
{
	D_WINDOW(("CDocument::AddWindow(%s)\n", window->Name()));

	StSubjectLock lock(*this, Lock_Exclusive);

	AddObserver(window);
	if (window->IsMasterWindow())
		m_masterWindow = window;
	else
		m_windows.AddItem(window);
}

void
CDocument::RemoveWindow(
	CDocWindow *window)
{
	D_WINDOW(("CDocument::RemoveWindow(%s)\n", window->Name()));

	if (window->IsMasterWindow())
	{
		D_WINDOW((" -> '%s' is master window\n", window->Name()));
		RemoveObserver(window);
		if (!IsSaving())
		{
			D_WINDOW((" -> delete self\n"));
			delete this;
		}
	}
	else
	{
		StSubjectLock lock(*this, Lock_Exclusive);
		m_windows.RemoveItem(window);
		RemoveObserver(window);
	}
}

void
CDocument::Save()
{
	m_saving = true;
	if (m_named == false)
	{
		SaveAs();
	}
	else
	{
		SaveDocument();
		m_saving = false;
	}
}

void
CDocument::SaveAs()
{
	if (m_savePanel == NULL)
		m_savePanel = CreateSavePanel();
	m_savePanel->Show();
}

// END - Document.cpp
