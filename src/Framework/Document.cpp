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

// ---------------------------------------------------------------------------
//	Class Data Initialization

int32
CDocument::s_newDocCount = 0;

// ---------------------------------------------------------------------------
//	Constructor/Destructor

CDocument::CDocument(
	CDocApp &app)
	:	app(app),
		m_modified(false),
		m_named(false),
		m_valid(false),
		m_savePanel(NULL),
		m_saving(false)
{
	char name[32];

	app.AddDocument(this);
	
	//	Get a unique name
	if (++s_newDocCount == 1)
		sprintf(name, "./Untitled");
	else
		sprintf(name, "./Untitled-%ld", s_newDocCount);
	
	//	Um, where should the default directory be?
	//	A preferences item?
	m_entry.SetTo(name);
}

CDocument::CDocument(
	CDocApp &app,
	entry_ref &ref)
	:	m_entry(&ref),
		app(app),
		m_modified(false),
		m_named(true),
		m_valid(false),
		m_savePanel(NULL),
		m_saving(false)
{
	app.AddDocument(this);
}

CDocument::~CDocument()
{
	if (m_savePanel)
		delete m_savePanel;
	app.RemoveDocument(this);
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
	StSubjectLock(*this, Lock_Exclusive);

	if (window->IsMasterWindow())
		m_masterWindow = window;
	else
		m_windows.AddItem(window);
}

void
CDocument::RemoveWindow(
	CDocWindow *window)
{
	StSubjectLock(*this, Lock_Exclusive);

	if (window->IsMasterWindow())
	{
		for (int32 i = 0; i < CountWindows(); i++)
		{
			WindowAt(i)->Lock();
			WindowAt(i)->QuitRequested();
			WindowAt(i)->Quit();
		}
		if (!IsSaving())
		{
			RequestDelete();
			Application()->RemoveDocument(this);
		}
	}
	else
	{
		m_windows.RemoveItem(window);
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
	CRefCountObject::Acquire();
	m_savePanel->Show();
}

// END - Document.cpp
