/* ===================================================================== *
 * MeVModule.cpp (MeV/Midi)
 * ===================================================================== */

#include "MeVModule.h"

#include "Observable.h"

// Support Kit
#include <Debug.h>

// Debugging Macros
#define D_ALLOC(x) //PRINT(x)		// Constructor/Destructor
#define D_ACCESS(x) //PRINT(x)		// Accessors
#define D_HOOK(x) //PRINT(x)		// Hook Functions
#define D_MESSAGE(x) //PRINT(x)		// MessageReceived()

// ---------------------------------------------------------------------------
// Constructor/Destructor

CMeVModule::CMeVModule(
	uint32 type,
	const char *name)
	:	BLooper(name),
		m_type(type)
{
	D_ALLOC(("CMeVModule::CMeVModule()\n"));

}

CMeVModule::~CMeVModule()
{
	D_ALLOC(("CMeVModule::~CMeVModule()\n"));

}

// ---------------------------------------------------------------------------
// Hook Functions

CDestination *
CMeVModule::CreateDestination(
	CMeVDoc *document,
	int32 *id,
	const char *name)
{
	D_HOOK(("CMeVModule::CreateDestination()\n"));

	return NULL;
}

void
CMeVModule::DocumentOpened(
	CMeVDoc *document)
{
	D_HOOK(("CMeVModule::DocumentOpened()\n"));
}

bool
CMeVModule::SubjectReleased(
	CObservable *subject)
{
	D_HOOK(("CMeVModule::SubjectReleased()\n"));
}

void
CMeVModule::SubjectUpdated(
	BMessage *message)
{
	D_HOOK(("CMeVModule::SubjectUpdated()\n"));
}

// ---------------------------------------------------------------------------
// BLooper Implementation

void
CMeVModule::MessageReceived(
	BMessage *message)
{
	D_MESSAGE(("CMeVModule::MessageReceived()\n"));

	switch (message->what)
	{
		case CObservable::UPDATED:
		{
			D_MESSAGE((" -> CObserver::UPDATED\n"));

			SubjectUpdated(message);
			break;
		}
		default:
		{
			BLooper::MessageReceived(message);
		}
	}
}

// ---------------------------------------------------------------------------
// Accessors

unsigned long
CMeVModule::Type() const
{
	D_ACCESS(("CMeVModule::Type()\n"));

	return m_type;
}

// ---------------------------------------------------------------------------
// CObserver Implementation

bool
CMeVModule::Released(
	CObservable *subject)
{
	bool released = false;

	if (Lock())
	{
		released = SubjectReleased(subject);
		Unlock();
	}

	return released;
}

void
CMeVModule::Updated(
	BMessage *message)
{
	PostMessage(message, this);
}

// END - MeVModule.cpp
