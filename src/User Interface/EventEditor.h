/* ===================================================================== *
 * EventEditor.h (MeV/User Interface)
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
#include "Polygon.h"
#include "StripView.h"
#include "TrackWindow.h"

class CAbstractEventHandler;

const int			Max_PB_Markers = 8;

class CEventEditor :
	public CStripView,
	public CObserver
{
	friend class		CEndEventHandler;
	
public:								// Constructor/Destructor

									CEventEditor(
										BLooper	&inLooper,
										CTrackEditFrame	&inFrame,
										BRect rect,
										const char *name,
										bool makeScroller = false,
										bool makeMagButtons = false);
					
									CEventEditor(
										BLooper &inLooper,
										CTrackEditFrame &inFrame,
										BRect rect,
										CEventTrack *inTrack,
										const char *name,
										bool makeScroller = false,
										bool makeMagButtons = false );
	
	virtual								~CEventEditor();

protected:

	CEventTrack		*track;				// Which track we're editing
	CTrackEditFrame	&frame;
	BView			*labelView;
	CPolygon			*lassoPoints;

		// Dragging variables
	BPoint			clickPos;		// initial click position
	short			clickPart;		// which part was clicked
	short			dragType;		// What kind of drag
	long				timeDelta,		// time-dragging offset
					valueDelta;		// value dragging offset
	Event			newEv;			// Newly-created event

		// For drag selection -- used by subclasses
	BPoint			anchorPos,				// For drag selection
					cursorPos;

		// For showing playback markers...					
	int32			pbMarkers[ Max_PB_Markers ];	// Playback markers
	int32			pbCount;						// Number of playback markers

		// Individual strips can have rulers as well.
	CScrollerTarget	*ruler;
	
		// Used for subclasses which are dragging events
	EventOp			*dragOp;					// Drag operator

		/** Return the address of our parent track edit window. */
	CTrackWindow *TrackWindow() { return dynamic_cast<CTrackWindow *>(Window()); }

		/** Array of handlers for each event type. */
	CAbstractEventHandler *handlers[ EvtType_Count ];
	
	void Init();

public:

	enum EDragTypes {
		DragType_None = 0,
		DragType_Select,				// Dragging selection rect
		DragType_Lasso,				// Selection lasso
		DragType_TimeSelect,			// Selecting time regions
		DragType_Events,				// Dragging selected events
		DragType_CopyEvents,			// Dragging and copying selected events
		DragType_Create,				// Dragging a newly-created event
		DragType_Sculpt,				// Drag-editing of cartesian events
		DragType_DropTarget,			// The drop target of a drag & drop operation
		
		DragType_Count
	};

		// ---------- Getters

		/**	Returns the address of the track associated with this editor. */
	CEventTrack *Track() { return track; }

		/** Returns the document associated with this editor. */
	CMeVDoc &Document() const
	{
		return ((CTrackWindow *)Window())->Document();
	}
	
		// ---------- Subclass helpers

	const Event *PickEvent(
		EventMarker		&resultMarker,
		const BPoint	&pickPt,
		short			&resultPartCode );

		// Helps subclasses in picking events
	long PickDurationEvent( 
			const Event		&ev,
			int				yTop,
			int				yBottom,
			BPoint			pickPt,
			short			&partCode );

		// ---------- Hooks

	void MouseDown( BPoint point );

		/**	Hook functions called by dragging code.
			Called whem mouse is first pressed.
		*/
	virtual void StartDrag( BPoint point, ulong buttons );

		/**	Hook functions called by dragging code.
			Called each time mouse moves.
		*/
	virtual bool DoDrag( BPoint point, ulong buttons );

		/**	Hook functions called by dragging code.
			Called when mouse is released
		*/
	virtual void FinishDrag( BPoint point, ulong buttons, bool commit );
	
		/** Construct a new event */
	virtual bool ConstructEvent( BPoint point ) { return false; }

		/**	Update message from another observer */
	virtual void OnUpdate( BMessage * )
	{
		Invalidate();
	}
	
		/** Do additional audio feedback or selection for this event.. */
	virtual void DoEventFeedback( const Event &ev ) {}

		/** Remove any feedback artifacts for this event. */
	virtual void KillEventFeedback() {}
	
		/**	Returns TRUE if this editor supports "shadowing"
			of events being dragged in the inspector window. */
	virtual bool SupportsShadowing() { return false; }

		/**	Draw standard grid lines representing time. */
	virtual void DrawGridLines( BRect updateRect );

		/**	Look up a handler for a given event. */
	virtual const CAbstractEventHandler &Handler( const Event &ev ) const
	{
		return *handlers[ ev.Command() ];
	}
	
		/**	Invalidate all selected events. */
	virtual void InvalidateSelection();
	
		/**	Invaludate all selected events -- with a displacement */
	virtual void InvalidateSelection( EventOp &inDisplacement );
	
		/**	Return the pending operation for this window. */
	EventOp *PendingOperation() { return TrackWindow()->PendingOperation(); }

		/** Draw the echo of a created event being dragged */
	void DrawCreateEcho( int32 startTime, int32 stopTime );

		/** Draw the echo of a bunch of events being dragged */
	void DrawEchoEvents( int32 startTime, int32 stopTime );

		/**	Return the current grid-snap setting in time units. */
// long TimeGridSize() { return Track()->TimeGridSize(); }

		/**	Return the time of the nearest grid line to the given time. */
	int32 SnapToGrid( int32 inTime, bool inInitial = false );

		/**	Convert a unit of time to horizontal pixel coords. */
	virtual double TimeToViewCoords( long timeVal )
	{
		return frame.TimeToViewCoords( timeVal, track->ClockType() );
	}

		/**	Convert a horizontal pixel coordinate to unit of time. */
	virtual long ViewCoordsToTime( double relPos )
	{
		return frame.ViewCoordsToTime( relPos, track->ClockType() );
	}
	
		/**	Adjust the scroll range of this frame to match track. */
	void RecalcScrollRangeH()
	{
		frame.RecalcScrollRange();
	}
	
		// ---------- Setters

	void SetScrollValue( float inScrollValue, orientation inOrient )
	{
		CStripView::SetScrollValue( inScrollValue, inOrient );
		if (ruler) ruler->ScrollTo( scrollValue.x, 0.0 );
	}

		// ---------- Rulers and markers

		/**	Add an individual ruler to this strip. */
	void SetRuler( CScrollerTarget *inRuler );
	
		/** Update the markers showing playback position. */
	void UpdatePBMarkers();

		/** Draw markers showing playback position */
	void DrawPBMarkers( int32 *inArray, int32 inCount, BRect inUpdateRect, bool inErase );
	
		// ---------- Selection rectangle operations

		/** Draw selection rectangle */
	void DrawSelectRect();

		/** Select all events within the current selection rectangle */
	void DoRectangleSelection();

		// ---------- Lasso operations
		
		/**	Add a point to the current lasso operation. */
	void AddLassoPoint( BPoint &inPoint );
	
		/**	Return TRUE if there's a lasso in progress... */
	bool IsLassoInProgress() { return lassoPoints != NULL; }

		/**	Get the current lasso frame */
	BRect LassoFrame() { return lassoPoints->Frame(); }
	
		/**	Draw the Lasso poly. */
	void DrawLasso();

		/**	Test for a rect inside the polygon */
	bool IsRectInLasso( const BRect &inExtent, bool inInclusive )
	{
		if (lassoPoints == NULL) return false;
		else return lassoPoints->RectInPoly( inExtent, inInclusive );
	}
	
		/**	Done with lassoing... */
	void FinishLasso()
	{
		DrawLasso();
		delete lassoPoints;
		lassoPoints = NULL;
	}

		/** Select all events within the current lasso region */
	void DoLassoSelection();
};

// ---------------------------------------------------------------------------
// Label views are used by editor strips

class CLabelView : public CBorderView {
	BFont		labelFont;

public:
	CLabelView(	BRect			rect,
				const char		*name,
				const rgb_color	&inFill,
				const rgb_color	&inBorder,
				ulong			resizeFlags,
				ulong			flags )
		:	CBorderView( rect, name, resizeFlags, flags, &inFill),
			labelFont( be_bold_font )
	{
		labelFont.SetRotation( -90.0 );
		SetFont( &labelFont );
// 	SetFontName( "Arial MT Bold" );
	}

	CLabelView(	BRect			rect,
				const char		*name,
				ulong			resizeFlags,
				ulong			flags )
		:	CBorderView( rect, name, resizeFlags, flags ),
			labelFont( be_bold_font )
	{
		labelFont.SetRotation( -90.0 );
		SetFont( &labelFont );
// 	SetFontName( "Arial MT Bold" );
	}

	void Draw( BRect updateRect );

	const Event *PickEvent(
				EventMarker		&resultMarker,
				const BPoint	&pickPt,
				short			&resultPartCode );
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