/* ===================================================================== *
 * StdButton.cpp (MeV/User Interface)
 * ===================================================================== */

#include "StdButton.h"
#include "StdBevels.h"

// Interface Kit
#include <Bitmap.h>

void CPushOnButton::Draw( BRect inUpdateRect )
{
	BRect		r( Bounds() );
	
	if (IsEnabled())
	{
		DrawBorderBevel( *this, r, Value() ? Bevel_Depressed : Bevel_Normal );
	}
	else
	{
		DrawBorderBevel( *this, r, Bevel_Dimmed );
	}

	BBitmap		*image = (Value() && glyph[ 1 ]) ? glyph[ 1 ] : glyph[ 0 ];
	
	if (image != NULL)
	{
		BRect	br( image->Bounds() );
		
		SetDrawingMode( IsEnabled() ? B_OP_OVER : B_OP_BLEND );
		DrawBitmapAsync( image, BPoint(	(r.left + r.right - br.Width())/2,
										(r.top + r.bottom - br.Height())/2 ) );
		SetDrawingMode( B_OP_COPY );
	}
}

void CPushOnButton::MouseDown( BPoint point )
{
	if (!Value()) 
	{
		SetValue( true );
		Invoke();
	}
}

void CToggleButton::MouseDown( BPoint point )
{
	SetValue( !Value() );
	Invoke();
}

