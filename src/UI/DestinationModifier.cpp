/* ===================================================================== *
 * DestinationModifier.cpp (MeV/UI)
 * ===================================================================== */

#include "DestinationModifier.h"

#include "ConsoleView.h"
#include "Destination.h"
#include "DestinationView.h"
#include "MeVDoc.h"
#include "DestinationListView.h"
#include "MidiModule.h"
#include "IconMenuItem.h"

#include <stdio.h>
// Interface Kit
#include <CheckBox.h>
#include <ColorControl.h>
#include <MenuField.h>
#include <PopUpMenu.h>
#include <TextControl.h>
// Midi Kit
#include <MidiConsumer.h>
// Support Kit
#include <Debug.h>

#define D_HOOK(x) //PRINT(x)		// CAppWindow Implementation

using namespace Midi;

// ---------------------------------------------------------------------------
// Constructor/Destructor

CDestinationModifier::CDestinationModifier(
	BRect frame,
	int32 id,
	CMeVDoc *doc,
	BHandler *parent)
	:	CAppWindow(frame, "", B_TITLED_WINDOW,
				B_NOT_ZOOMABLE | B_NOT_RESIZABLE),
		m_dest(doc->FindDestination(id)),
		m_parent(parent)
{
	ASSERT(m_dest != NULL);

	CWriteLock lock(m_dest);
	m_dest->AddObserver(this);

	CDestinationView *view = new CDestinationView(Bounds(), m_dest);
	AddChild(view);
	float width, height;
	view->GetPreferredSize(&width, &height);
	view->ResizeTo(width, height);
	ResizeTo(width, height);

	_updateName();
}

CDestinationModifier::~CDestinationModifier()
{
	if (m_dest)
		m_dest->RemoveObserver(this);
}

// ---------------------------------------------------------------------------
// CAppWindow Implementation


bool 
CDestinationModifier::QuitRequested()
{
	BMessage *msg = new BMessage(WINDOW_CLOSED);
	msg->AddInt32("destination_id", m_dest->ID());
	BMessenger *amsgr = new BMessenger(m_parent);
	amsgr->SendMessage(msg);

	return false;
}

bool
CDestinationModifier::SubjectReleased(
	CObservable *subject)
{
	if (subject == m_dest)
	{
		m_dest->RemoveObserver(this);
		m_dest = NULL;
		Quit();
		return true;
	}

	return CAppWindow::SubjectReleased(subject);	
}

void
CDestinationModifier::SubjectUpdated(
	BMessage *message)
{
	D_HOOK(("CDestinationModifier::SubjectUpdated()\n"));

	int32 destAttrs;
	if (message->FindInt32("DestAttrs", &destAttrs) != B_OK)
		return;
	if (destAttrs & CDestination::Update_Name)
		_updateName();
}

void
CDestinationModifier::_updateName()
{
	CReadLock lock(m_dest);

	BString title;
	title << "Destination: ";
	title << m_dest->Name();
	SetTitle(title.String());
}

// END - DestinationModifier.cpp
