/* ===================================================================== *
 * ScreenUtils.cpp (MeV/Application Framework)
 * ===================================================================== */

#include "ScreenUtils.h"

// Interface Kit
#include <Screen.h>

BPoint		UScreenUtils::windowPos( 0, 0 );

#pragma export on
BRect UScreenUtils::CenterOnScreen( int32 inWidth, int32 inHeight, screen_id id )
{
	BRect	screenRect = BScreen( id ).Frame();
	BRect	r( 0, 0, inWidth, inHeight );
	
	r.OffsetTo( (screenRect.Width() - inWidth) / 2, (screenRect.Height() - inHeight) / 2 );
	return r;
}

BRect UScreenUtils::StackOnScreen( int32 inWidth, int32 inHeight, screen_id id )
{
	BRect	screenRect = BScreen( id ).Frame();
	BRect	r( 0, 0, inWidth, inHeight );
	
	if (inWidth > screenRect.Width() || inHeight > screenRect.Height())
	{
		r.OffsetTo( 20, 20 );
	}
	else
	{
		windowPos += BPoint( 12, 24 );
		r.OffsetTo( windowPos );
		if (!screenRect.Contains( r ))
		{
			windowPos.Set( 12, 24 );
			r.OffsetTo( windowPos );
		}
	}
	
	return r;
}

BRect UScreenUtils::ConstrainToScreen( BRect inRect, screen_id id )
{
	BRect	screenRect = BScreen( id ).Frame();
	BPoint	p = inRect.LeftTop();
	
	p.x = p.x < screenRect.left ? screenRect.left : 
								  p.x > screenRect.right - inRect.Width() ? screenRect.right - inRect.Width() :
								  											p.x;
	p.y = p.y < screenRect.top ? screenRect.top :
								 p.y > screenRect.bottom - inRect.Height() ? screenRect.bottom - inRect.Height() :
								 											 p.y;
	inRect.OffsetTo( p );
	
	return inRect;
}

BRect UScreenUtils::CenterOnWindow( int32 inWidth, int32 inHeight, BWindow *parent )
{
	BRect	pFrame( parent->Frame() );
	BPoint	p(	(pFrame.left + pFrame.right - inWidth) / 2,
				(pFrame.top + pFrame.bottom - inHeight) / 2 );
	BRect screenRect = BScreen( parent ).Frame();

	p.x = p.x < screenRect.left ? screenRect.left : 
								  p.x > screenRect.right - inWidth ? screenRect.right - inWidth :
								  											p.x;
	p.y = p.y < screenRect.top ? screenRect.top :
								 p.y > screenRect.bottom - inHeight ? screenRect.bottom - inHeight :
																			 p.y;
	return BRect( p.x, p.y, p.x + inWidth, p.y + inHeight );
}

#pragma export off
