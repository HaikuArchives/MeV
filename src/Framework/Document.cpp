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
	BMessenger messenger(NULL, be_app);
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

	m_named = true;
	return B_OK;
}

// ---------------------------------------------------------------------------
// Operations

void
CDocument::AddWindow(
	CDocWindow *window)
{
	int32 plainWindowCount = 0;
	StSubjectLock(*this, Lock_Exclusive);

	m_windows.AddItem(window);
	for (int32 i = 0; i < CountWindows(); i++)
	{
		CDocWindow *window = WindowAt(i);
		if (window->WindowNumber() >= 0)
			plainWindowCount++;
	}

	if (plainWindowCount > 1)
	{
		for (int i = 0; i < CountWindows(); i++)
		{
			CDocWindow *window = WindowAt(i);
			if (window->WindowNumber() == 0)
			{
				window->SetWindowNumber(CalcUniqueWindowNumber());
				window->RecalcWindowTitle();
			}
		}
	}
}

void
CDocument::RemoveWindow(
	CDocWindow *window)
{
	StSubjectLock(*this, Lock_Exclusive);

	int32 plainWindowCount = 0;
	m_windows.RemoveItem(window);
	for (int32 i = 0; i < CountWindows(); i++)
	{
		CDocWindow *window = WindowAt(i);
		if (window->WindowNumber() >= 0)
			plainWindowCount++;
	}
	
	if (plainWindowCount == 1)
	{
		for (int32 i = 0; i < CountWindows(); i++)
		{
			CDocWindow *window = WindowAt(i);
			if (window->WindowNumber() > 0)
			{
				window->SetWindowNumber(0);
				window->RecalcWindowTitle();
			}
		}
	}

	//	If there are any other observers hanging on to this, then
	//	ask them to please observe something else.
	if ((CountWindows() == 0) && !IsSaving())
	{
		RequestDelete();
		Application()->RemoveDocument(this);
	}
}

void
CDocument::Save()
{
	m_saving = true;
	if (m_named == false)
		SaveAs();
	else
		SaveDocument();
}

void
CDocument::SaveAs()
{
	if (m_savePanel == NULL)
		m_savePanel = CreateSavePanel();
	CRefCountObject::Acquire();
	m_savePanel->Show();
}

// ---------------------------------------------------------------------------
// Internal Operations

long
CDocument::CalcUniqueWindowNumber()
{
	long trialID = 1, prevID = -1;

	//	Loop until we can run through the entire list
	//	without a collision.
	while (trialID != prevID)
	{
		prevID = trialID;

		//	If we collide, then increase the ID by one
		for (int i = 0; i < CountWindows(); i++)
		{
			CDocWindow *window = WindowAt(i);
			if (window->WindowNumber() <= 0)
				continue;
			if (trialID == window->WindowNumber())
				trialID++;
		}
	}

	return trialID;
}

// END - Document.cpp