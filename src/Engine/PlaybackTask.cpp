/* ===================================================================== *
 * PlaybackTask.cpp (MeV/Engine)
 * ===================================================================== */
 
#include "PlaybackTask.h"
#include "PlaybackTaskGroup.h"
#include "Player.h"

// ---------------------------------------------------------------------------
// CPlaybackTask constructor

CPlaybackTask::CPlaybackTask(
	CPlaybackTaskGroup 	&inGroup,
	CTrack				*tr,
	CPlaybackTask		*par,
	long					start )
	: group( inGroup )
{
	uint8				tasksUsed[ cMaxNormalTask ];
	int					i;
	CPlaybackTaskGroup	*tm;
	CPlaybackTask		*task;

	LOCK_PLAYER;

		// Set the task-used array to all zeroes, then mark which task id's
		// have been used
	memset( tasksUsed, 0, sizeof tasksUsed );

	for (	tm = (CPlaybackTaskGroup *)thePlayer.m_groupList.First();
			tm;
			tm = (CPlaybackTaskGroup *)tm->Next() )
	{
		for (	task = (CPlaybackTask *)tm->tasks.First();
				task;
				task = (CPlaybackTask *)task->Next() )
		{
			tasksUsed[ task->taskID ] = 1;
		}
	}

		// Pick an unused task ID
	for (i = 0; i < cMaxNormalTask; i++)
	{
		if (tasksUsed[ i ] == 0) break;
	}
			
	taskID		= i;
	flags		= 0;
	track		= tr;
	parent		= par;
	
		// Tracks which are not root tracks have an implicit loop
	if (parent) flags |= Task_ImplicitLoop;

	startTime = originTime = start;
	currentTime = 0;

	inGroup.tasks.AddTail( this );
}

// ---------------------------------------------------------------------------
// copy constructor -- can copy to a different group

CPlaybackTask::CPlaybackTask( CPlaybackTaskGroup &group,
								CPlaybackTask &th )
	: group( group )
{
	LOCK_PLAYER;

	taskID		= th.taskID;
	flags		= th.flags;
	track		= th.track;
	parent		= th.parent;

	startTime	= th.startTime;
	originTime	= th.originTime;
	currentTime	= th.currentTime;

	group.tasks.AddTail( this );
}

// ---------------------------------------------------------------------------
// CPlaybackTask destructor

CPlaybackTask::~CPlaybackTask()
{
	Remove();
}

// ---------------------------------------------------------------------------
// CEventTask constructor

void CPlaybackTask::ReQueue( CEventStack &stack, long time )
{
	Event			ev;

	ev.task.start		= time;
	ev.task.taskPtr	= this;
	ev.task.command	= EvtType_TaskMarker;
	stack.Push( ev );
}
