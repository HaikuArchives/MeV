/* ===================================================================== *
 * Observer.cpp (MeV/Application Framework)
 * ===================================================================== */

#include "Observer.h"
#include "AppHelp.h"

// Application Kit
#include <Looper.h>
// Support Kit
#include <Debug.h>

// ---------------------------------------------------------------------------
//	CObserver: Construction & Destruction

CObserver::CObserver( BLooper &inLooper, CObservableSubject *inSubject )
	:	BHandler( "CObserver" ),
		subject( inSubject )
{
		//	Add us to the looper, and add us to the list of observers for the subject
	inLooper.Lock();
	inLooper.AddHandler( this );
	if (subject) subject->AddObserver( *this );
	inLooper.Unlock();
}

CObserver::~CObserver()
{
		//	Remove us from the looper and the observer's subject list.
	if (subject)
	{
		subject->RemoveObserver( *this );
		CRefCountObject::Release( subject );
	}
	ASSERT( Looper() );
	Looper()->RemoveHandler( this );
}

void CObserver::SetSubject( CObservableSubject *inSubject )
{
	if (inSubject != subject)
	{
		Looper()->Lock();
		if (subject)
		{
			subject->RemoveObserver( *this );
			CRefCountObject::Release( subject );
		}
		subject = inSubject;
		if (subject) subject->AddObserver( *this );
		Looper()->Unlock();
	}
}

// ---------------------------------------------------------------------------
//	CObserver: Message handling

	//	Post an update hint to the other observers
void CObserver::PostUpdate( CUpdateHint *inHint, bool inExcludeOriginal )
{
	if (subject) subject->PostUpdate( inHint, inExcludeOriginal ? this : NULL );
}

	//	Receive an update message from the other observers and dispatch
void CObserver::MessageReceived( BMessage *inMessage )
{
	switch (inMessage->what) {
	case Update_ID:

			//	Notify that subject has changed.
		OnUpdate( inMessage );
		break;
		
	case Delete_ID:

			//	Request observers to let go of observable.
		OnDeleteRequested( inMessage );
		break;

	default:
		BHandler::MessageReceived( inMessage );
		break;
	};
}
	
// ---------------------------------------------------------------------------
//	CObservableSubject: Construction & Destruction

CObservableSubject::CObservableSubject()
{
}

CObservableSubject::~CObservableSubject()
{
}

// ---------------------------------------------------------------------------
//	CObservableSubject: Observer list management

void CObservableSubject::AddObserver( CObserver &inObserver )
{
	StSubjectLock	myLock( *this, Lock_Exclusive );
	Acquire();
	CheckSuccess( observers.AddItem( &inObserver ) );
}

void CObservableSubject::RemoveObserver( CObserver &inObserver )
{
	StSubjectLock	myLock( *this, Lock_Exclusive );
	CheckSuccess( observers.RemoveItem( &inObserver ) );
}

// ---------------------------------------------------------------------------
//	CObservableSubject: Update messaging

void CObservableSubject::PostUpdate( CUpdateHint *inHint, CObserver *inExcludeObserver )
{
	int32		count = observers.CountItems();
	StSubjectLock	myLock( *this, Lock_Shared );

		//	For each observer, send them a copy of the message.
	for (int i = 0; i < count; i++)
	{
		CObserver		*ob = (CObserver *)observers.ItemAt( i );

		ASSERT( ob );
		ASSERT( ob->Looper() );

		if (ob && ob != inExcludeObserver)
			ob->Looper()->PostMessage( inHint, ob );
	}
}

	/*	Request to delete. This means asking all observers to
		release the subject, so that the subject's reference count
		will go to zero and the subject will be deleted.
	*/
void CObservableSubject::RequestDelete()
{
	CUpdateHint		msg( Delete_ID );
	
	PostUpdate( &msg, NULL );
}
