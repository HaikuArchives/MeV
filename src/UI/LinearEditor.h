/* ===================================================================== *
 * LinearEditor.h (MeV/User Interface)
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
 *  classes relating to the linear editor
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

#ifndef __C_LinearEditor_H__
#define __C_LinearEditor_H__

#include "TrackEditFrame.h"
#include "EventEditor.h"
#include "DocWindow.h"

// ---------------------------------------------------------------------------
// Linear editor strip view

class CLinearEditor : public CEventEditor {
protected:

	friend class	CLinearNoteEventHandler;
	friend class CPianoKeyboardView;

	short			verticalZoom,			// Zoom amount in vertical direction
					whiteKeyStep,			// step interval of white key
					octaveStep,				// step interval of octave
					stripLogicalHeight;		// logical height of strip.

//	static CAbstractEventHandler *handlers[ EvtType_Count ];

//	const CAbstractEventHandler &Handler( const Event &ev ) const
//	{
//		return (ev.Command() < EvtType_Count)
//			? *handlers[ ev.Command() ]
//			: (CAbstractEventHandler &)gNullEventHandler;
//	}

	void SetScrollValue( float inScrollValue, orientation inOrient )
	{
		CStripView::SetScrollValue( inScrollValue, inOrient );
		labelView->ScrollTo( 0.0, scrollValue.y );
	}

	void MouseMoved(
		BPoint		point,
		ulong		transit,
		const BMessage	* );

	void AttachedToWindow();
	void MessageReceived( BMessage *msg );
	void Pulse();

	void CalcZoom();
	
public:
		// ---------- Constructor

	CLinearEditor(	BLooper			&inLooper,
					CTrackEditFrame &frame,
					BRect			rect );
					
		// ---------- Hooks

	void Draw( BRect updateRect );

/*	void StartDrag( BPoint point, ulong buttons );
	bool DoDrag( BPoint point, ulong buttons );
	void FinishDrag( BPoint point, ulong buttons, bool commit ); */
	bool ConstructEvent( BPoint point );
	void DoEventFeedback( const Event &ev );
	void KillEventFeedback();

		// Update message from another observer
	void OnUpdate( BMessage *inMsg );

		// ---------- Conversion funcs

		// returns y-pos for pitch
	long PitchToViewCoords( int pitch );

		// returns pitch for y-pos and optionally clamp to legal values
	long ViewCoordsToPitch( int yPos, bool limit = true );

		// ---------- Getters

	bool SupportsShadowing() { return true; }

		// ---------- General operations

	void DeselectAll();
	
		/**	Called when the window activates to tell this view
			to make the selection visible.
		*/
	virtual void OnGainSelection() { InvalidateSelection(); }
	
		/**	Called when some other window activates to tell this view
			to hide the selection.
		*/
	virtual void OnLoseSelection() { InvalidateSelection(); }

};

// ---------------------------------------------------------------------------
// Piano keyboard view

class CPianoKeyboardView : public BView {
	CLinearEditor		*editor;
	int32				selKey;

public:
	CPianoKeyboardView(	BRect		rect,
						CLinearEditor *lEditor,
						ulong		resizeFlags,
						ulong		flags )
		:	BView( rect, NULL, resizeFlags, flags )
	{
		editor = lEditor;
		SetViewColor( B_TRANSPARENT_32_BIT );
		selKey = -1;
	}

	void Draw( BRect updateRect );
	
	void SetSelKey( int32 newKey );
};

// ---------------------------------------------------------------------------
// Note handler class for linear editor

class CLinearNoteEventHandler : public CAbstractEventHandler {
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

	EventOp *CreateTimeOp(
		CEventEditor	&editor,			// The editor
		const Event		&ev,				// The clicked event
		short			partCode,			// Part of event clicked
		long			timeDelta,			// The horizontal drag delta
		long			valueDelta ) const;
};

#endif /* __C_LinearEditor_H__ */
