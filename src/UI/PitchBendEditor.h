/* ===================================================================== *
 * PitchBendEditor.h (MeV/User Interface)
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
 *  classes relating to the pich bend editor strip
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

#ifndef __C_PitchBendEditor_H__
#define __C_PitchBendEditor_H__

#include "ContinuousValueEditor.h"

// ---------------------------------------------------------------------------
// Pitch bend editor strip view

class CPitchBendEditor : public CContinuousValueEditor {
protected:

	friend class	CPitchBendEventHandler;

	void MessageReceived( BMessage *msg );
	void OnUpdate( BMessage *inMsg );
	
public:
		// ---------- Constructor

	CPitchBendEditor(	BLooper			&inLooper,
					CStripFrameView &frame,
					BRect			rect );
					
		// ---------- Hooks

	bool ConstructEvent( BPoint point );

		// ---------- Conversion funcs

		// returns y-pos for pitch
	long ValueToViewCoords( int value );

		// returns pitch for y-pos and optionally clamp to legal values
	long ViewCoordsToValue( int yPos, bool limit = true );

	virtual void ZoomChanged(int32 diff);
};

#endif /* __C_PitchBendEditor_H__ */
