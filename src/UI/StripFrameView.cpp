/* ===================================================================== *
 * StripFrameView.cpp (MeV/User Interface)
 * ===================================================================== */

#include "StripFrameView.h"
#include "StWindowUtils.h"
#include "DocWindow.h"
#include "StripView.h"
#include "ResourceUtils.h"

#include "MeV.h"

const int		StripDivider_Height = 5;

// ---------------------------------------------------------------------------
// Constructor/Destructor

CStripFrameView::CStripFrameView(
	BRect frame,
	char *name,
	ulong resizingMode)
	: CScrollerTarget(frame, name, resizingMode, B_WILL_DRAW | B_FRAME_EVENTS),
	  m_ruler(NULL)
{
	SetViewColor(B_TRANSPARENT_32_BIT);
}

CStripFrameView::~CStripFrameView()
{
	for (int32 i = 0; i < m_childInfoList.CountItems(); i++)
	{
		ChildInfo *childInfo = (ChildInfo *)m_childInfoList.ItemAt(i);	
		if (childInfo)
			delete childInfo;
	}
}

// ---------------------------------------------------------------------------
// CScrollerTarget Implementation

void
CStripFrameView::AttachedToWindow()
{
	AdjustScrollers();

	CScrollerTarget::AttachedToWindow();
}

void
CStripFrameView::FrameResized(
	float width,
	float height)
{
	ArrangeViews();
	Draw(Bounds());
	AdjustScrollers();
	Window()->UpdateIfNeeded();
}

void
CStripFrameView::Draw(
	BRect updateRect)
{
	if (m_childViews.IsEmpty())
	{
		// Fill with gray
		SetHighColor( 128, 128, 128 );
		FillRect( updateRect );
	}
	else
	{
		float y;
		BRect r(updateRect);
	
		for (int i = 1; i < m_childViews.CountItems(); i++)
		{
			BView		*childView = (BView *)m_childViews.ItemAt( i );
			ChildInfo	*childInfo = (ChildInfo *)m_childInfoList.ItemAt( i );
			
			if (childInfo->fixedSize == false)
			{
				y = childView->Frame().top - 4;

				r.top = r.bottom = y;
				SetHighColor( 150, 150, 150 );
				FillRect( r );

				r.top = r.bottom = r.top + 1;
				SetHighColor( 255, 255, 255 );
				FillRect( r );

				r.top = r.top + 1;
				r.bottom = r.top + 1;
				SetHighColor( 225, 225, 225 );
				FillRect( r );

				r.top = r.bottom = r.top + 1;
				SetHighColor( 150, 150, 150 );
				FillRect( r );
			}
			else
			{
				y = childView->Frame().top - 1;

				r.top = r.bottom = y;
				SetHighColor( 150, 150, 150 );
				FillRect( r );
			}
		}
	}
}

void
CStripFrameView::MouseDown(
	BPoint point)
{
	ulong		buttons;
	BView		*prevView,
				*nextView;

	// Which strip did we hit?
	int y;
	short selectedStrip = -1;
	float stripPos, stripTopLimit, stripBottomLimit, mouseOffset;
	
// buttons = Window()->CurrentMessage()->FindLong( "buttons" );

	for (int i = 0; i < m_childViews.CountItems() - 1; i++)
	{
		prevView = (BView *)m_childViews.ItemAt( i );
		ChildInfo	*childInfo = (ChildInfo *)m_childInfoList.ItemAt( i + 1 );
		
		y = prevView->Frame().bottom;

		if (!childInfo->fixedSize
			&& (point.y >= y)
			&& (point.y <= y + StripDivider_Height))
		{
			selectedStrip = i;
			stripPos = y;
			mouseOffset = point.y - y;
			stripTopLimit = prevView->Frame().top + MinimumViewSize( prevView ) - 1.0;
			break;
		}
	}
	
	if (selectedStrip < 0
		||	selectedStrip >= m_childViews.CountItems()) return;

	nextView = (BView *)m_childViews.ItemAt( selectedStrip + 1 );

	stripBottomLimit	= nextView->Frame().bottom
						- StripDivider_Height
						- MinimumViewSize( nextView ) + 1;

	do
	{
		float		newStripPos;
	
		snooze(20 * 1000);
		GetMouse( &point, &buttons, TRUE );
		
		newStripPos = point.y - mouseOffset;

		if (newStripPos < stripTopLimit)
			newStripPos = stripTopLimit;

		if (newStripPos > stripBottomLimit)
			newStripPos = stripBottomLimit;
		
		if (newStripPos != stripPos)
		{
			if (newStripPos > stripPos)
			{
				nextView->ResizeBy( 0, stripPos - newStripPos );
				nextView->MoveBy  ( 0, newStripPos - stripPos );
				prevView->ResizeBy( 0, newStripPos - stripPos );
			}
			else
			{
				prevView->ResizeBy( 0, newStripPos - stripPos );
				nextView->MoveBy  ( 0, newStripPos - stripPos );
				nextView->ResizeBy( 0, stripPos - newStripPos );
			}
			Invalidate( BRect(	0,
								MIN( newStripPos, stripPos ),
								Frame().Width(),
								MAX( newStripPos, stripPos ) + StripDivider_Height ) );
			stripPos = newStripPos;

			Window()->UpdateIfNeeded();
		}
	}
	while (buttons) ;

	for (int i = 0; i < m_childViews.CountItems(); i++)
	{
		BView		*childView = (BView *)m_childViews.ItemAt( i );
		ChildInfo	*childInfo = (ChildInfo *)m_childInfoList.ItemAt( i );
		
		childInfo->proportion = childView->Frame().Height();
	}
}

void CStripFrameView::MouseMoved(
	BPoint		point,
	ulong		transit,
	const BMessage	* )
{
	switch (transit) {
	case B_ENTERED_VIEW: 
//		if (cursor != NULL) be_app->SetCursor( cursor );
		break;

	case B_EXITED_VIEW:
//		be_app->SetCursor( B_HAND_CURSOR );
		break;
	}
}
	
// ---------------------------------------------------------------------------
// Operations

bool
CStripFrameView::AddChildView(
	BView		*inView,
	int			inHeight,
	int			inIndex,
	bool			inFixed )
{
	StWindowLocker			svLock  ( Window() );
	StWindowUpdateDisabler	svUpdate( Window() );

	ChildInfo	*newInfo = new ChildInfo;
	
	if (inHeight < 0) inHeight = 0;

	newInfo->proportion = inHeight;
	newInfo->fixedSize = inFixed;

	inView->SetResizingMode( B_FOLLOW_LEFT_RIGHT );
	
		// If it's empty, then it's simple
	if (m_childViews.IsEmpty())
	{
		if (m_childInfoList.AddItem( newInfo ))
		{
			if (m_childViews.AddItem( inView ))
			{
				inView->MoveTo( 0.0, 0.0 );
				AddChild( inView );
				ArrangeViews();
				return true;
			}
			else
			{
				m_childInfoList.RemoveItem( newInfo );
			}
		}
		return false;
	}

		// Range-limit the input index
	if (inIndex < 0 || inIndex < m_childViews.CountItems())
		inIndex = m_childViews.CountItems();
	
		// Add the view into the list.
	if (m_childViews.AddItem( inView, inIndex ) == false)
	{
		return false;
	}
	else if (m_childInfoList.AddItem( newInfo ) == false)
	{
		if (m_childViews.RemoveItem( inView ) == false)
		return false;
	}
	
	AddChild( inView );
	ArrangeViews();
	
	return true;
}

void
CStripFrameView::RemoveChildView(
	BView *inView)
{
	int32		index;
	
	if ((index = m_childViews.IndexOf( inView )) >= 0)
	{
		BView		*childView = (BView *)m_childViews.ItemAt( index );
		ChildInfo	*childInfo = (ChildInfo *)m_childInfoList.ItemAt( index );
		
		m_childViews.RemoveItem( index );
		m_childInfoList.RemoveItem( index );
		
		RemoveChild( childView );
		delete childView;
		delete childInfo;
		
		StWindowLocker			svLock  ( Window() );
		StWindowUpdateDisabler	svUpdate( Window() );
		ArrangeViews();
	}
}

void
CStripFrameView::ArrangeViews()
{
	int			i;
	float		y = 0;
	float		width = Frame().Width(),
				height = Frame().Height();
	int32		totalProportion = 0,
				totalHeight = height;
	int32		totalChildHeight = 0;
	int32		childCount = m_childViews.CountItems();

	for (i = 0; i < childCount; i++)
	{
		BView		*childView = (BView *)m_childViews.ItemAt( i );
		ChildInfo	*childInfo = (ChildInfo *)m_childInfoList.ItemAt( i );
		
		if (childInfo->fixedSize)
		{
			totalHeight -= childInfo->proportion;
			if (i > 0)
			{
				totalHeight		-= 2;
				totalChildHeight	+= 2;
			}
		}
		else
		{
			totalProportion += childInfo->proportion;
			totalHeight -= MinimumViewSize( childView );
			if (i > 0)
			{
				totalHeight		-= StripDivider_Height;
				totalChildHeight	+= StripDivider_Height;
			}
		}
		totalChildHeight += childView->Frame().Height();
	}
	
	for (i = 0; i < childCount; i++)
	{
		BView		*childView = (BView *)m_childViews.ItemAt( i );
		ChildInfo	*childInfo = (ChildInfo *)m_childInfoList.ItemAt( i );
		int32		h;

		if (childInfo->fixedSize)
		{
			if (i > 0) y += 2;
			h = childInfo->proportion;
		}
		else
		{
			if (i > 0) y += StripDivider_Height;
			h = MinimumViewSize( childView );
			if (childInfo->proportion > 0)
			{
				h += totalHeight * childInfo->proportion / totalProportion;
				totalProportion -= childInfo->proportion;
			}
		}

		totalHeight -= h;
		
		childInfo->y = y;
		childInfo->h = h;
		
		y += /* StripDivider_Height + */ h;
	}
	
	if (height == totalChildHeight) return;

		// Apply the new height and position to each item, in the proper order.
	if (height - totalChildHeight > 0)
	{
		for (i = childCount - 1; i >= 0; i--)
		{
			BView		*childView = (BView *)m_childViews.ItemAt( i );
			ChildInfo	*childInfo = (ChildInfo *)m_childInfoList.ItemAt( i );
			CStripView	*sv;

			childView->MoveTo( 0, childInfo->y );
			childView->ResizeTo(	width, childInfo->h );
			if ((sv = dynamic_cast<CStripView *>(childView)))
			{
				sv->bounds = sv->Bounds();
			}
		}
	}
	else
	{
		for (i = 0; i < childCount; i++)
		{
			BView		*childView = (BView *)m_childViews.ItemAt( i );
			ChildInfo	*childInfo = (ChildInfo *)m_childInfoList.ItemAt( i );
			CStripView	*sv;

			childView->ResizeTo(	width, childInfo->h );
			childView->MoveTo( 0, childInfo->y );
			if ((sv = dynamic_cast<CStripView *>(childView)))
			{
				sv->bounds = sv->Bounds();
			}
		}
	}
}

void
CStripFrameView::SetScrollValue(
	float			inScrollValue,
	orientation		inOrient )
{
	if (inOrient == B_HORIZONTAL)
	{
		float	scrollDelta = inScrollValue - scrollValue.x;
	
		scrollValue.x = inScrollValue;
		
		if (m_ruler)
			m_ruler->SetScrollValue( inScrollValue, B_HORIZONTAL );

		for (int i = 0; i < m_childViews.CountItems(); i++)
		{
			BView			*childView = (BView *)m_childViews.ItemAt( i );
			CScrollerTarget	*st = dynamic_cast<CScrollerTarget *>(childView);
			if (st)
			{
				st->SetScrollValue( inScrollValue, B_HORIZONTAL );
			}
			else childView->ScrollBy( scrollDelta, 0.0 );
		}
	}
}

// END - StripFrameView.cpp