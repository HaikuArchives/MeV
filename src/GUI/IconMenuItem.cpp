/* ===================================================================== *
 * IconMenuItem.cpp (MeV/User Interface)
 * ===================================================================== */

#include "IconMenuItem.h"

// Interface Kit
#include <Bitmap.h>
// Support Kit
#include <Debug.h>

// Debugging Macros
#define D_ALLOC(x) //PRINT (x)		// Constructor/Destructor
#define D_HOOK(x) //PRINT (x)		// BMenuItem Implementation

// ---------------------------------------------------------------------------
// Constructor/Destructor

CIconMenuItem::CIconMenuItem(
	const char *label,
	BMessage *message,
	BBitmap *bitmap,
	char shortcut,
	uint32 modifiers)
	:	BMenuItem(label, message, shortcut, modifiers),
		m_bitmap(bitmap)
{
	D_ALLOC(("CIconMenuItem::CIconMenuItem(%s)\n", label));
}

CIconMenuItem::~CIconMenuItem()
{
	D_ALLOC(("CIconMenuItem::~CIconMenuItem()\n"));

	if (m_bitmap)
	{
		delete m_bitmap;
		m_bitmap = NULL;
	}
}

// ---------------------------------------------------------------------------
// BMenuItem Implementation

void
CIconMenuItem::DrawContent()
{
	D_HOOK(("CIconMenuItem::DrawContent()\n"));

	BPoint drawPoint(ContentLocation());
	drawPoint.x += 20;
	Menu()->MovePenTo(drawPoint);
	BMenuItem::DrawContent();

	BPoint where(ContentLocation());
	where.y = Frame().top;
	
	if (m_bitmap) {
		if (IsEnabled())
			Menu()->SetDrawingMode(B_OP_OVER);
		else
			Menu()->SetDrawingMode(B_OP_BLEND);	
		
		Menu()->DrawBitmapAsync(m_bitmap, where);
	}

}

void
CIconMenuItem::GetContentSize(
	float *width,
	float *height)
{
	D_HOOK(("CIconMenuItem::DrawContent()\n"));

	BMenuItem::GetContentSize(width, height);

	*width += 20.0;
	*height += 3.0;
}

// END - IconMenuItem.cpp
