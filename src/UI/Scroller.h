/* ===================================================================== *
 * Scroller.h (MeV/User Interface)
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
 *  Subclass of BScrollBar which is a bit more flexible
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

#ifndef __C_Scroller_H__
#define __C_Scroller_H__

#include <ScrollBar.h>

class CScroller;

class CScrollerTarget : public BView {
protected:

		// Total range of scrollbar
	BPoint			scrollRange;
	BPoint			scrollValue;		// Current scroll value
	CScrollerTarget	*redirect;			// Redirect to 2nd target

	void AdjustScrollers();

	void AdjustScroller(
		BScrollBar		*sBar,
		float			range,
		float			*value,
		float			smallStep,
		float			frameSize );

public:
	CScrollerTarget(	BRect frame,
						const char *name,
						ulong resizeMask,
						ulong flags)
		:	BView( frame, name, resizeMask, flags ),
			scrollRange( 0.0, 0.0 ),
			scrollValue( 0.0, 0.0 )
	{
		redirect = NULL;
	}

	float ScrollValue( orientation inOrient )
		{ return inOrient == B_HORIZONTAL ? scrollValue.x : scrollValue.y; }

	virtual void SetScrollValue( float inScrollValue, orientation inOrient );

	float ScrollRange( orientation inOrient )
		{ return inOrient == B_HORIZONTAL ? scrollRange.x : scrollRange.y; }

	virtual void SetScrollRange(	float	inHorizontalRange,
								float	inHotizontalValue,
								float	inVerticalRange,
								float	inVerticalValue );

	void SetTarget( CScrollerTarget *target )
	{
		redirect = target;
	}
	
		/**	Adjust the current scroll value by a delta amount. */
	void ScrollBy( float inAmount, orientation inOrient );
	
		/**	Adjust the current scroll value to an absolute amount. */
// void SetScrollValueScrollTo( float inAmount, orientation inOrient, bool inAdjustScrollers = true );
	
		/**	Return the actual size of the frame being scrolled,
			which might not be this frame.
		*/
	virtual BPoint FrameSize()
	{
		return BPoint( Frame().Width(), Frame().Height() );
	}

		/**	Return the amount of scrolling overlap
		*/
	virtual BPoint StepSize()
	{
		return BPoint( 10.0, 10.0 );
	}
};

class CScroller : public BScrollBar {
public:
	CScroller(	BRect			frame,
				const char		*name,
				CScrollerTarget	*target,
				long min,
				long max,
				orientation direction )
	: BScrollBar(	frame, name, target, min, max, direction ) {}

	void ValueChanged( float newValue )
	{
		CScrollerTarget		*st;
		
		if ((st = dynamic_cast<CScrollerTarget *>(Target())))
		{
			st->SetScrollValue( newValue, Orientation() );
		}
	}
};

#endif /* __C_Scroller_H__ */
