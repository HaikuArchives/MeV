/* ===================================================================== *
 * RulerView.cpp (MeV/User Interface)
 * ===================================================================== */

#include "RulerView.h"
#include "TrackEditFrame.h"

// Support Kit
#include <Debug.h>

// Debugging Macros
#define D_ALLOC(x) //PRINT (x)		// Constructor/Destructor
#define D_HOOK(x) //PRINT (x)		// BControl Implementation

// ---------------------------------------------------------------------------
// Constructor/Destructor

CRulerView::CRulerView(
	BRect frame,
	const char *name,
	CStripFrameView *frameView,
	uint32 resizingMode = B_FOLLOW_LEFT | B_FOLLOW_TOP,
	uint32 flags = B_WILL_DRAW)
	:	CScrollerTarget(frame, name, resizingMode, flags),
		m_frameView(frameView)
{
	m_frameView->SetRuler(this);
}

// ---------------------------------------------------------------------------
// Operations

void
CRulerView::SetScrollValue(
	float value,
	orientation which)
{
	CScrollerTarget::SetScrollValue(value, which);
	ScrollTo(scrollValue.x, scrollValue.y);
}

// END - RulerView.cpp
