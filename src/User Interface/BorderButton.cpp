/* ===================================================================== *
 * BorderButton.cpp (MeV/User Interface)
 * ===================================================================== */

#include "BorderButton.h"
#include "StdBevels.h"

// Interface Kit
#include <Bitmap.h>
#include <Window.h>

void CBorderButton::Draw( BRect inUpdateRect )
{
	BRect		r( Bounds() );
	
	DrawBorderBevel( *this, r, Value() ? Bevel_Depressed : Bevel_Normal );
	BBitmap		*image = (Value() && glyph[ 1 ]) ? glyph[ 1 ] : glyph[ 0 ];
	
	if (image != NULL)
	{
		BRect	br( image->Bounds() );

		SetDrawingMode( B_OP_OVER );
		DrawBitmapAsync( image, BPoint(	(r.left + r.right - br.Width())/2,
										(r.top + r.bottom - br.Height())/2 ) );
	}
}

void CBorderButton::MouseDown( BPoint point )
{
	SetValue( true );
	uint32		buttons;
	bool			sel;

	GetMouse( &point, &buttons, TRUE );

	do
	{
		GetMouse( &point, &buttons, TRUE );
		
		sel = Bounds().Contains( point );

		if (sel != Value())
			BControl::SetValue( sel );
		
		Window()->UpdateIfNeeded();
		snooze( 20 * 1000 );
	}
	while (buttons) ;
	
	if (Value()) Invoke();
	BControl::SetValue( false );
}

