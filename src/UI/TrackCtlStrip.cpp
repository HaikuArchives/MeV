/* ===================================================================== *
 * TrackCtlStrip.cpp (MeV/UI)
 * ===================================================================== */

#include "TrackCtlStrip.h"

#include "Idents.h"
#include "MeVApp.h"
#include "MeVDoc.h"
#include "EventTrack.h"
#include "PlayerControl.h"
#include "StdEventOps.h"
#include "MidiDeviceInfo.h"
#include "PlayerControl.h"
#include "ResourceUtils.h"
// StripView
#include "StripLabelView.h"

// Gnu C Library
#include <stdio.h>
// Interface Kit
#include <Bitmap.h>
#include <Region.h>
// Support Kit
#include <Debug.h>

// Debugging Macros
#define D_ALLOC(x) PRINT(x)		// Constructor/Destructor

extern const uint8	*resizeCursor,
					*crossCursor;

static rgb_color GREY_PALETTE [] =
{
	{0xff, 0xff, 0xff},
	{0xee, 0xee, 0xee},
	{0xdd, 0xdd, 0xdd},
	{0xcc, 0xcc, 0xcc},
	{0xbb, 0xbb, 0xbb},
	{0x77, 0x77, 0x77}
};

static rgb_color BLUE_PALETTE [] =
{
	{0xff, 0xff, 0xff},
	{0xdd, 0xff, 0xff},
	{0xcc, 0xcc, 0xff},
	{0xbb, 0xbb, 0xff},
	{0x88, 0x88, 0xff},
	{0x66, 0x66, 0xff}
};

// ---------------------------------------------------------------------------
// Event handler class for repeats

class CTrackEventHandler : public CAbstractEventHandler {

		// No constructor

		// Invalidate the event
	void Invalidate(	CEventEditor	&editor,
					const Event		&ev ) const ;

		// Invalidate the event
	BRect Extent(		CEventEditor	&editor,
					const Event		&ev ) const;

		// Pick a single event and returns the distance.
	long Pick(		CEventEditor	&editor,
					const Event		&ev,
					BPoint			pickPt,
					short			&partCode ) const;

		// For a part code returned earlier, return a cursor
		// image...
	const uint8 *CursorImage( short partCode ) const;

		// Quantize the vertical position of the mouse based
		// on the event type and return a value delta.
	long QuantizeDragValue(
		CEventEditor	&editor,
		const Event	&inClickEvent,
		short		partCode,			// Part of event clicked
		BPoint		inClickPos,
		BPoint		inDragPos ) const;

		// Make a drag op for dragging notes...
	EventOp *CreateDragOp(
		CEventEditor	&editor,
		const Event	&ev,
		short		partCode,
		long			timeDelta,			// The horizontal drag delta
		long			valueDelta ) const;

	EventOp *CreateTimeOp(
		CEventEditor	&editor,			// The editor
		const Event	&ev,				// The clicked event
		short		partCode,			// Part of event clicked
		long			timeDelta,			// The horizontal drag delta
		long			valueDelta ) const;
};

	// Invalidate the event
void CTrackEventHandler::Invalidate(
	CEventEditor		&editor,
	const Event		&ev ) const
{
	CTrackCtlStrip	&rEditor = (CTrackCtlStrip &)editor;
	BRect			r;

	r.left		= editor.TimeToViewCoords( ev.Start() ) - 1.0;
	r.right	= editor.TimeToViewCoords( ev.Stop()  ) + 1.0;
	r.top		= rEditor.VPosToViewCoords( ev.repeat.vPos ) + 1.0;
	r.bottom	= r.top + rEditor.BarHeight() - 2;

	rEditor.Invalidate( r );
}

	// Compute the extent of the event.
BRect CTrackEventHandler::Extent(
	CEventEditor		&editor,
	const Event		&ev ) const
{
	CTrackCtlStrip	&rEditor = (CTrackCtlStrip &)editor;
	BRect			r;

	r.left		= editor.TimeToViewCoords( ev.Start() );
	r.right	= editor.TimeToViewCoords( ev.Stop()  );
	r.top		= rEditor.VPosToViewCoords( ev.repeat.vPos ) + 1.0;
	r.bottom	= r.top + rEditor.BarHeight() - 2;

	return r;
}

	// Pick a single event and return the part code
	// (or -1 if event not picked)
long CTrackEventHandler::Pick(
	CEventEditor	&editor,
	const Event		&ev,
	BPoint			pickPt,
	short			&partCode ) const
{
	CTrackCtlStrip	&rEditor = (CTrackCtlStrip &)editor;
	BRect			r;

	r.top	= rEditor.VPosToViewCoords( ev.repeat.vPos ) + 1.0;
	r.bottom	= r.top + rEditor.BarHeight() - 2;

	return editor.PickDurationEvent( ev, r.top, r.bottom, pickPt, partCode );
}

const uint8 *CTrackEventHandler::CursorImage( short partCode ) const
{
	switch (partCode) {
	case 0:
		return B_HAND_CURSOR;			// Return the normal hand cursor

	case 1:								// Return resizing cursor
		if (resizeCursor == NULL)
		{
			resizeCursor = ResourceUtils::LoadCursor(2);
		}
		return resizeCursor;
	}
	
	return NULL;
}

	// Quantize the vertical position of the mouse based
	// on the event type and return a value delta.
long CTrackEventHandler::QuantizeDragValue(
	CEventEditor	&editor,
	const Event		&inClickEvent,
	short			partCode,			// Part of event clicked
	BPoint			inClickPos,
	BPoint			inDragPos ) const
{
	CTrackCtlStrip	&tEditor = (CTrackCtlStrip &)editor;

		// Get the y position of the old note.
	long			oldPos	= inClickEvent.repeat.vPos;
	float		oldYPos	= tEditor.VPosToViewCoords( oldPos );
	long			newPos;

	newPos = tEditor.ViewCoordsToVPos(
		oldYPos + inDragPos.y - inClickPos.y + tEditor.BarHeight() / 2, false );

	return newPos - oldPos;
}

EventOp *CTrackEventHandler::CreateDragOp(
	CEventEditor	&editor,
	const Event		&ev,
	short			partCode,
	long				timeDelta,			// The horizontal drag delta
	long				valueDelta ) const
{
	if (partCode == 0)
		return new VPosOffsetOp( valueDelta );
	else return NULL;
}

EventOp *CTrackEventHandler::CreateTimeOp(
	CEventEditor	&editor,
	const Event		&ev,
	short			partCode,
	long			timeDelta,			// The horizontal drag delta
	long			valueDelta ) const
{
	if (partCode == 1)
		return new DurationOffsetOp( timeDelta );
	else return CAbstractEventHandler::CreateTimeOp( editor, ev, partCode, timeDelta, valueDelta );
}

// ---------------------------------------------------------------------------
// Event handler class for repeats

class CRepeatEventHandler
	:	public CTrackEventHandler
{

public:							// CTrackEventHandler Implementation

	/** Draw the event (or an echo) */
	void						Draw(
									CEventEditor &editor,
									const Event &ev,
									bool shadowed) const;
};

void
CRepeatEventHandler::Draw(
	CEventEditor &editor,
	const Event &ev,
	bool shadowed ) const
{
	CTrackCtlStrip &rEditor = (CTrackCtlStrip &)editor;

	BRect r;
	r.left = editor.TimeToViewCoords(ev.Start());
	r.right = editor.TimeToViewCoords(ev.Stop());
	r.top = rEditor.VPosToViewCoords(ev.repeat.vPos) + 1.0;
	r.bottom = r.top + rEditor.BarHeight() - 2.0;

	rgb_color *color;
	if (shadowed && (editor.DragOperation() != NULL))
	{
		editor.SetDrawingMode(B_OP_BLEND);
		color = BLUE_PALETTE;
	}
	else if (ev.IsSelected() && editor.IsSelectionVisible())
	{
		editor.SetDrawingMode(B_OP_COPY);
		color = BLUE_PALETTE;
	}
	else
	{
		editor.SetDrawingMode(B_OP_COPY);
		color = GREY_PALETTE;
	}

	editor.BeginLineArray(8);
	editor.AddLine(r.RightBottom(), r.LeftBottom(), color[5]);
	editor.AddLine(r.RightBottom(), r.RightTop(), color[5]);
	editor.AddLine(r.LeftTop(), r.LeftBottom(), color[5]);
	editor.AddLine(r.LeftTop(), r.RightTop(), color[5]);
	r.InsetBy(1.0, 1.0);
	editor.AddLine(r.RightBottom(), r.LeftBottom(), color[4]);
	editor.AddLine(r.RightBottom(), r.RightTop(), color[4]);
	editor.AddLine(r.LeftTop(), r.LeftBottom(), color[0]);
	editor.AddLine(r.LeftTop(), r.RightTop(), color[0]);
	editor.EndLineArray();

	r.InsetBy(1.0, 1.0);
	editor.SetHighColor(color[shadowed ? 3 : 2 ]);
	editor.FillRect(r);

	// Now, for the little dots...
	if (r.Width() > 6)
	{
		BString repeatText = "";
		if (ev.repeat.repeatCount == 0)
			repeatText << "oo";
		else
			repeatText << ev.repeat.repeatCount;
	
		editor.SetDrawingMode(B_OP_OVER);
		if (shadowed)
			editor.SetHighColor(128, 128, 128, 255);
		else
			editor.SetHighColor(0, 0, 0, 255);
		font_height fh;
		be_plain_font->GetHeight(&fh);
		float textWidth = be_plain_font->StringWidth(repeatText.String());
		editor.MovePenTo((r.left + r.right - textWidth) / 2.0,
						 (r.top + r.bottom - fh.descent + fh.ascent) / 2 );
		editor.DrawString(repeatText.String());

		BPoint offset;
		editor.BeginLineArray(20);
		for (int32 i = 0; i < 4; i++)
		{
			if (i == 0)
				offset = r.LeftTop() + BPoint(1.0, 1.0);
			else if (i == 1)
				offset = r.LeftBottom() + BPoint(1.0, -4.0);
			else if (i == 2)
				offset = r.RightTop() + BPoint(-4.0, 1.0);
			else
				offset = r.RightBottom() + BPoint(-4.0, -4.0);
			editor.AddLine(offset, offset + BPoint(1.0, 0.0), color[0]);
			editor.AddLine(offset, offset + BPoint(0.0, 1.0), color[0]);
			offset += BPoint(1.0, 1.0);
			editor.AddLine(offset, offset, color[2]);
			offset += BPoint(0.0, 1.0);
			editor.AddLine(offset, offset + BPoint(1.0, 0.0), color[5]);
			editor.AddLine(offset + BPoint(1.0, 0.0),
						   offset + BPoint(1.0, -1.0), color[5]);
		}
		editor.EndLineArray();
	}
}

// ---------------------------------------------------------------------------
// Event handler class for repeats

class CSequenceEventHandler : public CTrackEventHandler {

		// No constructor

		// Draw the event (or an echo)
	void Draw(		CEventEditor	&editor,
					const Event	&ev,
					bool 		shadowed ) const;
};

void
CSequenceEventHandler::Draw(
	CEventEditor &editor,
	const Event &ev,
	bool shadowed) const
{
	CTrackCtlStrip &rEditor = (CTrackCtlStrip &)editor;

	BRect r;
	r.left = editor.TimeToViewCoords(ev.Start());
	r.right = editor.TimeToViewCoords(ev.Stop());
	r.top = rEditor.VPosToViewCoords(ev.repeat.vPos) + 1.0;
	r.bottom = r.top + rEditor.BarHeight() - 2.0;

	rgb_color *color;
	if (shadowed && (editor.DragOperation() != NULL))
	{
		editor.SetDrawingMode(B_OP_BLEND);
		color = BLUE_PALETTE;
	}
	else if (ev.IsSelected() && editor.IsSelectionVisible())
	{
		editor.SetDrawingMode(B_OP_COPY);
		color = BLUE_PALETTE;
	}
	else
	{
		editor.SetDrawingMode(B_OP_COPY);
		color = GREY_PALETTE;
	}

	editor.BeginLineArray(8);
	editor.AddLine(r.RightBottom(), r.LeftBottom(), color[5]);
	editor.AddLine(r.RightBottom(), r.RightTop(), color[5]);
	editor.AddLine(r.LeftTop(), r.LeftBottom(), color[5]);
	editor.AddLine(r.LeftTop(), r.RightTop(), color[5]);
	r.InsetBy(1.0, 1.0);
	editor.AddLine(r.RightBottom(), r.LeftBottom(), color[4]);
	editor.AddLine(r.RightBottom(), r.RightTop(), color[4]);
	editor.AddLine(r.LeftTop(), r.LeftBottom(), color[0]);
	editor.AddLine(r.LeftTop(), r.RightTop(), color[0]);
	editor.EndLineArray();

	r.InsetBy(1.0, 1.0);
	editor.SetHighColor(color[shadowed ? 3 : 2 ]);
	editor.FillRect(r);
	
	if (r.Width() >= 6.0)
	{
		CTrack *track = editor.Track()->Document().FindTrack(ev.sequence.sequence);
		BString trackName;
		if (track)
			trackName = track->Name();
		else
			trackName = "(None)";
	
		be_plain_font->TruncateString(&trackName, B_TRUNCATE_END,
									  r.Width() - 8.0);

		editor.SetDrawingMode(B_OP_OVER);
		if ((track == NULL) || shadowed)
			editor.SetHighColor(128, 128, 128, 255);
		else
			editor.SetHighColor(0, 0, 0, 255);
		font_height fh;
		be_plain_font->GetHeight(&fh);
		editor.MovePenTo(r.left + 4,
						 (r.top + r.bottom - fh.descent + fh.ascent) / 2 );
		editor.DrawString(trackName.String());
	}
}

// ---------------------------------------------------------------------------
// Event handler class for time signatures

class CTimeSigEventHandler : public CAbstractEventHandler {

		// No constructor

		// Invalidate the event
	void Invalidate(	CEventEditor		&editor,
					const Event		&ev ) const ;

		// Draw the event (or an echo)
	void Draw(		CEventEditor		&editor,
					const Event		&ev,
					bool 			shadowed ) const;
		// Invalidate the event
	BRect Extent(		CEventEditor		&editor,
					const Event		&ev ) const;

		// Pick a single event and returns the distance.
	long Pick(		CEventEditor		&editor,
					const Event		&ev,
					BPoint			pickPt,
					short			&partCode ) const;

		// For a part code returned earlier, return a cursor
		// image...
	const uint8 *CursorImage( short partCode ) const;

		// Quantize the vertical position of the mouse based
		// on the event type and return a value delta.
	long QuantizeDragValue(
		CEventEditor	&editor,
		const Event		&inClickEvent,
		short			partCode,			// Part of event clicked
		BPoint			inClickPos,
		BPoint			inDragPos ) const;

		// Make a drag op for dragging notes...
	EventOp *CreateDragOp(
		CEventEditor	&editor,
		const Event		&ev,
		short			partCode,
		long			timeDelta,			// The horizontal drag delta
		long			valueDelta ) const;
};

	// Invalidate the event
void CTimeSigEventHandler::Invalidate(
	CEventEditor		&editor,
	const Event		&ev ) const
{
	((CTrackCtlStrip &)editor).Invalidate( Extent( editor, ev ) );
}

void
CTimeSigEventHandler::Draw(
	CEventEditor &editor,
	const Event &ev,
	bool shadowed) const
{
	CTrackCtlStrip &rEditor = (CTrackCtlStrip &)editor;

	BString sigText;
	sigText << ev.sigChange.numerator << "/" << (1 << ev.sigChange.denominator);
	float textWidth = editor.StringWidth(sigText.String());

	BRect r;
	r.left = editor.TimeToViewCoords(ev.Start());
	r.right	= r.left + textWidth + 2.0;
	r.top = rEditor.VPosToViewCoords(ev.repeat.vPos) + 1.0;
	r.bottom = r.top + rEditor.BarHeight() - 2.0;

	if (shadowed)
	{
		editor.SetDrawingMode(B_OP_BLEND);
		editor.SetHighColor(128, 0, 128, 255);
	}
	else if (ev.IsSelected() && editor.IsSelectionVisible())
	{
		editor.SetDrawingMode(B_OP_OVER);
		editor.SetHighColor(64, 64, 255, 255);
	}
	else
	{
		editor.SetDrawingMode(B_OP_OVER);
		editor.SetHighColor(0, 0, 0, 255);
	}

	font_height fh;
	be_plain_font->GetHeight(&fh);
	editor.MovePenTo(r.left + 1.0,
					 (r.top + r.bottom - fh.descent + fh.ascent) / 2.0);
	editor.DrawString(sigText.String());
}

	// Compute the extent of the event.
BRect CTimeSigEventHandler::Extent(
	CEventEditor		&editor,
	const Event		&ev ) const
{
	CTrackCtlStrip	&rEditor = (CTrackCtlStrip &)editor;
	BRect			r;
	float			pWidth;
	char				text[ 32 ];

	sprintf( text, "%d/%d", ev.sigChange.numerator, 1 << (ev.sigChange.denominator) );
	pWidth = rEditor.StringWidth( text );

	r.left	= editor.TimeToViewCoords( ev.Start() );
	r.right	= r.left + pWidth + 2;
	r.top	= rEditor.VPosToViewCoords( ev.repeat.vPos ) + 1.0;
	r.bottom	= r.top + rEditor.BarHeight() - 2;

	return r;
}

	// Pick a single event and return the part code
	// (or -1 if event not picked)
long CTimeSigEventHandler::Pick(
	CEventEditor	&editor,
	const Event		&ev,
	BPoint			pickPt,
	short			&partCode ) const
{
//	CTrackCtlStrip	&rEditor = (CTrackCtlStrip &)editor;
	BRect			r( Extent( editor, ev ) );
	
	if (r.Contains(pickPt))
	{
		partCode = 0;
		return static_cast<long>(fabs((r.top + r.bottom) - pickPt.y * 2));
	}

	return LONG_MAX;
}

const uint8 *CTimeSigEventHandler::CursorImage( short partCode ) const
{
	return B_HAND_CURSOR;			// Return the normal hand cursor
}

	// Quantize the vertical position of the mouse based
	// on the event type and return a value delta.
long CTimeSigEventHandler::QuantizeDragValue(
	CEventEditor	&editor,
	const Event		&inClickEvent,
	short			partCode,			// Part of event clicked
	BPoint			inClickPos,
	BPoint			inDragPos ) const
{
	CTrackCtlStrip	&tEditor = (CTrackCtlStrip &)editor;

		// Get the y position of the old note.
	long			oldPos	= inClickEvent.repeat.vPos;
	float		oldYPos	= tEditor.VPosToViewCoords( oldPos );
	long			newPos;

	newPos = tEditor.ViewCoordsToVPos(
		oldYPos + inDragPos.y - inClickPos.y + tEditor.BarHeight() / 2, false );

	return newPos - oldPos;
}

EventOp *CTimeSigEventHandler::CreateDragOp(
	CEventEditor	&editor,
	const Event		&ev,
	short			partCode,
	long				timeDelta,			// The horizontal drag delta
	long				valueDelta ) const
{
	return new VPosOffsetOp( valueDelta );
}

// ---------------------------------------------------------------------------
// Event handler class for program changes

class CProgramChangeEventHandler : public CAbstractEventHandler {

		// No constructor

		// Invalidate the event
	void Invalidate(	CEventEditor	&editor,
					const Event		&ev ) const ;

		// Draw the event (or an echo)
	void Draw(		CEventEditor	&editor,
					const Event		&ev,
					bool 			shadowed ) const;
		// Invalidate the event
	BRect Extent(		CEventEditor	&editor,
					const Event		&ev ) const;

		// Pick a single event and returns the distance.
	long Pick(		CEventEditor	&editor,
					const Event		&ev,
					BPoint			pickPt,
					short			&partCode ) const;

		// For a part code returned earlier, return a cursor
		// image...
	const uint8 *CursorImage( short partCode ) const;

		// Quantize the vertical position of the mouse based
		// on the event type and return a value delta.
	long QuantizeDragValue(
		CEventEditor		&editor,
		const Event		&inClickEvent,
		short			partCode,			// Part of event clicked
		BPoint			inClickPos,
		BPoint			inDragPos ) const;

		// Make a drag op for dragging notes...
	EventOp *CreateDragOp(
		CEventEditor		&editor,
		const Event		&ev,
		short			partCode,
		long				timeDelta,			// The horizontal drag delta
		long				valueDelta ) const;

	const char *GetPatchName( CEventEditor &editor, const Event &ev, char *outName ) const;
};

	// Invalidate the event
void CProgramChangeEventHandler::Invalidate(
	CEventEditor		&editor,
	const Event		&ev ) const
{
	CTrackCtlStrip	&rEditor = (CTrackCtlStrip &)editor;

	rEditor.Invalidate( Extent( editor, ev ) );
}

void
CProgramChangeEventHandler::Draw(
	CEventEditor &editor,
	const Event &ev,
	bool shadowed) const
{
	CTrackCtlStrip &rEditor = (CTrackCtlStrip &)editor;
	VChannelEntry *vce = editor.Track()->Document().GetVChannel( ev.GetVChannel() );
	bool locked = editor.Track()->IsChannelLocked(ev.GetVChannel());

	// get bitmap
	// !!! this MUST be cached somewhere
	BBitmap *horn = ResourceUtils::LoadImage("ProgramTool");
	BRect hornRect = horn->Bounds();

	// acquire patch name
	char patchNameBuf[64];
	const char *patchName;
	patchName = GetPatchName(editor, ev, patchNameBuf);

	BRect r;
	r.left = editor.TimeToViewCoords(ev.Start());
	r.right	= r.left + rEditor.StringWidth(patchName) + 9.0 + hornRect.Width();
	r.top = rEditor.VPosToViewCoords(ev.repeat.vPos) + 1.0;
	r.bottom = r.top + rEditor.BarHeight() - 2.0;
	
	// Reduce size of rectangle a bit.
	font_height fh;
	be_plain_font->GetHeight(&fh);
	if (r.Height() > fh.ascent + fh.descent + 2.0)
		r.InsetBy(0.0, 1.0);

	if (shadowed && rEditor.m_dragOp != NULL)
		rEditor.SetDrawingMode(B_OP_BLEND);
	else
		rEditor.SetDrawingMode(B_OP_OVER);
	
	// A test -- don't show the existing event if we're adjusting via slider.
	if (!shadowed && rEditor.PendingOperation() != NULL)
		return;

	BPoint offset(r.left, (r.top + r.bottom - hornRect.Height()) / 2);
	editor.DrawBitmap(horn, offset);
	r.left += hornRect.Width() + 3.0;

	offset.x = r.left + 3.0;
	offset.y = (r.top + r.bottom - fh.descent + fh.ascent + 1.0) / 2;
	
	if (locked)
	{
		editor.SetHighColor(128, 128, 128, 255);
		editor.StrokeRect(r);
		r.InsetBy(1.0, 1.0);
		rEditor.SetHighColor(192, 192, 192, 255);
		rEditor.FillRect(r);
	}
	else
	{
		if (ev.IsSelected() && editor.IsSelectionVisible())
		{
			editor.SetHighColor(0, 0, 255, 255);
			editor.StrokeRect(r);
			r.InsetBy(1.0, 1.0);
		}
		editor.SetHighColor(vce->fillColor);
		editor.FillRect(r);
	}

	editor.SetHighColor(0, 0, 0, 255);
	editor.DrawString(patchName, offset);

	delete horn;
}

	// Compute the extent of the event.
BRect CProgramChangeEventHandler::Extent(
	CEventEditor		&editor,
	const Event		&ev ) const
{
	CTrackCtlStrip	&rEditor = (CTrackCtlStrip &)editor;
	BRect			r;
	char			patchNameBuf[ 64 ];
	const char		*patchName;
	BBitmap			*horn;
	BRect			hornRect;
	
	horn = ResourceUtils::LoadImage("ProgramTool");
	hornRect = horn->Bounds();
	
	patchName = GetPatchName( editor, ev, patchNameBuf );

	float			pWidth;
	static float	maxPWidth = 0.0;

	pWidth = rEditor.StringWidth(patchName);
	if (pWidth > maxPWidth)
		maxPWidth = pWidth;

	r.left		= editor.TimeToViewCoords( ev.Start() );
	r.right	= r.left + maxPWidth + 9.0 + hornRect.Width();
	r.top		= rEditor.VPosToViewCoords( ev.repeat.vPos ) + 1.0;
	r.bottom	= r.top + rEditor.BarHeight() - 2;
	
	if (r.Height() < hornRect.Height())
	{
		r.top -= (hornRect.Height() - r.Height()) / 2;
		r.bottom = r.top + hornRect.Height();
	}

	return r;
}

	// Pick a single event and return the part code
	// (or -1 if event not picked)
long CProgramChangeEventHandler::Pick(
	CEventEditor	&editor,
	const Event		&ev,
	BPoint			pickPt,
	short			&partCode ) const
{
	BRect			r( Extent( editor, ev ) );
	
	if (r.Contains( pickPt ))
	{
		partCode = 0;
		return static_cast<long>(fabs((r.top + r.bottom) - pickPt.y * 2));
	}

	return LONG_MAX;
}

const uint8 *CProgramChangeEventHandler::CursorImage( short partCode ) const
{
	return B_HAND_CURSOR;			// Return the normal hand cursor
}

	// Quantize the vertical position of the mouse based
	// on the event type and return a value delta.
long CProgramChangeEventHandler::QuantizeDragValue(
	CEventEditor		&editor,
	const Event		&inClickEvent,
	short			partCode,			// Part of event clicked
	BPoint			inClickPos,
	BPoint			inDragPos ) const
{
	CTrackCtlStrip		&tEditor = (CTrackCtlStrip &)editor;

		// Get the y position of the old note.
	long			oldPos	= inClickEvent.repeat.vPos;
	float		oldYPos	= tEditor.VPosToViewCoords( oldPos );
	long			newPos;

	newPos = tEditor.ViewCoordsToVPos(
		oldYPos + inDragPos.y - inClickPos.y + tEditor.BarHeight() / 2, false );

	return newPos - oldPos;
}

EventOp *CProgramChangeEventHandler::CreateDragOp(
	CEventEditor	&editor,
	const Event		&ev,
	short			partCode,
	long				timeDelta,			// The horizontal drag delta
	long				valueDelta ) const
{
	return new VPosOffsetOp( valueDelta );
}

const char *CProgramChangeEventHandler::GetPatchName(
	CEventEditor &editor, const Event &ev, char *tempBuf ) const
{
	uint16			programBank;
	CMeVDoc			&doc = editor.Track()->Document();
	VChannelEntry		*vc = doc.GetVChannel( ev.GetVChannel() );
	MIDIDeviceInfo	*instrument;
	PatchInfo			*patch;

	instrument = ((CMeVApp *)be_app)->LookupInstrument( 1, vc->channel - 1 );
	if (instrument == NULL) return "[Invalid Device]";
	
	programBank = (ev.programChange.bankMSB << 7) | ev.programChange.bankLSB;
	patch = instrument->GetPatch( programBank, ev.programChange.program );

	if (patch == NULL || patch->Name() == NULL)
	{
		sprintf( tempBuf, "Pgm: %d:%d", programBank, ev.programChange.program );
		return tempBuf;
	}
	else return patch->Name();
}

// ---------------------------------------------------------------------------
// Event handler class for tempo events

class CTempoEventHandler : public CAbstractEventHandler {

		// No constructor

		// Invalidate the event
	void Invalidate(	CEventEditor	&editor,
					const Event		&ev ) const ;

		// Draw the event (or an echo)
	void Draw(		CEventEditor	&editor,
					const Event		&ev,
					bool 			shadowed ) const;
		// Invalidate the event
	BRect Extent(		CEventEditor	&editor,
					const Event		&ev ) const;

		// Pick a single event and returns the distance.
	long Pick(		CEventEditor	&editor,
					const Event		&ev,
					BPoint			pickPt,
					short			&partCode ) const;

		// For a part code returned earlier, return a cursor
		// image...
	const uint8 *CursorImage( short partCode ) const;

		// Quantize the vertical position of the mouse based
		// on the event type and return a value delta.
	long QuantizeDragValue(
		CEventEditor		&editor,
		const Event		&inClickEvent,
		short			partCode,			// Part of event clicked
		BPoint			inClickPos,
		BPoint			inDragPos ) const;

		// Make a drag op for dragging notes...
	EventOp *CreateDragOp(
		CEventEditor		&editor,
		const Event		&ev,
		short			partCode,
		long				timeDelta,			// The horizontal drag delta
		long				valueDelta ) const;

	EventOp *CreateTimeOp(
		CEventEditor	&editor,			// The editor
		const Event	&ev,				// The clicked event
		short		partCode,			// Part of event clicked
		long			timeDelta,			// The horizontal drag delta
		long			valueDelta ) const;
};

	// Invalidate the event
void CTempoEventHandler::Invalidate(
	CEventEditor		&editor,
	const Event		&ev ) const
{
	CTrackCtlStrip	&rEditor = (CTrackCtlStrip &)editor;

	rEditor.Invalidate( Extent( editor, ev ) );
}

void
CTempoEventHandler::Draw(
	CEventEditor &editor,
	const Event &ev,
	bool shadowed) const
{
	CTrackCtlStrip	&rEditor = (CTrackCtlStrip &)editor;
	char			tempoText[ 64 ];
	float			x, y;
	BBitmap			*icon;
	
	icon = ResourceUtils::LoadImage("SmallClock");
	
	font_height fh;
	be_plain_font->GetHeight(&fh);
	
	sprintf( tempoText, "Tempo: %.2f", (float)ev.tempo.newTempo / 1000.0 );

	BRect			r;
	r.left		= editor.TimeToViewCoords( ev.Start() );
	r.right	= editor.TimeToViewCoords( ev.Stop()  ) - 1.0;
	r.top		= rEditor.VPosToViewCoords( ev.repeat.vPos ) + 1.0;
	r.bottom	= r.top + rEditor.BarHeight() - 2;

	if (shadowed && rEditor.m_dragOp != NULL)
		rEditor.SetDrawingMode(B_OP_BLEND);
	else
		rEditor.SetDrawingMode(B_OP_OVER);
	
	// A test -- don't show the existing event if we're adjusting via slider.
	if (!shadowed && rEditor.PendingOperation() != NULL)
		return;

	if (ev.IsSelected() && editor.IsSelectionVisible())
	{
		if (shadowed)
			rEditor.SetHighColor( 0, 0, 0 );
		else
			rEditor.SetHighColor( 0, 0, 255 );
	}
	else
		rEditor.SetHighColor( 192, 192, 192 );

	rEditor.FillRect( BRect( r.left, r.top, r.right, r.top + 2.0 ) );

	rEditor.DrawBitmapAsync( icon, BPoint( r.left, (r.top + r.bottom - 7.0)/2.0 ) );

	x = r.left + 11.0;
	y = (r.top + 3.0 + r.bottom - fh.descent + fh.ascent + 1.0) / 2;
	
	rEditor.SetHighColor( 0, 0, 0 );
	rEditor.DrawString( tempoText, BPoint( x, y ) );
}

	// Compute the extent of the event.
BRect CTempoEventHandler::Extent(
	CEventEditor		&editor,
	const Event		&ev ) const
{
	CTrackCtlStrip	&rEditor = (CTrackCtlStrip &)editor;
	BRect			r;
	BBitmap			*metro;
	BRect			metroRect;
	
//	metro = ResourceUtils::LoadImage("ProgramTool");
	metro = ResourceUtils::LoadImage("MetroTool");
	metroRect = metro->Bounds();
	
	r.left		= editor.TimeToViewCoords( ev.Start() );
	r.right	= editor.TimeToViewCoords( ev.Stop()  ) + 1.0;
	r.top		= rEditor.VPosToViewCoords( ev.repeat.vPos ) + 1.0;
	r.bottom	= r.top + rEditor.BarHeight() - 2;
	
		// REM: This constant, 80.0, is based on string width and should be precomputed
	float	tr = r.left + 80.0;
	if (r.right < tr) r.right = tr;

	if (r.Height() < metroRect.Height())
	{
		r.top -= (metroRect.Height() - r.Height()) / 2;
		r.bottom = r.top + metroRect.Height();
	}

	return r;
}

	// Pick a single event and return the part code
	// (or -1 if event not picked)
long CTempoEventHandler::Pick(
	CEventEditor	&editor,
	const Event		&ev,
	BPoint			pickPt,
	short			&partCode ) const
{
	CTrackCtlStrip	&rEditor = (CTrackCtlStrip &)editor;
	BRect			r;
	int32			result;

	r.top	= rEditor.VPosToViewCoords( ev.tempo.vPos ) + 1.0;
	r.bottom	= r.top + rEditor.BarHeight() - 2;

	result = editor.PickDurationEvent( ev, r.top, r.bottom, pickPt, partCode );
	if (result == LONG_MAX)
	{
		float left = rEditor.TimeToViewCoords( ev.Start()  );
		float right = rEditor.TimeToViewCoords( ev.Stop()  );
		
		if (pickPt.y > r.top
		 && pickPt.y < r.bottom
		 && pickPt.x > right
		 && pickPt.x < left + 80.0)
		{
			partCode = 0;
			result = 0;
		}
	}
	return result;
}

const uint8 *CTempoEventHandler::CursorImage( short partCode ) const
{
	switch (partCode) {
	case 0:
		return B_HAND_CURSOR;			// Return the normal hand cursor

	case 1:								// Return resizing cursor
		if (resizeCursor == NULL)
		{
			resizeCursor = ResourceUtils::LoadCursor(2);
		}
		return resizeCursor;
	}
	
	return NULL;
}

	// Quantize the vertical position of the mouse based
	// on the event type and return a value delta.
long CTempoEventHandler::QuantizeDragValue(
	CEventEditor	&editor,
	const Event		&inClickEvent,
	short			partCode,			// Part of event clicked
	BPoint			inClickPos,
	BPoint			inDragPos ) const
{
	CTrackCtlStrip	&tEditor = (CTrackCtlStrip &)editor;

		// Get the y position of the old note.
	long			oldPos	= inClickEvent.repeat.vPos;
	float		oldYPos	= tEditor.VPosToViewCoords( oldPos );
	long			newPos;

	newPos = tEditor.ViewCoordsToVPos(
		oldYPos + inDragPos.y - inClickPos.y + tEditor.BarHeight() / 2, false );

	return newPos - oldPos;
}

EventOp *CTempoEventHandler::CreateDragOp(
	CEventEditor	&editor,
	const Event		&ev,
	short			partCode,
	long				timeDelta,			// The horizontal drag delta
	long				valueDelta ) const
{
	if (partCode == 0)
		return new VPosOffsetOp( valueDelta );
	else return NULL;
}

EventOp *CTempoEventHandler::CreateTimeOp(
	CEventEditor	&editor,
	const Event		&ev,
	short			partCode,
	long			timeDelta,			// The horizontal drag delta
	long			valueDelta ) const
{
	if (partCode == 1)
		return new DurationOffsetOp( timeDelta );
	else return CAbstractEventHandler::CreateTimeOp( editor, ev, partCode, timeDelta, valueDelta );
}

// ---------------------------------------------------------------------------
// Dispatch table for Track Control editor ~~~EVENTLIST

CRepeatEventHandler		repeatEventHandler;
CSequenceEventHandler		sequenceEventHandler;
CTimeSigEventHandler		timeSigEventHandler;
CProgramChangeEventHandler programChangeEventHandler;
CTempoEventHandler 		tempoEventHandler;

// ---------------------------------------------------------------------------
// Linear Editor class

	// ---------- Constructor

CTrackCtlStrip::CTrackCtlStrip(
	BLooper	&looper,
	CTrackEditFrame	&frame,
	BRect rect,
	CEventTrack *track,
	char *name)
	:	CEventEditor(looper, frame, rect, track, name, true, true),
		m_barHeight(16)
{
	SetHandlerFor(EvtType_End, &gEndEventHandler);
	SetHandlerFor(EvtType_ProgramChange, &programChangeEventHandler);
	SetHandlerFor(EvtType_Repeat, &repeatEventHandler);
	SetHandlerFor(EvtType_Sequence, &sequenceEventHandler);
	SetHandlerFor(EvtType_TimeSig, &timeSigEventHandler);
	SetHandlerFor(EvtType_Tempo, &tempoEventHandler);

	CalcZoom();
	SetZoomTarget((CObserver *)this);

		// Make the label view on the left-hand side
	SetLabelView(new CStripLabelView(BRect(-1.0, 0.0, 20.0, rect.Height()),
									 name, B_FOLLOW_TOP_BOTTOM,
									 B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE));

	SetFlags(Flags() | B_PULSE_NEEDED);
}

float
CTrackCtlStrip::VPosToViewCoords(
	int32 pos) const
{
	return BarHeight() * pos;
}

int32
CTrackCtlStrip::ViewCoordsToVPos(
	float y,
	bool limit) const
{
	int32 pos = static_cast<int32>(y / BarHeight());

	if (limit)
	{
		if (pos < 0)
			return 0;
		if (pos > 127)
			return 127;
	}
	return pos;
}

void
CTrackCtlStrip::Draw( BRect updateRect )
{
	long				startTime = ViewCoordsToTime( updateRect.left - 128.0 ),
					stopTime  = ViewCoordsToTime( updateRect.right + 1.0 );

	SetHighColor( 255, 255, 255 );
	FillRect( updateRect );

	DrawGridLines( updateRect );

		// Initialize an event marker for this track.
	StSubjectLock		trackLock( *Track(), Lock_Shared );
	EventMarker		marker( Track()->Events() );

	bounds = Bounds();

		// For each event that overlaps the current view, draw it. (locked channels first)
	for (	const Event *ev = marker.FirstItemInRange( startTime, stopTime );
			ev;
			ev = marker.NextItemInRange( startTime, stopTime ) )
	{
		if (Track()->IsChannelLocked( *ev ))
			Handler( *ev ).Draw( *this, *ev, false );
	}
	
		// For each event that overlaps the current view, draw it. (unlocked channels overdraw!)
	for (	const Event *ev = marker.FirstItemInRange( startTime, stopTime );
			ev;
			ev = marker.NextItemInRange( startTime, stopTime ) )
	{
		if (!Track()->IsChannelLocked( *ev ))
			Handler( *ev ).Draw( *this, *ev, false );
	}
	
	EventOp	*echoOp = PendingOperation();
	if (echoOp == NULL) echoOp = DragOperation();

	if (m_dragType == DragType_DropTarget)
	{
		DrawCreateEcho( startTime, stopTime );
	}
	else if (IsSelectionVisible())
	{	
		if (m_dragType == DragType_Create)
			DrawCreateEcho( startTime, stopTime );
		else if (echoOp != NULL)
			DrawEchoEvents( startTime, stopTime );
		else if (m_dragType == DragType_Select)
			DrawSelectRect();
		else if (m_dragType == DragType_Lasso)
			DrawLasso();
	}

	DrawPlaybackMarkers(m_pbMarkers, m_pbCount, updateRect, false);
}

	// REM: Here's where we process both new events and playback markers

void CTrackCtlStrip::Pulse()
{
	UpdatePBMarkers();
}

void
CTrackCtlStrip::OnUpdate(
	BMessage *message)
{
	PRINT(("CTrackCtlStrip::OnUpdate()\n"));
	message->PrintToStream();

	BRect r(Bounds());
	bounds = r;

	bool selChange = false;
	if (message->FindBool("SelChange", 0, &selChange) == B_OK)
	{
		if (!IsSelectionVisible())
			return;
	}

	int32 trackHint = 0;
	if (message->FindInt32("TrackAttrs", 0, &trackHint) == B_OK)
	{
		if (!(trackHint & (CTrack::Update_Duration | CTrack::Update_SigMap |
						   CTrack::Update_TempoMap | CTrack::Update_Name)))
			return;
	}

	int32 minTime = 0;
	if (message->FindInt32("MinTime", 0, &minTime) == B_OK)
	{
		r.left = TimeToViewCoords(minTime) - 1.0;
	}

	int32 maxTime = LONG_MAX;
	if (message->FindInt32("MaxTime", 0, &maxTime) == B_OK)
	{
		r.right = TimeToViewCoords(maxTime) + 1.0;
	}
	
	if (trackHint & CTrack::Update_Duration)
		RecalcScrollRangeH();

	uint8 channel;
	if (trackHint & (CTrack::Update_SigMap | CTrack::Update_TempoMap))
	{
		// Invalidate everything if signature map changed
		Invalidate();
	}
	else if (trackHint & CTrack::Update_Name)
	{
		int32 trackID;
		if (message->FindInt32("TrackID", 0, &trackID) != B_OK)
			return;

		StSubjectLock trackLock(*Track(), Lock_Shared);
		EventMarker	marker(Track()->Events());

		// redraw every instance of the changed track
		for (const Event *ev = marker.FirstItemInRange(minTime, maxTime);
			 ev;
			 ev = marker.NextItemInRange(minTime, maxTime))
		{
			if ((ev->Command() == EvtType_Sequence)
			 && (ev->sequence.sequence == trackID))
			{
				Handler(*ev ).Invalidate(*this, *ev);
			}
		}
	}
	else if (message->FindInt8("channel", 0, (int8 *)&channel) == B_OK)
	{
		StSubjectLock trackLock(*Track(), Lock_Shared);
		EventMarker	marker(Track()->Events());

		// For each event that overlaps the current view, draw it.
		for (const Event *ev = marker.FirstItemInRange(minTime, maxTime);
			 ev;
			 ev = marker.NextItemInRange(minTime, maxTime))
		{
			if ((ev->HasProperty(Event::Prop_Channel))
			 && (ev->GetVChannel() == channel))
			{
				Handler(*ev ).Invalidate(*this, *ev);
			}
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
			Handler(*ev).Invalidate(*this, *ev);
		}
	}
	else
	{
		StSubjectLock trackLock(*Track(), Lock_Shared);
		EventMarker marker(Track()->Events());

		// Funny bit of code here. Because of the fact that track control
		// strips have a lot of funky event types who's graphical size bears
		// little relation to their duration, we need to insure that the entirety
		// of these events get included in the damage region. This is in addition
		// to invalidating the entire damage region, since there are also cases
		// where simply invalidating the events isn't enough.
		for (const Event *ev = marker.FirstItemInRange(minTime, maxTime);
			 ev;
			 ev = marker.NextItemInRange(minTime, maxTime))
		{
			Handler(*ev).Invalidate(*this, *ev);
		}
		Invalidate(r);
	}
}

void
CTrackCtlStrip::CalcZoom()
{
	m_stripLogicalHeight = static_cast<int32>(BarHeight()) * 64 - 1;
}

void
CTrackCtlStrip::AttachedToWindow()
{
	SetViewColor( B_TRANSPARENT_32_BIT );
	SetScrollRange(scrollRange.x, scrollValue.x, m_stripLogicalHeight, 0.0);

	SetFont(be_plain_font);
}

void
CTrackCtlStrip::MessageReceived(
	BMessage *message)
{
	switch (message->what)
	{
		case ZoomOut_ID:
		{	
			if (BarHeight() < 32)
			{
				m_barHeight++;
				CalcZoom();
				Hide();
				SetScrollRange(scrollRange.x, scrollValue.x,
							   m_stripLogicalHeight, scrollValue.y);
				Show();
			}
			break;
		}
		case ZoomIn_ID:
		{
			if (BarHeight() > 10)
			{
				m_barHeight--;
				CalcZoom();
				Hide();
				SetScrollRange(scrollRange.x, scrollValue.x,
							   m_stripLogicalHeight, scrollValue.y);
				Show();
			}
			break;
		}	
		case MeVDragMsg_ID:
		{
			if (m_dragType == DragType_DropTarget)
			{
				// Initialize an event marker for this track.
				StSubjectLock trackLock(*Track(), Lock_Exclusive);
				long prevTrackDuration = Track()->LastEventTime();
					
				// Creating a new event
				Track()->DeselectAll(this);
				Handler(m_newEv).Invalidate(*this, m_newEv);
				Track()->CreateEvent(this, m_newEv, "Create Event");
	
				if (prevTrackDuration != Track()->LastEventTime())
					RecalcScrollRangeH();
			}
			else
			{
				BPoint point;
				ulong buttons;
				int32 evtType;

				if (message->FindInt32("EventType", 0, &evtType) != B_OK)
					break;
				GetMouse(&point, &buttons, true);

				if (ConstructEvent(point, (TEventType)evtType) == false)
					return;
				
				// Initialize an event marker for this track.
				StSubjectLock trackLock(*Track(), Lock_Exclusive);

				// Invalidate the new event and insert it into the track.
				Track()->DeselectAll(this);
				Handler(m_newEv).Invalidate(*this, m_newEv);
				Track()->CreateEvent(this, m_newEv, "Create Event");
			}

			m_dragType = DragType_None;
			Window()->Activate();
			break;
		}	
		case Update_ID:
		case Delete_ID:
		{
			CObserver::MessageReceived(message);
			break;
		}
		default:
		{
			CStripView::MessageReceived(message);
		}
	}
}

// ---------------------------------------------------------------------------
// Linear editor mouse movement handler

void CTrackCtlStrip::MouseMoved(
	BPoint			point,
	ulong			transit,
	const BMessage	*dragMsg )
{
	CEventEditor::MouseMoved(point, transit, dragMsg);

	const Event		*ev;
	short			partCode;
	const uint8		*newCursor;

	if (transit == B_EXITED_VIEW)
	{
		if (m_dragType == DragType_DropTarget)
		{
			Handler( m_newEv ).Invalidate( *this, m_newEv );
			m_dragType = DragType_None;
		}
	
		TrackWindow()->DisplayMouseTime( NULL, 0 );
//		be_app->SetCursor(B_CURSOR_SYSTEM_DEFAULT);
		return;
	}
	
	StSubjectLock		trackLock( *Track(), Lock_Shared );
	EventMarker		marker( Track()->Events() );

	TrackWindow()->DisplayMouseTime( Track(), ViewCoordsToTime( point.x ) );
	bounds = Bounds();
	
		// If there's a drag message, and we're not already doing another kind of
		// dragging...
	if (		dragMsg != NULL
		&&	(m_dragType == DragType_None || m_dragType == DragType_DropTarget))
	{
		int32			msgType;

			// Check the message type to see if the message is acceptable.

		if (dragMsg->what == MeVDragMsg_ID
			&&	dragMsg->FindInt32( "Type", 0, &msgType ) == B_OK)
		{
			switch (msgType) {
			case DragTrack_ID:
				int32		trackID;
				void			*dragDoc;
				
				if (		dragMsg->FindInt32( "TrackID", 0, &trackID ) == B_OK
					&&	dragMsg->FindPointer( "Document", 0, &dragDoc ) == B_OK
					&&	dragDoc == TrackWindow()->Document())
				{
					int32		time;
					Event		dragEv;
					CTrack		*tk;
		
						// Initialize a new event.
					dragEv.SetCommand( EvtType_Sequence );
					time = Handler( dragEv ).QuantizeDragTime(
						*this,
						dragEv,
						0,
						BPoint( 16.0, 0.0 ),
						point,
						true );
					if (time < 0) time = 0;
					dragEv.SetStart( time );
					dragEv.SetVChannel( 0 );
					dragEv.sequence.vPos = static_cast<uint8>(point.y / BarHeight());
							// Rem: Change this to the logical length of the track we are ADDING. */
					dragEv.sequence.transposition	= TrackWindow()->Document()->GetDefaultAttribute( EvAttr_Transposition );
					dragEv.sequence.sequence		= trackID;
					tk = TrackWindow()->Document()->FindTrack(trackID);
					if (tk == NULL) tk = Track();
					dragEv.SetDuration( tk->LogicalLength() );
					
					if (		m_dragType != DragType_DropTarget
						||	memcmp( &dragEv, &m_newEv, sizeof m_newEv ) != 0)
					{
						if (m_dragType == DragType_DropTarget)
							Handler( m_newEv ).Invalidate( *this, m_newEv );
						m_newEv = dragEv;
						Handler( m_newEv ).Invalidate( *this, m_newEv );

						TrackWindow()->DisplayMouseTime( Track(), time );
						m_dragType = DragType_DropTarget;
					}
//					be_app->HideCursor();
					return;
				}
				break;
			}
		}

		newCursor = B_HAND_CURSOR;
	}
	else
	{
		if ((ev = PickEvent( marker, point, partCode )) != NULL)
		{
			newCursor = Handler( *ev ).CursorImage( partCode );
		}
		else newCursor = NULL;
	
		if (newCursor == NULL)
		{
			if (crossCursor == NULL)
			{
				crossCursor = ResourceUtils::LoadCursor(1);
			}

			newCursor = crossCursor;
		}
	}
}

bool
CTrackCtlStrip::ConstructEvent(
	BPoint point)
{
	return ConstructEvent(point,
						  TrackWindow()->GetNewEventType(EvtType_ProgramChange));
}

bool
CTrackCtlStrip::ConstructEvent(
	BPoint point,
	TEventType type)
{
	int32 time;

	// Initialize a new event.
	m_newEv.SetCommand(type);

	// Compute the difference between the original
	// time and the new time we're dragging the events to.
	time = Handler(m_newEv).QuantizeDragTime(*this, m_newEv, 0,
											 BPoint(0.0, 0.0), point, true);

	TrackWindow()->DisplayMouseTime(Track(), time);
	m_newEv.SetStart(time);
	m_newEv.SetDuration(TrackWindow()->NewEventDuration());
	m_newEv.SetVChannel(0);

	switch (m_newEv.Command())
	{
		case EvtType_End:
		{
			m_newEv.SetDuration(0);
			break;
		}
		case EvtType_Sequence:
		{
			m_newEv.sequence.vPos = static_cast<uint8>(ViewCoordsToVPos(point.y));
			m_newEv.sequence.transposition = TrackWindow()->Document()->GetDefaultAttribute(EvAttr_Transposition);
			m_newEv.sequence.sequence = TrackWindow()->Document()->GetDefaultAttribute(EvAttr_SequenceNumber);
			m_newEv.sequence.flags = 0;
			CTrack *track = TrackWindow()->Document()->FindTrack(m_newEv.sequence.sequence);
			if (track == NULL)
				track = Track();
			m_newEv.SetDuration(track->LogicalLength());
			break;
		}
		case EvtType_TimeSig:
		{
			m_newEv.sigChange.vPos = static_cast<uint8>(ViewCoordsToVPos(point.y));
			m_newEv.sigChange.numerator = TrackWindow()->Document()->GetDefaultAttribute(EvAttr_TSigBeatCount);
			m_newEv.sigChange.denominator = TrackWindow()->Document()->GetDefaultAttribute(EvAttr_TSigBeatSize);
			break;
		}
		case EvtType_Repeat:
		{
			m_newEv.repeat.vPos = static_cast<uint8>(ViewCoordsToVPos(point.y));
			m_newEv.repeat.repeatCount = TrackWindow()->Document()->GetDefaultAttribute(EvAttr_RepeatCount);
			break;
		}
		case EvtType_ProgramChange:
		{
			// check if destination is set
			int32 destination = TrackWindow()->Document()->GetDefaultAttribute(EvAttr_Channel);
			if (TrackWindow()->Document()->GetVChannel(destination) == NULL)
				return false;
			m_newEv.SetVChannel(TrackWindow()->Document()->GetDefaultAttribute( EvAttr_Channel ) );
			m_newEv.programChange.vPos = static_cast<uint8>(ViewCoordsToVPos(point.y));
			m_newEv.programChange.program = TrackWindow()->Document()->GetDefaultAttribute( EvAttr_Program );
			m_newEv.SetAttribute(EvAttr_ProgramBank, TrackWindow()->Document()->GetDefaultAttribute( EvAttr_ProgramBank ) );
			break;
		}
		case EvtType_Tempo:
		{
			m_newEv.SetVChannel(0);
			m_newEv.tempo.vPos = static_cast<uint8>(ViewCoordsToVPos(point.y));
			m_newEv.tempo.newTempo = static_cast<uint32>(CPlayerControl::Tempo(TrackWindow()->Document()) * 1000.0);
			break;
		}
		default:
		{
			return false;
		}
	}

	return true;
}
