/* ===================================================================== *
 * MultiColumnListView.cpp (MeV/User Interface)
 * ===================================================================== */

#include "MultiColumnListView.h"

// Gnu C Library
#include <stdio.h>
#include <string.h>
// Application Kit
#include <Application.h>
// Interface Kit
#include <ScrollView.h>
#include <Window.h>

class CChildListView : public BListView {
	bool		cursorHidden;
	int32		dragInsertIndex;
	
	void SetInsertIndex( int32 inIndex );
	BRect GetInsertFrameRect( int32 index );
	
	void WindowActivated( bool inActive )
	{
		if (inActive == false) SetInsertIndex( -1 );
	}
	
	void MessageReceived( BMessage* theMessage );
	bool InitiateDrag(BPoint point, int32 index, bool wasSelected);
	
	void MakeFocus( bool focused )
	{
		BListView::MakeFocus( focused );
		BView				*gParent = Parent()->Parent();
		CMultiColumnListView	*mcl = dynamic_cast<CMultiColumnListView *>(gParent);
		BScrollView			*scrollView = mcl->scrollView;
		
		scrollView->SetBorderHighlighted( focused );
	}
	
public:
	CChildListView(
		BRect			frame,
		const char		*name,
		list_view_type	type = B_SINGLE_SELECTION_LIST,
		uint32			resizeMask = B_FOLLOW_ALL_SIDES,
		uint32			flags = B_WILL_DRAW | B_FRAME_EVENTS)
	: BListView(	frame, name, type, resizeMask, flags )
	{
		cursorHidden = false;
		dragInsertIndex = -1;
	}

	void Draw( BRect inUpdateRect );
	void MouseDown( BPoint where );
	void MouseMoved( BPoint point, ulong transit, const BMessage *dragMsg );
};

void CChildListView::MouseDown( BPoint where )
{
	BView					*gParent = Parent()->Parent();
	CMultiColumnListView	*mcl = dynamic_cast<CMultiColumnListView *>(gParent);

	SetInsertIndex( -1 );

	if (mcl)
	{
		int32				clickColumn;
		int32				clickRow;
		int32				clickCount;
		bool					dClick = false;
		
		if (Window()->CurrentMessage()->FindInt32( "clicks", 0, &clickCount ) != B_OK)
			clickCount = 0;

		clickColumn = mcl->PickColumn( where.x );
		clickRow = IndexOf( where );
		
		if (		mcl->clickColumn == clickColumn
			&&	mcl->clickRow == clickRow
			&&	clickCount >= 2)
		{
			dClick = true;
		}
		
		mcl->clickColumn = clickColumn;
		mcl->clickRow = clickRow;

		if (clickColumn >= 0)
		{
			CColumnField			*colField;
			
			colField = (CColumnField *)mcl->columns.ItemAt( mcl->clickColumn );

			if (		mcl->clickRow >= 0
				&&	mcl->clickRow < CountItems())
			{
				if (colField->selectable != false)
				{
					Select( mcl->clickRow );
				}

				if (dClick && InvocationMessage())
				{
					Invoke( InvocationMessage() );
				}
				else if (colField->draggable)
				{
					BListView::MouseDown( where );
				}
				else if (mcl->columnMsg != NULL)
				{
					BMessage		msg( *mcl->columnMsg );
				
					msg.AddInt32( "cell", mcl->clickRow );
					msg.AddInt32( "cell", mcl->clickColumn );
					Invoke( &msg );
				}
			}
			if (!mcl->stayFocused) MakeFocus( false );
			return;
		}

	}

	if (!mcl->stayFocused) BListView::MouseDown( where );
	MakeFocus( false );
}

bool CChildListView::InitiateDrag( BPoint point, int32 index, bool wasSelected )
{
	if (wasSelected)
	{
		BView					*gParent = Parent()->Parent();
		CMultiColumnListView	*mcl = dynamic_cast<CMultiColumnListView *>(gParent);

		if (mcl)
		{
			int32				clickColumn;
//			bool					dClick = false;

			clickColumn = mcl->PickColumn( point.x );
		
			mcl->clickColumn = clickColumn;
			mcl->clickRow = index;

			if (clickColumn >= 0)
			{
				CColumnField			*colField;
			
				colField = (CColumnField *)mcl->columns.ItemAt( clickColumn );

				if (colField->draggable && mcl->columnMsg)
				{
					BMessage		msg( *mcl->columnMsg );
				
					msg.AddInt32( "cell", mcl->clickRow );
					msg.AddInt32( "cell", mcl->clickColumn );
					Invoke( &msg );
					return true;
				}
			}
		}
	}
	return false;
}

void CChildListView::MouseMoved(
	BPoint			point,
	ulong			transit,
	const BMessage	*dragMsg )
{
	BView					*gParent = Parent()->Parent();
	CMultiColumnListView	*mcl = dynamic_cast<CMultiColumnListView *>(gParent);
	
	if (transit == B_EXITED_VIEW)
	{
		SetInsertIndex( -1 );
	}
	else
	{
		if (mcl->IsDragAcceptable( dragMsg ))
		{
			int32 index = IndexOf( point );
			
			if (index >= 0)
			{
				BRect	r( ItemFrame( index ) );
				
				if (point.y > (r.top + r.bottom) / 2) index++;
				
				SetInsertIndex( index );
			}
// 		else if (point.y >= 0) SetInsertIndex( CountItems() );
			else SetInsertIndex( -1 );
		}
		
		if (cursorHidden && !be_app->IsCursorHidden()) be_app->HideCursor();
	}
	
	BListView::MouseMoved( point, transit, dragMsg );
}

void CChildListView::MessageReceived( BMessage* theMessage )
{
	if (theMessage->WasDropped())
	{
		BView					*gParent = Parent()->Parent();
		CMultiColumnListView	*mcl = dynamic_cast<CMultiColumnListView *>(gParent);
		
		if (		mcl->IsDragAcceptable( theMessage )
			&&	dragInsertIndex >= 0)
			mcl->OnDrop( theMessage, dragInsertIndex );

		SetInsertIndex( -1 );
	}
}

void CChildListView::SetInsertIndex( int32 index )
{
	if (index != dragInsertIndex)
	{
		if (dragInsertIndex >= 0)
		{
			Invalidate( GetInsertFrameRect( dragInsertIndex ) );
		}

		dragInsertIndex = index;

		if (dragInsertIndex >= 0)
		{
			if (!cursorHidden)
			{
				cursorHidden = true;
				if (!be_app->IsCursorHidden()) be_app->HideCursor();
			}
			Invalidate( GetInsertFrameRect( dragInsertIndex ) );
		}
		else
		{
			if (cursorHidden)
			{
				cursorHidden = false;
				if (be_app->IsCursorHidden()) be_app->ShowCursor();
			}
		}
	}
}

void CChildListView::Draw( BRect inUpdateRect )
{
	BListView::Draw( inUpdateRect );
	if (dragInsertIndex >= 0)
	{
		BRect	r( GetInsertFrameRect( dragInsertIndex ) );
		SetHighColor( 128, 128, 128 );
		FillRect( r );
	}
}

BRect CChildListView::GetInsertFrameRect( int32 index )
{
	if (index < CountItems())
	{
		BRect	r( ItemFrame( dragInsertIndex ) );
		
		r.top--;
		r.bottom = r.top + 1.0;
		return r;
	}
	else
	{
		BRect	r( ItemFrame( dragInsertIndex - 1 ) );
		
//		r.bottom;
		r.top = r.bottom - 1.0;
		return r;
	}
}

/*void CChildListView::MouseUp( BPoint where )
{
	BView					*gParent = Parent()->Parent();
	CMultiColumnListView		*mcl = cast_as( gParent, CMultiColumnListView );
	
	BView::MouseUp( where );

	if (mcl && mcl->clickColumn >= 0)
	{
		if (		mcl->PickColumn( where.x ) == mcl->clickColumn
			&&	IndexOf( where ) == mcl->clickRow
			&&	mcl->clickRow >= 0
			&&	mcl->clickRow < CountItems())
		{
			if (mcl->columnMsg)
			{
				BMessage		msg( *mcl->columnMsg );
				
				msg.AddInt32( "cell", mcl->clickRow );
				msg.AddInt32( "cell", mcl->clickColumn );
				Invoke( &msg );
			}
		}
	}
}
*/

CMultiColumnListView::CMultiColumnListView(
		BRect			frame,
		const char		*name,
		list_view_type	type,
		uint32			resizeMask,
		uint32			flags )
	: BView( frame, name, resizeMask, flags )
{
	BRect				r( frame );
	
	GetFontHeight( &fh );
	labelHeight = static_cast<int32>(fh.ascent + fh.descent + 4);
	labelBaseline = static_cast<int32>(fh.ascent + 2);
	
	r.OffsetTo( B_ORIGIN );
	
	r.top += labelHeight + 2;
	r.left += 1;
	r.right -= 1 + B_V_SCROLL_BAR_WIDTH;
	r.bottom -= 1;

	listView = new CChildListView(
		r,
		NULL,
		type,
		B_FOLLOW_ALL,
		B_WILL_DRAW | B_FRAME_EVENTS | (flags & B_FULL_UPDATE_ON_RESIZE) );

	scrollView = new BScrollView(	
		NULL,
		listView,
		B_FOLLOW_ALL_SIDES,
		B_FRAME_EVENTS,
		false, true, B_FANCY_BORDER );
	AddChild( scrollView );
	
	columnMsg = NULL;
	stayFocused = true;
}

CMultiColumnListView::~CMultiColumnListView()
{
	int32		count = columns.CountItems();
	
	for (int i = 0; i < count; i++)
	{
		CColumnField		*colField = (CColumnField *)columns.ItemAt( i );
		
		delete colField;
	}

	delete columnMsg;
}

void CMultiColumnListView::Draw( BRect inUpdateRect )
{
	int32		i, count = columns.CountItems();

	BRect		r( Bounds() );
	int32		y1 = static_cast<int32>(r.top) + labelHeight;

	BView::Draw( inUpdateRect );
	
	SetHighColor( 128, 128, 128 );
	StrokeRect( BRect( r.left, r.top, r.right, y1 + 1 ) );

	SetHighColor( 255, 255, 255 );
	FillRect( BRect( r.left + 1, r.top + 1, r.right - 2, r.top + 1 ) );	
	FillRect( BRect( r.left + 1, r.top + 1, r.left + 1, y1 - 1 ) );

	SetHighColor( 230, 230, 230 );
	FillRect( BRect( r.left + 2, r.top + 2, r.right - 3, r.top + 2 ) );	
	FillRect( BRect( r.left + 2, r.top + 2, r.left + 2, y1 - 2 ) );

	int			x = 0;
	BRect		bottomSpace( listView->Bounds() );
	
	if (listView->CountItems() > 0)
	{
		bottomSpace.top = listView->ItemFrame( listView->CountItems() - 1 ).bottom + 1;
	}

	for (i = 0; i < count; i++)
	{
		CColumnField	*colField = (CColumnField *)columns.ItemAt( i );
		int32			w = static_cast<int32>(StringWidth(colField->title));

		SetHighColor( 0, 0, 0 );
		
		DrawString(	colField->title,
					BPoint( x + 1 + (colField->actualWidth - w) / 2, r.top + fh.ascent + 2 ) );

		bottomSpace.left = x;
		x += colField->actualWidth;
		bottomSpace.right = x - 1;
		listView->SetHighColor( 128, 128, 128 );
		listView->FillRect( BRect( x, 0, x, r.bottom ) );
		x++;
		
		SetHighColor( 128, 128, 128 );
		FillRect( BRect( x, 0, x, y1 ) );
		SetHighColor( 255, 255, 255 );
		FillRect( BRect( x + 1, 1, x + 1, y1 ) );
		listView->SetHighColor( 255, 255, 255 );
		listView->FillRect( bottomSpace );
	}
}

void CMultiColumnListView::Layout()
{
	BRect		r( Bounds() );
	int32		i, count = columns.CountItems();
	int32		totalMinWidth = -1,
				totalElasticity = 0,
				fieldCount = 0,
				extraSpace;

	r.OffsetTo( B_ORIGIN );
	
	for (i = 0; i < count; i++)
	{
		CColumnField		*colField = (CColumnField *)columns.ItemAt( i );

		totalMinWidth += colField->minWidth + 1;
		totalElasticity += colField->elasticity;
		colField->fieldIndex = fieldCount;
		fieldCount++;
	}

	extraSpace = static_cast<int32>(r.Width() - 1 - totalMinWidth - B_V_SCROLL_BAR_WIDTH);

	for (i = 0; i < count; i++)
	{
		int32			space;
	
		CColumnField		*colField = (CColumnField *)columns.ItemAt( i );

		if (colField->elasticity > 0 && totalElasticity > 0)
		{
			space = extraSpace * colField->elasticity / totalElasticity;
			totalElasticity -= colField->elasticity;
		}
		else space = 0;
		
		colField->actualWidth = colField->minWidth + space;
		extraSpace -= space;
	}
	
	BScrollBar		*scroller = scrollView->ScrollBar( B_VERTICAL );
	BRect			lr( listView->ItemFrame( listView->CountItems() - 1 ) );
	float			totalHeight = lr.bottom,
					visibleHeight = scrollView->Frame().Height() - 4.0;
	if (totalHeight > visibleHeight) scroller->SetProportion( visibleHeight / totalHeight );
	else scroller->SetProportion( 1.0 );
	scroller->SetRange( 0.0, totalHeight - visibleHeight );
}

int32 CMultiColumnListView::PickColumn( int32 inX )
{
	int32		i, count = columns.CountItems();
	
	if (inX < 0) return -1;

	for (i = 0; i < count; i++)
	{
		CColumnField		*colField = (CColumnField *)columns.ItemAt( i );

		inX -= colField->actualWidth + 1;
		if (inX < 0) return i;
	}
	
	return -1;
}

void CMultiColumnListItem::DrawItem(
	BView	*owner,
	BRect	bounds,
	bool		complete )
{
	BView					*parent = owner->Parent()->Parent();
	CMultiColumnListView	*mcl = dynamic_cast<CMultiColumnListView *>(parent);

	int32					count = mcl->columns.CountItems();
	
	if (IsSelected()) owner->SetLowColor( 128, 128, 255 );
	else owner->SetLowColor( 255, 255, 255 );
		
	owner->FillRect( bounds, B_SOLID_LOW );

	for (int i = 0; i < count; i++)
	{
		CColumnField		*colField = (CColumnField *)mcl->columns.ItemAt( i );

		bounds.right = bounds.left + colField->actualWidth - 1;

		owner->SetHighColor( 0, 0, 0 );
		colField->Draw( this, owner, bounds, false );
		bounds.left = bounds.right + 2;
		owner->SetHighColor( 128, 128, 128 );
		owner->FillRect( BRect( bounds.right + 1, bounds.top, bounds.right + 1, bounds.bottom ) );
	}
}

CColumnField::CColumnField(	CMultiColumnListView	&inList,
							int32				inMinWidth,
							int32				inElasticity,
							char					*inTitle )
	: list( inList )
{
	title		= inTitle;
	minWidth		= inMinWidth;
	elasticity	= inElasticity;
	actualWidth	= 0;
	fieldIndex	= 0;
	useEllipses	= false;
	align		= B_ALIGN_CENTER;
	selectable	= true;
	useHighlight	= true;
	draggable	= false;
	
	inList.columns.AddItem( this );
}

int32 CColumnField::Justify( int32 totalWidth, int32 contentWidth )
{
	switch (align) {
	case B_ALIGN_LEFT:	return 3;
	case B_ALIGN_CENTER:	return (totalWidth - contentWidth) / 2;
	case B_ALIGN_RIGHT:	return totalWidth - contentWidth - 3;
	}
	return 0;
}

void CColumnField::DrawString( BView *drawView, BRect bounds, char *str )
{
	if (str != NULL)
	{
		int32			w = static_cast<int32>(drawView->StringWidth(str));
		int32			x = static_cast<int32>(bounds.left + Justify( bounds.Width(), w ));
//		int32			y = static_cast<int32>(bounds.top + bounds.bottom - list.fh.descent + list.fh.ascent) / 2;

		drawView->DrawString( str, BPoint( x, bounds.top + 10 ) );
	}
}

void CNumericColumnField::Draw(
	CMultiColumnListItem	*item,
	BView				*drawView,
	BRect 				bounds,
	bool					complete )
{
	int32				v = item->GetFieldIntData( fieldIndex );
	char					text[ 32 ];

	sprintf( text, "%ld", v );
	DrawString( drawView, bounds, text );
}

void CStringColumnField::Draw(
	CMultiColumnListItem	*item,
	BView				*drawView,
	BRect 				bounds,
	bool					complete )
{
	DrawString( drawView, bounds, item->GetFieldStringData( fieldIndex ) );
}

void CCheckmarkColumnField::Draw(
	CMultiColumnListItem	*item,
	BView				*drawView,
	BRect 				bounds,
	bool					complete )
{
	int32				val = item->GetFieldIntData( fieldIndex );

	if (val == 1)		DrawString( drawView, bounds, "\xE2\x88\x9A" );
	else if (val == 2)	DrawString( drawView, bounds, "-" );
}

void CColorSwatchColumnField::Draw(
	CMultiColumnListItem	*item,
	BView				*drawView,
	BRect 				bounds,
	bool					complete )
{
	rgb_color			*rgb = (rgb_color *)item->GetFieldData( fieldIndex );
	
	drawView->SetHighColor( 0, 0, 0 );
	bounds.InsetBy( 2.0, 2.0 );
	drawView->FillRect( bounds );
	drawView->SetHighColor( *rgb );
	bounds.InsetBy( 1.0, 1.0 );
	drawView->FillRect( bounds );
}
