/* ===================================================================== *
 * LinearEditor.cpp (MeV/User Interface)
 * ===================================================================== */

#include "LinearEditor.h"

#include "EventTrack.h"
#include "Idents.h"
#include "Destination.h"
#include "PlayerControl.h"
#include "MeVApp.h"
#include "MeVDoc.h"
#include "StdEventOps.h"
#include "ResourceUtils.h"

// Interface Kit
#include <Region.h>
// Support Kit
#include <Debug.h>

const uint8			*resizeCursor,
					*crossCursor;

// ---------------------------------------------------------------------------
// Dispatch table for linear editor ~~~EVENTLIST

CLinearNoteEventHandler		linearNoteHandler;

// ---------------------------------------------------------------------------
// Class Data Initialization

const rgb_color
CLinearEditor::NORMAL_GRID_LINE_COLOR = {220, 220, 220, 255};

const rgb_color
CLinearEditor::OCTAVE_GRID_LINE_COLOR = {180, 180, 180, 255};

const rgb_color
CLinearEditor::BACKGROUND_COLOR = {255, 255, 255, 255};
const pattern
CLinearNoteEventHandler::C_MIXED_COLORS = {0xf0,0xf0,0xf0,0xf0,0x0f,0x0f,0x0f,0x0f};

// ---------------------------------------------------------------------------
// Constructor/Destructor

CLinearEditor::CLinearEditor(
	BLooper &looper,
	CStripFrameView	&frame,
	BRect rect)
	:	CEventEditor(looper, frame, rect, "Piano Roll", true, true),
		m_whiteKeyStep(8)
{
	SetHandlerFor(EvtType_Note, &linearNoteHandler);
	SetHandlerFor(EvtType_End, &gEndEventHandler);

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
	if (TrackWindow()->Document()->GetVChannel(destination) == NULL)
		return false;

	// Initialize a new event
	m_newEv.SetCommand(TrackWindow()->NewEventType(EvtType_Note));

	// Compute the difference between the original
	// time and the new time we're dragging the events to.
	int32 time;
	time = HandlerFor(m_newEv)->QuantizeDragTime(*this, m_newEv, 0,
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

	bounds = Bounds();
	
	// REM: We should be able to figure out the maximum and minimum pitch of notes 
	// which are in the update rect.

	// For each event that overlaps the current view, draw it. (locked channels first)
	for (const Event *ev = marker.FirstItemInRange(startTime, stopTime);
		 ev;
		 ev = marker.NextItemInRange(startTime, stopTime))
	{
		if ((ev->Command() == EvtType_Note)
		 && ((ev->note.pitch < minPitch) || (ev->note.pitch > maxPitch)))
			continue;

		if (Track()->IsChannelLocked(*ev))
			HandlerFor(*ev)->Draw(*this, *ev, false);
	}

	// For each event that overlaps the current view, draw it. (unlocked channels overdraw!)
	for (const Event *ev = marker.FirstItemInRange(startTime, stopTime);
		 ev;
		 ev = marker.NextItemInRange(startTime, stopTime))
	{
		if ((ev->Command() == EvtType_Note)
		 && ((ev->note.pitch < minPitch) || (ev->note.pitch > maxPitch)))
		 	continue;

		if (!Track()->IsChannelLocked(*ev))
			HandlerFor(*ev)->Draw(*this, *ev, false);
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
CLinearEditor::MessageReceived(
	BMessage *message)
{
	switch (message->what)
	{
		case Update_ID:
		case Delete_ID:
		{
			CObserver::MessageReceived(message);
			break;
		}
		default:
		{
			CStripView::MessageReceived(message);
			break;
		}
	}
}

void
CLinearEditor::MouseMoved(
	BPoint point,
	uint32 transit,
	const BMessage *message)
{
	CEventEditor::MouseMoved(point, transit, message);

	if (transit == B_EXITED_VIEW)
	{
		TrackWindow()->SetHorizontalPositionInfo(NULL, 0);
		TrackWindow()->SetVerticalPositionInfo("");
		return;
	}
	
	TrackWindow()->SetHorizontalPositionInfo(Track(), ViewCoordsToTime(point.x));
	DisplayPitchInfo(point);

	StSubjectLock trackLock(*Track(), Lock_Shared);
	EventMarker marker(Track()->Events());

	bounds = Bounds();
}

void
CLinearEditor::OnUpdate(
	BMessage *message)
{
	int32 minTime = 0;
	int32 maxTime = LONG_MAX;
	int32 trackHint;
	bool flag;
	bool selChange = false;
	int8 channel = -1;
	BRect r(Bounds());

	bounds = r;

	if (message->FindBool("SelChange", 0, &flag) == B_OK)
	{
		if (!IsSelectionVisible())
			return;
		selChange = flag;
	}

	if (message->FindInt32("TrackAttrs", 0, &trackHint) == B_OK)
	{
		// REM: what do we do if track changes name?
		if (!(trackHint &
			(CTrack::Update_Duration | CTrack::Update_SigMap | CTrack::Update_TempoMap)))
				return;
	}
	else
		trackHint = 0;

	if (message->FindInt32("MinTime", 0, &minTime) == B_OK)
	{
		if (minTime == LONG_MIN)
			r.left = Bounds().left;
		else
			r.left = TimeToViewCoords(minTime) - 1.0;
	}
	else
		minTime = 0;

	if (message->FindInt32("MaxTime", 0, &maxTime) == B_OK)
	{
		if (maxTime == LONG_MAX)
			r.right = Bounds().right;
		else
			r.right = TimeToViewCoords(maxTime) + 1.0;
	}
	else
		maxTime = LONG_MAX;
	
	if (message->FindInt8("channel", 0, &channel) != B_OK)
		channel = -1;

	if (trackHint & CTrack::Update_Duration)
		RecalcScrollRangeH();

	if (trackHint & (CTrack::Update_SigMap | CTrack::Update_TempoMap))
	{
		RecalcScrollRangeH();
		// Invalidate everything if signature map changed
		Invalidate();
	}
	else if (channel >= 0)
	{
		StSubjectLock trackLock(*Track(), Lock_Shared);
		EventMarker marker(Track()->Events());

		// For each event that overlaps the current view, draw it.
		for (const Event *ev = marker.FirstItemInRange(minTime, maxTime);
			 ev;
			 ev = marker.NextItemInRange(minTime, maxTime))
		{
			if (ev->HasProperty(Event::Prop_Channel) && ev->GetVChannel() == channel)
				HandlerFor(*ev)->Invalidate(*this, *ev);
		}
	}
	else if (selChange)
	{
		StSubjectLock trackLock(*Track(), Lock_Shared);
		EventMarker marker(Track()->Events());

		// For each event that overlaps the current view, draw it.
		for (const Event *ev = marker.FirstItemInRange(minTime, maxTime);
			 ev;
			 ev = marker.NextItemInRange(minTime, maxTime))
		{
			HandlerFor(*ev)->Invalidate(*this, *ev);
		}
	}
	else
	{
		Invalidate(r);
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
	int32 pitch = ViewCoordsToPitch(point.y, false);
	int32 octave = pitch / 12;
	int32 note = pitch % 12;
	BString text;
	switch (note)
	{
		case 0:		text = "C";		break;
		case 1:		text = "C#";	break;
		case 2:		text = "D";		break;
		case 3:		text = "D#";	break;
		case 4:		text = "E";		break;
		case 5:		text = "F";		break;
		case 6:		text = "F#";	break;
		case 7:		text = "G";		break;
		case 8:		text = "G#";	break;
		case 9:		text = "A";		break;
		case 10:	text = "A#";	break;
		case 11:	text = "B";		break;
	}
	text << octave - 2 << "(" << pitch << ")";

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
CLinearNoteEventHandler::DEFAULT_BORDER_COLOR = {0, 0, 0, 255};
const rgb_color
CLinearNoteEventHandler::DEFAULT_HIGHLIGHT_COLOR = {224, 224, 224, 255};
const rgb_color
CLinearNoteEventHandler::SELECTED_BORDER_COLOR = {0, 0, 255, 255};
const rgb_color
CLinearNoteEventHandler::DISABLED_BORDER_COLOR = {128, 128, 128, 255};
const rgb_color
CLinearNoteEventHandler::DISABLED_FILL_COLOR = {192, 192, 192, 255};

// ---------------------------------------------------------------------------
// CAbstractEventHandler Implementation

void
CLinearNoteEventHandler::Invalidate(
	CEventEditor &editor,
	const Event &ev) const
{
	CLinearEditor &lEditor = (CLinearEditor &)editor;

	BRect r;

	r.left = lEditor.TimeToViewCoords(ev.Start()) - 1.0;
	r.right	= lEditor.TimeToViewCoords(ev.Stop()) + 1.0;
	r.bottom = lEditor.PitchToViewCoords(ev.note.pitch) + 1.0;
	r.top = r.bottom - lEditor.m_whiteKeyStep - 2.0;

	lEditor.Invalidate(r);
}

void
DrawNoteShape(	
	BView *view,
	BRect inRect,
	rgb_color outline,
	rgb_color fill,
	rgb_color highlight,
	bool drawHighlight,
	pattern apattern=B_SOLID_HIGH)
{
	rgb_color contrast;
	contrast.red=(fill.red+128) % 255;
	contrast.green=(fill.green+128) % 255;
	contrast.blue=(fill.blue+128) % 255;
	
	view->SetHighColor(fill);
	view->SetLowColor(contrast);
	view->FillRect(inRect.InsetByCopy(1.0, 1.0), apattern);
	view->SetHighColor(outline);
	view->StrokeRoundRect(inRect, 3.0, 3.0, B_SOLID_HIGH);
	if (drawHighlight && ((inRect.Width() >= 4.0) && (inRect.Height() >= 4.0))) {
		BPoint pLeft(inRect.left + 2.0, inRect.top + 2.0);
		BPoint pRight(inRect.right - 2.0, inRect.top + 2.0);
		view->SetHighColor(255, 255, 255, 255);
		view->StrokeLine(pLeft, pLeft, B_SOLID_HIGH);
		pLeft.x++;
		view->SetHighColor(highlight);
		view->StrokeLine(pLeft, pRight, B_SOLID_HIGH);
	}
}

void
CLinearNoteEventHandler::Draw(
	CEventEditor &editor,
	const Event &ev,
	bool shadowed) const
{
	CLinearEditor &lEditor = (CLinearEditor &)editor;
	CEventTrack *track = editor.Track();
	BRect r(Extent(editor, ev));

	if (track->IsChannelLocked(ev.GetVChannel()))
	{
		if (!shadowed)
			DrawNoteShape(&lEditor, r, DISABLED_BORDER_COLOR,
						  DISABLED_FILL_COLOR, DEFAULT_HIGHLIGHT_COLOR, false);
		return;
	}


	Destination *dest = lEditor.TrackWindow()->Document()->GetVChannel(ev.GetVChannel());

	
	if (dest->flags & Destination::deleted)
	{
		DrawNoteShape(&lEditor, r, DISABLED_BORDER_COLOR,
				  DISABLED_FILL_COLOR, DEFAULT_HIGHLIGHT_COLOR,false);
	}
	else if ((dest->flags & Destination::mute) || (dest->flags & Destination::disabled))
	{
		DrawNoteShape(&lEditor, r, DEFAULT_BORDER_COLOR,
					  dest->fillColor, dest->highlightColor, true,C_MIXED_COLORS);
	}
	else if (shadowed)
	{
		lEditor.SetDrawingMode(B_OP_BLEND);
		DrawNoteShape(&lEditor, r, DEFAULT_BORDER_COLOR, 
					  dest->fillColor, dest->highlightColor, true);
		lEditor.SetDrawingMode(B_OP_COPY);
	}
	else if (ev.IsSelected() && editor.IsSelectionVisible())
	{
		DrawNoteShape(&lEditor, r, SELECTED_BORDER_COLOR,
					  dest->fillColor, dest->highlightColor, true);
	}
	else 
	{
		DrawNoteShape(&lEditor, r, DEFAULT_BORDER_COLOR,
					  dest->fillColor, dest->highlightColor, true);
	}

}

BRect
CLinearNoteEventHandler::Extent(
	CEventEditor &editor,
	const Event &ev) const
{
	CLinearEditor &lEditor = (CLinearEditor &)editor;
	BRect r;
	r.left = lEditor.TimeToViewCoords(ev.Start());
	r.right = lEditor.TimeToViewCoords(ev.Stop()),
	r.bottom = lEditor.PitchToViewCoords(ev.note.pitch);
	r.top = r.bottom - lEditor.m_whiteKeyStep;

	return r;
}

long
CLinearNoteEventHandler::Pick(
	CEventEditor &editor,
	const Event &ev,
	BPoint pickPt,
	short &partCode) const
{
	CLinearEditor &lEditor = (CLinearEditor &)editor;
	int bottom	= lEditor.PitchToViewCoords(ev.note.pitch);
	int top = bottom - lEditor.m_whiteKeyStep;

	return lEditor.PickDurationEvent(ev, top, bottom, pickPt, partCode);
}

const uint8 *
CLinearNoteEventHandler::CursorImage(
	short partCode) const
{
	switch (partCode)
	{
		case 0:
		{
			return B_HAND_CURSOR;
		}
		case 1:
		{
			if (resizeCursor == NULL)
				resizeCursor = ResourceUtils::LoadCursor(2);
			return resizeCursor;
		}
	}
	
	return NULL;
}

long
CLinearNoteEventHandler::QuantizeDragValue(
	CEventEditor &editor,
	const Event &inClickEvent,
	short partCode,
	BPoint inClickPos,
	BPoint inDragPos) const
{
	CLinearEditor	&lEditor = (CLinearEditor &)editor;

	// Get the pitch and y position of the old note.
	long oldPitch = inClickEvent.note.pitch;
	float oldYPos = lEditor.PitchToViewCoords(oldPitch);
	long newPitch;

	// Add in the vertical drag delta to the note position,
	// and compute the new pitch.
	newPitch = lEditor.ViewCoordsToPitch(oldYPos + inDragPos.y
										 - inClickPos.y - lEditor.m_whiteKeyStep / 2,
										 false);
					
	return newPitch - oldPitch;
}

EventOp *
CLinearNoteEventHandler::CreateDragOp(
	CEventEditor &editor,
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
CLinearNoteEventHandler::CreateTimeOp(
	CEventEditor &editor,
	const Event &ev,
	short partCode,
	long timeDelta,
	long valueDelta) const
{
	if (partCode == 1)
		return new DurationOffsetOp(timeDelta);
	else
		return CAbstractEventHandler::CreateTimeOp(editor, ev, partCode,
												   timeDelta, valueDelta);
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
