/* ===================================================================== *
 * LinearEditor.h (MeV/UI)
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
 *	07/02/2000	cell
 *		Major cleanup, simpler and more efficient drawing code
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __C_LinearEditor_H__
#define __C_LinearEditor_H__

#include "DocWindow.h"
#include "EventEditor.h"
#include "StripFrameView.h"

// ---------------------------------------------------------------------------
// Linear editor strip view

class CLinearEditor
	:	public CEventEditor
{
	friend class CLinearNoteEventHandler;
	friend class CPianoKeyboardView;
								
public:							// Constants

	static const rgb_color		NORMAL_GRID_LINE_COLOR;
	static const rgb_color		OCTAVE_GRID_LINE_COLOR;
	static const rgb_color		BACKGROUND_COLOR;
	

public:							// Constructor/Destructor

								CLinearEditor(
									BLooper &looper,
									CStripFrameView &frame,
									BRect rect);

public:							// CEventEditor Implementation

	void						AttachedToWindow();

	virtual bool				ConstructEvent(
									BPoint point);

	// Do additional audio feedback or selection for this event
	virtual void				DoEventFeedback(
									const Event &event);

	virtual void				Draw(
									BRect updateRect);

	// Remove any feedback artifacts for this event
	virtual void				KillEventFeedback();

	virtual void				MessageReceived(
									BMessage *message);

	virtual void				MouseMoved(
									BPoint point,
									ulong transit,
									const BMessage *message);

	// Called when the window activates to tell this view
	// to make the selection visible.
	virtual void				OnGainSelection()
								{ InvalidateSelection(); }
	
	// Called when some other window activates to tell this view
	// to hide the selection.
	virtual void				OnLoseSelection()
								{ InvalidateSelection(); }

	// Update message from another observer
	virtual void				OnUpdate(
									BMessage *message);

	virtual void				Pulse();

	virtual void				SetScrollValue(
									float inScrollValue,
									orientation inOrient);

	virtual bool				SupportsShadowing()
								{ return true; }

	virtual void				ZoomChanged(
									int32 diff);

protected:						// Internal Operations

	void						CalcZoom();

	void						DeselectAll();

	void						DisplayPitchInfo(
									BPoint point);

	// convert a pitch-value to a y-coordinate
	long						PitchToViewCoords(
									int pitch);

	// Convert a y-coordinate to a pitch value
	long						ViewCoordsToPitch(
									int yPos,
									bool limit = true);

private:						// Instance Data

	// Zoom amount in vertical direction
	short int					m_verticalZoom;

	// step interval of white key
	short int					m_whiteKeyStep;

	// step interval of octave
	short int					m_octaveStep;

	// logical height of strip.
	short int					m_stripLogicalHeight;
};

// ---------------------------------------------------------------------------
// Piano keyboard view

class CPianoKeyboardView
	:	public CStripLabelView
{

public:							// Constructor/Destructor

								CPianoKeyboardView(
									BRect frame,
									CLinearEditor *editor,
									uint32 resizeFlags,
									uint32 flags);

public:							// Operations
	
	void						SetSelectedKey(
									int32 key);

public:							// CStripLabelView Implementation

	virtual void				DrawInto(
									BView *view,
									BRect updateRect);

private:						// Instance Data

	CLinearEditor *				m_editor;
	int32						m_selectedKey;

};

#endif /* __C_LinearEditor_H__ */
