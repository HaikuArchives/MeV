/* ===================================================================== *
 * TextSlider.h (MeV/User Interface)
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
 *  Slider which displays text string in thumb.
 * ---------------------------------------------------------------------
 * History:
 *	1997		Talin
 *		Original implementation
 *	04/08/2000	cell
 *		General cleanup in preparation for initial SourceForge checkin
 *	04/29/2000	vember
 *		Fixed drawing
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __C_TextSlider_H__
#define __C_TextSlider_H__

// Interface Kit
#include <Control.h>
#include <Region.h>

class CTextSlider : public BControl {

public:
	class CTextHook {
	public:

			/**	Return the width in pixels of the largest possible knob text. */
		virtual int32 Largest( BView *inView, int32 inMin, int32 inMax ) = 0;

			/**	Format the text for the text slider knob */
		virtual void FormatText( char *outText, int32 inValue, int32 inMaxLen ) = 0;
	};

protected:

	int32			minVal,
					maxVal,
					currentVal;
	int32			knobWidth;
	BPicture			*decArrow,
					*decArrowLit,
					*decArrowDim,
					*incArrow,
					*incArrowLit,
					*incArrowDim;
	rgb_color		backColor;
	BRegion			knobRegion;
	BPoint			mousePos;
	bool				decLit,
					incLit;
	CTextHook		*textHook;
	int32			bodyIncrement;
	
	enum {
		Arrow_Width = 10,
	};

	virtual int32 LargestText();
	virtual void DrawContents( const BRect &frame );
	virtual void InvalidateKnob();
	void Draw( BRect r );
	void AttachedToWindow();
	void CalcKnobRegion();
	int32 KnobSize()
	{
		if (knobWidth < 0)
		{
			knobWidth = LargestText() + 4;
			CalcKnobRegion();
		}
		return knobWidth;
	}
	void KnobPosition( int32 &outPos, int32 &outWidth );
	void MouseDown( BPoint point );
	static long drag_entry(void *arg);
	long Drag();
	void UpdateValue( int32 inValue );

public:
		/**	Constructor */
	CTextSlider(	BRect 		inFrame,
				BMessage		*inMessage,
				const char		*inName,
				uint32		inResizingMode = B_FOLLOW_LEFT | B_FOLLOW_RIGHT,
				uint32		inFlags = B_WILL_DRAW );
	~CTextSlider();

		/** Set the value of the slider. */
	void SetValue( int32 inValue );

		/** Set the range of the slider. */
	void SetRange( int32 inMIN, int32 inMAX );
	
		/** Return the value of the slider. */
	int32 Value() const { return currentVal; }
	
		/** Set the text hook for the slider. */
	void SetTextHook( CTextHook *inTH, bool inRefresh = true )
	{
		if (textHook != inTH)
		{
			textHook = inTH;
			knobWidth = -1;
			if (inRefresh) Invalidate();
		}
	}
	
		/** Set the amount of increment when the container (not arrows and not
			knob) is clicked on.
		*/
	void SetBodyIncrement( int32 inIncrement ) { bodyIncrement = inIncrement; }
};

#endif /* __C_TextSlider_H__ */
