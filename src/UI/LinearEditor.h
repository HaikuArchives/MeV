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
 *	History:
 *	1997		Talin
 *	Original implementation
 *	04/08/2000	cell
 *	General cleanup in preparation for initial SourceForge checkin
 *	07/02/2000	cell
 *	Major cleanup, simpler and more efficient drawing code
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __C_LinearEditor_H__
#define __C_LinearEditor_H__

#include "DocWindow.h"
#include "EventEditor.h"
#include "StripFrameView.h"

/**
 *		Classes relating to the linear editor.
 *		@author	Talin, Christoper Lenz.  
 */

class CLinearEditor
	:	public CEventEditor
{
	friend class CLinearNoteEventRenderer;
	friend class CPianoKeyboardView;
								
public:							// Constants

	static const rgb_color		NORMAL_GRID_LINE_COLOR;
	static const rgb_color		OCTAVE_GRID_LINE_COLOR;
	static const rgb_color		BACKGROUND_COLOR;
	

public:							// Constructor/Destructor

								/**	Constructor.	*/
								CLinearEditor(
									CStripFrameView &frame,
									BRect rect);

public:							// CEventEditor Implementation

	void						AttachedToWindow();

	virtual bool				ConstructEvent(
									BPoint point);

	/**	Do additional audio feedback or selection for this event.	*/
	virtual void				DoEventFeedback(
									const Event &event);

	virtual void				Draw(
									BRect updateRect);

	/**	Remove any feedback artifacts for this event.	*/
	virtual void				KillEventFeedback();

	virtual void				MouseMoved(
									BPoint point,
									ulong transit,
									const BMessage *message);

	/**	Called when the window activates to tell this view
			to make the selection visible.	*/
	
	virtual void				OnGainSelection()
								{ InvalidateSelection(); }
	
	/**	Called when some other window activates to tell this view
			 to hide the selection.	*/
			 
	virtual void				OnLoseSelection()
								{ InvalidateSelection(); }

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

	/**	Convert a pitch-value to a y-coordinate.	*/
	long						PitchToViewCoords(
									int pitch);

	/**	Convert a y-coordinate to a pitch value.	*/
	long						ViewCoordsToPitch(
									int yPos,
									bool limit = true);

private:						// Instance Data

	/**	Zoom amount in vertical direction.	*/
	short int					m_verticalZoom;

	/**	Step interval of white key.	*/
	short int					m_whiteKeyStep;

	/**	Step interval of octave.	*/
	short int					m_octaveStep;

	/**	Logical height of strip.	*/
	short int					m_stripLogicalHeight;
};

#endif /* __C_LinearEditor_H__ */
