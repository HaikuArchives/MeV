/* ===================================================================== *
 * LinearEditor.cpp (MeV/UI)
 * ===================================================================== */

#include "LinearEditor.h"

#include "CursorCache.h"
#include "EventTrack.h"
#include "Idents.h"
#include "MidiDestination.h"
#include "PlayerControl.h"
#include "MeVApp.h"
#include "MeVDoc.h"
#include "StdEventOps.h"
#include "ResourceUtils.h"

// Interface Kit
#include <Region.h>
// Support Kit
#include <Debug.h>

// ---------------------------------------------------------------------------
// Piano keyboard view

class CPianoKeyboardView
	:	public CStripLabelView
{

public:							// Constructor/Destructor

								CPianoKeyboardView(
									BRect frame,
									CLinearEditor *editor,
									uint32 resizeFlags,
									uint32 flags);

public:							// Operations
	
	void						SetSelectedKey(
									int32 key);

public:							// CStripLabelView Implementation

	virtual void				DrawInto(
									BView *view,
									BRect updateRect);

private:						// Instance Data

	CLinearEditor *				m_editor;

	int32						m_selectedKey;
};

// ---------------------------------------------------------------------------
// Note renderer class for linear editor

class CLinearNoteEventRenderer
	:	public CEventRenderer
{

public:							// Constants

	static const rgb_color		DEFAULT_BORDER_COLOR;
	static const rgb_color		DEFAULT_HIGHLIGHT_COLOR;
	static const rgb_color		SELECTED_BORDER_COLOR;
	static const rgb_color		DISABLED_BORDER_COLOR;
	static const rgb_color		DISABLED_FILL_COLOR;
	static const pattern		MIXED_COLORS;

public:							// Constructor

								CLinearNoteEventRenderer(
									CEventEditor * const editor)
									:	CEventRenderer(editor)
								{ }

public:							// CAbstractEventRenderer Implementation

	// Invalidate the event
	virtual void				Invalidate(
									const Event	&ev) const;

	// Draw the event (or an echo)
	virtual void				Draw(
									const Event &ev,
									bool shadowed) const;

	// Compute the extent of the event
	virtual BRect				Extent(
									const Event &ev) const;

	// Pick a single event and return the part code
	// (or -1 if event not picked)
	virtual long				Pick(
									const Event &ev,
									BPoint pickPt,
									short &partCode) const;

	// For a part code returned earlier, return a cursor
	// image...
	virtual const BCursor *		Cursor(
									short partCode,
									int32 editMode = CEventEditor::TOOL_SELECT,
									bool dragging = false) const;

	// Quantize the vertical position of the mouse based
	// on the event type and return a value delta.
	virtual long				QuantizeDragValue(
									const Event	&inClickEvent,
									short partCode,
									BPoint inClickPos,
									BPoint inDragPos) const;

	// Make a drag op for dragging notes...
	virtual EventOp *			CreateDragOp(
									const Event	&ev,
									short partCode,
									long timeDelta,
									long valueDelta) const;

	virtual EventOp *			CreateTimeOp(
									const Event &ev,
									short partCode,
									long timeDelta,
									long valueDelta) const;

protected:						// Accessors

	CLinearEditor * const		Editor() const
								{ return (CLinearEditor *)CEventRenderer::Editor(); }
};

// ---------------------------------------------------------------------------
// Class Data Initialization

const rgb_color
CLinearEditor::NORMAL_GRID_LINE_COLOR = {220, 220, 220, 255};

const rgb_color
CLinearEditor::OCTAVE_GRID_LINE_COLOR = {180, 180, 180, 255};

const rgb_color
CLinearEditor::BACKGROUND_COLOR = {255, 255, 255, 255};

// ---------------------------------------------------------------------------
// Constructor/Destructor

CLinearEditor::CLinearEditor(
	CStripFrameView	&frame,
	BRect rect)
	:	CEventEditor(frame, rect, "Piano Roll", true, true),
		m_whiteKeyStep(8)
{
	SetRendererFor(EvtType_Note, new CLinearNoteEventRenderer(this));

	CalcZoom();

	// Make the label view on the left-hand side
	SetLabelView(new CPianoKeyboardView(BRect(-1.0, 0.0, 20.0, rect.Height()),
										this, B_FOLLOW_TOP_BOTTOM,
										B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE));

	SetFlags(Flags() | B_PULSE_NEEDED);
}

// ---------------------------------------------------------------------------
// CEventEditor Implementation

void
CLinearEditor::AttachedToWindow()
{
	BRect r(Bounds());

	SetViewColor(B_TRANSPARENT_32_BIT);
	SetScrollRange(scrollRange.x, scrollValue.x, m_stripLogicalHeight,
				   (m_stripLogicalHeight - r.Height() / 2) / 2);
}

bool
CLinearEditor::ConstructEvent(
	BPoint point)
{
	// check if destination is set
	int32 destination = TrackWindow()->Document()->GetDefaultAttribute(EvAttr_Channel);
	if (!TrackWindow()->Document()->IsDefinedDest(destination))
		return false;

	// Initialize a new event
	m_newEv.SetCommand(TrackWindow()->NewEventType(EvtType_Note));

	// Compute the difference between the original
	// time and the new time we're dragging the events to.
	int32 time;
	time = RendererFor(m_newEv)->QuantizeDragTime(m_newEv, 0,
												 BPoint(0.0, 0.0), point,
												 true);
	TrackWindow()->SetHorizontalPositionInfo(Track(), time);

	m_newEv.SetStart(time);

	switch (m_newEv.Command())
	{
		case EvtType_End:
		{
			m_newEv.SetDuration(0);
			break;
		}
		case EvtType_Note:
		{
			m_newEv.note.pitch = ViewCoordsToPitch(point.y, true);
			m_newEv.note.attackVelocity = TrackWindow()->Document()->GetDefaultAttribute(EvAttr_AttackVelocity);
			m_newEv.note.releaseVelocity = TrackWindow()->Document()->GetDefaultAttribute(EvAttr_ReleaseVelocity);
			m_newEv.SetDuration(TrackWindow()->Document()->GetDefaultAttribute(EvAttr_Duration));
			m_newEv.SetVChannel(destination);
			break;
		}
		default:
		{
			return false;
		}
	}

	return true;
}

void
CLinearEditor::DoEventFeedback(
	const Event &event)
{
	// Start audio feedback
	if (gPrefs.FeedbackEnabled(EvAttr_Pitch))
		CPlayerControl::DoAudioFeedback(&Track()->Document(), EvAttr_Pitch,
										event.note.pitch, &event);
			
	// If it has a pitch, then show that pitch
	if (event.HasAttribute(EvAttr_Pitch))
		((CPianoKeyboardView *)LabelView())->SetSelectedKey(event.note.pitch);
}

void
CLinearEditor::Draw(
	BRect updateRect)
{
	long startTime = ViewCoordsToTime(updateRect.left - 1.0);
	long stopTime  = ViewCoordsToTime(updateRect.right + 1.0);
	long minPitch = ViewCoordsToPitch(updateRect.bottom + m_whiteKeyStep, true);
	long maxPitch = ViewCoordsToPitch(updateRect.top - m_whiteKeyStep, true);

	// Draw horizontal grid lines.
	int top = static_cast<int>(updateRect.top);
	int bottom = static_cast<int>(updateRect.bottom);
	if (bottom > m_stripLogicalHeight)
		bottom = m_stripLogicalHeight;
	int absGridLine = (m_stripLogicalHeight - top) / m_whiteKeyStep;
	if (bottom >= top) {
		BeginLineArray(bottom - top + 1);
		BPoint p1(updateRect.left, top);
		BPoint p2(updateRect.right, top);
		while (top <= bottom)
		{
			p1.y = p2.y = top++;
			if (top % m_whiteKeyStep) {
				AddLine(p1, p2, BACKGROUND_COLOR);
			}
			else {
				AddLine(p1, p2,	(absGridLine-- % 7) ? NORMAL_GRID_LINE_COLOR
											 		: OCTAVE_GRID_LINE_COLOR);
			}
		}
		EndLineArray();
	}

	if (updateRect.bottom > m_stripLogicalHeight) {
		SetHighColor(255, 255, 255, 255);
		FillRect(BRect(updateRect.left, m_stripLogicalHeight + 1,
					   updateRect.right, updateRect.bottom),
				 B_SOLID_HIGH);
	}

	DrawGridLines(updateRect);

	// Initialize an event marker for this track.
	StSubjectLock trackLock(*Track(), Lock_Shared);
	EventMarker marker(Track()->Events());

	// REM: We should be able to figure out the maximum and minimum pitch of notes 
	// which are in the update rect.

	// For each event that overlaps the current view, draw it.
	for (const Event *ev = marker.FirstItemInRange(startTime, stopTime);
		 ev != NULL;
		 ev = marker.NextItemInRange(startTime, stopTime))
	{
		if ((ev->Command() == EvtType_Note)
		 && ((ev->note.pitch < minPitch) || (ev->note.pitch > maxPitch)))
		 	continue;

		RendererFor(*ev)->Draw(*ev, false);
	}
	
	EventOp	*echoOp = PendingOperation();
	if (echoOp == NULL)
		echoOp = DragOperation();

	if (!IsSelectionVisible())
	{
		// Do nothing...
	}
	else if (m_dragType == DragType_Create)
	{
		DrawCreateEcho(startTime, stopTime);
	}
	else if (echoOp != NULL)
	{
		DrawEchoEvents(startTime, stopTime);
	}
	else if (m_dragType == DragType_Select)
	{
		DrawSelectRect();
	}
	else if (m_dragType == DragType_Lasso)
	{
		DrawLasso();
	}

	DrawPlaybackMarkers(m_pbMarkers, m_pbCount, updateRect, false);
}

void
CLinearEditor::KillEventFeedback()
{
	((CPianoKeyboardView *)LabelView())->SetSelectedKey(-1);
}

void
CLinearEditor::MouseMoved(
	BPoint point,
	uint32 transit,
	const BMessage *message)
{
	CEventEditor::MouseMoved(point, transit, message);

	if (Window()->IsActive())
	{
		if (transit == B_EXITED_VIEW)
			TrackWindow()->SetVerticalPositionInfo("");
		else
			DisplayPitchInfo(point);
	}
}

void
CLinearEditor::Pulse()
{
	UpdatePBMarkers();
	
	// REM: Add code to process newly recorded events
	// REM: Add code to edit events via MIDI.
}

void
CLinearEditor::SetScrollValue(
	float inScrollValue,
	orientation inOrient)
{
	CStripView::SetScrollValue(inScrollValue, inOrient);
	LabelView()->ScrollTo(0.0, scrollValue.y);
}

void
CLinearEditor::ZoomChanged(
	int32 diff)
{
	BRect r(Frame());
	float scroll = (ScrollValue(B_VERTICAL) + (r.Height()) / 2) / m_whiteKeyStep;

	m_whiteKeyStep += diff;
	if (m_whiteKeyStep > 12)
		m_whiteKeyStep = 12;
	else if (m_whiteKeyStep < 4)
		m_whiteKeyStep = 4;

	Hide();
	CalcZoom();
	SetScrollRange(scrollRange.x, scrollValue.x, m_stripLogicalHeight,
				   (scroll * m_whiteKeyStep) - (r.Height()) / 2);
	Show();
	LabelView()->Invalidate();
}

// ---------------------------------------------------------------------------
// Internal Operations

void
CLinearEditor::CalcZoom()
{
	m_octaveStep = 7 * m_whiteKeyStep;
	m_stripLogicalHeight = 75 * m_whiteKeyStep - 1;
}

void
CLinearEditor::DisplayPitchInfo(
	BPoint point)
{
	unsigned char note = ViewCoordsToPitch(point.y, false);
	char text[NOTE_NAME_LENGTH];

	int32 destID = TrackWindow()->Document()->GetDefaultAttribute(EvAttr_Channel);
	CDestination *dest = TrackWindow()->Document()->FindDestination(destID);
	if ((dest == NULL)
	 || (!((Midi::CMidiDestination *)dest)->GetNoteName(note, text)))
		snprintf(text, NOTE_NAME_LENGTH, "%d", note);

	TrackWindow()->SetVerticalPositionInfo(text);
}

long
CLinearEditor::PitchToViewCoords(
	int pitch)
{
	static uint8 offset[12] = {0, 1, 2, 3, 4, 6, 7, 8, 9, 10, 11, 12};
	int octave = pitch / 12;
	int note = pitch % 12;

	return	m_stripLogicalHeight - octave * m_octaveStep
								 - ((offset[note] * m_whiteKeyStep) >> 1);
}

long
CLinearEditor::ViewCoordsToPitch(
	int yPos,
	bool limit)
{
	static uint8 offsetTable[28] =
	{
		0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 4, 5, 5, 5,
		6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 11
	};
	
	int octave = (m_stripLogicalHeight - yPos) / m_octaveStep;
	int offset = (m_stripLogicalHeight - yPos) % m_octaveStep;
	int key;
					
	if (offset < 0)
	{
		octave--;
		offset += m_octaveStep;
	}

	key = offsetTable[(offset * 4) / m_whiteKeyStep] + octave * 12;

	if (limit)
	{
		if (key < 0)
			return 0;
		if (key > 127)
			return 127;
	}

	return key;
}

// ---------------------------------------------------------------------------
// Class Data Initialization

const rgb_color
CLinearNoteEventRenderer::DEFAULT_BORDER_COLOR = {0, 0, 0, 255};
const rgb_color
CLinearNoteEventRenderer::DEFAULT_HIGHLIGHT_COLOR = {224, 224, 224, 255};
const rgb_color
CLinearNoteEventRenderer::SELECTED_BORDER_COLOR = {0, 0, 255, 255};
const rgb_color
CLinearNoteEventRenderer::DISABLED_BORDER_COLOR = {128, 128, 128, 255};
const rgb_color
CLinearNoteEventRenderer::DISABLED_FILL_COLOR = {192, 192, 192, 255};
const pattern
CLinearNoteEventRenderer::MIXED_COLORS = { 0xf0, 0xf0, 0xf0, 0xf0, 
										  0x0f, 0x0f, 0x0f, 0x0f };

// ---------------------------------------------------------------------------
// CAbstractEventRenderer Implementation

void
CLinearNoteEventRenderer::Invalidate(
	const Event &ev) const
{
	Editor()->Invalidate(Extent(ev));
}

void
CLinearNoteEventRenderer::Draw(
	const Event &ev,
	bool shadowed) const
{
	BRect r(Extent(ev));
	CDestination *dest = Document()->FindDestination(ev.GetVChannel());

	if (shadowed)
		Editor()->SetDrawingMode(B_OP_BLEND);

	rgb_color fill = dest->Color();
	rgb_color shadow = {168, 168, 168, 255};
	rgb_color outline = ev.IsSelected() ? SELECTED_BORDER_COLOR
										: DEFAULT_BORDER_COLOR;
	pattern fillPattern = B_SOLID_HIGH;

	if (dest->IsMuted() || dest->IsDisabled())
	{
		fillPattern = MIXED_COLORS;
		Editor()->SetLowColor(shadow);
		if (dest->IsDisabled())
		{
			fill = tint_color(fill, B_LIGHTEN_1_TINT);
			outline = DISABLED_BORDER_COLOR;
		}
	}

	Editor()->BeginLineArray(12);
	// draw shadow
	Editor()->AddLine(BPoint(r.left + 2.0, r.bottom),
					  BPoint(r.right - 1.0, r.bottom), shadow);
	Editor()->AddLine(BPoint(r.right, r.top + 2.0),
					  BPoint(r.right, r.bottom - 1.0), shadow);
	r.right -= 1.0;
	r.bottom -= 1.0;
	Editor()->AddLine(r.RightBottom(), r.RightBottom(), shadow);
	// draw outline
	Editor()->AddLine(BPoint(r.left + 1.0, r.top),
					  BPoint(r.right - 1.0, r.top),
					  outline);
	Editor()->AddLine(BPoint(r.right, r.top + 1.0),
					  BPoint(r.right, r.bottom - 1.0),
					  outline);
	Editor()->AddLine(BPoint(r.left + 1.0, r.bottom),
					  BPoint(r.right - 1.0, r.bottom),
					  outline);
	Editor()->AddLine(BPoint(r.left, r.top + 1.0),
					  BPoint(r.left, r.bottom - 1.0),
					  outline);
	r.InsetBy(1.0, 1.0);
	// draw bevel
	Editor()->AddLine(r.RightBottom(), r.LeftBottom(),
					  tint_color(fill, B_DARKEN_1_TINT));
	Editor()->AddLine(r.RightTop(), r.RightBottom(),
					  tint_color(fill, B_DARKEN_1_TINT));
	Editor()->AddLine(r.LeftTop(), r.RightTop(),
					  tint_color(fill, B_LIGHTEN_1_TINT));
	Editor()->AddLine(r.LeftBottom(), r.LeftTop(),
					  tint_color(fill, B_LIGHTEN_1_TINT));
	Editor()->AddLine(r.LeftTop(), r.LeftTop(),
					  tint_color(fill, B_LIGHTEN_MAX_TINT));
	Editor()->EndLineArray();

	r.InsetBy(1.0, 1.0);
	Editor()->SetHighColor(fill);
	Editor()->FillRect(r, fillPattern);
}

BRect
CLinearNoteEventRenderer::Extent(
	const Event &ev) const
{
	BRect r;
	r.left = Editor()->TimeToViewCoords(ev.Start());
	r.right = Editor()->TimeToViewCoords(ev.Stop()),
	r.bottom = Editor()->PitchToViewCoords(ev.note.pitch);
	r.top = r.bottom - Editor()->m_whiteKeyStep;

	return r;
}

long
CLinearNoteEventRenderer::Pick(
	const Event &ev,
	BPoint pickPt,
	short &partCode) const
{
	int bottom	= Editor()->PitchToViewCoords(ev.note.pitch);
	int top = bottom - Editor()->m_whiteKeyStep;

	return Editor()->PickDurationEvent(ev, top, bottom, pickPt, partCode);
}

const BCursor *
CLinearNoteEventRenderer::Cursor(
	short partCode,
	int32 editMode,
	bool dragging) const
{
	if ((editMode == CEventEditor::TOOL_SELECT)
	 || (editMode == CEventEditor::TOOL_CREATE))
	{
		switch (partCode)
		{
			case 0:
			{
				if (dragging)
					return CCursorCache::GetCursor(CCursorCache::DRAGGING);
				return CCursorCache::GetCursor(CCursorCache::DRAGGABLE);
			}
			case 1:
			{
				return CCursorCache::GetCursor(CCursorCache::HORIZONTAL_RESIZE);
			}
		}
		return NULL;
	}
	else
	{
		return Editor()->CursorFor(editMode);
	}
}

long
CLinearNoteEventRenderer::QuantizeDragValue(
	const Event &ev,
	short partCode,
	BPoint clickPos,
	BPoint dragPos) const
{
	// Get the pitch and y position of the old note.
	long oldPitch = ev.note.pitch;
	float oldYPos = Editor()->PitchToViewCoords(oldPitch);
	long newPitch;

	// Add in the vertical drag delta to the note position,
	// and compute the new pitch.
	newPitch = Editor()->ViewCoordsToPitch(oldYPos + dragPos.y - clickPos.y
										   - Editor()->m_whiteKeyStep / 2,
										   false);
					
	return newPitch - oldPitch;
}

EventOp *
CLinearNoteEventRenderer::CreateDragOp(
	const Event &ev,
	short partCode,
	long timeDelta,
	long valueDelta) const
{
	if (partCode == 0)
		return new PitchOffsetOp(valueDelta);
	else
		return NULL;
}

EventOp *
CLinearNoteEventRenderer::CreateTimeOp(
	const Event &ev,
	short partCode,
	long timeDelta,
	long valueDelta) const
{
	if (partCode == 1)
		return new DurationOffsetOp(timeDelta);

	return CEventRenderer::CreateTimeOp(ev, partCode, timeDelta, valueDelta);
}

// ---------------------------------------------------------------------------
// Constructor/Destructor

CPianoKeyboardView::CPianoKeyboardView(
	BRect frame,
	CLinearEditor *editor,
	uint32 resizeFlags,
	uint32 flags)
	:	CStripLabelView(frame, "PianoKeyboardView", resizeFlags, flags),
		m_editor(editor),
		m_selectedKey(-1)
{
	SetViewColor(B_TRANSPARENT_32_BIT);
}

// ---------------------------------------------------------------------------
// Operations

void
CPianoKeyboardView::SetSelectedKey(
	int32 key)
{
	BRect r(Bounds());

	if (key != m_selectedKey)
	{
		r.bottom = m_editor->PitchToViewCoords(m_selectedKey) + 1.0;
		r.top = r.bottom - m_editor->m_whiteKeyStep - 2.0;
		Invalidate(r);

		m_selectedKey = key;

		r.bottom = m_editor->PitchToViewCoords(m_selectedKey) + 1.0;
		r.top = r.bottom - m_editor->m_whiteKeyStep - 2.0;
		Invalidate(r);
	}
}

// ---------------------------------------------------------------------------
// BView Implementation

void
CPianoKeyboardView::DrawInto(
	BView *view,
	BRect updateRect)
{
	BRect r(view->Bounds()), rect;
	BRegion wRegion(updateRect), bRegion;
	int32 yPos;
	int32 lineCt;
	int32 wh = m_editor->m_whiteKeyStep;
	int32 bl = (wh + 1) / 4;
	int32 key;
	static int8	black[7] = {1, 1, 0, 1, 1, 1, 0};

	if (updateRect.right >= r.right) {
		view->SetHighColor(128, 128, 128, 255);
		view->StrokeLine(BPoint(r.right, updateRect.top),
						 BPoint(r.right, updateRect.bottom),
						 B_SOLID_HIGH);
		wRegion.Exclude(BRect(r.right, updateRect.top, 
							  r.right, updateRect.bottom));
	}

	view->SetHighColor(255, 255, 255, 255);
	view->SetLowColor(128, 128, 128, 255);

	// Draw horizontal grid lines.
	// REM: This needs to be faster.
	for (yPos = m_editor->m_stripLogicalHeight, lineCt = 0, key = 0;
		 yPos >= updateRect.top;
		 yPos -= wh, lineCt++, key++)
	{
		if (lineCt >= 7)
			lineCt = 0;

		// Fill solid rectangle with gridline
		if (yPos <= (updateRect.bottom + wh + bl))
		{
			rect = BRect(updateRect.left, yPos, updateRect.right, yPos);
			view->StrokeLine(rect.LeftTop(), rect.RightTop(), B_SOLID_LOW);
			wRegion.Exclude(rect);

			if (m_selectedKey == key)
			{
				view->SetHighColor(148, 148, 255, 255);
				rect = BRect(r.left, yPos - wh + 1, r.right - 1, yPos - 1);
				view->FillRect(rect, B_SOLID_HIGH);
				wRegion.Exclude(rect);
			}
		}
		if (black[lineCt])
			key++;
	}

	// Draw black keys
	// REM: This needs to be faster.
	view->SetLowColor(148, 148, 255, 255);
	for (yPos = m_editor->m_stripLogicalHeight, lineCt = 0, key = 0;
		 yPos >= updateRect.top;
		 yPos -= wh, lineCt++, key++ )
	{
		if (lineCt >= 7)
			lineCt = 0;
		if (black[lineCt])
			key++;

		// Fill solid rectangle with gridline
		if (yPos <= (updateRect.bottom + wh + bl))
		{
			if (black[lineCt])
			{
				rect = BRect(r.left, yPos - wh - bl,
							 r.left + 11, yPos - wh + bl);
				bRegion.Include(rect);

				if (m_selectedKey == key)
				{
					rect = BRect(r.left, yPos - wh - bl + 1,
								 r.left + 10, yPos - wh + bl - 1);
					view->FillRect(rect, B_SOLID_LOW);
					bRegion.Exclude(rect);
					wRegion.Exclude(rect);
				}
			}
		}
	}

	view->SetHighColor(96, 96, 96, 255);
	view->FillRegion(&bRegion, B_SOLID_HIGH);
	view->SetHighColor(255, 255, 255, 255);
	wRegion.Exclude(&bRegion);
	view->FillRegion(&wRegion, B_SOLID_HIGH);
}

// END - LinearEditor.cpp
