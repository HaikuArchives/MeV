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
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __C_TrackWindow_H__
#define __C_TrackWindow_H__

#include "DocWindow.h"
#include "TrackEditFrame.h"
#include "EventOp.h"
#include "DynamicMenu.h"
#include "Idents.h"
#include "EventTrack.h"

class CScroller;
class CMeVDoc;
class CTrackOperation;

const int Ruler_Height	 = 12;

enum ETrackWinToolIDs {
	TOOL_SELECT = 0,
	TOOL_CREATE,
	TOOL_ERASE,
	TOOL_TEXT,
	TOOL_GRID,
	TOOL_TEMPO,
};

// ---------------------------------------------------------------------------
// A window which displays and edits strips

class CTrackWindow : public CDocWindow, public CObserver {
protected:
	CTrackEditFrame		*stripFrame;
	CEventTrack			*track;
	CScroller			*stripScroll;
	EventOp				*trackOp;
	CDynamicMenuInstance	plugInMenuInstance;
	enum E_EventType		newEventType;	// Type of newly created events
	int32				newEventDuration;

		// Addresses of menus (for disabling)
	BMenuItem			*undoMenu,
						*redoMenu,
						*clearMenu,
						*playMenu,
						*setSectionMenu,
						*pauseMenu,
						*inspectorMenu,
						*gridWindowMenu,
						*transportMenu;
	
	BMenu				*plugInMenu;

	void UpdateActiveSelection( bool inActive );
	void WindowActivated( bool inActive );
	void MessageReceived( BMessage* theMessage );
	void MenusBeginning();
	void CreateFrames( BRect frame, CTrack *inTrack );
	void OnDeleteRequested( BMessage *inMsg );
	void OnUpdate( BMessage *inMsg );

	CWindowState		prefsWinState;
	
	void CreateFileMenu( BMenuBar *inBar );
	
public:
	CTrackWindow( BRect frame, CMeVDoc &inDocument, CEventTrack *inTrack );
	~CTrackWindow();

		// ---------- Getters

		/**	Returns a pointer to the current document */
	CMeVDoc &Document() { return (CMeVDoc &)document; }
	
		/**	Return a pointer to the track that this window is viewing. */
	CEventTrack *Track() { return track; }
	
		/**	Set the EventOp representing a pending operation. */
	void SetPendingOperation( EventOp *inOp );
	
		/**	Get the pending operation. */
	EventOp *PendingOperation() { return trackOp; }

		/**	Finish the operation on this track. */
	void FinishTrackOperation( int32 inCommit );

		/**	Show the editor preference window. */
	void ShowPrefs();

		/**	Overridden by Assembly and Linear windows. */
	virtual int32 CurrentTool() = 0;
	
		/**	Returns the type of new events to be created. */
	enum E_EventType GetNewEventType( enum E_EventType inDefault )
	{
		if (newEventType >= EvtType_Count) return inDefault;
		return newEventType;
	}
	
		/**	Returns the default event duration */
	int32 GetNewEventDuration() { return newEventDuration; }
	
		/**	Display time of mouse event. */
	virtual void DisplayMouseTime( CTrack *track, int32 time ) {}
	
		/**	For windows which edit dual tracks, select which one
			has selected events. */
	virtual CEventTrack *ActiveTrack() { return track; }

		/**	Set which track we're editing. */	
	virtual void SelectActive( CEventTrack * ) {}
};

	/**	Add this filter function to any control which would normally
		accept the focus. This will cause it to lose focus on any
		TAB or RETURN function.
	*/
filter_result DefocusTextFilterFunc(
	BMessage			*msg,
	BHandler			**target,
	BMessageFilter	*messageFilter );

// ---------------------------------------------------------------------------
// Ruler view for asembly window

class CAssemblyRulerView : public CRulerView, public CObserver {
	bool showMarkers;

protected:

	void OnUpdate( BMessage * );
	void Draw( BRect updateRect );
	void MouseDown( BPoint point );
	void AttachedToWindow()
	{
//		SetViewColor( 255, 255, 220 );
		SetViewColor( B_TRANSPARENT_32_BIT );
	}
	
public:
	CAssemblyRulerView(	BLooper			&inLooper,
						CTrackEditFrame	&inFrameView,
						CEventTrack		*inTrack,
						BRect			rect,
						const char		*name,
						ulong			resizeMask,
						ulong			flags)
		:	CRulerView( inFrameView, rect, name, resizeMask, flags ),
			CObserver( inLooper, inTrack )
	{
		showMarkers = true;
		LoadImage( sectionMarkerImage, SectionMarker_Image );
	}
	
	void ShowMarkers( bool inShow )
	{
		if (inShow != showMarkers)
		{
			showMarkers = inShow;
			Invalidate();
		}
	}
};

#endif /* __C_TrackWindow_H__ */
