/* ===================================================================== *
 * TimeIntervalEditor.h (MeV/User Interface)
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
 *  A composite control for editing intervals of time.
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

#ifndef __C_TimeIntervalEditor_H__
#define __C_TimeIntervalEditor_H__

#include <interface/Control.h>

enum	 ENote_Note {
	DoubleNote_Index = 0,
	WholeNote_Index,
	HalfNote_Index,
	QuarterNote_Index,
	EighthNote_Index,
	SixteenthNote_Index,
	ThirtySecondNote_Index,
	
	Note_IndexCount
};

class CBitmapMenuButton;
class CToolBar;
class CTextSlider;
class CTextDisplay;
class CSpinner;
class CPopUpMatrixMenu;

class CTimeIntervalEditor : public BControl {
	int32				numerator,
					denominator;
	uint8				base;
	CBitmapMenuButton	*baseDuration;
	CToolBar			*modBar;
	CTextSlider			*durationSlider;
	CTextDisplay			*ratio;
	CSpinner			*nSpinner,
					*dSpinner;
	char				ratioText[ 32 ];
	CPopUpMatrixMenu		*menu;
	
	enum EToolIDs {
		BaseDuration_ID	= 1000,
		ModBar_ID,
		Tuplet3_ID,
		Tuplet5_ID,
		Tuplet7_ID,
		Dot_ID,
		DoubleDot_ID,
		DurationSlider_ID,
		Numerator_ID,
		Denominator_ID
	};
	
	void AttachedToWindow();
	void MessageReceived( BMessage *msg );
	void ShowRatio( bool inUpdateSlider = true );
	void CalcInterval();
	void Reduce( int32 &num, int32 &den );
	void NoteLengthFromInterval();
	void NoteLengthFromRatio();
	void Notify();

public:
	CTimeIntervalEditor( BRect rect, const char *name,
						BMessage	*msg,
						uint32 resizingMode = B_FOLLOW_LEFT | B_FOLLOW_TOP,
						uint32 flags = B_WILL_DRAW );

	void SetValue( int32 newValue );
};

void LoadNoteImages();

#endif /* __C_TimeIntervalEditor_H__ */
