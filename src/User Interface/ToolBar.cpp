/* ===================================================================== *
 * ToolBar.cpp (MeV/User Interface)
 * ===================================================================== */

#include "ToolBar.h"

// Interface Kit
#include <Bitmap.h>
#include <Picture.h>
#include <PopUpMenu.h>
#include <Window.h>
// Support Kit
#include <Debug.h>

	// Constructor
CToolBar::CToolBar(	BRect 		inFrame,
					BMessage		*inMessage,
					BPoint		inToolSize,
					bool			inVertical,
					uint32		inResizingMode,
					uint32		inFlags )
	: BControl( inFrame, NULL, NULL, inMessage, inResizingMode,
		inFlags | B_FRAME_EVENTS )
{
	lastToolPos	= BPoint( 0, 0 );
	vertical		= inVertical;
	toolSize		= inToolSize;
// SetViewColor( 205, 205, 205 );
	toolPict = selectPict = dimPict = NULL;
}

CToolBar::~CToolBar()
{
	for (int i = 0; i < toolList.CountItems(); i++)
	{
		CToolDef		*def = (CToolDef *)toolList.ItemAt( i );
		
		if (def && def->menu) delete def->menu;
		delete def;
	}
	delete toolPict;
	delete selectPict;
	delete dimPict;
}

void CToolBar::SelectTool( CToolDef *def, bool inNotify, int32 inNewState )
{
	int32		flags = def->flags;

	if (inNewState == 0)			flags &= ~Tool_Selected;
	else if (inNewState == 1)		flags |= Tool_Selected;
	else if (flags & Tool_Toggle)	flags ^= Tool_Selected;
	else							flags |= Tool_Selected;

	if (def->group >= 0 && (flags & Tool_Selected))
	{
		for (int i = 0; i < toolList.CountItems(); i++)
		{
			CToolDef		*td = (CToolDef *)toolList.ItemAt( i );

			if (		td->group == def->group
				&&	td->toolID != def->toolID
				&&	td->flags & Tool_Selected)
			{
				td->flags &= ~Tool_Selected;
				if (Window() && Window()->Lock())
				{
					DrawTool( td, ToolPos( td ) );
					Window()->Unlock();
				}
			}
		}
	}
	
	if (def->flags != flags)
	{
		def->flags = flags;
		if (Window() && Window()->Lock())
		{
			DrawTool( def, ToolPos( def ) );
			Window()->Unlock();
		
			if (inNotify)
			{
				BMessage		msg(*Message());
				
				msg.AddInt32( "tool",		def->toolID );
				msg.AddInt32( "state",	def->flags & Tool_Selected ? 1 : 0 );
				if (def->group >= 0)
					msg.AddInt32( "group", def->group );
					
				Invoke( &msg );
			}
		}
	}
}
	
CToolBar::CToolDef *CToolBar::FindTool( int32 inToolID )
{
	for (int i = 0; i < toolList.CountItems(); i++)
	{
		CToolDef		*def = (CToolDef *)toolList.ItemAt( i );
		
		if (def->toolID == inToolID) return def;
	}
	return NULL;
}

void CToolBar::DrawTool(	CToolDef *tool, const BPoint &p )
{
	BBitmap			*bm = NULL;

	if (!(tool->flags & Tool_Enabled))
	{
		DrawPicture( dimPict, p );
		SetDrawingMode( B_OP_BLEND );
		bm = tool->bitmap[ 0 ];
	}
	else if (tool->flags & Tool_Selected)
	{
		DrawPicture( selectPict, p );
		bm = tool->bitmap[ 1 ] ? tool->bitmap[ 1 ] : tool->bitmap[ 0 ];
		SetDrawingMode( B_OP_OVER );
	}
	else
	{
		DrawPicture( toolPict, p );
		bm = tool->bitmap[ 0 ];
		SetDrawingMode( B_OP_OVER );
	}
	
	if (bm != NULL)
	{
		BRect	br( bm->Bounds() );
		BRect	r( p.x, p.y, p.x + toolSize.x - 1, p.y + toolSize.y - 1 );

		DrawBitmapAsync( bm, BPoint(	(r.left + r.right - br.Width())/2,
									(r.top + r.bottom - br.Height())/2 ) );
	}

	SetDrawingMode( B_OP_COPY );
}

void CToolBar::AttachedToWindow()
{
	if (toolPict == NULL)
	{
			// Construct the unselected picture

		SetDrawingMode( B_OP_COPY );

		BeginPicture( new BPicture );

		SetHighColor( 108, 108, 108 );
		FillRect( BRect( 1.0, 0.0, toolSize.x - 2.0, 0.0 ) );
		FillRect( BRect( 0.0, 1.0, 0.0, toolSize.y - 2.0 ) );
		FillRect( BRect( 1.0, toolSize.y - 1.0, toolSize.x - 2.0, toolSize.y - 1.0 ) );
		FillRect( BRect( toolSize.x - 1.0, 1.0, toolSize.x - 1.0, toolSize.y - 2.0 ) );

// 	SetHighColor( 235, 235, 235 );
		SetHighColor( 225, 225, 225 );
		FillRect( BRect( 1.0, 1.0, toolSize.x - 2.0, toolSize.y - 2.0 ) );
		SetHighColor( 255, 255, 255 );
		FillRect( BRect( 1.0, 1.0, toolSize.x - 2.0, 1.0 ) );
		FillRect( BRect( 1.0, 1.0, 1.0, toolSize.y - 2.0 ) );
		SetHighColor( 200, 200, 200 );
		FillRect( BRect( 1.0, toolSize.y - 2.0, toolSize.x - 2.0, toolSize.y - 2.0 ) );
		FillRect( BRect( toolSize.x - 2.0, 1.0, toolSize.x - 2.0, toolSize.y - 2.0 ) );
	
		toolPict = EndPicture();

			// Construct the selected picture
	
		BeginPicture( new BPicture );

		SetHighColor( 108, 108, 108 );
		FillRect( BRect( 1.0, 0.0, toolSize.x - 2.0, 0.0 ) );
		FillRect( BRect( 0.0, 1.0, 0.0, toolSize.y - 2.0 ) );
		FillRect( BRect( 1.0, toolSize.y - 1.0, toolSize.x - 2.0, toolSize.y - 1.0 ) );
		FillRect( BRect( toolSize.x - 1.0, 1.0, toolSize.x - 1.0, toolSize.y - 2.0 ) );
	
		SetHighColor( 200, 200, 200 );
		FillRect( BRect( 1.0, 1.0, toolSize.x - 2.0, toolSize.y - 2.0 ) );
		SetHighColor( 175, 175, 175 );
		FillRect( BRect( 1.0, 1.0, toolSize.x - 2.0, 1.0 ) );
		FillRect( BRect( 1.0, 1.0, 1.0, toolSize.y - 2.0 ) );
	
		selectPict = EndPicture();

		BeginPicture( new BPicture );

		SetDrawingMode( B_OP_BLEND );
		SetHighColor( 108, 108, 108 );
		FillRect( BRect( 1.0, 0.0, toolSize.x - 2.0, 0.0 ) );
		FillRect( BRect( 0.0, 1.0, 0.0, toolSize.y - 2.0 ) );
		FillRect( BRect( 1.0, toolSize.y - 1.0, toolSize.x - 2.0, toolSize.y - 1.0 ) );
		FillRect( BRect( toolSize.x - 1.0, 1.0, toolSize.x - 1.0, toolSize.y - 2.0 ) );

		SetHighColor( 235, 235, 235 );
		FillRect( BRect( 1.0, 1.0, toolSize.x - 2.0, toolSize.y - 2.0 ) );
		SetHighColor( 255, 255, 255 );
		FillRect( BRect( 1.0, 1.0, toolSize.x - 2.0, 1.0 ) );
		FillRect( BRect( 1.0, 1.0, 1.0, toolSize.y - 2.0 ) );
		SetHighColor( 200, 200, 200 );
		FillRect( BRect( 1.0, toolSize.y - 2.0, toolSize.x - 2.0, toolSize.y - 2.0 ) );
		FillRect( BRect( toolSize.x - 2.0, 1.0, toolSize.x - 2.0, toolSize.y - 2.0 ) );
	
		dimPict = EndPicture();

		SetDrawingMode( B_OP_COPY );
	}
}

void CToolBar::Draw( BRect r )
{
	BRect		frame( Frame() );
	BPoint		next( 0.0, 0.0 );
	
	ASSERT( toolPict != NULL );
	
	frame.OffsetTo( B_ORIGIN );

	SetHighColor( 220, 220, 220 );
	FillRect( r );

	for (int i = 0; i < toolList.CountItems(); i++)
	{
		CToolDef		*def = (CToolDef *)toolList.ItemAt( i );
		BPoint		p = NextToolPos( def, next );

		if (		!(def->flags & Tool_Seperator)
			&&	p.x < r.right && p.x + toolSize.x >= r.left
			&&	p.y < r.bottom && p.y + toolSize.y >= r.top )
		{
			DrawTool( def, p );
		}
		lastToolPos = p;
	}
}

	// Overrider keydown to deal with navigation.
void CToolBar::KeyDown( const char*text, long count )
{
	BControl::KeyDown( text, count );
}
	
BPoint CToolBar::NextToolPos( CToolDef *def, BPoint &outNext )
{
	BPoint		result( outNext );
	
	if (def->flags & Tool_Seperator)
	{
		if (vertical)	result.y += toolSize.y / 4.0;
		else				result.x += toolSize.x / 4.0;
		
		outNext = result;
	}
	else
	{
		if (vertical)
		{
			if (result.y + toolSize.y >= Frame().Height())
			{
				result.y = 0.0;
				result.x += toolSize.x - 1;
			}
		}
		else
		{
			if (result.x + toolSize.x >= Frame().Width())
			{
				result.x = 0.0;
				result.y += toolSize.y - 1;
			}
		}
		
		outNext = result;
		
		if (vertical)		outNext.y += toolSize.y - 1;
		else					outNext.x += toolSize.x - 1;
	}
	return result;
}

CToolBar::CToolDef *CToolBar::PickTool( BPoint &inMouse, BPoint &outPos )
{
	BRect		frame( Frame() );
	BPoint		next( 0.0, 0.0 );
	
	frame.OffsetTo( B_ORIGIN );

	for (int i = 0; i < toolList.CountItems(); i++)
	{
		CToolDef		*def = (CToolDef *)toolList.ItemAt( i );
		BPoint		p = NextToolPos( def, next );

		if (		!(def->flags & Tool_Seperator)
			&&	(def->flags & Tool_Enabled)
			&&	inMouse.x >= p.x && inMouse.x < p.x + toolSize.x
			&&	inMouse.y >= p.y && inMouse.y < p.y + toolSize.y)
		{
			outPos = inMouse - p;
			return def;
		}
	}
	return NULL;
}
	
BPoint CToolBar::ToolPos( CToolDef *inDef )
{
	BRect		frame( Frame() );
	BPoint		next( 0.0, 0.0 );
	
	frame.OffsetTo( B_ORIGIN );

	for (int i = 0; i < toolList.CountItems(); i++)
	{
		CToolDef		*def = (CToolDef *)toolList.ItemAt( i );
		BPoint		p = NextToolPos( def, next );
		
		if (def == inDef) return p;
	}
	return next;
}
	
		/**	Add a tool to the array. (Doesn't redraw) */
void CToolBar::AddTool(
	int32	inToolID,
	bool		inToggle,
	BBitmap	*img,
	BBitmap	*selImg,
	BBitmap	*altImg,
	BPopUpMenu *menu )
{
	CToolDef		*def = new CToolDef;
	
	def->flags = Tool_Enabled;
	if (inToggle) def->flags |= Tool_Toggle;
	
	def->group = -1;
	def->toolID = inToolID;
	def->bitmap[ 0 ] = img;
	def->bitmap[ 1 ] = selImg;
	def->bitmap[ 2 ] = altImg;
	def->menu = menu;

	toolList.AddItem( def );
}

	// Add a seperator
void CToolBar::AddSeperator(	int32 inToolID )
{
	CToolDef		*def = new CToolDef;
	
	def->flags = Tool_Seperator;
	
	def->group = -1;
	def->toolID = inToolID;
	def->bitmap[ 0 ] = NULL;
	def->bitmap[ 1 ] = NULL;
	def->bitmap[ 2 ] = NULL;
	def->menu = NULL;

	toolList.AddItem( def );
}
	
	// Add a specific tool to an exclusion group.
void CToolBar::ExcludeTool( int32 inToolID, int32 inGroupID )
{
	CToolDef		*def = FindTool( inToolID );
	
	if (def != NULL)
	{
		def->group = inGroupID;
	}
}

	// Delete a tool
void CToolBar::RemoveTool( int32 inToolID )
{
	CToolDef		*def = FindTool( inToolID );
	
	if (def != NULL)
	{	
		toolList.RemoveItem( def );
		if (def->menu) delete def->menu;
		delete def;
	}
}

void CToolBar::MouseDown( BPoint point )
{
	CToolDef		*def;
	uint32		buttons;
	BPoint		pos;

	GetMouse( &point, &buttons, TRUE );

	if ((def = PickTool( point, pos )))
	{
		SelectTool( def );
		
		if (def->menu)
		{
			Window()->UpdateIfNeeded();
			snooze(200 * 1000);
			GetMouse( &point, &buttons, TRUE );
			if (buttons != 0)
			{
				BPoint	pos = ToolPos( def );

				pos.y += toolSize.y;
				ConvertToScreen( &pos );
				def->menu->Go( pos, true, false, true );
			}
		}
	}
}

void CToolBar::Select( int32 toolID, bool selected, bool inNotify )
{
	CToolDef		*def = FindTool( toolID );
	
	if (		def != NULL
		&&	!(def->flags & Tool_Seperator)
		&&	(def->flags & Tool_Enabled))
	{
		SelectTool( def, inNotify, selected ? 1 : 0 );
	}
}

void CToolBar::EnableTool( int32 toolID, bool inEnabled )
{
	CToolDef		*def = FindTool( toolID );
	
	if (	def != NULL)
	{
		int32	newFlags = def->flags;
		
		if (inEnabled) newFlags |= Tool_Enabled;
		else newFlags &= ~Tool_Enabled;
		
		if (def->flags != newFlags)
		{
			def->flags = newFlags;
			BPoint	p = ToolPos( def );
			
			if (Window())
			{
				Window()->Lock();
				Invalidate( BRect( p.x, p.y, p.x + toolSize.x, p.y + toolSize.y ) );
				Window()->Unlock();
			}
		}
	}
}

void CToolBar::SetToolImage(
	int32 inToolID,
	BBitmap	*img,	// unselected image
	BBitmap	*selImg,	// selected image
	BBitmap	*altImg )	// alternate image
{
	CToolDef		*def = FindTool( inToolID );
	
	if (	def != NULL)
	{
		def->bitmap[ 0 ] = img;
		def->bitmap[ 1 ] = selImg;
		def->bitmap[ 2 ] = altImg;

		BPoint	p = ToolPos( def );
			
		if (Window())
		{
			Window()->Lock();
			Invalidate( BRect( p.x, p.y, p.x + toolSize.x, p.y + toolSize.y ) );
			Window()->Unlock();
		}
	}
}

void CToolBar::FrameResized( float width, float height )
{
	BRect		frame( Frame() );
	BPoint		next( 0.0, 0.0 );
	BPoint		newLastPos;
	
	frame.OffsetTo( B_ORIGIN );

	for (int i = 0; i < toolList.CountItems(); i++)
	{
		CToolDef		*def = (CToolDef *)toolList.ItemAt( i );

		newLastPos = NextToolPos( def, next );
	}
	
	if (newLastPos != lastToolPos) Invalidate();
}

bool CToolBar::IsSelected( int32 toolID )
{
	CToolDef		*def = FindTool( toolID );
	
	if (	def != NULL) return (def->flags & Tool_Selected) ? true : false;
	return false;
}
