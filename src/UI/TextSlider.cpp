/* ===================================================================== *
 * TextSlider.cpp (MeV/User Interface)
 * ===================================================================== */

#include "TextSlider.h"

// Gnu C Library
#include <stdio.h>
// Standard Template Library
#include <algorithm>
// Interface Kit
#include <Picture.h>
#include <Window.h>

inline const int32 CLAMP( const int32 a, const int32 b, const int32 c )
{
	if (b < a) return a;
	if (b > c) return c;
	return b;
}

#pragma export on

CTextSlider::CTextSlider(
	BRect 		inFrame,
	BMessage		*inMessage,
	char			*inName,
	uint32		inResizingMode,
	uint32		inFlags )
	: BControl( inFrame, inName, NULL, inMessage, inResizingMode, inFlags )
{
	bodyIncrement = 10;
	minVal = maxVal = currentVal = knobWidth = -1;
	decArrow = decArrowLit = decArrowDim = NULL;
	incArrow = incArrowLit = incArrowDim = NULL;
	decLit = incLit = false;
	textHook = NULL;
}

CTextSlider::~CTextSlider()
{
	delete decArrow;
	delete decArrowLit;
	delete decArrowDim;
	delete incArrow;
	delete incArrowLit;
	delete incArrowDim;
}

int32 CTextSlider::LargestText()
{
	if (textHook) return textHook->Largest( this, minVal, maxVal );

	int32		l = max( abs( minVal ), abs( maxVal ) );
	int32		count = 1,
				w;
	while (l > 9) { count++; l /= 10; }
	
	w = count * StringWidth( "0" );
	if (minVal < 0) w += StringWidth( "-" );
	
	return w;
}

void CTextSlider::CalcKnobRegion()
{
	BRect		r( Bounds() );

	knobRegion.MakeEmpty();
	knobRegion.Include( BRect( 0.0, 0.0, knobWidth + 1.0, r.Height() ) );

	knobRegion.Exclude( BRect( 0.0, 0.0, 0.0, 0.0 ) );

	knobRegion.Exclude( BRect( knobWidth, 0.0, knobWidth + 1.0, 0.0 ) );
	knobRegion.Exclude( BRect( knobWidth + 1.0, 1.0, knobWidth + 1.0, 1.0 ) );

	knobRegion.Exclude( BRect( knobWidth + 1.0, r.bottom, knobWidth + 1.0, r.bottom ) );

	knobRegion.Exclude( BRect( 0.0, r.bottom - 1, 0.0, r.bottom ) );
	knobRegion.Exclude( BRect( 1.0, r.bottom, 1.0, r.bottom ) );
}

void CTextSlider::KnobPosition( int32 &outPos, int32 &outWidth )
{
	BRect		r( Bounds() );
	int32		kw = KnobSize();
	int32		rg = maxVal - minVal;
	
	r.InsetBy( Arrow_Width, 0.0 );
	r.right -= kw;
	
	if (rg > 0)
	{
		outPos = r.left + (r.Width() * (currentVal - minVal) + rg / 2) / rg;
	}
	else
	{
		outPos = r.left;
	}
	
	outWidth = kw;
}

void CTextSlider::DrawContents( const BRect &inContentRect )
{
	font_height	fh;
		
	char		text[ 32 ];
	int		x;
	
	GetFontHeight( &fh );

	if (textHook) textHook->FormatText( text, currentVal, sizeof text );
	else sprintf( text, "%d", currentVal );
	
	x = inContentRect.left + (inContentRect.Width() - StringWidth( text )) / 2 + 1;

	MovePenTo( x, (inContentRect.top + inContentRect.bottom - fh.descent + fh.ascent) / 2 );
	DrawString( text );
}

void CTextSlider::InvalidateKnob()
{
	BRect		r( Bounds() ),
				knob;
	int32		kPos,
				kWidth;
				
	KnobPosition( kPos, kWidth );
	knob.left	= kPos;
	knob.right	= kPos + kWidth + 1;		// Include shadow
	knob.top		= r.top;
	knob.bottom	= r.bottom;
	
	Invalidate( knob );
}

void CTextSlider::Draw( BRect inInvalRect )
{
	BRect		r( Bounds() ),
				knob;
	int32		kPos,
				kWidth;
	int32		vCenter = (int32)(r.top + r.bottom) / 2;
	BRegion		clip;
	bool			dim = false;

	KnobPosition( kPos, kWidth );
	knob.left	= kPos;
	knob.right	= kPos + kWidth;
	knob.top		= r.top;
	knob.bottom	= r.bottom;

	BRegion		knobTemp( knobRegion );

	knobTemp.OffsetBy( kPos, 0 );	
	clip.Include( r );
//	clip.Exclude( &knobTemp );
	ConstrainClippingRegion( &clip );

	SetHighColor( backColor );
	FillRect( r );
	
	if (IsEnabled())
	{
		DrawPicture( decLit ? decArrowLit : decArrow, BPoint( 0.0, vCenter ) );
		DrawPicture( incLit ? incArrowLit : incArrow, BPoint( r.right, vCenter ) );
	}
	else
	{
		DrawPicture( decArrowDim, BPoint( 0.0, vCenter ) );
		DrawPicture( incArrowDim, BPoint( r.right, vCenter ) );
		dim = true;
	}
	
	r.InsetBy( Arrow_Width + 2, 0.0 );
	
		// Do the shadows first
	if (dim) SetHighColor( 190, 190, 190 );
	else SetHighColor( 158, 158, 158 );
	FillRect( BRect( r.left + 1.0, vCenter - 3.0, r.right - 1.0, vCenter - 3.0 ) );
	FillRect( BRect( r.left + 2.0, vCenter - 1.0, r.right - 1.0, vCenter - 1.0 ) );
	FillRect( BRect( r.left, vCenter - 2.0, r.left, vCenter + 1.0 ) );
	FillRect( BRect( r.left + 1.0, vCenter - 2.0, r.left + 1.0, vCenter - 2.0 ) );

		// Now, do all of the black lines
	if (dim) SetHighColor( 137, 137, 137 );
	else SetHighColor( 0, 0, 0 );
	FillRect( BRect( r.left + 2.0, vCenter - 2.0, r.right - 1.0, vCenter - 2.0 ) );
	FillRect( BRect( r.left + 1.0, vCenter - 1.0, r.left + 1.0, vCenter + 1.0 ) );
	
		// Now, the white parts
	if (dim) SetHighColor( 235, 235, 235 );
	else SetHighColor( 255, 255, 255 );
	FillRect( BRect( r.left + 2.0, vCenter + 2.0, r.right - 1.0, vCenter + 2.0 ) );
	FillRect( BRect( r.right, vCenter - 2.0, r.right, vCenter + 1.0 ) );

//	ConstrainClippingRegion( &knobTemp );
/* 
	I simply removed the clipping between the knob and the backround.
	Maybe an more elegant solution should be added later but it works now at least

*/
		// And the knob shadows	
	if (dim) SetHighColor( 190, 190, 190 );
	else SetHighColor( 148, 148, 148 );
	FillRect( BRect( knob.right + 1.0, knob.top + 2.0, knob.right + 1, knob.bottom - 1.0 ) );
	FillRect( BRect( knob.left + 2.0, knob.bottom, knob.right, knob.bottom ) );
	FillRect( BRect( knob.right, knob.bottom - 1.0, knob.right, knob.bottom - 1.0 ) );
	
		// Black lines on knob
	if (dim) SetHighColor( 127, 127, 127 );
	else SetHighColor( 0, 0, 0 );
	FillRect( BRect( knob.left + 1.0, knob.bottom - 1.0, knob.right - 1.0, knob.bottom - 1.0 ) );
	FillRect( BRect( knob.right, knob.top + 1.0, knob.right, knob.bottom - 2.0 ) );
	FillRect( BRect( knob.right + 1.0, vCenter - 2.0, knob.right + 1.0, vCenter - 2.0 ) );
	
		// White parts of the knob
	if (dim) SetHighColor( 235, 235, 235 );
	else SetHighColor( 255, 255, 255 );
	FillRect( BRect( knob.left + 1.0, knob.top, knob.right - 1.0, knob.top ) );
	FillRect( BRect( knob.left, knob.top + 1.0, knob.left, knob.bottom - 2.0 ) );

	SetHighColor( 235, 235, 235 );
	FillRect( BRect( knob.left + 1.0, knob.top + 1.0, knob.right - 2.0, knob.top + 1.0 ) );
	FillRect( BRect( knob.left + 1.0, knob.top + 1.0, knob.left + 1.0, knob.bottom - 3.0 ) );

	if (dim) SetHighColor( 205, 205, 205 );
	else SetHighColor( 178, 178, 178 );
	FillRect( BRect( knob.left + 1.0, knob.bottom - 2.0, knob.right - 2.0, knob.bottom - 2.0 ) );
	FillRect( BRect( knob.right- 1.0, knob.top + 1.0, knob.right - 1.0, knob.bottom - 2.0 ) );

		// Overdraw the knob
	knob.InsetBy( 2.0, 2.0 );
	knob.bottom -= 1.0;

	SetHighColor( backColor );
	FillRect( knob );

	if (dim) SetHighColor( 137, 137, 137 );
	else SetHighColor( 0, 0, 0 );
	SetLowColor( backColor );
	DrawContents( knob );
}

void CTextSlider::AttachedToWindow()
{
	BControl::AttachedToWindow();

//	if (Parent()) backColor = Parent()->ViewColor();
	backColor = ViewColor();
	SetViewColor( B_TRANSPARENT_32_BIT );

	if (decArrow == NULL)
	{
			// Construct the unselected picture
		for (int i = 0; i < 3; i++)
		{
			bool		dim = (i == 2);
		
			BeginPicture( new BPicture );

			if (dim) SetHighColor( 180, 180, 180 );
			else SetHighColor( 148, 148, 148 );
			StrokeLine( BPoint( 2.0, 2.0 ), BPoint( 7.0, 4.0 ) );
			StrokeLine( BPoint(10.0,-3.0 ), BPoint(10.0, 4.0 ) );
			StrokeLine( BPoint( 8.0, 5.0 ), BPoint(10.0, 5.0 ) );

			if (dim) SetHighColor( 160, 160, 160 );
			else SetHighColor( 102, 106, 111 );
			StrokeLine( BPoint( 8.0,-4.0 ), BPoint( 8.0,-4.0 ) );
			StrokeLine( BPoint( 0.0, 0.0 ), BPoint( 7.0,-4.0 ) );
			StrokeLine( BPoint( 0.0, 0.0 ), BPoint( 7.0, 4.0 ) );

			SetHighColor( 255, 255, 255 );
			if (i == 1)
			{
				static BPoint	p[ 3 ];
				
				p[ 0 ].x = 1.0; p[ 0 ].y = 0.0;
				p[ 1 ].x = 8.0; p[ 1 ].y =-3.0;
				p[ 2 ].x = 8.0; p[ 2 ].y = 3.0;
				
				FillPolygon( p, 3 );
			}
			else
			{
				StrokeLine( BPoint( 1.0, 0.0 ), BPoint( 8.0, -3.0 ) );
			}

			if (dim) SetHighColor( 127, 127, 127 );
			else SetHighColor( 0, 0, 0 );
			StrokeLine( BPoint( 9.0,-3.0 ), BPoint( 9.0, 3.0 ) );
			StrokeLine( BPoint( 8.0, 4.0 ), BPoint( 9.0, 4.0 ) );

			switch (i) {
			case 0: decArrow    = EndPicture(); break;
			case 1: decArrowLit = EndPicture(); break;
			case 2: decArrowDim = EndPicture(); break;
			}

			BeginPicture( new BPicture );

			if (dim) SetHighColor( 180, 180, 180 );
			else SetHighColor( 148, 148, 148 );
			StrokeLine( BPoint(-9.0, 5.0 ), BPoint( 0.0, 1.0 ) );
			StrokeLine( BPoint(-8.0, 5.0 ), BPoint(-1.0, 2.0 ) );

			if (dim) SetHighColor( 160, 160, 160 );
			else SetHighColor( 102, 106, 111 );
			StrokeLine( BPoint(-10.0,-4.0 ), BPoint(-10.0,4.0 ) );
			StrokeLine( BPoint(-9.0, -4.0 ), BPoint(-2.0,-1.0 ) );

			SetHighColor( 255, 255, 255 );
			if (i == 1)
			{
				static BPoint	p[ 3 ];
				
				p[ 0 ].x =-9.0; p[ 0 ].y =-3.0;
				p[ 1 ].x =-2.0; p[ 1 ].y = 0.0;
				p[ 2 ].x =-9.0; p[ 2 ].y = 3.0;
				
				FillPolygon( p, 3 );
			}
			else
			{
				StrokeLine( BPoint(-9.0,-3.0 ), BPoint(-2.0,  0.0 ) );
				StrokeLine( BPoint(-9.0,-2.0 ), BPoint(-9.0,  3.0 ) );
			}
			
			if (dim) SetHighColor( 127, 127, 127 );
			else SetHighColor( 0, 0, 0 );
			StrokeLine( BPoint(-9.0, 4.0 ), BPoint(-2.0, 1.0 ) );
			StrokeLine( BPoint(-1.0, 0.0 ), BPoint(-1.0, 0.0 ) );

			switch (i) {
			case 0: incArrow    = EndPicture(); break;
			case 1: incArrowLit = EndPicture(); break;
			case 2: incArrowDim = EndPicture(); break;
			}

		}
	}
}

void CTextSlider::SetValue( int32 inValue )
{
	inValue = CLAMP( minVal, inValue, maxVal );
	if (currentVal != inValue)
	{
		InvalidateKnob();
		currentVal = inValue;
		InvalidateKnob();
	}
}

void CTextSlider::UpdateValue( int32 inValue )
{
	inValue = CLAMP( minVal, inValue, maxVal );
	if (currentVal != inValue)
	{
		BMessage		msg(*Message());

		InvalidateKnob();
		currentVal = inValue;
		InvalidateKnob();

		msg.AddInt32( "value", currentVal );
		Invoke( &msg );
	}
}

void CTextSlider::SetRange( int32 inMIN, int32 inMAX )
{
	inMAX = max( inMAX, inMIN );
	if (minVal != inMIN || maxVal != inMAX)
	{
		knobWidth = -1;			// Invalidate knob size

		minVal = inMIN;
		maxVal = inMAX;
		currentVal = CLAMP( minVal, currentVal, maxVal );
		Invalidate();
	}
}

long CTextSlider::drag_entry(void *arg)
{
	return ((CTextSlider *) arg)->Drag();
}

long CTextSlider::Drag()
{
	int32		prevValue = currentVal;
	uint32		buttons;
	int32		kPos,
				kWidth,
				newValue;
	float		delay = 200 * 1000.0;

	LockLooper();
	BRect		r( Bounds() );
	KnobPosition( kPos, kWidth );
	UnlockLooper();
				
	if (mousePos.x < Arrow_Width)
	{
		for (;;)
		{
			bool		inside;
		
			LockLooper();
			GetMouse( &mousePos, &buttons, TRUE );
			if (!buttons)
			{
				UnlockLooper();
				break;
			}
			
			inside = (r.Contains( mousePos ) && mousePos.x < Arrow_Width);
			if (inside != decLit)
			{
				decLit = inside;
				Invalidate( BRect( r.left, r.top, r.left + Arrow_Width, r.bottom ) );
			}
			
			if (decLit && currentVal > minVal) UpdateValue( currentVal - 1 );
			
			Window()->UpdateIfNeeded();
			UnlockLooper();
			snooze( delay );
			delay = 50.0 * 1000.0;
		}
		
		decLit = false;
		LockLooper();
		Invalidate( BRect( r.left, r.top, r.left + Arrow_Width, r.bottom ) );
		UnlockLooper();
	}
	else if (mousePos.x >= r.right - Arrow_Width)
	{
		for (;;)
		{
			bool		inside;
		
			LockLooper();
			GetMouse( &mousePos, &buttons, TRUE );
			if (!buttons)
			{
				UnlockLooper();
				break;
			}
			
			inside = (r.Contains( mousePos ) && r.right - Arrow_Width);
			if (inside != incLit)
			{
				incLit = inside;
				Invalidate( BRect( r.right - Arrow_Width, r.top, r.right, r.bottom ) );
			}
			
			if (incLit && currentVal < maxVal) UpdateValue( currentVal + 1 );
			
			Window()->UpdateIfNeeded();
			UnlockLooper();
			snooze( delay );
			delay = 50.0 * 1000.0;
		}
		
		incLit = false;
		LockLooper();
		Invalidate( BRect( r.right - Arrow_Width, r.top, r.right, r.bottom ) );
		UnlockLooper();
	}
	else if (mousePos.x < kPos)
	{
		for (;;)
		{
			LockLooper();
			GetMouse( &mousePos, &buttons, TRUE );
			if (!buttons)
			{
				UnlockLooper();
				break;
			}
	
			KnobPosition( kPos, kWidth );
	
			if (mousePos.x < kPos) newValue = currentVal - bodyIncrement;
		
			UpdateValue( newValue );
			Window()->UpdateIfNeeded();
			UnlockLooper();
	
			snooze( delay * 2 );
			delay = 70.0 * 1000.0;
		}
	}
	else if (mousePos.x > kPos + kWidth)
	{
		for (;;)
		{
			LockLooper();
			GetMouse( &mousePos, &buttons, TRUE );
			if (!buttons)
			{
				UnlockLooper();
				break;
			}
	
			KnobPosition( kPos, kWidth );
	
			if (mousePos.x > kPos + kWidth) newValue = currentVal + bodyIncrement;
		
			UpdateValue( newValue );
			Window()->UpdateIfNeeded();
			UnlockLooper();
	
			snooze( delay * 2 );
			delay = 70.0 * 1000.0;
		}
	}
	else
	{
		float		offset = mousePos.x - kPos + r.left + Arrow_Width;
	
		do
		{
			LockLooper();
			GetMouse( &mousePos, &buttons, TRUE );
	
				// If both buttons are held down, it means cancel the drag
			if ((~buttons & (B_PRIMARY_MOUSE_BUTTON|B_SECONDARY_MOUSE_BUTTON)) == 0)
			{
				BMessage		msg(*Message());

				SetValue( prevValue );

				msg.AddInt32( "value", prevValue );
				msg.AddBool( "canceled", true );
				Invoke( &msg );

				UnlockLooper();
				break;
			}
		
			if (maxVal > minVal)
			{
				int32		pixelRange = r.Width() - Arrow_Width * 2 - kWidth;
	
				newValue =
					((mousePos.x - offset) * (maxVal - minVal) + pixelRange/2)
							/ pixelRange + minVal;
			}
			else
			{
				newValue = minVal;
			}
		
			UpdateValue( newValue );
			Window()->UpdateIfNeeded();
			UnlockLooper();
	
			snooze( 20.0 * 1000.0 );
		}
		while (buttons) ;
	}

	if (currentVal != prevValue)
	{
		BMessage		msg(*Message());

		msg.AddInt32( "value", currentVal );
		msg.AddBool( "final", true );
		Invoke( &msg );
	}

	return B_OK;
}

void CTextSlider::MouseDown( BPoint point )
{
	if (IsEnabled())
	{
		mousePos = point;
	
			// spawn a thread to drag the slider
		thread_id tid;
		tid = spawn_thread(drag_entry, "", B_NORMAL_PRIORITY, this);
		resume_thread(tid);
	}
}

#pragma export off
