/* ===================================================================== *
 * Spinner.h (MeV/User Interface)
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
 *  Up/down increment/decrement arrows
 * ---------------------------------------------------------------------
 * History:
 *	1997		Talin
 *		Original implementation
 *	04/08/2000	cell
 *		General cleanup in preparation for initial SourceForge checkin
 *	04/30/2000	cell
 *		Updated the drawing routines to make use of StdBevels & the
 *		LineArray API
 * ---------------------------------------------------------------------
 * To Do:
 *	- update mouse handling to use SetMouseEventMask() etc
 *	- Implementation clean-up
 * ===================================================================== */

#ifndef __C_Spinner_H__
#define __C_Spinner_H__

// Interface Kit
#include <Control.h>

class CSpinner :
	public BControl
{

public:							// Constants

	enum {
								Inc_Lit = 1,
								Dec_Lit = 2
	};

public:							// Constructor/Destructor

								CSpinner(
									BRect frame,
									const char *name,
									BMessage *message,
									uint32 resizingMode = B_FOLLOW_LEFT | B_FOLLOW_TOP,
									uint32 flags = B_WILL_DRAW);

public:							// Operations

	void						SetRange(
									int32 minValue,
									int32 maxValue);

public:							// BControl Implementation

	virtual void				AttachedToWindow();
	virtual void				Draw(
									BRect updateRect);
	virtual void				MouseDown(
									BPoint point);

	void						SetValue(
									int32 value);

private:						// Instance Data

	int32						minVal;

	int32						maxVal;

	uint8						lit;

	BPoint						mousePos;

protected:						// Internal Operations

	void						UpdateValue(
									int32 value);

protected:						// Thread Management

	static long					drag_entry(
									void *arg);

	long						Drag();
};

#endif /* __C_Spinner_H__ */
