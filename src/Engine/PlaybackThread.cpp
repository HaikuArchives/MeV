/* ===================================================================== *
 * PlaybackThread.cpp (MeV/Engine)
 * ===================================================================== */
 
#include "PlaybackThread.h"
#include "PlaybackThreadTeam.h"
#include "Player.h"

// ---------------------------------------------------------------------------
// CPlaybackThread constructor

CPlaybackThread::CPlaybackThread(
	CPlaybackThreadTeam 	&inTeam,
	CTrack				*tr,
	CPlaybackThread		*par,
	long					start )
	: team( inTeam )
{
	uint8				threadsUsed[ cMaxNormalThread ];
	int					i;
	CPlaybackThreadTeam	*tm;
	CPlaybackThread		*thread;

	LOCK_PLAYER;

		// Set the thread-used array to all zeroes, then mark which thread id's
		// have been used
	memset( threadsUsed, 0, sizeof threadsUsed );

	for (	tm = (CPlaybackThreadTeam *)thePlayer.teamList.First();
			tm;
			tm = (CPlaybackThreadTeam *)tm->Next() )
	{
		for (	thread = (CPlaybackThread *)tm->threads.First();
				thread;
				thread = (CPlaybackThread *)thread->Next() )
		{
			threadsUsed[ thread->threadID ] = 1;
		}
	}

		// Pick an unused thread ID
	for (i = 0; i < cMaxNormalThread; i++)
	{
		if (threadsUsed[ i ] == 0) break;
	}
			
	threadID		= i;
	flags		= 0;
	track		= tr;
	parent		= par;
	
		// Tracks which are not root tracks have an implicit loop
	if (parent) flags |= Thread_ImplicitLoop;

	startTime = originTime = start;
	currentTime = 0;

	inTeam.threads.AddTail( this );
}

// ---------------------------------------------------------------------------
// copy constructor -- can copy to a different team

CPlaybackThread::CPlaybackThread( CPlaybackThreadTeam &team,
								CPlaybackThread &th )
	: team( team )
{
	LOCK_PLAYER;

	threadID		= th.threadID;
	flags		= th.flags;
	track		= th.track;
	parent		= th.parent;

	startTime	= th.startTime;
	originTime	= th.originTime;
	currentTime	= th.currentTime;

	team.threads.AddTail( this );
}

// ---------------------------------------------------------------------------
// CPlaybackThread destructor

CPlaybackThread::~CPlaybackThread()
{
	Remove();
}

// ---------------------------------------------------------------------------
// CEventThread constructor

void CPlaybackThread::ReQueue( CEventStack &stack, long time )
{
	Event			ev;

	ev.thread.start		= time;
	ev.thread.threadPtr	= this;
	ev.thread.command	= EvtType_ThreadMarker;
	stack.Push( ev );
}
