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

CObservable::CObservable()
{
}

CObservable::~CObservable()
{
	D_OBSERVE(("CObservable::~CObservable()\n"));

#if DEBUG
	if (m_observers.CountItems() > 0)
	{
		D_OBSERVE((" -> %ld observers still listening!\n",
			   m_observers.CountItems()));
	}
#endif

	RequestDelete();
}

// ---------------------------------------------------------------------------
//	Observer list management

bool
CObservable::AddObserver(
	CObserver *observer)
{
	StSubjectLock lock(*this, Lock_Exclusive);
	return m_observers.AddItem(observer);
}

bool
CObservable::RemoveObserver(
	CObserver *observer)
{
	StSubjectLock lock(*this, Lock_Exclusive);
	return m_observers.RemoveItem(observer);
}

// ---------------------------------------------------------------------------
//	CObservable: Update messaging

void
CObservable::PostUpdate(
	CUpdateHint *hint,
	CObserver *excludeObserver)
{
	D_OBSERVE(("CObservable::PostUpdate()\n"));

	StSubjectLock myLock(*this, Lock_Shared);

	//	For each observer, send them a copy of the message.
	int32 count = m_observers.CountItems();
	D_OBSERVE((" -> notify %ld observers\n", count));
	for (int i = 0; i < count; i++)
	{
		CObserver *ob = (CObserver *)m_observers.ItemAt(i);
		if (ob != excludeObserver)
			ob->Updated(hint);
	}
}

void
CObservable::RequestDelete()
{
	D_OBSERVE(("CObservable<%p>::RequestDelete()\n", this));

	int32 count = 0;
	if (Lock(Lock_Shared))
	{
		count = m_observers.CountItems();
		Unlock(Lock_Shared);
	}

	while (count > 0)
	{
		D_OBSERVE((" -> %ld observers hanging on\n", count));

		Lock(Lock_Shared);
		CObserver *ob = (CObserver *)m_observers.ItemAt(count - 1);
		Unlock(Lock_Shared);

		D_OBSERVE((" -> releasing observer at %p\n", ob));
		if (ob->Released(this) == false)
		{
			D_OBSERVE((" !! observer did not release subject\n"));
			RemoveObserver(ob);
		}

		Lock(Lock_Shared);
		count = m_observers.CountItems();
		Unlock(Lock_Shared);
	}
}

// END - Observable.cpp
