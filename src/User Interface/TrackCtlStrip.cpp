/* ===================================================================== *
 * TrackCtlStrip.cpp (MeV/User Interface)
 * ===================================================================== */

#include "TrackCtlStrip.h"
#include "Idents.h"
#include "MeVApp.h"
#include "MeVDoc.h"
#include "PlayerControl.h"
#include "StdEventOps.h"
#include "MidiDeviceInfo.h"
#include "PlayerControl.h"
#include "ResourceUtils.h"

// Gnu C Library
#include <stdio.h>
// Interface Kit
#include <Bitmap.h>

extern const uint8	*resizeCursor,
					*crossCursor;

static rgb_color	greys[] = {	{ 0xff, 0xff, 0xff },
							{ 0xee, 0xee, 0xee },
							{ 0xdd, 0xdd, 0xdd },
							{ 0xcc, 0xcc, 0xcc },
							{ 0xbb, 0xbb, 0xbb },
							{ 0x77, 0x77, 0x77 } };

static rgb_color	ltBlues[] = {	{ 0xff, 0xff, 0xff },
								{ 0xdd, 0xff, 0xff },
								{ 0xcc, 0xcc, 0xff },
								{ 0xbb, 0xbb, 0xff },
								{ 0x88, 0x88, 0xff },
								{ 0x66, 0x66, 0xff } };

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
	r.bottom	= r.top + rEditor.barHeight - 2;

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
	r.bottom	= r.top + rEditor.barHeight - 2;

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
	r.bottom	= r.top + rEditor.barHeight - 2;

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
		oldYPos + inDragPos.y - inClickPos.y + tEditor.barHeight / 2, false );

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

class CRepeatEventHandler : public CTrackEventHandler {

		// No constructor

		// Draw the event (or an echo)
	void Draw(			CEventEditor	&editor,
						const Event		&ev,
						bool 			shadowed ) const;
};

	// Draw the event (or an echo)
void CRepeatEventHandler::Draw(
	CEventEditor	&editor,
	const Event		&ev,
	bool 			shadowed ) const
{
	CTrackCtlStrip		&rEditor = (CTrackCtlStrip &)editor;
	BRect			r;
	rgb_color			*grad = greys;

	r.left	= editor.TimeToViewCoords( ev.Start() );
	r.right	= editor.TimeToViewCoords( ev.Stop()  );
	r.top	= rEditor.VPosToViewCoords( ev.repeat.vPos ) + 1.0;
	r.bottom	= r.top + rEditor.barHeight - 2;

	rEditor.SetDrawingMode( B_OP_COPY );

	if (shadowed && rEditor.dragOp != NULL)
	{
		rEditor.SetDrawingMode( B_OP_BLEND );
		grad = ltBlues;
	}
	else if (ev.IsSelected() && editor.IsSelectionVisible())
	{
		grad = ltBlues;
	}
	
	rEditor.SetHighColor( grad[ 5 ] );

	rEditor.StrokeRect( r );
	
	if (r.Width() > 2)
	{
		r.InsetBy( 1.0, 1.0 );
		rEditor.SetHighColor( grad[ 0 ] );
		rEditor.FillRect( BRect( r.left, r.top, r.left, r.bottom ) );
		rEditor.FillRect( BRect( r.left, r.top, r.right, r.top ) );

		rEditor.SetHighColor( grad[ 4 ] );
		rEditor.FillRect( BRect( r.left + 1, r.bottom, r.right, r.bottom ) );
		rEditor.FillRect( BRect( r.right, r.top + 1, r.right, r.bottom ) );
	}

	r.InsetBy( 1.0, 1.0 );
	rEditor.SetHighColor( grad[ shadowed ? 3 : 2 ] );
	rEditor.FillRect( r );
	
	// Now, for the little dots...
	
	if (r.Width() > 6)
	{
		char		text[ 16 ];
		int32	pWidth;
		
		if (ev.repeat.repeatCount == 0)
		{
			strcpy( text, "*" );
		}
		else
		{
			sprintf( text, "%d", ev.repeat.repeatCount );
		}
		
		rEditor.SetFont( be_plain_font );
		rEditor.SetFontSize( 10 );
		pWidth = rEditor.StringWidth( text );

		rEditor.SetHighColor( grad[ 0 ] );
		rEditor.FillRect( BRect( r.left + 1, r.top + 1, r.left + 2, r.top + 2 ) );
		rEditor.FillRect( BRect( r.left + 1, r.bottom - 3, r.left + 2, r.bottom - 2 ) );
		rEditor.FillRect( BRect( r.right - 3, r.top + 1, r.right - 2, r.top + 2 ) );
		rEditor.FillRect( BRect( r.right - 3, r.bottom - 3, r.right - 2, r.bottom - 2 ) );

		rEditor.SetHighColor( grad[ 5 ] );
		rEditor.FillRect( BRect( r.left + 2, r.top + 2, r.left + 3, r.top + 3 ) );
		rEditor.FillRect( BRect( r.left + 2, r.bottom - 2, r.left + 3, r.bottom - 1 ) );
		rEditor.FillRect( BRect( r.right - 2, r.top + 2, r.right - 1, r.top + 3 ) );
		rEditor.FillRect( BRect( r.right - 2, r.bottom - 2, r.right - 1, r.bottom - 1 ) );

		rEditor.SetHighColor( grad[ 2 ] );
		rEditor.FillRect( BRect( r.left + 2, r.top + 2, r.left + 2, r.top + 2 ) );
		rEditor.FillRect( BRect( r.left + 2, r.bottom - 2, r.left + 2, r.bottom - 2 ) );
		rEditor.FillRect( BRect( r.right - 2, r.top + 2, r.right - 2, r.top + 2 ) );
		rEditor.FillRect( BRect( r.right - 2, r.bottom - 2, r.right - 2, r.bottom - 2 ) );

		if (r.Width() > 6 + pWidth && r.Height() >= 6 && !shadowed)
		{
			rEditor.SetDrawingMode( B_OP_OVER );
			rEditor.SetHighColor( 0, 0, 0 );
			rEditor.MovePenTo(	(r.left + r.right - pWidth) / 2,
								(r.top + r.bottom - rEditor.fontSpec.descent + rEditor.fontSpec.ascent) / 2 );
			rEditor.DrawString( text );
		}
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

	// Draw the event (or an echo)
void CSequenceEventHandler::Draw(
	CEventEditor	&editor,
	const Event		&ev,
	bool 			shadowed ) const
{
	CTrackCtlStrip		&rEditor = (CTrackCtlStrip &)editor;
	BRect			r;
	rgb_color			*grad = greys;
	CTrack			*track = editor.Track()->Document().FindTrack( ev.sequence.sequence );

	r.left	= editor.TimeToViewCoords( ev.Start() );
	r.right	= editor.TimeToViewCoords( ev.Stop()  );
	r.top	= rEditor.VPosToViewCoords( ev.repeat.vPos ) + 1.0;
	r.bottom	= r.top + rEditor.barHeight - 2;

	rEditor.SetDrawingMode( B_OP_COPY );

	if (shadowed && rEditor.dragOp != NULL)
	{
		rEditor.SetDrawingMode( B_OP_BLEND );
		grad = ltBlues;
	}
	else if (ev.IsSelected() && editor.IsSelectionVisible())
	{
		grad = ltBlues;
	}
	
	rEditor.SetHighColor( grad[ 5 ] );

	rEditor.StrokeRect( r );
	
	r.InsetBy( 1.0, 1.0 );
	rEditor.SetHighColor( grad[ 0 ] );
	rEditor.FillRect( BRect( r.left, r.top, r.left, r.bottom ) );
	rEditor.FillRect( BRect( r.left, r.top, r.right, r.top ) );

	rEditor.SetHighColor( grad[ 4 ] );
	rEditor.FillRect( BRect( r.left + 1, r.bottom, r.right, r.bottom ) );
	rEditor.FillRect( BRect( r.right, r.top + 1, r.right, r.bottom ) );

	r.InsetBy( 1.0, 1.0 );
	rEditor.SetHighColor( grad[ shadowed ? 3 : 2 ] );
	rEditor.FillRect( r );
	
	if (track != NULL)
	{
		int32	l = track->LogicalLength();
		for (int32 t = ev.Start() + l; t < ev.Stop(); t += l)
		{
			float	p = editor.TimeToViewCoords( t );
		
			if (p <= r.left + 4) break;

			rEditor.SetHighColor( grad[ 4 ] );
			rEditor.FillRect( BRect( p - 1, r.top, p - 1, r.bottom ) );
	
			rEditor.SetHighColor( grad[ 0 ] );
			rEditor.FillRect( BRect( p, r.top, p, r.bottom ) );
		}
	}
	
	char		text[ 80 ],
			ttext[ 80 ];
	const char	*t1 = text;
	char		*t2 = ttext;
		
	if (track)
	{
		if (ev.sequence.transposition != 0)
			sprintf( text, "%s [%d]", track->Name(), ev.sequence.transposition );
		else strcpy( text, track->Name() );
	}
	else strcpy( text, "### Error Invalid Track" );

	rEditor.SetFont( be_plain_font );
	rEditor.SetFontSize( 10 );

	be_plain_font->GetTruncatedStrings(
		&t1, 1, B_TRUNCATE_END, r.Width() - 8.0, &t2 );

	if (r.Height() >= 6)
	{
		if (!shadowed) rEditor.SetDrawingMode( B_OP_OVER );
		if (track == NULL)	rEditor.SetHighColor( 255, 0, 0 );
		else					rEditor.SetHighColor( 0, 0, 0 );
		rEditor.MovePenTo(	r.left + 4,
							(r.top + r.bottom - rEditor.fontSpec.descent + rEditor.fontSpec.ascent) / 2 );
		rEditor.DrawString( ttext );
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

	// Draw the event (or an echo)
void CTimeSigEventHandler::Draw(
	CEventEditor	&editor,
	const Event		&ev,
	bool 			shadowed ) const
{
	CTrackCtlStrip	&rEditor = (CTrackCtlStrip &)editor;
	BRect			r;
	rgb_color		*grad = greys;
	int32			pWidth;
	char				text[ 32 ];

	rEditor.SetFont( be_plain_font );
	rEditor.SetFontSize( 10 );

	sprintf( text, "%d/%d", ev.sigChange.numerator, 1 << (ev.sigChange.denominator) );
	pWidth = rEditor.StringWidth( text );

	r.left	= editor.TimeToViewCoords( ev.Start() );
	r.right	= r.left + pWidth + 2;
	r.top	= rEditor.VPosToViewCoords( ev.repeat.vPos ) + 1.0;
	r.bottom	= r.top + rEditor.barHeight - 2;

	rEditor.SetDrawingMode( B_OP_OVER );

	if (shadowed)
	{
		rEditor.SetHighColor( 128, 0, 128 );
		rEditor.SetDrawingMode( B_OP_BLEND );
		grad = ltBlues;
	}
	else if (ev.IsSelected() && editor.IsSelectionVisible())
	{
		rEditor.SetHighColor( 64, 64, 255 );
		grad = ltBlues;
	}
	else
	{
		rEditor.SetHighColor( 0, 0, 0 );
	}
	

	rEditor.MovePenTo(	r.left + 1,
						(r.top + r.bottom - rEditor.fontSpec.descent + rEditor.fontSpec.ascent) / 2 );
	rEditor.DrawString( text );
}

	// Compute the extent of the event.
BRect CTimeSigEventHandler::Extent(
	CEventEditor		&editor,
	const Event		&ev ) const
{
	CTrackCtlStrip	&rEditor = (CTrackCtlStrip &)editor;
	BRect			r;
	int32			pWidth;
	char				text[ 32 ];

	rEditor.SetFont( be_plain_font );
	rEditor.SetFontSize( 10 );

	sprintf( text, "%d/%d", ev.sigChange.numerator, 1 << (ev.sigChange.denominator) );
	pWidth = rEditor.StringWidth( text );

	r.left	= editor.TimeToViewCoords( ev.Start() );
	r.right	= r.left + pWidth + 2;
	r.top	= rEditor.VPosToViewCoords( ev.repeat.vPos ) + 1.0;
	r.bottom	= r.top + rEditor.barHeight - 2;

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
	
	if (r.Contains( pickPt ))
	{
		partCode = 0;
		return abs( (r.top + r.bottom) - pickPt.y*2 );
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
		oldYPos + inDragPos.y - inClickPos.y + tEditor.barHeight / 2, false );

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

	// Draw the event (or an echo)
void CProgramChangeEventHandler::Draw(
	CEventEditor		&editor,
	const Event		&ev,
	bool 			shadowed ) const
{
	CTrackCtlStrip		&rEditor = (CTrackCtlStrip &)editor;
	VChannelEntry		&vce = editor.Track()->Document().GetVChannel( ev.GetVChannel() );
	bool				locked = 	editor.Track()->IsChannelLocked( ev.GetVChannel() );
	char				patchNameBuf[ 64 ];
	const char		*patchName;
	int32			x, y;
//	int32			radius = rEditor.barHeight / 2 - 1;
	BBitmap			*horn;
	BRect			hornRect;
	
	horn = ResourceUtils::LoadImage("ProgramTool");
	hornRect = horn->Bounds();
	
	patchName = GetPatchName( editor, ev, patchNameBuf );

	rEditor.SetFont( be_plain_font );
	rEditor.SetFontSize( 10 );

	BRect			r;
	r.left		= editor.TimeToViewCoords( ev.Start() );
	r.right	= r.left + rEditor.StringWidth( patchName ) + 9.0 + hornRect.Width();
	r.top		= rEditor.VPosToViewCoords( ev.repeat.vPos ) + 1.0;
	r.bottom	= r.top + rEditor.barHeight - 2;
	
		// Reduce size of rectangle a bit.
	if (r.Height() > rEditor.fontSpec.ascent + rEditor.fontSpec.descent + 2)
	{
		r.top += 1.0;
		r.bottom -= 1.0;
	}

	if (shadowed && rEditor.dragOp != NULL) rEditor.SetDrawingMode( B_OP_BLEND );
	else rEditor.SetDrawingMode( B_OP_OVER );
	
		// A test -- don't show the existing event if we're adjusting via slider.
	if (!shadowed && rEditor.PendingOperation() != NULL) return;

	rEditor.DrawBitmapAsync( horn, BPoint( r.left, (r.top + r.bottom - hornRect.Height())/2 ) );
	
	r.left += hornRect.Width() + 3.0;

	x	= r.left + 3.0;
	y	= (r.top + r.bottom - rEditor.fontSpec.descent + rEditor.fontSpec.ascent + 1.0) / 2;
	
	if (locked)
	{
		rEditor.SetHighColor( 128, 128, 128 );
		rEditor.StrokeRect( r );
		r.InsetBy( 1, 1 );
		rEditor.SetHighColor( 192, 192, 192 );
		rEditor.FillRect( r );
	}
	else
	{
		rEditor.SetHighColor( vce.fillColor );
		rEditor.FillRect( r );

		if (ev.IsSelected() && editor.IsSelectionVisible())
		{
			rEditor.SetHighColor( 0, 0, 255 );
			rEditor.StrokeRect( r );
		}
	}

	rEditor.SetHighColor( 0, 0, 0 );
	rEditor.DrawString( patchName, BPoint( x, y ) );
	rEditor.SetDrawingMode( B_OP_COPY );
}

	// Compute the extent of the event.
BRect CProgramChangeEventHandler::Extent(
	CEventEditor		&editor,
	const Event		&ev ) const
{
	CTrackCtlStrip		&rEditor = (CTrackCtlStrip &)editor;
	VChannelEntry		&vce = editor.Track()->Document().GetVChannel( ev.GetVChannel() );
	BRect			r;
	char				patchNameBuf[ 64 ];
	const char		*patchName;
	BBitmap			*horn;
	BRect			hornRect;
	
	horn = ResourceUtils::LoadImage("ProgramTool");
	hornRect = horn->Bounds();
	
	patchName = GetPatchName( editor, ev, patchNameBuf );

	int32			pWidth;
	static int32		maxPWidth = 0;

	rEditor.SetFont( be_plain_font );
	rEditor.SetFontSize( 10 );

	pWidth = rEditor.StringWidth( patchName );
	if (pWidth > maxPWidth) maxPWidth = pWidth;

	r.left		= editor.TimeToViewCoords( ev.Start() );
	r.right	= r.left + maxPWidth + 9.0 + hornRect.Width();
	r.top		= rEditor.VPosToViewCoords( ev.repeat.vPos ) + 1.0;
	r.bottom	= r.top + rEditor.barHeight - 2;
	
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
		return abs( (r.top + r.bottom) - pickPt.y*2 );
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
		oldYPos + inDragPos.y - inClickPos.y + tEditor.barHeight / 2, false );

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
	VChannelEntry		&vc = doc.GetVChannel( ev.GetVChannel() );
	MIDIDeviceInfo	*instrument;
	PatchInfo			*patch;

	instrument = ((CMeVApp *)be_app)->LookupInstrument( vc.port, vc.channel - 1 );
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

	// Draw the event (or an echo)
void CTempoEventHandler::Draw(
	CEventEditor		&editor,
	const Event		&ev,
	bool 			shadowed ) const
{
	CTrackCtlStrip		&rEditor = (CTrackCtlStrip &)editor;
	VChannelEntry		&vce = editor.Track()->Document().GetVChannel( ev.GetVChannel() );
	char				tempoText[ 64 ];
	int32			x, y;
	int32			radius = rEditor.barHeight / 2 - 1;
	BBitmap			*icon;
	
	icon = ResourceUtils::LoadImage("SmallClock");
	
	rEditor.SetFont( be_plain_font );
	rEditor.SetFontSize( 10 );
	
	sprintf( tempoText, "Tempo: %.2f", (float)ev.tempo.newTempo / 1000.0 );

	BRect			r;
	r.left		= editor.TimeToViewCoords( ev.Start() );
	r.right	= editor.TimeToViewCoords( ev.Stop()  ) - 1.0;
	r.top		= rEditor.VPosToViewCoords( ev.repeat.vPos ) + 1.0;
	r.bottom	= r.top + rEditor.barHeight - 2;

	if (shadowed && rEditor.dragOp != NULL) rEditor.SetDrawingMode( B_OP_BLEND );
	else rEditor.SetDrawingMode( B_OP_OVER );
	
		// A test -- don't show the existing event if we're adjusting via slider.
	if (!shadowed && rEditor.PendingOperation() != NULL) return;

	if (ev.IsSelected() && editor.IsSelectionVisible())
	{
		if (shadowed) rEditor.SetHighColor( 0, 0, 0 );
		else rEditor.SetHighColor( 0, 0, 255 );
	}
	else rEditor.SetHighColor( 192, 192, 192 );

	rEditor.FillRect( BRect( r.left, r.top, r.right, r.top + 2.0 ) );

	rEditor.DrawBitmapAsync( icon, BPoint( r.left, (r.top + r.bottom - 7.0)/2.0 ) );

	x = r.left + 11.0;
	y = (r.top + 3.0 + r.bottom - rEditor.fontSpec.descent + rEditor.fontSpec.ascent + 1.0) / 2;
	
	rEditor.SetHighColor( 0, 0, 0 );
	rEditor.DrawString( tempoText, BPoint( x, y ) );
	rEditor.SetDrawingMode( B_OP_COPY );
}

	// Compute the extent of the event.
BRect CTempoEventHandler::Extent(
	CEventEditor		&editor,
	const Event		&ev ) const
{
	CTrackCtlStrip		&rEditor = (CTrackCtlStrip &)editor;
	VChannelEntry		&vce = editor.Track()->Document().GetVChannel( ev.GetVChannel() );
	BRect			r;
	BBitmap			*metro;
	BRect			metroRect;
	
//	metro = ResourceUtils::LoadImage("ProgramTool");
	metro = ResourceUtils::LoadImage("MetroTool");
	metroRect = metro->Bounds();
	
	rEditor.SetFont( be_plain_font );
	rEditor.SetFontSize( 10 );

	r.left		= editor.TimeToViewCoords( ev.Start() );
	r.right	= editor.TimeToViewCoords( ev.Stop()  ) + 1.0;
	r.top		= rEditor.VPosToViewCoords( ev.repeat.vPos ) + 1.0;
	r.bottom	= r.top + rEditor.barHeight - 2;
	
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
	r.bottom	= r.top + rEditor.barHeight - 2;

	result = editor.PickDurationEvent( ev, r.top, r.bottom, pickPt, partCode );
	if (result == LONG_MAX)
	{
		long			left = rEditor.TimeToViewCoords( ev.Start()  );
		long			right = rEditor.TimeToViewCoords( ev.Stop()  );
		
		if (	pickPt.y > r.top
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
		oldYPos + inDragPos.y - inClickPos.y + tEditor.barHeight / 2, false );

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
	BLooper			&inLooper,
	CTrackEditFrame	&inFrame,
	BRect			rect,
	CEventTrack		*inTrack,
	char				*inName )
	:	CEventEditor(	inLooper, inFrame, rect, inTrack, inName, true, true )
{
	handlers[ EvtType_End ]				= &gEndEventHandler;
	handlers[ EvtType_ProgramChange ]	= &programChangeEventHandler;
	handlers[ EvtType_Repeat ]			= &repeatEventHandler;
	handlers[ EvtType_Sequence ]		= &sequenceEventHandler;
	handlers[ EvtType_TimeSig ]			= &timeSigEventHandler;
	handlers[ EvtType_Tempo]			= &tempoEventHandler;

	barHeight		= 16;
	CalcZoom();
	SetZoomTarget( (CObserver *)this );

		// Make the label view on the left-hand side
	labelView = new CLabelView(	BRect(	- 1.0,
										- 1.0,
										20.0,
										rect.Height() + 1 ),
								inName,
								B_FOLLOW_TOP_BOTTOM,
								B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE );

	ResizeBy( -21.0, 0.0 );
	MoveBy( 21.0, 0.0 );
	TopView()->AddChild( labelView );

	SetFlags( Flags() | B_PULSE_NEEDED );
}

// ---------------------------------------------------------------------------
// Convert a position-value to a y-coordinate

long CTrackCtlStrip::VPosToViewCoords( int pos )
{
	return barHeight * pos;
}

// ---------------------------------------------------------------------------
// Convert a y-coordinate to a position value

long CTrackCtlStrip::ViewCoordsToVPos( int yPos, bool limit )
{
	int				pos = yPos / barHeight;
	
	if (limit)
	{
		if (pos < 0) return 0;
		if (pos > 127) return 127;
	}
	return pos;
}

void CTrackCtlStrip::Draw( BRect updateRect )
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
	if (echoOp == NULL) echoOp = dragOp;

	if (dragType == DragType_DropTarget)
	{
		DrawCreateEcho( startTime, stopTime );
	}
	else if (IsSelectionVisible())
	{	
		if (dragType == DragType_Create)	DrawCreateEcho( startTime, stopTime );
		else if (echoOp != NULL)			DrawEchoEvents( startTime, stopTime );
		else if (dragType == DragType_Select)	DrawSelectRect();
		else if (dragType == DragType_Lasso)	DrawLasso();
	}

	DrawPBMarkers( pbMarkers, pbCount, updateRect, false );
}

	// REM: Here's where we process both new events and playback markers

void CTrackCtlStrip::Pulse()
{
	UpdatePBMarkers();
}

// ---------------------------------------------------------------------------
// Update message from another observer

void CTrackCtlStrip::OnUpdate( BMessage *inMsg )
{
	int32		minTime = 0,
				maxTime = LONG_MAX;
	int32		trackHint;
	bool			flag;
	bool			selChange = false;
	int8			channel = -1;
	BRect		r( Bounds() );
	
	bounds = Bounds();

	if (inMsg->FindBool( "SelChange", 0, &flag ) == B_OK)
	{
		if (!IsSelectionVisible()) return;
		selChange = flag;
	}

	if (inMsg->FindInt32( "TrackAttrs", 0, &trackHint ) == B_OK)
	{
			// REM: what do we do if track changes name?
	
		if (!(trackHint &
			(CTrack::Update_Duration|CTrack::Update_SigMap|CTrack::Update_TempoMap)))
				return;
	}
	else trackHint = 0;

	if (inMsg->FindInt32( "MinTime", 0, &minTime ) == B_OK)
	{
		r.left = TimeToViewCoords( minTime ) - 1.0;
	}
	else minTime = 0;

	if (inMsg->FindInt32( "MaxTime", 0, &maxTime ) == B_OK)
	{
		r.right = TimeToViewCoords( maxTime ) + 1.0;
	}
	else maxTime = LONG_MAX;
	
	if (inMsg->FindInt8( "channel", 0, &channel ) != B_OK) channel = -1;

	if (trackHint & CTrack::Update_Duration) RecalcScrollRangeH();

	if (trackHint & (CTrack::Update_SigMap|CTrack::Update_TempoMap))
	{
// 	TrackWindow()->InvalidateRuler();
		Invalidate();			// Invalidate everything if signature map changed
	}
	else if (channel >= 0)
	{
		StSubjectLock		trackLock( *Track(), Lock_Shared );
		EventMarker		marker( Track()->Events() );

			// For each event that overlaps the current view, draw it.
		for (	const Event *ev = marker.FirstItemInRange( minTime, maxTime );
				ev;
				ev = marker.NextItemInRange( minTime, maxTime ) )
		{
			if (ev->HasProperty( Event::Prop_Channel ) && ev->GetVChannel() == channel)
				Handler( *ev ).Invalidate( *this, *ev );
		}
	}
	else if (selChange)
	{
		StSubjectLock		trackLock( *Track(), Lock_Shared );
		EventMarker		marker( Track()->Events() );

			// For each event that overlaps the current view, draw it.
		for (	const Event *ev = marker.FirstItemInRange( minTime, maxTime );
				ev;
				ev = marker.NextItemInRange( minTime, maxTime ) )
		{
			Handler( *ev ).Invalidate( *this, *ev );
		}
	}
	else
	{
		StSubjectLock		trackLock( *Track(), Lock_Shared );
		EventMarker		marker( Track()->Events() );

			// Funny bit of code here. Because of the fact that track control
			// strips have a lot of funky event types who's graphical size bears
			// little relation to their duration, we need to insure that the entirety
			// of these events get included in the damage region. This is in addition
			// to invalidating the entire damage region, since there are also cases
			// where simply invalidating the events isn't enough.
		for (	const Event *ev = marker.FirstItemInRange( minTime, maxTime );
				ev;
				ev = marker.NextItemInRange( minTime, maxTime ) )
		{
			Handler( *ev ).Invalidate( *this, *ev );
		}

		Invalidate( r );
	}
}

void CTrackCtlStrip::CalcZoom()
{
	stripLogicalHeight = barHeight * 64 - 1;
}

void CTrackCtlStrip::AttachedToWindow()
{
	BRect		r( Frame() );

	SetViewColor( B_TRANSPARENT_32_BIT );
	SetScrollRange(	scrollRange.x, scrollValue.x, stripLogicalHeight, 0.0 );

	SetFont( be_plain_font );
	SetFontSize( 10 );
	GetFontHeight( &fontSpec );
}

void CTrackCtlStrip::MessageReceived( BMessage *msg )
{
	switch (msg->what) {
	case ZoomOut_ID:

		if (barHeight < 32)
		{
			BRect		r( Frame() );

			barHeight++;
			CalcZoom();
			Hide();
			SetScrollRange(	scrollRange.x, scrollValue.x,
							stripLogicalHeight, scrollValue.y );
			Show();
		}
		break;

	case ZoomIn_ID:

		if (barHeight > 10)
		{
			BRect		r( Frame() );

			barHeight--;
			CalcZoom();
			Hide();
			SetScrollRange(	scrollRange.x, scrollValue.x,
							stripLogicalHeight, scrollValue.y );
			Show();
		}
		break;
		
	case MeVDragMsg_ID:
	
		if (dragType == DragType_DropTarget)
		{
				// Initialize an event marker for this track.
			StSubjectLock		trackLock( *Track(), Lock_Exclusive );
			long				prevTrackDuration = Track()->LastEventTime();
				
				// Creating a new event
			Track()->DeselectAll( this );
			Handler( newEv ).Invalidate( *this, newEv );
			Track()->CreateEvent( this, newEv, "Create Event" );

			if (prevTrackDuration != Track()->LastEventTime())
				RecalcScrollRangeH();
		}
		else
		{
			BPoint		point;
			ulong		buttons;
			int32		evtType;
			CMeVDoc		&doc = Document();

			if (msg->FindInt32( "EventType", 0, &evtType ) != B_OK) break;
			GetMouse( &point, &buttons, TRUE );
			
			if (ConstructEvent( point, (TEventType)evtType ) == false) return;
			
				// Initialize an event marker for this track.
			StSubjectLock		trackLock( *Track(), Lock_Exclusive );

				// Invalidate the new event and insert it into the track.
			Track()->DeselectAll( this );
			Handler( newEv ).Invalidate( *this, newEv );
			Track()->CreateEvent( this, newEv, "Create Event" );
		}

		dragType = DragType_None;
		TrackWindow()->RestoreCursor();
		Window()->Activate();
		break;

	case Update_ID:
	case Delete_ID:
		CObserver::MessageReceived( msg );
		break;

	default:
		CStripView::MessageReceived( msg );
		break;
	}
}

// ---------------------------------------------------------------------------
// Linear editor mouse movement handler

void CTrackCtlStrip::MouseMoved(
	BPoint			point,
	ulong			transit,
	const BMessage	*dragMsg )
{
	const Event		*ev;
	short			partCode;
	const uint8		*newCursor;

	if (transit == B_EXITED_VIEW)
	{
		if (dragType == DragType_DropTarget)
		{
			Handler( newEv ).Invalidate( *this, newEv );
			dragType = DragType_None;
		}
	
		TrackWindow()->DisplayMouseTime( NULL, 0 );
		TrackWindow()->RestoreCursor();
		return;
	}
	
	StSubjectLock		trackLock( *Track(), Lock_Shared );
	EventMarker		marker( Track()->Events() );

	TrackWindow()->DisplayMouseTime( Track(), ViewCoordsToTime( point.x ) );
	bounds = Bounds();
	
		// If there's a drag message, and we're not already doing another kind of
		// dragging...
	if (		dragMsg != NULL
		&&	(dragType == DragType_None || dragType == DragType_DropTarget))
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
					&&	dragDoc == &Document())
				{
					int32		time;
					CMeVDoc		&doc = Document();
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
					dragEv.sequence.vPos = point.y / barHeight;
							// Rem: Change this to the logical length of the track we are ADDING. */
					dragEv.sequence.transposition	= doc.GetDefaultAttribute( EvAttr_Transposition );
					dragEv.sequence.sequence		= trackID;
					tk = doc.FindTrack( trackID );
					if (tk == NULL) tk = Track();
					dragEv.SetDuration( tk->LogicalLength() );
					
					if (		dragType != DragType_DropTarget
						||	memcmp( &dragEv, &newEv, sizeof newEv ) != 0)
					{
						if (dragType == DragType_DropTarget)
							Handler( newEv ).Invalidate( *this, newEv );
						newEv = dragEv;
						Handler( newEv ).Invalidate( *this, newEv );

						TrackWindow()->DisplayMouseTime( Track(), time );
						dragType = DragType_DropTarget;
					}
					TrackWindow()->HideCursor();
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
				size_t		size;
				crossCursor = ResourceUtils::LoadCursor(1);
			}

			newCursor = crossCursor;
		}
	}
	
	TrackWindow()->SetCursor( newCursor );
}

bool CTrackCtlStrip::ConstructEvent( BPoint point )
{
	return ConstructEvent(	point,
						TrackWindow()->GetNewEventType( EvtType_ProgramChange ) );
}

bool CTrackCtlStrip::ConstructEvent( BPoint point, TEventType inType )
{
	int32		time;
	CMeVDoc		&doc = Document();
	CTrack		*tk;

		// Initialize a new event.
	newEv.SetCommand( inType );
	
	// Compute the difference between the original
	// time and the new time we're dragging the events to.
	time = Handler( newEv ).QuantizeDragTime(
		*this,
		newEv,
		0,
		BPoint( 0.0, 0.0 ),
		point,
		true );

	TrackWindow()->DisplayMouseTime( Track(), time );
	newEv.SetStart( time );
	newEv.SetDuration( TrackWindow()->GetNewEventDuration() );
	newEv.SetVChannel( 0 );

	switch (newEv.Command()) {
	case EvtType_End:
		newEv.SetDuration( 0 );
		break;

	case EvtType_Sequence:
		newEv.sequence.vPos = point.y / barHeight;
		newEv.sequence.transposition	= doc.GetDefaultAttribute( EvAttr_Transposition );
		newEv.sequence.sequence		= doc.GetDefaultAttribute( EvAttr_SequenceNumber );
		newEv.sequence.flags			= 0;
		tk = doc.FindTrack( newEv.sequence.sequence );
		if (tk == NULL) tk = Track();
		newEv.SetDuration( tk->LogicalLength() );
		break;

	case EvtType_TimeSig:
		newEv.sigChange.vPos = point.y / barHeight;
		newEv.sigChange.numerator		= doc.GetDefaultAttribute( EvAttr_TSigBeatCount );
		newEv.sigChange.denominator	= doc.GetDefaultAttribute( EvAttr_TSigBeatSize );
		break;

	case EvtType_Repeat:
		newEv.repeat.vPos = point.y / barHeight;
		newEv.repeat.repeatCount		= doc.GetDefaultAttribute( EvAttr_RepeatCount );
		break;

	case EvtType_ProgramChange:
		newEv.SetVChannel( doc.GetDefaultAttribute( EvAttr_Channel ) );
		newEv.programChange.vPos		= point.y / barHeight;
		newEv.programChange.program	= doc.GetDefaultAttribute( EvAttr_Program );
		newEv.SetAttribute( EvAttr_ProgramBank, doc.GetDefaultAttribute( EvAttr_ProgramBank ) );
		break;

	case EvtType_Tempo:
		newEv.SetVChannel( 0 );
		newEv.tempo.vPos		= point.y / barHeight;
		newEv.tempo.newTempo	= CPlayerControl::Tempo( &doc ) * 1000.0;
		break;

	default:
		return false;
	};

	return true;
}
