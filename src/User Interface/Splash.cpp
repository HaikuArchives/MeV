/* ===================================================================== *
 * Splash.cpp (MeV/User Interface)
 * ===================================================================== */

#include "ScreenUtils.h"
#include "Splash.h"
#include "AppHelp.h"

// Gnu C Library
#include "stdarg.h"
// Interface Kit
#include <Bitmap.h>
#include <Box.h>
#include <StringView.h>
#include <TextView.h>
// Support Kit
#include <Debug.h>

// Debugging Macros
#define D_ALLOC(x) PRINT(x)			// Constructor/Destructor
#define D_HOOK(x) PRINT(x)			// BView Implementation

CSplashWindow		*CSplashWindow::window;
BStringView			*CSplashWindow::message;

class CBitmapView :
	public BView {

	public:						// Constructor/Destructor

		// Constructor for popup menu button. Args are the same as for a BView
								CBitmapView(
									BRect frame,
									const char *name,
									BBitmap *bitmap,
									uint32 resizingMode = B_FOLLOW_LEFT | B_FOLLOW_TOP,
									uint32 flags = B_WILL_DRAW | B_NAVIGABLE)
								: BView(frame, name, resizingMode, flags),
								  m_bitmap(bitmap)
								{ }

	public:						// BView Implementation

		virtual void			Draw(
									BRect updateRect)
								{ if (m_bitmap) DrawBitmap(m_bitmap, BPoint(0, 0)); }
								

	private:					// Instance Data

		BBitmap	*				m_bitmap;
};

CSplashWindow::CSplashWindow()
	: BWindow(UScreenUtils::CenterOnScreen( 372 + 8, 200 ),
			  "MeV Splash Window",
			  B_BORDERED_WINDOW, 
			  B_NOT_RESIZABLE | B_NOT_ZOOMABLE | B_NOT_MINIMIZABLE) {
	D_ALLOC(("CSplashWindow::CSplashWindow()\n"));

	BTextView	*tv;
	CBitmapView	*bv;
	BBox		*bb;
	
	BRect r = Bounds();

	bb = new BBox(r, "", B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW, B_PLAIN_BORDER);
	AddChild(bb);

	r.InsetBy(4.0, 4.0);

	splash = LoadBitmap( 'BMAP', 400 );

	/**	Constructor for popup menu button. Args are the same as for a BView */
	if (splash) {
		bv = new CBitmapView(splash->Bounds(), "", splash);
		bb->AddChild(bv);
	}

	tv = new BTextView(	BRect( r.left, r.top + 147, r.right, r.bottom - 20 ),
						NULL,	
						BRect( 0, 0, r.Width(), r.Height() ),
						B_FOLLOW_ALL, B_WILL_DRAW );
	tv->MakeSelectable( false );
	tv->MakeEditable( false );
	tv->SetViewColor( 220, 220, 220 );
	tv->SetText( "© 1998 Sylvan Technical Arts. Written by Talin.\nSee about box for additional credits." );
	bb->AddChild( tv );

	message = new BStringView( BRect( r.left, r.bottom - 16, r.right, r.bottom ), NULL, "" );
	bb->AddChild( message );
	
	window = this;
	Show();
}

CSplashWindow::~CSplashWindow()
{
	delete splash;
}

void CSplashWindow::DisplayStatus( char *format, ... )
{
	char			buf[ 256 ];
	va_list		argptr;
	
	va_start( argptr, format );
	vsprintf( buf, format, argptr );
	va_end( argptr );
	
	if (window == NULL) window = new CSplashWindow();
	window->Lock();
	message->SetText( buf );
	window->Unlock();
}

void CSplashWindow::HideSplash()
{
	if (window)
	{
		window->PostMessage( B_QUIT_REQUESTED );
		window = NULL;
	}
}
