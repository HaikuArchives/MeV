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
		StdBevels::DrawBorderBevel(this, r, 
								   Value() ? StdBevels::DEPRESSED_BEVEL
								   		   : StdBevels::NORMAL_BEVEL);
	}
	else
	{
		StdBevels::DrawBorderBevel(this, r, StdBevels::DIMMED_BEVEL);
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

