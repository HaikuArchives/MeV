/* ===================================================================== *
 * Junk.cpp (MeV/UI)
 * ===================================================================== */

#include "Junk.h"

#include "EventTrack.h"

// Gnu C Library
#include <stdio.h>

// =============================================================================

enum E_TimeComponents {
	TC_Milliseconds,
	TC_Seconds,
	TC_Minutes,
	TC_Hours,
	TC_Clocks,
	TC_Beats,
	TC_Measures,
};

inline int32 udiv( int32 dividend, int32 divisor )
{
	if (dividend >= 0) return dividend / divisor;
	else return (dividend - divisor + 1) / divisor;
}

CTimeEditControl::CTimeEditControl(
	BRect		inFrame,
	BMessage		*inMessage,
	uint32		inResizingMode,
	uint32		inFlags )
	: BControl( inFrame, NULL, NULL, inMessage, inResizingMode, inFlags )
{
	beatSize = 240;
	measureSize = beatSize * 4;
	measureBase = 0;

	SetValue( 0 );
	
	clockType = ClockType_Metered;
}

	// REM: This is still too flickery for my taste.

void CTimeEditControl::Draw( BRect inUpdateRect )
{
	BRect		r( Bounds() );
	BFont		font1( be_bold_font ),
				font2( be_plain_font );
	escapement_delta ed;
	
	ed.space = ed.nonspace = 0.0;
         				
	font1.SetSize( 14.0 );
	font2.SetSize( 9.0 );

//	font1.SetSpacing( B_BITMAP_SPACING );
	font1.SetSpacing( B_FIXED_SPACING );
	SetLowColor( Parent()->ViewColor() );
	SetDrawingMode( B_OP_COPY );

	SetHighColor( Parent()->ViewColor() );
	FillRect( BRect( r.left, r.top, r.left, r.top ) );
	FillRect( BRect( r.left, r.bottom, r.left, r.bottom ) );
	FillRect( BRect( r.right, r.top, r.right, r.top ) );
	FillRect( BRect( r.right, r.bottom, r.right, r.bottom ) );

	SetHighColor( 100, 100, 100 );
	FillRect( BRect( r.left + 1, r.top, r.right - 1, r.top ) );
	FillRect( BRect( r.left + 1, r.bottom, r.right - 1, r.bottom ) );
	FillRect( BRect( r.left, r.top + 1, r.left, r.bottom - 1 ) );
	FillRect( BRect( r.right, r.top + 1, r.right, r.bottom - 1 ) );
	
	SetHighColor( 255, 255, 255 );
	FillRect( BRect( r.left + 1, r.top + 1, r.right - 1, r.top + 2 ) );
	FillRect( BRect( r.left + 1, r.top + 3, r.left + 1, r.bottom - 3 ) );

	SetHighColor( 200, 200, 200 );
	FillRect( BRect( r.left + 1, r.bottom - 2, r.right - 1, r.bottom - 2 ) );
	
	SetHighColor( 180, 180, 180 );
	FillRect( BRect( r.left + 1, r.bottom - 1, r.right - 1, r.bottom - 1 ) );
	FillRect( BRect( r.right - 1, r.top + 3, r.right - 1, r.bottom - 3 ) );
	
	Sync();

	if (clockType == ClockType_Metered)
	{
		int32		measures, beats, clocks;
	
		clocks = Value() - measureBase;
		measures = udiv( clocks, measureSize );
		clocks -= measures * measureSize;
		beats = clocks / beatSize;
		clocks -= beats * beatSize;
		
		sprintf(	text, "%4.4ld:%2.2ld:%3.3ld", measures, beats, clocks );

		SetFont( &font1 );

		SetHighColor( Parent()->ViewColor() );
		FillRect( BRect( r.left + 2, r.top + 2, r.right - 2, r.top + 12 ) );

		MovePenTo( r.left + 4, r.top + 13 );
		SetHighColor( 0, 0, 255 );
		DrawString( &text[ 0 ], 6, &ed );
		SetHighColor( 0, 0, 0 );
		DrawString( &text[ 6 ], 12 - 6, &ed );
		SetFont( &font2 );
		SetHighColor( Parent()->ViewColor() );
		FillRect( BRect( r.left + 2, r.top + 13, r.right - 2, r.bottom - 2 ) );
		SetHighColor( 0, 0, 0 );
		MovePenTo( r.left + 6, r.top + 21 );
		DrawString( "Meas.  Beat  Tick" );
	}
	else
	{
		int32		hours, minutes, seconds, ms;
		
		ms = Value();
		hours = udiv( ms, 60 * 60 * 1000 );
		ms -= hours * 60 * 60 * 1000;
		minutes = ms / (60 * 1000);
		ms -= minutes * 60 * 1000;
		seconds = ms / 1000;
		ms -= seconds * 1000;
	
		sprintf( text, "%2.2ld:%2.2ld:%2.2ld.%3.3ld", hours, minutes, seconds, ms );

		SetFont( &font1 );

		SetHighColor( Parent()->ViewColor() );
		FillRect( BRect( r.left + 2, r.top + 2, r.right - 2, r.top + 12 ) );

		MovePenTo( r.left + 2, r.top + 13 );
		SetHighColor( 0, 0, 255 );
		DrawString( &text[ 0 ], 6, &ed );
		SetHighColor( 0, 0, 0 );
		DrawString( &text[ 6 ], 12 - 6, &ed );
		SetFont( &font2 );
		SetHighColor( Parent()->ViewColor() );
		FillRect( BRect( r.left + 2, r.top + 13, r.right - 2, r.bottom - 2 ) );
		SetHighColor( 0, 0, 0 );
		MovePenTo( r.left + 5, r.top + 21 );
		DrawString( "Hr.   Min.  Sec.   ms" );
	}
}

void CTimeEditControl::SetValue( int32 value )
{
	if ( value != Value() )
	{
		BControl::SetValue( value );
	}
}

void CTimeEditControl::MouseDown( BPoint point )
{
	if (IsEnabled())
	{
		mousePos = point;
	
			// spawn a thread to drag the slider
		thread_id tid;
		tid = spawn_thread( drag_entry, "", B_NORMAL_PRIORITY, this );
		resume_thread( tid );
	}
}

long CTimeEditControl::drag_entry( void *arg )
{
	return ((CTimeEditControl *) arg)->Drag();
}

long CTimeEditControl::Drag()
{
	// First, we need to determine which digit was clicked on...

	return B_OK;
}
