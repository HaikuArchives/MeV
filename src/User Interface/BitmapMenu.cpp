/* ===================================================================== *
 * BitmapMenu.cpp (MeV/User Interface)
 * ===================================================================== */

#include "BitmapMenu.h"

BMenuItem *CPopUpMatrixMenu::Go(
	BPoint screenPoint,
	bool deliversMessage,
	bool openAnyway,
	BRect *clickToOpenRect )
{
	BMenuItem	*selected;

	Show();
	selected = Track( openAnyway, clickToOpenRect );

	if (selected)
	{
		if (IsRadioMode())
		{
			BMenuItem	*prevSel = FindMarked();
			
			if (selected != prevSel)
			{
				if (prevSel) prevSel->SetMarked( false );
				selected->SetMarked( true );
			}
		}
	
		if (deliversMessage)
		{
			BLooper		*looper;
			BHandler		*target = selected->Target( &looper );
				
			looper->PostMessage( selected->Message(), target );
		}
	}
	Hide();
	
	return selected;
}

void CPopUpMatrixMenu::SetPosition( BRect inScreenRect )
{
	BMenuItem	*selected = FindMarked();

	if (selected == NULL) selected = ItemAt( 0 );
	
	SetItemMargins( 0, 0, 0, 0 );
	
	if (selected)
	{
		BRect		r( selected->Frame() );
		screenPos.x = (inScreenRect.left + inScreenRect.right - r.left - r.right) / 2 - 1.0;
		screenPos.y = (inScreenRect.top + inScreenRect.bottom - r.top - r.bottom) / 2 - 1.0;
	}
	else screenPos = inScreenRect.LeftTop();
}

void CBitmapMenuButton::MouseDown( BPoint point )
{
	if (menu != NULL)
	{
		BMenuItem	*selected;
		BRect		r( Bounds() );
		
		ConvertToScreen( &r );
		ConvertToScreen( &point );
		menu->SetPosition( r );
		selected = menu->Go( point, false, true, NULL );

		if (selected)
		{
			BLooper		*looper;
			BHandler		*target = selected->Target( &looper );
			
			looper->PostMessage( selected->Message(), target );

			SelectItem( selected );
		}
	}
}

void CBitmapMenuButton::SelectItem( BMenuItem *inItem )
{
	CBitmapMenuItem	*bmItem;

	if ((bmItem = dynamic_cast<CBitmapMenuItem *>(inItem)))
	{
		if (bitmap != bmItem->Bitmap())
		{
			bitmap = bmItem->Bitmap();
			Invalidate();
		}
	}

	if (menu && menu->IsRadioMode())
	{
		BMenuItem	*prevSel = menu->FindMarked();
		
		if (inItem != prevSel)
		{
			if (prevSel) prevSel->SetMarked( false );
			inItem->SetMarked( true );
		}
	}
}

void CBitmapMenuButton::Draw( BRect updateRect )
{
	BRect		r( Bounds() );

	SetHighColor( 145, 145, 145 );
	FillRect( BRect( r.left, r.top, r.right - 1, r.top ) );
	FillRect( BRect( r.left, r.top, r.left, r.bottom - 1 ) );
	FillRect( BRect( r.right, r.top + 2, r.right, r.bottom ) );
	FillRect( BRect( r.left + 2, r.bottom, r.right, r.bottom ) );

	SetHighColor( 82, 82, 82 );
	FillRect( BRect( r.left, r.bottom - 1, r.right - 1, r.bottom - 1 ) );
	FillRect( BRect( r.right - 1, r.top, r.right - 1, r.bottom - 1 ) );

	SetHighColor( 200, 200, 200 );
	FillRect( BRect( r.left + 1, r.bottom - 2, r.right - 2, r.bottom - 2 ) );
	FillRect( BRect( r.right - 2, r.top + 1, r.right - 2, r.bottom - 2 ) );

	SetHighColor( 240, 240, 240 );
	FillRect( BRect( r.left + 1, r.top + 1, r.right - 3, r.top + 1 ) );
	FillRect( BRect( r.left + 1, r.top + 1, r.left + 1, r.bottom - 3 ) );

	if (bitmap)
	{
		BPoint		p;

		p.x = (r.left + r.right - bitmap->Bounds().right) / 2;
		p.y = (r.top + r.bottom - bitmap->Bounds().bottom) / 2;

		SetDrawingMode( B_OP_OVER );
		DrawBitmap( bitmap, p );
		SetDrawingMode( B_OP_COPY );
	}
}

void CBitmapMenuItem::Draw()
{
	BRect		r( Frame() );
	BPoint		p, s;

	r.right--;
	r.bottom--;
	
	GetContentSize( &s.x, &s.y );

	p.x = (r.left + r.right - s.x) / 2;
	p.y = (r.top + r.bottom - s.y) / 2;

	if (IsSelected())	Menu()->SetHighColor( 148, 148, 148 );
	else					Menu()->SetHighColor( 220, 220, 220 );
	Menu()->FillRect( r );

	Menu()->SetDrawingMode( B_OP_OVER );
	Menu()->DrawBitmap( bm, p );
	Menu()->SetDrawingMode( B_OP_COPY );
	
	if (IsMarked())
	{
		Menu()->SetHighColor( 0, 0, 255 );
		Menu()->StrokeRect( r );
	}
}
