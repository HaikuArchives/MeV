/* ===================================================================== *
 * TextDisplay.cpp (MeV/User Interface)
 * ===================================================================== */

#include "TextDisplay.h"

void CTextDisplay::Draw( BRect updateRect )
{
	BRect		r( Bounds() );
	font_height	fh;
	
	if (borders)
	{
		SetHighColor( 190, 190, 190 );
		FillRect( BRect( 0.0, 0.0, r.right - 1, 0.0 ) );
		FillRect( BRect( 0.0, 0.0, 0.0, r.bottom - 1 ) );
	
		SetHighColor( 255, 255, 255 );
		FillRect( BRect( 1.0, r.bottom, r.right, r.bottom ) );
		FillRect( BRect( r.right, 1.0, r.right, r.bottom ) );

		r.InsetBy( 1.0, 1.0 );
	}
	
	SetHighColor( backColor );
	FillRect( BRect( r.left, r.top, r.right, r.bottom ) );

	if (text)
	{
		int32	length = StringWidth( text ),
				x;

		GetFontHeight( &fh );
		
		if (align == B_ALIGN_LEFT)		x = 3.0;
		else if (align == B_ALIGN_RIGHT)	x = r.Width() - length - 3.0;
		else								x = (r.Width() - length) / 2;

		SetHighColor( 0, 0, 0 );
		MovePenTo( x, (r.top + r.bottom - fh.descent + fh.ascent) / 2 );
		SetDrawingMode(B_OP_OVER);
		DrawString( text );
	}
}
