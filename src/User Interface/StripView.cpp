/* ===================================================================== *
 * StripView.cpp (MeV/User Interface)
 * ===================================================================== */

#include "Idents.h"
#include "MeV.h"
#include "StripView.h"
#include "BorderView.h"
#include "DocWindow.h"
#include "BorderButton.h"

CStripView::CStripView(
	CStripFrameView	&inFrame,
	BRect			rect,
	const char		*name,
	bool			makeScroller,
	bool			makeMagButtons )
	: frame( inFrame ),
	  CScrollerTarget(	BRect( 0.0, 0.0, rect.Width(), rect.Height() ),
	  					name,
	  					B_FOLLOW_ALL,
	  					B_WILL_DRAW | B_FRAME_EVENTS )
{
	rightScroller = NULL;
	rightSpacer = NULL;
	rect.OffsetTo( B_ORIGIN );
	magIncButton = magDecButton = NULL;

	container = new CScrollerTarget(	rect,
										NULL,
										B_FOLLOW_LEFT_RIGHT,
										B_FRAME_EVENTS );

	container->AddChild( this );

	if (makeScroller)
	{
		ResizeBy( -14.0, 0.0 );
		
		float		bottom = rect.bottom + 1;
		
		if (makeMagButtons)
		{
			magIncButton = new CBorderButton(
				BRect(	rect.right - 13,
						rect.bottom - 27,
						rect.right + 1,
						rect.bottom - 13 ),
				NULL, LoadImage( smallPlusImage, SmallPlus_Image ),
				new BMessage( ZoomOut_ID ),
				B_FOLLOW_BOTTOM | B_FOLLOW_RIGHT,
				B_WILL_DRAW );
			container->AddChild( magIncButton );
// 		magIncButton->SetTarget( this );

			magDecButton = new CBorderButton(
				BRect(	rect.right - 13,
						rect.bottom - 13,
						rect.right + 1,
						rect.bottom + 1 ),
				NULL, LoadImage( smallMinusImage, SmallMinus_Image ),
				new BMessage( ZoomIn_ID ),
				B_FOLLOW_BOTTOM | B_FOLLOW_RIGHT,
				B_WILL_DRAW );
			container->AddChild( magDecButton );
// 		magDecButton->SetTarget( this );
			
			bottom = rect.bottom - 27;
		}
	
		rightScroller = new CScroller(	BRect(	rect.right  - 13,
												rect.top    - 1,
												rect.right  + 1,
												bottom ),
										NULL,
										this,
										0.0, 50.0, B_VERTICAL );
		container->AddChild( rightScroller );
	}
	else
	{
		ResizeBy( -14.0, 0.0 );
		rightSpacer = new CBorderView(	BRect(	rect.right	- 13,
												rect.top	- 1,
												rect.right	+ 1,
												rect.bottom + 1 ),
										NULL,
										B_FOLLOW_RIGHT | B_FOLLOW_TOP_BOTTOM,
										B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE );
		container->AddChild( rightSpacer );
	}

	container->SetTarget( this );
	SetZoomTarget( this );
}

void CStripView::Draw( BRect updateRect )
{
	StrokeLine( BPoint( 0.0, 0.0 ), BPoint( 300.0, 100.0 ) );
	StrokeLine( BPoint( 300.0, 0.0 ), BPoint( 0.0, 100.0 ) );
}

void CStripView::FrameResized( float width, float height )
{
	AdjustScrollers();
}

void CStripView::SetSelectionVisible( bool inVisible )
{
	if (inVisible != selectionVisible)
	{
		selectionVisible = inVisible;
		if (inVisible) OnGainSelection();
		else OnLoseSelection();
	}
}
