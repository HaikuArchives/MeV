/* ===================================================================== *
 * DocApp.cpp (MeV/Framework)
 * ===================================================================== */

#include "DocApp.h"

#include "DocWindow.h"

// Application Kit
#include <Roster.h>
// Interface Kit
#include <Alert.h>
#include <Bitmap.h>
// Storage Kit
#include <FilePanel.h>
#include <Resources.h>
// Support Kit
#include <Debug.h>

#define D_HOOK(x) //PRINT(x)			// BApplication Implementation

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
		PostMessage(B_QUIT_REQUESTED);
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
		case CDocWindow::DONE_SAVING:
		{
			CDocument *doc;
			if (message->FindPointer("Document", (void **)&doc) == B_OK)
				RemoveDocument(doc);
			break;
		}
		default:
		{
			BApplication::MessageReceived(message);
		}
	}
}

bool
CDocApp::QuitRequested()
{
	D_HOOK(("CDocApp::QuitRequested()\n"));

	for (int32 i = 0; i < CountDocuments(); i++)
	{
		if (DocumentAt(i)->IsSaving())
			return false;
	}

	return true;
}

void
CDocApp::ReadyToRun()
{
	if (CountDocuments() == 0)
		NewDocument(true, NULL);
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
				NewDocument(true, &ref);
		}
	}
}

// END - DocApp.cpp
