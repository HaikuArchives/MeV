/* ===================================================================== *
 * EventRenderer.h (MeV/UI)
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
 *		Separated from EventEditor.h/.cpp
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __C_EventRenderer_H__
#define __C_EventRenderer_H__

// Interface Kit
#include <Rect.h>

class CEvent;
class CEventEditor;
class EventOp;
class CMeVDoc;

class BCursor;

/**
 *	This class represents all of the functions that
 *	normally need to operate on events which depend both
 *	on the event type and on the view type (in other words,
 *	it's a lame attempt to make up for the lack of multi-methods
 *	in C++)
 *
 *	ONLY functions which need to differ based on the type
 *	of the event AND the type of the view should go in here.
 *
 *	@author	Talin, Christopher Lenz
 */
class CEventRenderer
{

public:							// Constructor/Destructor

								CEventRenderer(
									CEventEditor * const editor);

public:							// Hook Functions

	/**	Drag operation. What this function does is to create
	 *	a function object to implement the drag operation.
	 *	This function object can modify the events in question
	 *	in a manner consistent with the part of the event
	 *	clicked and the current mouse position.
	 *
	 *	This is used in two ways. First, it is used to create
	 *	temporary proxy events which are rendered as a drag
	 *	echo. Secondly, it is used to permanently modify the
	 *	events when the drag operation is completed.
	 */
	virtual EventOp *			CreateDragOp(
									const CEvent &ev,
									short partCode,
									long timeDelta,
									long valueDelta) const;

	virtual EventOp *			CreateTimeOp(
									const CEvent &ev,
									short partCode,
									long timeDelta,
									long valueDelta) const;

	/**	For a part code returned earlier, return a cursor
	 *	image...
	 */
	virtual const BCursor *		Cursor(
									short partCode,
									int32 editMode,
									bool dragging = false) const;
	
	/** Draw the event (or an echo). */
	virtual	void				Draw(
									const CEvent &ev,
									bool shadowed) const = 0;

	/** Invalidate the event. */
	virtual	BRect				Extent(
									const CEvent &ev) const;

	/** Invalidate the event. */
	virtual	void				Invalidate(
									const CEvent &ev) const = 0;

	/** Pick a single event and return the part code,
	 *	or -1 if event not picked.
	 */
	virtual long				Pick(
									const CEvent &ev,
									BPoint point,
									short &partCode) const;

	/**	Quantize the time of the dragging operation and
	 *	return a time delta.
	 */
	virtual long				QuantizeDragTime(
									const CEvent &clickedEvent,
									short partCode,
									BPoint clickPos,
									BPoint dragPos,
									bool initial = false) const;

	/**	Quantize the vertical position of the mouse based
	 *	on the event type and return a value delta.
	 */
	virtual long				QuantizeDragValue(
									const CEvent &clickedEvent,
									short partCode,
									BPoint clickPos,
									BPoint dragPos) const;

protected:						// Accessors

	CMeVDoc * const				Document() const;

	CEventEditor * const		Editor() const;

private:						// Instance Data

	CEventEditor * const		m_editor;
};

// ---------------------------------------------------------------------------
// The null renderer does nothing to the event.

class CNullEventRenderer
	:	public CEventRenderer
{

public:							// Constructor/Destructor

								CNullEventRenderer(
									CEventEditor *editor)
									:	CEventRenderer(editor)
								{ }

public:							// CAbstractEventRenderer Implementation

	// Invalidate the event
	void						Invalidate(
									const CEvent &ev) const
								{ }

	// Draw the event (or an echo)
	void						Draw(
									const CEvent &ev,
									bool shadowed) const
								{ }
};

// ---------------------------------------------------------------------------
// Event renderer class for "end" events

class CEndEventRenderer
	:	public CEventRenderer
{

public:							// Constructor/Destructor

								CEndEventRenderer(
									CEventEditor *editor)
									:	CEventRenderer(editor)
								{ }

public:							// CAbstractEventRenderer Implementation

	// Invalidate the event
	void						Invalidate(
									const CEvent &ev) const;

	// Draw the event (or an echo)
	void						Draw(
									const CEvent &ev,
									bool shadowed) const;
	// Invalidate the event
	BRect						Extent(
									const CEvent &ev) const;

	// Pick a single event and returns the distance.
	long						Pick(
									const CEvent &ev,
									BPoint pickPt,
									short &partCode) const;

	// For a part code returned earlier, return a cursor
	// image...
	const BCursor *				Cursor(
									short partCode,
									int32 mode = 0,
									bool dragging = false) const;

	// Quantize the vertical position of the mouse based
	// on the event type and return a value delta.
	long						QuantizeDragValue(
									const CEvent &clickedEvent,
									short partCode,
									BPoint clickPos,
									BPoint dragPos) const
								{ return 0; }

	// Make a drag op for dragging notes...
	EventOp *					CreateDragOp(
									const CEvent &ev,
									short partCode,
									long timeDelta,
									long valueDelta) const
								{ return NULL; }
};

#endif /* __C_EventEditor_H__ */
