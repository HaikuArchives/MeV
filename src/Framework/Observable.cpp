/* ===================================================================== *
 * Observable.cpp (MeV/Framework)
 * ===================================================================== */

#include "Observable.h"

#include "AppHelp.h"
#include "Observer.h"

// Application Kit
#include <Looper.h>
// Support Kit
#include <Debug.h>

// ---------------------------------------------------------------------------
// Constructor/Destructor

CObservableSubject::CObservableSubject()
{
}

CObservableSubject::~CObservableSubject()
{
}

// ---------------------------------------------------------------------------
//	CObservableSubject: Observer list management

void
CObservableSubject::AddObserver(
	CObserver &observer)
{
	StSubjectLock myLock(*this, Lock_Exclusive);
	Acquire();
	CheckSuccess(m_observers.AddItem(&observer));
}

void
CObservableSubject::RemoveObserver(
	CObserver &observer)
{
	StSubjectLock myLock(*this, Lock_Exclusive);
	CheckSuccess(m_observers.RemoveItem(&observer));
}

// ---------------------------------------------------------------------------
//	CObservableSubject: Update messaging

void
CObservableSubject::PostUpdate(
	CUpdateHint *hint,
	CObserver *excludeObserver)
{
	int32 count = m_observers.CountItems();
	StSubjectLock myLock(*this, Lock_Shared);

	//	For each observer, send them a copy of the message.
	for (int i = 0; i < count; i++)
	{
		CObserver *ob = (CObserver *)m_observers.ItemAt(i);
		if (ob != excludeObserver)
			BMessenger(ob).SendMessage(hint);
	}
}

void
CObservableSubject::RequestDelete()
{
	CUpdateHint msg(Delete_ID);
	
	PostUpdate(&msg, NULL);
}

// END - Observable 