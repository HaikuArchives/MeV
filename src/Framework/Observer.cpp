/* ===================================================================== *
 * Observer.cpp (MeV/Framework)
 * ===================================================================== */

#include "Observer.h"

#include "Observable.h"

// Support Kit
#include <Debug.h>

// ---------------------------------------------------------------------------
//	Constructor/Destructor

CObserver::CObserver(
	CObservable *subject)
{
	// Add us to the list of observers for the subject
	if (subject)
		subject->AddObserver(this);
}

CObserver::~CObserver()
{
}

// END - Observer.cpp
