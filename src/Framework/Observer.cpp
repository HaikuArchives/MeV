/* ===================================================================== *
 * Observer.cpp (MeV/Framework)
 * ===================================================================== */

#include "Observer.h"

#include "AppHelp.h"
#include "Observable.h"

// Application Kit
#include <Looper.h>
// Support Kit
#include <Debug.h>

// ---------------------------------------------------------------------------
//	CObserver: Construction & Destruction

CObserver::CObserver(
	BLooper &looper,
	CObservableSubject *subject)
	:	BHandler("Observer"),
		m_subject(subject)
{
	// Add us to the looper, and add us to the list of observers for the subject
	looper.Lock();
	looper.AddHandler(this);
	if (subject)
		subject->AddObserver(*this);
	looper.Unlock();
}

CObserver::~CObserver()
{
	//	Remove us from the looper and the observer's subject list.
	if (m_subject)
	{
		m_subject->RemoveObserver(*this);
		CRefCountObject::Release(m_subject);
	}

	Looper()->RemoveHandler(this);
}

// ---------------------------------------------------------------------------
//	Operations

void
CObserver::SetSubject(
	CObservableSubject *subject)
{
	if (subject != m_subject)
	{
		Looper()->Lock();
		if (m_subject)
		{
			m_subject->RemoveObserver(*this);
			CRefCountObject::Release(m_subject);
		}
		m_subject = subject;
		if (m_subject)
			m_subject->AddObserver(*this);
		Looper()->Unlock();
	}
}

void
CObserver::PostUpdate(
	CUpdateHint *hint,
	bool excludeOriginal)
{
	if (m_subject)
		m_subject->PostUpdate(hint, excludeOriginal ? this : NULL );
}

// ---------------------------------------------------------------------------
//	BHandler Implementation

void
CObserver::MessageReceived(
	BMessage *message)
{
	switch (message->what)
	{
		case Update_ID:
		{	
			// Notify that subject has changed.
			OnUpdate(message);
			break;
		}	
		case Delete_ID:
		{
			// Request observers to let go of observable.
			OnDeleteRequested(message);
			break;
		}
		default:
		{
			BHandler::MessageReceived(message);
			break;
		}
	}
}

// END - Observer.cpp
