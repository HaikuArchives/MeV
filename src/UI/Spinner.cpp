/* ===================================================================== *
 * Spinner.cpp (MeV/User Interface)
 * ===================================================================== */

#include "Spinner.h"
// User Interface
#include "StdBevels.h"

// Standard Template Library
#include <algorithm>
// Interface Kit
#include <Window.h>

inline const int32 CLAMP(const int32 &a, const int32 &b, const int32 &c)
{
	if (b < a) return a;
	if (b > c) return c;
	return b;
}

#pragma export on

CSpinner::CSpinner( BRect frame, const char *name, BMessage *msg,
	uint32 resizingMode, uint32 flags )
	: BControl( frame, name, NULL, msg, resizingMode, flags )
{
	minVal = maxVal = 0;
}

void CSpinner::AttachedToWindow()
{
	BControl::AttachedToWindow();
//	if (Parent()) SetViewColor( Parent()->ViewColor() );
}

void
CSpinner::Draw(
	BRect updateRect)
{
	BRect upperRect(Bounds());
	upperRect.bottom = floor((Bounds().Height() + 1.0) / 2.0);
	BRect lowerRect(Bounds());
	lowerRect.top = upperRect.bottom + 1.0;
	BRect arrowRect;
	rgb_color arrowColor;

	if (!IsEnabled())
	{
		StdBevels::DrawBorderBevel(this, upperRect, StdBevels::DIMMED_BEVEL);
		StdBevels::DrawBorderBevel(this, lowerRect, StdBevels::DIMMED_BEVEL);
		arrowColor = tint_color(ui_color(B_PANEL_BACKGROUND_COLOR),
								B_LIGHTEN_1_TINT);
	}
	else
	{
		StdBevels::DrawBorderBevel(this, upperRect, (lit == Inc_Lit) ?
													StdBevels::DEPRESSED_BEVEL :
													StdBevels::NORMAL_BEVEL);
		StdBevels::DrawBorderBevel(this, lowerRect, (lit == Dec_Lit) ?
													StdBevels::DEPRESSED_BEVEL :
													StdBevels::NORMAL_BEVEL);
		arrowColor = tint_color(ui_color(B_PANEL_BACKGROUND_COLOR),
								B_DARKEN_3_TINT);
	}

	// draw inc arrow
	arrowRect = upperRect;
	arrowRect.InsetBy(arrowRect.Width() / 2.0,
					  arrowRect.Height() / 2.0 - 1.0);
	BeginLineArray(3);
	AddLine(arrowRect.LeftTop(), arrowRect.RightTop(), arrowColor);
	arrowRect.InsetBy(-1.0, 0.0);
	arrowRect.top += 1.0;
	AddLine(arrowRect.LeftTop(), arrowRect.RightTop(), arrowColor);
	arrowRect.InsetBy(-1.0, 0.0);
	arrowRect.top += 1.0;
	AddLine(arrowRect.LeftTop(), arrowRect.RightTop(), arrowColor);
	EndLineArray();

	// draw inc arrow
	arrowRect = lowerRect;
	arrowRect.InsetBy(arrowRect.Width() / 2.0 - 2.0,
					  arrowRect.Height() / 2.0 - 1.0);
	BeginLineArray(3);
	AddLine(arrowRect.LeftTop(), arrowRect.RightTop(), arrowColor);
	arrowRect.InsetBy(1.0, 0.0);
	arrowRect.top += 1.0;
	AddLine(arrowRect.LeftTop(), arrowRect.RightTop(), arrowColor);
	arrowRect.InsetBy(1.0, 0.0);
	arrowRect.top += 1.0;
	AddLine(arrowRect.LeftTop(), arrowRect.RightTop(), arrowColor);
	EndLineArray();
}

void CSpinner::SetValue( int32 inValue )
{
	inValue = CLAMP( minVal, inValue, maxVal );
	if (Value() != inValue)
	{
		BControl::SetValue( inValue );
	}
}

void CSpinner::UpdateValue( int32 inValue )
{
	inValue = CLAMP( minVal, inValue, maxVal );
	if (Value() != inValue)
	{
		BMessage		msg(*Message());

		BControl::SetValue( inValue );

		msg.AddInt32( "value", Value() );
		Invoke( &msg );
	}
}

void CSpinner::SetRange( int32 inMIN, int32 inMAX )
{
	inMAX = std::max( inMAX, inMIN );
	if (minVal != inMIN || maxVal != inMAX)
	{
		minVal = inMIN;
		maxVal = inMAX;
		SetValue( Value() );
	}
}

long CSpinner::drag_entry(void *arg)
{
	return ((CSpinner *) arg)->Drag();
}

long CSpinner::Drag()
{
	int32		prevValue = Value();
	uint32		buttons;
	double		newVal = prevValue;
	float		incr,
				mid;
	uint8		whichLit;

	LockLooper();
	BRect		r( Bounds() );
	UnlockLooper();
	mid  = (r.top + r.bottom) / 2;

	if (mousePos.y < mid)
	{
		incr = 0.1;
		whichLit = Inc_Lit;
		r.bottom = mid;
		newVal += 0.5;
	}
	else
	{
		incr = -0.1;
		r.top = mid;
		whichLit = Dec_Lit;
		newVal -= 0.5;
	}

	for (;;)
	{
		uint8		newLit;

		LockLooper();
		GetMouse( &mousePos, &buttons, TRUE );
		if ((buttons & (B_PRIMARY_MOUSE_BUTTON|B_SECONDARY_MOUSE_BUTTON)) == 0)
		{
			UnlockLooper();
			break;
		}
		else if ((~buttons & (B_PRIMARY_MOUSE_BUTTON|B_SECONDARY_MOUSE_BUTTON)) == 0)
		{
			// If both buttons are held down, it means cancel the drag
			BMessage		msg(*Message());

			SetValue( prevValue );

			msg.AddInt32( "value", prevValue );
			msg.AddBool( "canceled", true );
			Invoke( &msg );

			UnlockLooper();
			break;
		}

		newLit = (r.Contains( mousePos )) ? whichLit : 0;
		if (newLit != lit)
		{
			lit = newLit;
			Invalidate( r );
		}

		if (lit)
		{
			if (newVal < minVal)			newVal = minVal;
			else if (newVal > maxVal)		newVal = maxVal;
			UpdateValue( (int32)floor( newVal + 0.5 ) );
			newVal += incr;

				// Accelerating spinners!
			incr *= 1.01;
		}

		Window()->UpdateIfNeeded();
		UnlockLooper();
		snooze( 20 * 1000 );
	}

	lit = 0;
	LockLooper();
	Invalidate( r );
	UnlockLooper();

	if (Value() != prevValue)
	{
		BMessage		msg(*Message());

		msg.AddInt32( "value", Value() );
		msg.AddBool( "final", true );
		Invoke( &msg );
	}

	return B_OK;
}

void CSpinner::MouseDown( BPoint point )
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
