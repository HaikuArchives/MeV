/* ===================================================================== *
 * EventRenderer.cpp (MeV/UI)
 * ===================================================================== */

#include "EventRenderer.h"

#include "CursorCache.h"
#include "DataSnap.h"
#include "EventEditor.h"
#include "EventTrack.h"
#include "StdEventOps.h"
#include "TrackWindow.h"

// Support Kit
#include <Debug.h>

// Debugging Macros
#define D_ALLOC(x) //PRINT(x)		// Constructor/Destructor
#define D_HOOK(x) //PRINT(x)		// CStripView Implementation
#define D_INTERNAL(x) //PRINT(x)	// Internal Operations

// ---------------------------------------------------------------------------
// Constructor/Destructor

CEventRenderer::CEventRenderer(
	CEventEditor * const editor)
	:	m_editor(editor)
{
}

// ---------------------------------------------------------------------------
// Hook Functions

EventOp *
CEventRenderer::CreateDragOp(
	const CEvent &ev,
	short partCode,
	long timeDelta,
	long valueDelta) const
{
	return NULL;
}

EventOp *
CEventRenderer::CreateTimeOp(
	const CEvent	&ev,
	short partCode,
	long timeDelta,
	long valueDelta) const
{
	long limit;

	if (Editor()->Track()->GridSnapEnabled())
		limit = - DataSnapLower(Editor()->Track()->MinSelectTime(), 0,
								Editor()->Track()->TimeGridSize());
	else
		limit = - Editor()->Track()->MinSelectTime();

	timeDelta = MAX(timeDelta, limit);

	return new TimeOffsetOp(timeDelta);
}

const BCursor *
CEventRenderer::Cursor(
	short partCode,
	int32 editMode,
	bool dragging) const
{
	if ((editMode == CEventEditor::TOOL_SELECT)
	 || (editMode == CEventEditor::TOOL_CREATE))
	{
		if (dragging)
			return CCursorCache::GetCursor(CCursorCache::DRAGGING);
		return CCursorCache::GetCursor(CCursorCache::DRAGGABLE);
	}
	else
	{
		return Editor()->CursorFor(editMode);
	}
}

BRect
CEventRenderer::Extent(
	const CEvent &ev) const
{
	return BRect(0.0, 0.0, -1.0, -1.0);
}

long
CEventRenderer::Pick(
	const CEvent &ev,
	BPoint point,
	short &partCode) const
{
	return -1;
}

long
CEventRenderer::QuantizeDragTime(
	const CEvent &ev,
	short partCode,
	BPoint clickPos,
	BPoint dragPos,
	bool initial) const
{
	long timeDelta = Editor()->ViewCoordsToTime(dragPos.x - clickPos.x);

	// If no grid snap, then return just timeDelta
	if (!Editor()->Track()->GridSnapEnabled())
		return timeDelta;

	long t1 = Editor()->SnapToGrid(ev.Start(), initial);
	long t2 = Editor()->SnapToGrid(ev.Start() + timeDelta, initial);
	return t2 - t1;
}

long
CEventRenderer::QuantizeDragValue(
	const CEvent &clickedEvent,
	short partCode,
	BPoint clickPos,
	BPoint dragPos) const
{
	return (long)(clickPos.y - dragPos.y);
}

// ---------------------------------------------------------------------------
// Accessors

CMeVDoc * const
CEventRenderer::Document() const
{
	return m_editor->TrackWindow()->Document();
}

CEventEditor * const
CEventRenderer::Editor() const
{
	return m_editor;
}

// ---------------------------------------------------------------------------
// CEndEventRenderer

void
CEndEventRenderer::Invalidate(
	const CEvent &ev) const
{
	Editor()->Invalidate(Extent(ev));
}

static const pattern endPt = { { 0, 0, 0xff, 0xff, 0, 0, 0xff, 0xff } };

void
CEndEventRenderer::Draw(
	const CEvent	&ev,
	bool shadowed) const
{
	BRect r(Editor()->Bounds());

	r.left = Editor()->TimeToViewCoords(ev.Start());
	r.right	= r.left + 1.0;

	Editor()->SetDrawingMode(B_OP_OVER);

	if (shadowed)
	{
		Editor()->SetHighColor(0, 0, 0);
		Editor()->SetDrawingMode(B_OP_BLEND);
	}
	else if (ev.IsSelected() && Editor()->IsSelectionVisible())
	{
		Editor()->SetHighColor(128, 0, 128);
	}
	else
	{
		Editor()->SetHighColor(0, 0, 0);
	}

	Editor()->SetLowColor(B_TRANSPARENT_32_BIT);
	Editor()->FillRect(r, endPt);
}

BRect
CEndEventRenderer::Extent(
	const CEvent &ev) const
{
	BRect r(Editor()->Bounds());

	r.left = Editor()->TimeToViewCoords(ev.Start());
	r.right = r.left + 1.0;

	return r;
}

long
CEndEventRenderer::Pick(
	const CEvent &ev,
	BPoint pickPt,
	short &partCode) const
{
	BRect r(Extent(ev));
	float dist = fabs(r.left - pickPt.x);

	if (dist < 3.0)
	{
		partCode = 0;
		return static_cast<long>(dist);
	}

	return LONG_MAX;
}

const BCursor *
CEndEventRenderer::Cursor(
	short partCode,
	int32 editMode,
	bool dragging) const
{
	if ((editMode == CEventEditor::TOOL_SELECT)
	 || (editMode == CEventEditor::TOOL_CREATE))
	{
		if (dragging)
			return CCursorCache::GetCursor(CCursorCache::HORIZONTAL_MOVE);
	
		return CCursorCache::GetCursor(CCursorCache::HORIZONTAL_MOVE);
	}
	else
	{
		return Editor()->CursorFor(editMode);
	}
}

// END - EventRenderer.cpp
