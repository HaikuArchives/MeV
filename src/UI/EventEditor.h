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
 * ---------------------------------------------------------------------
 * Purpose:
 *  base class for event editor strip views
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
// Standard Template Library
#include <map>

class CAbstractEventHandler;
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
	friend class		CEndEventHandler;

public:							// Constants

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

								CEventEditor(
									BLooper	&looper,
									CStripFrameView	&frame,
									BRect rect,
									const char *name,
									bool makeScroller = false,
									bool makeMagButtons = false);

								CEventEditor(
									BLooper &looper,
									CStripFrameView &frame,
									BRect rect,
									CEventTrack *track,
									const char *name,
									bool makeScroller = false,
									bool makeMagButtons = false );
	
	virtual						~CEventEditor();

public:							// Hook Functions

	// Construct a new event
	virtual bool				ConstructEvent(
									BPoint point)
								{ return false; }

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

	void						SetHandlerFor(
									TEventType eventType,
									CAbstractEventHandler *handler)
								{ m_handlers[eventType] = handler; }

	// Look up a handler for a given event
	const CAbstractEventHandler &Handler(
									const Event &ev)
								{ return *HandlerFor(ev); }

	CAbstractEventHandler *		HandlerFor(
									const Event &ev) const;

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

	// Update message from another observer
	virtual void				OnUpdate(
									BMessage *message)
								{ Invalidate(); }

protected:						// Instance Data

	// Which track we're editing
	CEventTrack *				m_track;

	CStripFrameView	&			m_frame;

	// Array of handlers for each event type
	typedef map<event_type, CAbstractEventHandler *> handler_map;
	handler_map					m_handlers;

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

// ---------------------------------------------------------------------------
//  Abstract event handler.
//
// This class represents all of the functions that
// normally need to operate on events which depend both
// on the event type and on the view type (in other words,
// it's a lame attempt to make up for the lack of multi-methods
// in C++)
	
// ONLY functions which need to differ based on the type
// of the event AND the type of the view should go in here.

class CAbstractEventHandler {
public:
		// No constructor

		// Invalidate the event
	virtual	void Invalidate(	CEventEditor	&editor,
								const Event		&ev ) const = 0;

		// Draw the event (or an echo)
	virtual	void Draw(			CEventEditor	&editor,
								const Event		&ev,
								bool 			shadowed ) const = 0;

		// Invalidate the event
	virtual	BRect Extent(		CEventEditor	&editor,
								const Event		&ev ) const
	{
		return BRect( 0, 0, 0, 0 );
	}

		// Pick a single event and return the part code
		// (or -1 if event not picked)
	virtual long Pick(	CEventEditor	&editor,
						const Event		&ev,
						BPoint			pickPt,
						short			&partCode ) const
	{
		return -1;
	}
	
		// For a part code returned earlier, return a cursor
		// image...
	virtual const uint8 *CursorImage( short partCode ) const
	{
		return NULL;
	}
	
		// Quantize the time of the dragging operation and
		// return a time delta.
	virtual long QuantizeDragTime(
		CEventEditor	&editor,
		const Event		&inClickEvent,
		short			partCode,			// Part of event clicked
		BPoint			inClickPos,
		BPoint			inDragPos,
		bool				inInitial = false ) const;

		// Quantize the vertical position of the mouse based
		// on the event type and return a value delta.
	virtual long QuantizeDragValue(
		CEventEditor	&editor,
		const Event		&inClickEvent,
		short			partCode,			// Part of event clicked
		BPoint			inClickPos,
		BPoint			inDragPos ) const
	{
		return static_cast<long>(inClickPos.y - inDragPos.y);
	}

		// Drag operation. What this function does is to create
		// a function object to implement the drag operation.
		// This function object can modify the events in question
		// in a manner consistent with the part of the event
		// clicked and the current mouse position.
		//
		// This is used in two ways. First, it is used to create
		// temporary proxy events which are rendered as a drag
		// echo. Secondly, it is used to permanently modify the
		// events when the drag operation is completed.
	virtual EventOp *CreateDragOp(
		CEventEditor	&editor,			// The editor
		const Event		&ev,				// The clicked event
		short			partCode,			// Part of event clicked
		long			timeDelta,			// The horizontal drag delta
		long			valueDelta ) const
	{
		return NULL;
	}

	virtual EventOp *CreateTimeOp(
		CEventEditor	&editor,			// The editor
		const Event		&ev,				// The clicked event
		short			partCode,			// Part of event clicked
		long			timeDelta,			// The horizontal drag delta
		long			valueDelta ) const;
};

// ---------------------------------------------------------------------------
// The null handler does nothing to the event.

class CNullEventHandler : public CAbstractEventHandler {
public:
		// Invalidate the event
	void Invalidate(	CEventEditor	&editor,
						const Event		&ev ) const {};

		// Draw the event (or an echo)
	void Draw(			CEventEditor	&editor,
						const Event		&ev,
						bool 			shadowed ) const {};
};

// ---------------------------------------------------------------------------
// Event handler class for "end" events

class CEndEventHandler : public CAbstractEventHandler {

		// No constructor

		// Invalidate the event
	void Invalidate(	CEventEditor	&editor,
						const Event		&ev ) const ;

		// Draw the event (or an echo)
	void Draw(			CEventEditor	&editor,
						const Event		&ev,
						bool 			shadowed ) const;
		// Invalidate the event
	BRect Extent(		CEventEditor	&editor,
						const Event		&ev ) const;

		// Pick a single event and returns the distance.
	long Pick(			CEventEditor	&editor,
						const Event		&ev,
						BPoint			pickPt,
						short			&partCode ) const;

		// For a part code returned earlier, return a cursor
		// image...
	const uint8 *CursorImage( short partCode ) const;

		// Quantize the vertical position of the mouse based
		// on the event type and return a value delta.
	long QuantizeDragValue(
		CEventEditor	&editor,
		const Event		&inClickEvent,
		short			partCode,			// Part of event clicked
		BPoint			inClickPos,
		BPoint			inDragPos ) const;

		// Make a drag op for dragging notes...
	EventOp *CreateDragOp(
		CEventEditor	&editor,
		const Event		&ev,
		short			partCode,
		long			timeDelta,			// The horizontal drag delta
		long			valueDelta ) const;
};

// ---------------------------------------------------------------------------
// Global null handler

extern CNullEventHandler	gNullEventHandler;
extern CEndEventHandler	gEndEventHandler;

#endif /* __C_EventEditor_H__ */