/* ===================================================================== *
 * ScreenUtils.cpp (MeV/Framework)
 * ===================================================================== */

#include "ScreenUtils.h"

// Interface Kit
#include <Screen.h>

// ---------------------------------------------------------------------------
// Constants Initialization

const BPoint
UScreenUtils::DEFAULT_WINDOW_POSITION(20.0, 20.0);

BPoint
UScreenUtils::NEXT_WINDOW_POSITION(20.0, 20.0);

const BPoint
UScreenUtils::DEFAULT_WINDOW_OFFSET(17.0, 17.0);

// ---------------------------------------------------------------------------
// Operations

BRect
UScreenUtils::CenterOnScreen(
	int32 width,
	int32 height,
	screen_id id)
{
	BRect screenRect = BScreen(id).Frame();
	BRect windowRect(0.0, 0.0, width, height);

	windowRect.OffsetTo(floor((screenRect.Width() - width) / 2.0),
						floor((screenRect.Height() - height) / 2.0));
	return windowRect;
}

BRect
UScreenUtils::StackOnScreen(
	int32 width,
	int32 height,
	screen_id id)
{
	BRect screenRect = BScreen(id).Frame();
	BRect windowRect(0.0, 0.0, width, height);

	if ((width > screenRect.Width()) || (height > screenRect.Height()))
	{
		windowRect.OffsetTo(20.0, 20.0);
	}
	else
	{
		NEXT_WINDOW_POSITION += BPoint(DEFAULT_WINDOW_OFFSET);
		windowRect.OffsetTo(NEXT_WINDOW_POSITION);
		if (!screenRect.Contains(windowRect))
		{
			NEXT_WINDOW_POSITION = DEFAULT_WINDOW_POSITION;
			windowRect.OffsetTo(NEXT_WINDOW_POSITION);
		}
	}
	
	return windowRect;
}

BRect
UScreenUtils::ConstrainToScreen(
	BRect frame,
	screen_id id)
{
	BRect screenRect = BScreen(id).Frame();

	BPoint p = frame.LeftTop();
	p.x = (p.x < screenRect.left)
		  ? screenRect.left
		  : (p.x > (screenRect.right - frame.Width()))
		  	? screenRect.right - frame.Width()
		  	: p.y = p.y < screenRect.top 
		  	  ? screenRect.top
		  	  : (p.y > (screenRect.bottom - frame.Height()))
		  	  	? screenRect.bottom - frame.Height()
		  	  	: p.y;
	frame.OffsetTo(p);

	return frame;
}

BRect
UScreenUtils::CenterOnWindow(
	int32 width,
	int32 height,
	BWindow *parent)
{
	BRect parentRect(parent->Frame());
	BPoint p((parentRect.left + parentRect.right - width) / 2.0,
			 (parentRect.top + parentRect.bottom - height) / 2.0);
	BRect screenRect = BScreen(parent).Frame();

	p.x = p.x < screenRect.left ? screenRect.left : 
								  p.x > screenRect.right - width ? screenRect.right - width :
	p.y = p.y < screenRect.top ? screenRect.top :
								 p.y > screenRect.bottom - height ? screenRect.bottom - height :
																	p.y;
	return BRect(p.x, p.y, p.x + width, p.y + height);
}

// END - ScreenUtils.cpp
