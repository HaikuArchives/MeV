/* ===================================================================== *
 * Spinner.cpp (MeV/User Interface)
 * ===================================================================== */

#include "Spinner.h"

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

void CSpinner::Draw( BRect inInvalRect )
{
	BRect	r( Bounds() );
	float	yMid = floor((r.top + r.bottom + 1)/2),
			xMid = floor((r.left + r.right + 1)/2);
	bool		dim = !IsEnabled();
	
	if (dim)				SetHighColor( 160, 160, 160 );
	else					SetHighColor( 110, 110, 110 );
	
	FillRect( BRect( r.left + 1, r.top, r.right - 1, r.top ) );
	FillRect( BRect( r.left + 1, r.bottom, r.right - 1, r.bottom ) );
	FillRect( BRect( r.left + 1, yMid, r.right - 1, yMid ) );
	FillRect( BRect( r.left, r.top + 1, r.left, r.bottom - 1 ) );
	FillRect( BRect( r.right, r.top + 1, r.right, r.bottom - 1 ) );

	for (int i = 0; i < 2; i++)
	{
		BRect	r( Bounds() );
		bool		highlight;
		
		if (i == 0)
		{
			r.bottom = yMid;
			highlight = (lit == Inc_Lit);
		}
		else
		{
			r.top = yMid;
			highlight = (lit == Dec_Lit);
		}

		r.InsetBy( 1.0, 1.0 );

		if (dim)				SetHighColor( 240, 240, 240 );
		else if (!highlight)	SetHighColor( 255, 255, 255 );
		else					SetHighColor( 190, 190, 190 );

		FillRect( BRect( r.left, r.top, r.right, r.top ) );
		FillRect( BRect( r.left, r.top, r.left, r.bottom ) );
	
		if (dim)				SetHighColor( 210, 210, 210 );
		else if (highlight)	SetHighColor( 230, 230, 230 );
		else					SetHighColor( 190, 190, 190 );

		FillRect( BRect( r.left, r.bottom, r.right, r.bottom ) );
		FillRect( BRect( r.right, r.top, r.right, r.bottom ) );

		if (highlight)
		{
			r.InsetBy( 1.0, 1.0 );
			SetHighColor( 200, 200, 200 );
			FillRect( r );
		}
		
		float	y = floor( (r.top + r.bottom) / 2 );
		
		if (dim)		SetHighColor( 160, 160, 160 );
		else			SetHighColor( 148, 148, 148 );
		for (int j = 1; j >= 0; j--)
		{
			if (i == 0)
			{
				FillTriangle(	BPoint( xMid - 2.0 + j, y + 2.0 + j ),
								BPoint( xMid + j, y - 3.0 + j ),
								BPoint( xMid + 2.0 + j, y + 2.0 + j ) );
			}
			else
			{
				FillTriangle(	BPoint( xMid - 2.0 + j, y - 2.0 + j ),
								BPoint( xMid + j, y + 3.0 + j ),
								BPoint( xMid + 2.0 + j, y - 2.0 + j ) );
			}
			if (dim)		SetHighColor( 127, 127, 127 );
			else			SetHighColor( 0, 0, 0 );
		}
	}
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
	inMAX = max( inMAX, inMIN );
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

	Window()->Lock();
	BRect		r( Bounds() );
	Window()->Unlock();
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
		
		Window()->Lock();
		GetMouse( &mousePos, &buttons, TRUE );
		if ((buttons & (B_PRIMARY_MOUSE_BUTTON|B_SECONDARY_MOUSE_BUTTON)) == 0)
		{
			Window()->Unlock();
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

			Window()->Unlock();
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
		Window()->Unlock();
		snooze( 20 * 1000 );
	}
		
	lit = 0;
	Window()->Lock();
	Invalidate( r );
	Window()->Unlock();

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
