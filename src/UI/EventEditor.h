/* ===================================================================== *
 * EventEditor.h (MeV/UI)
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
 * History:
 *	1997		Talin
 *		Original implementation
 *	04/08/2000	cell
 *		General cleanup in preparation for initial SourceForge checkin
 *	11/27/2000	cell
 *		Separated from CEventRenderer and subclasses into
 *		EventRenderer.h/.cpp
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __C_EventEditor_H__
#define __C_EventEditor_H__

// General
#include "MeVSpec.h"
// Engine
#include "Event.h"
// Framework
#include "Observer.h"
// User Interface
#include "BorderView.h"
#include "StripView.h"
#include "TrackWindow.h"

/**
 *		Base class for event editor strip views.
 *		@author	Talin, Christoper Lenz.  
 */

class CEventRenderer;
class CEventTrack;
class CPolygon;
class CStripFrameView;
class EventMarker;
class EventOp;

const unsigned int MAX_PLAYBACK_MARKERS = 8;

class CEventEditor :
	public CStripView,
	public CObserver
{
	friend class		CEndEventRenderer;

public:							// Constants

	enum edit_mode
	{
								TOOL_SELECT,
								TOOL_CREATE,
								TOOL_ERASE,
								TOOL_TEXT,
								TOOL_GRID,
								TOOL_TEMPO
	};

	enum select_mode
	{
								RECTANGLE_SELECTION = 0,

								LASSO_SELECTION
	};

	enum EDragTypes {
		DragType_None = 0,
		DragType_Select,				// Dragging selection rect
		DragType_Lasso,					// Selection lasso
		DragType_TimeSelect,			// Selecting time regions
		DragType_Events,				// Dragging selected events
		DragType_CopyEvents,			// Dragging and copying selected events
		DragType_Create,				// Dragging a newly-created event
		DragType_Erase,					// Dragging the eraser
		DragType_Sculpt,				// Drag-editing of cartesian events
		DragType_DropTarget,			// The drop target of a drag & drop operation
		
		DragType_Count
	};

public:							// Constructor/Destructor

								/**	Constructor.	*/
								CEventEditor(
									CStripFrameView	&frame,
									BRect rect,
									const char *name,
									bool makeScroller = false,
									bool makeMagButtons = false);

								CEventEditor(
									CStripFrameView &frame,
									BRect rect,
									CEventTrack *track,
									const char *name,
									bool makeScroller = false,
									bool makeMagButtons = false );
									
									/**	Destructor.	*/	
	virtual						~CEventEditor();

public:							// Hook Functions

	// Construct a new event
	virtual bool				ConstructEvent(
									BPoint point)
								{ return false; }

	virtual const BCursor *		CursorFor(
									int32 editMode) const;

	// Hook functions called by dragging code.
	// Called each time mouse moves.
	virtual bool				DoDrag(
									BPoint point,
									ulong buttons);

	// Do additional audio feedback or selection for this event
	virtual void				DoEventFeedback(
									const Event &ev)
								{ }

	// Draw standard grid lines representing time
	virtual void				DrawGridLines(
									BRect updateRect);

	// Hook functions called by dragging code.
	// Called when mouse is released
	virtual void				FinishDrag(
									BPoint point,
									ulong buttons,
									bool commit);
	
	// Invalidate all selected events
	virtual void				InvalidateSelection();
	
	// Invaludate all selected events -- with a displacement
	virtual void				InvalidateSelection(
									EventOp &inDisplacement);

	// Remove any feedback artifacts for this event
	virtual void				KillEventFeedback()
								{ }
	
	// Hook functions called by dragging code.
	// Called when mouse is first pressed.
	virtual void				StartDrag(
									BPoint point,
									ulong buttons);

	virtual bool				SubjectReleased(
									CObservable *subject);

	virtual void				SubjectUpdated(
									BMessage *message);

	// Returns TRUE if this editor supports "shadowing"
	// of events being dragged in the inspector window.
	virtual bool				SupportsShadowing()
								{ return false; }

	// Conversion between time and coords
	virtual double				TimeToViewCoords(
									long timeVal) const;
	virtual long				ViewCoordsToTime(
									double relPos) const;

public:							// Accessors

	void						SetRendererFor(
									event_type type,
									CEventRenderer *renderer)
								{ m_renderers[type] = renderer; }

	CEventRenderer *			RendererFor(
									const Event &ev) const
								{ return m_renderers[ev.Command()]; }
	CEventRenderer *			RendererFor(
									event_type type) const
								{ return m_renderers[type]; }

	// Return the pending operation for this window
	EventOp *					DragOperation()
								{ return m_dragOp; }
	EventOp *					PendingOperation()
								{ return TrackWindow()->PendingOperation(); }

	CStripFrameView	&			FrameView() const
								{ return m_frame; }

	// Returns the address of the track associated with this editor
	CEventTrack *				Track() const
								{ return m_track; }

	// Return the address of our parent track edit window
	CTrackWindow *				TrackWindow() const
								{ return dynamic_cast<CTrackWindow *>(Window()); }

public:							// Operations

	// Helps subclasses in picking events
	long						PickDurationEvent( 
									const Event &ev,
									int yTop,
									int yBottom,
									BPoint pickPt,
									short &partCode);
	const Event *				PickEvent(
									EventMarker &resultMarker,
									const BPoint &pickPt,
									short &resultPartCode);

	// Return the time of the nearest grid line to the given time
	int32						SnapToGrid(
									int32 inTime,
									bool inInitial = false);

protected:						// Internal Operations

	// Add a point to the current lasso operation
	void						AddLassoPoint(
									BPoint &point);

	// Draw the echo of a created event being dragged
	void						DrawCreateEcho(
									int32 startTime,
									int32 stopTime);

	// Draw the echo of a bunch of events being dragged
	void						DrawEchoEvents(
									int32 startTime,
									int32 stopTime);

	// Draw the Lasso poly
	void						DrawLasso();

	// Draw markers showing playback position
	void						DrawPlaybackMarkers(
									int32 *inArray,
									int32 inCount,
									BRect inUpdateRect,
									bool inErase);
	
	// Draw selection rectangle
	void						DrawSelectRect();

	void						BeginLassoTracking(
									BPoint point);
	void						DoLassoTracking(
									BPoint point);
	void						EndLassoTracking();

	void						BeginRectangleTracking(
									BPoint point);
	void						DoRectangleTracking(
									BPoint point);
	void						EndRectangleTracking();

	// Done with lassoing...
	void						FinishLasso();

	// Return TRUE if there's a lasso in progress...
	bool						IsLassoInProgress()
								{ return m_lasso != NULL; }

	// Test for a rect inside the polygon
	bool						IsRectInLasso(
									const BRect &inExtent,
									bool inInclusive) const;

	// Get the current lasso frame
	BRect						LassoFrame() const;

	// Adjust the scroll range of this frame to match track
	void						RecalcScrollRangeH();
	
	// Update the markers showing playback position
	void						UpdatePBMarkers();

public:							// CStripView Implementation

	virtual void				KeyDown(
									const char *bytes,
									int32 numBytes);

	virtual void				MessageReceived(
									BMessage *message);

	virtual void				MouseDown(
									BPoint point);

	virtual void				MouseMoved(
									BPoint point,
									uint32 transit,
									const BMessage *message);

	virtual void				MouseUp(
									BPoint point);

	virtual void				SetScrollValue(
									float position,
									orientation orientation);

public:							// CObserver Implementation

	virtual bool				Released(
									CObservable *subject);

	virtual void				Updated(
									BMessage *message)
								{ Window()->PostMessage(message, this); }

private:						// Internal Operations

	void						_destinationAdded(
									BMessage *message);

	void						_destinationRemoved(
									BMessage *message);

	void						_destinationUpdated(
									BMessage *message);									

	void						_trackUpdated(
									BMessage *message);

protected:						// Instance Data

	// Which track we're editing
	CEventTrack *				m_track;

	CStripFrameView	&			m_frame;

	// Array of renderers for each event type
	CEventRenderer *			m_nullEventRenderer;
	CEventRenderer *			m_renderers[EvtType_Count];

	CPolygon *					m_lasso;

	// initial click position
	BPoint						m_clickPos;
	
	// which part was clicked
	short						m_clickPart;

	// What kind of drag
	short						m_dragType;

	// time-dragging offset
	long						m_timeDelta;
	// value dragging offset
	long						m_valueDelta;

	// Newly-created event
	Event						m_newEv;

	// For drag selection
	BPoint						m_anchorPos;
	BPoint						m_cursorPos;

	// Playback markers
	int32						m_pbMarkers[MAX_PLAYBACK_MARKERS];
	// Number of playback markers
	int32						m_pbCount;

	// Drag operator
	EventOp *					m_dragOp;

	// is currently dragging
	bool						m_dragging;
};

#endif /* __C_EventEditor_H__ */
