/* ===================================================================== *
 * RecentDocumentsMenu.cpp (MeV/Framework)
 * ===================================================================== */

#include "RecentDocumentsMenu.h"

#include "IconMenuItem.h"

// Application Kit
#include <Roster.h>
// Inteface Kit
#include <Bitmap.h>
#include <MenuItem.h>
// Storage Kit
#include <Entry.h>
#include <Node.h>
#include <NodeInfo.h>

// ---------------------------------------------------------------------------
// Constructor/Destructor

CRecentDocumentsMenu::CRecentDocumentsMenu(
	const char *name,
	BMessage *message,
	int32 maxCount,
	const char *ofType,
	const char *openedByApp)
	:	BMenu(name)
{
	if (message == NULL)
		message = new BMessage(B_REFS_RECEIVED);

	BMessage refList;
	be_roster->GetRecentDocuments(&refList, maxCount, ofType,
								  openedByApp);
	_populateMenu(&refList, message);

	delete message;
}

CRecentDocumentsMenu::CRecentDocumentsMenu(
	const char *name,
	BMessage *message,
	int32 maxCount,
	const char *ofType[],
	int32 ofTypeListCount,
	const char *openedByApp)
	:	BMenu(name)
{
	if (message == NULL)
		message = new BMessage(B_REFS_RECEIVED);

	BMessage refList;
	be_roster->GetRecentDocuments(&refList, maxCount, ofType,
								  ofTypeListCount, openedByApp);
	_populateMenu(&refList, message);

	delete message;
}

CRecentDocumentsMenu::~CRecentDocumentsMenu()
{
}

// ---------------------------------------------------------------------------
// Internal Operations

void
CRecentDocumentsMenu::_populateMenu(
	BMessage *refList,
	BMessage *itemMessage)
{
	int32 i = 0;
	entry_ref ref;
	while (refList->FindRef("refs", i++, &ref) == B_OK)
	{
		BEntry entry(&ref, true);
		char name[B_FILE_NAME_LENGTH];
		if ((entry.InitCheck() != B_OK) || (entry.GetName(name) != B_OK))
			continue;
		BNode node(&entry);
		BNodeInfo nodeInfo(&node);
		BBitmap *icon = new BBitmap(BRect(0.0, 0.0, B_MINI_ICON - 1.0,
										  B_MINI_ICON - 1.0), B_CMAP8);
		if (nodeInfo.GetTrackerIcon(icon, B_MINI_ICON) != B_OK)
		{
			delete icon;
			icon = NULL;
		}
		BMessage *refMsg = new BMessage(*itemMessage);
		refMsg->AddRef("refs", &ref);
		AddItem(new CIconMenuItem(name, refMsg, icon));
	}
}

// END - RecentDocumentsMenu.cpp
