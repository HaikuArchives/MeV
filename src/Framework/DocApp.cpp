/* ===================================================================== *
 * DocApp.cpp (MeV/Framework)
 * ===================================================================== */

#include "DocApp.h"

// Application Kit
#include <Roster.h>
// Interface Kit
#include <Alert.h>
#include <Bitmap.h>
// Storage Kit
#include <FilePanel.h>
#include <Resources.h>

// ---------------------------------------------------------------------------
// Constructor/Destructor

CDocApp::CDocApp(
	const char *signature)
	:	BApplication(signature),
		m_openPanel(NULL)
{
}

CDocApp::~CDocApp()
{
	if (m_openPanel)
	{
		delete m_openPanel;
		m_openPanel = NULL;
	}
}

// ---------------------------------------------------------------------------
// Hook Functions

BFilePanel *
CDocApp::CreateOpenPanel()
{
	// Create a new load panel
	BMessenger messenger(NULL, this);
	BFilePanel *panel = new BFilePanel(B_OPEN_PANEL, &messenger,
									   NULL, B_FILE_NODE, false);

	return panel;
}

// ---------------------------------------------------------------------------
// Operations

void
CDocApp::AddDocument(
	CDocument *doc) 
{
	m_documents.AddItem(doc);
}

void
CDocApp::OpenDocument()
{
	if (m_openPanel == NULL)
		m_openPanel = CreateOpenPanel();
	m_openPanel->Show();
}

void
CDocApp::RemoveDocument(
	CDocument *doc)
{
	m_documents.RemoveItem(doc);
	if (CountDocuments() == 0)
	{
		PostMessage(B_QUIT_REQUESTED);
	}
}

void
CDocApp::Error(
	char *errorMsg)
{
	BAlert *alert;
	alert = new BAlert(NULL, errorMsg, "OK", NULL, NULL,
					   B_WIDTH_AS_USUAL, B_WARNING_ALERT); 
	alert->Go();
}

// ---------------------------------------------------------------------------
// BApplication Implementation

void
CDocApp::MessageReceived(
	BMessage *message)
{
	switch (message->what)
	{
		case B_CANCEL:
		{
			CDocument *doc;
			if (message->FindPointer("Document",
									 reinterpret_cast<void **>(&doc)) == B_OK)
			{
				CRefCountObject::Release( doc );
			}
			break;
		}
		case B_SAVE_REQUESTED:
		{
			CDocument *doc;
			if (message->FindPointer("Document",
									 reinterpret_cast<void **>(&doc)) == B_OK)
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
					doc->SetEntry(&entry);
					doc->Save();
				}
			}
			break;
		}
		default:
		{
			BApplication::MessageReceived(message);
		}
	}
}

void
CDocApp::ReadyToRun()
{
	if (CountDocuments() == 0)
	{
		CDocument *doc = NewDocument(true, NULL);
		// Release reference because newly created window has a ref
		CRefCountObject::Release(doc);
	}
}

void
CDocApp::RefsReceived(
	BMessage *message)
{ 
	uint32 type;
	int32 count;
	entry_ref ref;

	message->GetInfo("refs", &type, &count);
	if (type == B_REF_TYPE)
	{
		for (int32 i = 0; i < count; i++)
		{
			if (message->FindRef("refs", i, &ref) == B_OK)
			{
				CRefCountObject::Release(NewDocument(true, &ref));
			}
		}
	}
}

// END - DocApp.cpp
