/* ===================================================================== *
 * VelocityEditor.h (MeV/User Interface)
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
 *  
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

#ifndef __C_VelocityEditor_H__
#define __C_VelocityEditor_H__

#include "TrackEditFrame.h"
#include "EventEditor.h"
#include "DocWindow.h"

class EventListUndoAction;

// ---------------------------------------------------------------------------
// Linear editor strip view

class CVelocityEditor : public CEventEditor {
protected:

	friend class	CVelocityNoteEventHandler;
	int32			dragTime,
					smallestTime,
					largestTime;
	int32			dragVelocity;
	EventListUndoAction *dragAction;

	void MouseMoved(
		BPoint		point,
		ulong		transit,
		const BMessage	* );

	void AttachedToWindow() { SetViewColor( B_TRANSPARENT_32_BIT ); }

public:
		// ---------- Constructor

	CVelocityEditor(	BLooper			&inLooper,
					CTrackEditFrame &frame,
					BRect			rect );

		// ---------- Hooks

	void Draw( BRect updateRect );

	void StartDrag( BPoint point, ulong buttons );
	bool DoDrag( BPoint point, ulong buttons );
	void FinishDrag( BPoint point, ulong buttons, bool commit );

		// Update message from another observer
	void OnUpdate( BMessage *inMsg );
	
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
// Note handler class for linear editor

class CVelocityNoteEventHandler : public CAbstractEventHandler {
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

#endif /* __C_VelocityEditor_H__ */
