/* ===================================================================== *
 * PitchBendEditor.h (MeV/UI)
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
 *	Original implementation
 *	04/08/2000	cell
 *	General cleanup in preparation for initial SourceForge checkin
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __C_PitchBendEditor_H__
#define __C_PitchBendEditor_H__

#include "ContinuousValueEditor.h"

 /**
 *		Classes relating to the Pich Bend editor strip.
 *		@author	Talin, Christoper Lenz.  
 */
 
class CPitchBendEditor
	:	public CContinuousValueEditor
{
	friend class CPitchBendEventRenderer;

protected:


public:							// Constructor/Destructor

								CPitchBendEditor(
									CStripFrameView &frame,
									BRect rect);

public:							// Operations

	/** returns y-pos for pitch */
	float						ValueToViewCoords(
									int value);

	/** returns pitch for y-pos and optionally clamp to legal values */
	long						ViewCoordsToValue(
									float y,
									bool limit = true);

public:							// CContinuousValueEditor Implementation

	virtual bool				ConstructEvent(
									BPoint point);

	const BCursor *				CursorFor(
									int32 editMode) const;

	virtual void				DrawHorizontalGrid(
									BRect updateRect);

	virtual void				MouseMoved(
									BPoint point,
									uint32 transit,
									const BMessage *message);

	virtual void				ZoomChanged(
									int32 diff);
};

#endif /* __C_PitchBendEditor_H__ */
