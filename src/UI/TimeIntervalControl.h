/* ===================================================================== *
 * TimeIntervalControl.h (MeV/User Interface)
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
 *	04/29/2000	cell
 *	Renamed from the less approproate CTimeIntervalEditor and fixed
 *	usage of CToolBar; complete clean-up
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __C_TimeIntervalControl_H__
#define __C_TimeIntervalControl_H__

// Interface Kit
#include <Control.h>
// Support Kit
#include <String.h>

class CToolBar;
class CTextSlider;
class CTextDisplay;
class CSpinner;

 /**
 *		A composite control for editing intervals of time.
 *		@author	Talin, Christoper Lenz.  
 */

class CTimeIntervalControl :
	public BControl
{

public:							// Constants

	enum message
	{
								BASE_DURATION_CHANGED = 'mTIb',
								MODIFIERS_CHANGED = 'mTIm',
								DURATION_SLIDER_CHANGED = 'mTIs',
								NUMERATOR_CHANGED = 'mTIn',
								DENOMINATOR_CHANGED = 'mTId'
	};

	enum base_duration
	{
								DOUBLE_NOTE = 0,
								WHOLE_NOTE,
								HALF_NOTE,
								QUARTER_NOTE,
								EIGHTH_NOTE,
								SIXTEENTH_NOTE,
								THIRTY_SECOND_NOTE,

								BASE_DURATION_COUNT
	};

	enum tuplet_modifiers
	{
								TUPLET_3 = 1,
								TUPLET_5,
								TUPLET_7
	};

	enum dot_modifiers
	{
								SINGLE_DOT = 1,
								DOUBLE_DOT
	};

	static const float			MENU_BORDER;
	static const float			IMAGE_BORDER;

public:							// Constructor/Destructor

								CTimeIntervalControl(
									BRect frame,
									const char *name,
									BMessage *message,
									uint32 resizingMode = B_FOLLOW_LEFT | B_FOLLOW_TOP,
									uint32 flags = B_WILL_DRAW );

public:							// BControl Implementation

	virtual void				AttachedToWindow();

	virtual void				GetPreferredSize(
									float *width,
									float *height);

	virtual void				MessageReceived(
									BMessage *message);

	virtual void				SetValue(
									int32 value);

protected:						// Internal Operations

	void						CalcInterval();

	void						NoteLengthFromInterval();

	void						NoteLengthFromRatio();

	void						Notify();

	/**	Reduce fraction to lowest terms.	*/
	void						Reduce(
									int32 *numerator,
									int32 *denominator);

	void						ShowRatio(
									bool updateSlider = true);

	/**	Update the toolbar to match the controls value.	*/
	void						UpdateToolBar();

private:						// Instance Data

	int32						m_numerator;
	int32						m_denominator;
	uint8						m_base;
	int32						m_tupletModifier;
	int32						m_dotModifier;

	CToolBar *					m_toolBar;
	CTextSlider *				m_durationSlider;
	CSpinner *					m_numSpinner;
	CSpinner *					m_denSpinner;
	CTextDisplay *				m_ratio;

	BString						m_ratioText;
};

#endif /* __C_TimeIntervalControl_H__ */
