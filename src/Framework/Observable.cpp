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

CObservable::CObservable(
	const char *name)
	:	CLockable(name)
{
	D_OBSERVE(("CObservable::CObservable(%s)\n", name ? name : "NULL"));

}

CObservable::~CObservable()
{
	D_OBSERVE(("CObservable::~CObservable()\n"));

	RequestDelete();
}

// ---------------------------------------------------------------------------
//	Observer list management

bool
CObservable::AddObserver(
	CObserver *observer)
{
	ASSERT(observer != NULL);

	CWriteLock lock(this);
	return m_observers.AddItem(observer);
}

bool
CObservable::IsObservedBy(
	CObserver *observer)
{
	ASSERT(observer != NULL);

	CReadLock lock(this);
	return m_observers.HasItem(observer);
}

bool
CObservable::RemoveObserver(
	CObserver *observer)
{
	ASSERT(observer != NULL);

	CWriteLock lock(this);
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

	CReadLock lock(this);

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
	if (ReadLock())
	{
		count = m_observers.CountItems();
		ReadUnlock();
	}

	while (count > 0)
	{
		D_OBSERVE((" -> %ld observers hanging on\n", count));

		ReadLock();
		CObserver *ob = (CObserver *)m_observers.LastItem();
		ReadUnlock();

		D_OBSERVE((" -> releasing observer at %p\n", ob));
		ASSERT(ob != NULL);
		if (ob->Released(this) == false)
		{
			D_OBSERVE((" !! observer did not release subject\n"));
			RemoveObserver(ob);
		}

		ReadLock();
		count = m_observers.CountItems();
		ReadUnlock();
	}
}

// END - Observable.cpp
