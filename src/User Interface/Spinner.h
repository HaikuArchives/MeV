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
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __C_Spinner_H__
#define __C_Spinner_H__

// Interface Kit
#include <Control.h>

class CSpinner : public BControl {
	int32			minVal,
					maxVal;
	uint8			lit;
	BPoint			mousePos;

protected:

	enum {
		Inc_Lit = 1,
		Dec_Lit = 2
	};

	void MouseDown( BPoint point );
	static long drag_entry( void *arg );
	long Drag();
	void UpdateValue( int32 inValue );
	void Draw( BRect inInvalRect );
	void AttachedToWindow();

public:
	CSpinner( BRect frame, const char *name, BMessage *msg,
		uint32 resizingMode = B_FOLLOW_LEFT | B_FOLLOW_TOP,
		uint32 flags = B_WILL_DRAW );
	
	void SetRange( int32 inMIN, int32 inMAX );
	void SetValue( int32 inValue );
};

#endif /* __C_Spinner_H__ */
