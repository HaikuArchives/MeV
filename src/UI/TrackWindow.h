/* ===================================================================== *
 * TrackWindow.h (MeV/User Interface)
 * ---------------------------------------------------------------------
 * License:
 *  The contents of this file are subject to the Mozilla Public
 *  License Version 1.1 (the "License"); you may not use this file
 *  except in compliance with the License. You may obtain a copy of
 *  the License at http://www.mozilla.org/MPL/
 *
 *  Software distributed under the License is distributed on an "AS
 *  IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 *  implied. See the License for the specific language governing
 *  rights and limitations under the License.
 *
 *  The Original Code is MeV (Musical Environment) code.
 *
 *  The Initial Developer of the Original Code is Sylvan Technical 
 *  Arts. Portions created by Sylvan are Copyright (C) 1997 Sylvan 
 *  Technical Arts. All Rights Reserved.
 *
 *  Contributor(s): 
 *		Christopher Lenz (cell)
 *
 * ---------------------------------------------------------------------
 * Purpose:
 *  Base class of windows that edit event tracks
 * ---------------------------------------------------------------------
 * History:
 *	1997		Talin
 *		Original implementation
 *	04/08/2000	cell
 *		General cleanup in preparation for initial SourceForge checkin
 *  05/19/2000	dwalton
 *		Quick fix of annoying seg-fault during MenuBeginning
 * ---------------------------------------------------------------------
 * To Do:
 * Further investigate menubeginning no-item seg-fault bug. 
 * ===================================================================== */

#ifndef __C_TrackWindow_H__
#define __C_TrackWindow_H__

#include "DocWindow.h"
#include "TrackEditFrame.h"
#include "EventOp.h"
#include "DynamicMenu.h"
#include "Idents.h"
#include "EventTrack.h"
#include "ResourceUtils.h"

class CScroller;
class CMeVDoc;
class CTrackOperation;

const int Ruler_Height	 = 12;

// ---------------------------------------------------------------------------
// A window which displays and edits strips

class CTrackWindow :
	public CDocWindow,
	public CObserver
{

public:							// Constants

	enum tool_id
	{
								TOOL_SELECT,
								TOOL_CREATE,
								TOOL_ERASE,
								TOOL_TEXT,
								TOOL_GRID,
								TOOL_TEMPO
	};

public:							// Constructor/Destructor

								CTrackWindow(
									BRect frame,
									CMeVDoc &inDocument,
									CEventTrack *inTrack);

	virtual						~CTrackWindow();

public:							// Hook Functions

	virtual int32				CurrentTool() = 0;
	
	// Display time of mouse event
	virtual void				DisplayMouseTime(
									CTrack *track,
									int32 time)
								{ }
	
	// Set which track we're editing
	virtual void				SelectActive(
									CEventTrack *track)
								{ }

public:							// Accessors

	// For windows which edit dual tracks, select which one
	// has selected events
	virtual CEventTrack *		ActiveTrack()
								{
									return track;
								}

	// Return a pointer to the track that this window is viewing
	CEventTrack *				Track()
								{
									return track;
								}

	// Get the pending operation
	EventOp *					PendingOperation()
								{
									return trackOp;
								}

	// Returns the default event duration
	int32						NewEventDuration()
								{
									return newEventDuration;
								}
	
public:							// Operations

	// Set the EventOp representing a pending operation
	void						SetPendingOperation(
									EventOp *op);
	
	// Finish the operation on this track
	void						FinishTrackOperation(
									int32 commit);

	// Show the editor preference window
	void						ShowPrefs();

	// Returns the type of new events to be created
	E_EventType					GetNewEventType(
									E_EventType defaultType)
								{
									if (newEventType >= EvtType_Count)
									{
										return defaultType;
									}
									return newEventType;
								}

public:							// CDocWindow Implementation

	// Returns a pointer to the current document
	CMeVDoc &					Document()
								{
									return (CMeVDoc &)document;
								}
	
	virtual void				MenusBeginning();

	virtual void				MessageReceived(
									BMessage* message);

	virtual void				WindowActivated(
									bool active);

public:							// CObserver Implementation

	// If the app wants us to stop looking at the document, then oblige it.
	virtual void				OnDeleteRequested(
									BMessage *message);

	virtual void				OnUpdate(
									BMessage *message);

protected:						// Internal Operations

	void						UpdateActiveSelection(
									bool active);

	void						CreateFrames(
									BRect frame,
									CTrack *track);

	void						CreateFileMenu(
									BMenuBar *menuBar);

protected:						// Instance Data

	CTrackEditFrame	*			stripFrame;

	CEventTrack *				track;

	CScroller *					stripScroll;

	EventOp *					trackOp;

//	CDynamicMenuInstance		plugInMenuInstance;

	// Type of newly created events
	enum E_EventType			newEventType;

	int32						newEventDuration;

	CWindowState				prefsWinState;
};

#endif /* __C_TrackWindow_H__ */
