/* ===================================================================== *
 * Junk.h (MeV/UI)
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
 *  where we put classes that are still in flux
 * ---------------------------------------------------------------------
 * History:
 *	1997		Talin
 *		Original implementation
 *	04/08/2000	cell
 *		General cleanup in preparation for initial SourceForge checkin
 *	08/06/2000	cell
 *		Moved prefs window into PreferencesWindow.h
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __Junk_H__
#define __Junk_H__

#include "TempoMap.h"

// Interface Kit
#include <Control.h>

class CTimeEditControl : public BControl {
	int16				numDigits;
	TClockType			clockType;
	BPoint				mousePos;
	char					text[ 16 ];
	int32				measureBase,
						measureSize,
						beatSize;
	BFont				digitFont;

	void MouseDown( BPoint point );
	static long drag_entry( void *arg );
	long Drag();
	void Draw( BRect r );

	void AttachedToWindow()
	{
		BControl::AttachedToWindow();
		SetViewColor( B_TRANSPARENT_32_BIT );
	}

public:
	CTimeEditControl(	BRect		inFrame,
						BMessage		*inMessage,
						uint32		inResizingMode = B_FOLLOW_LEFT | B_FOLLOW_RIGHT,
						uint32		inFlags = B_WILL_DRAW );
						
	void SetClockType( TClockType inClockType )
	{
		clockType = inClockType;
	}

	void SetValue( int32 value );
};

#endif /* __Junk_H__ */
