/* ===================================================================== *
 * TrackCtlStrip.cpp (MeV/UI)
 * ===================================================================== */

#include "TrackCtlStrip.h"

#include "CursorCache.h"
#include "MidiDestination.h"
#include "EventTrack.h"
#include "Idents.h"
#include "MeVApp.h"
#include "MeVDoc.h"
#include "MidiDeviceInfo.h"
#include "PlayerControl.h"
#include "ResourceUtils.h"
#include "StdEventOps.h"
#include "StripLabelView.h"

// Gnu C Library
#include <stdio.h>
// Interface Kit
#include <Bitmap.h>
#include <Region.h>
// Support Kit
#include <Debug.h>

// Debugging Macros
#define D_ALLOC(x) //PRINT(x)		// Constructor/Destructor

class CTrackEventRenderer
	:	public CEventRenderer
{

public:							// Constants

	static const rgb_color 		GREY_PALETTE[6];
	
	static const rgb_color 		BLUE_PALETTE[6];

public:							// Constructor/Destructor

								CTrackEventRenderer(
									CEventEditor * const editor)
									:	CEventRenderer(editor)
								{ }

public:							// CEventRenderer Implementation

	/** Invalidate the event. */
	virtual void				Invalidate(
									const Event &ev) const ;

	/** Invalidate the event. */
	virtual BRect				Extent(
									const Event &ev) const;

	/** Pick a single event and returns the distance. */
	virtual long				Pick(
									const Event &ev,
									BPoint pickPt,
									short &partCode) const;

	/** For a part code returned earlier, return a cursor
		image. */
	virtual const BCursor *		Cursor(
									short partCode,
									int32 editMode,
									bool dragging = false) const;

	/** Quantize the vertical position of the mouse based
		on the event type and return a value delta. */
	virtual long				QuantizeDragValue(
									const Event &ev,
									short partCode,
									BPoint clickPos,
									BPoint dragPos) const;

	/** Make a drag op for dragging notes...
		@param timeDelta The horizontal drag delta */
	virtual EventOp *			CreateDragOp(
									const Event &ev,
									short partCode,
									long timeDelta,
									long valueDelta) const;

	/**
	 @param timeDelta The horizontal drag delta
	*/
	virtual EventOp *			CreateTimeOp(
									const Event &ev,
									short partCode,
									long timeDelta,
									long valueDelta) const;

public:							// Accessors

	CTrackCtlStrip * const		Editor() const
								{ return (CTrackCtlStrip *)CEventRenderer::Editor(); }
};

const rgb_color
CTrackEventRenderer::GREY_PALETTE[] =
{
	{0xff, 0xff, 0xff},
	{0xee, 0xee, 0xee},
	{0xdd, 0xdd, 0xdd},
	{0xcc, 0xcc, 0xcc},
	{0xbb, 0xbb, 0xbb},
	{0x77, 0x77, 0x77}
};
	
const rgb_color
CTrackEventRenderer::BLUE_PALETTE[] =
{
	{0xff, 0xff, 0xff},
	{0xdd, 0xff, 0xff},
	{0xcc, 0xcc, 0xff},
	{0xbb, 0xbb, 0xff},
	{0x88, 0x88, 0xff},
	{0x66, 0x66, 0xff}
};

void
CTrackEventRenderer::Invalidate(
	const Event &ev) const
{
	Editor()->Invalidate(Extent(ev));
}

BRect
CTrackEventRenderer::Extent(
	const Event &ev) const
{
	BRect rect;
	rect.left = Editor()->TimeToViewCoords(ev.Start());
	rect.right = Editor()->TimeToViewCoords(ev.Stop());
	rect.top = Editor()->VPosToViewCoords(ev.repeat.vPos) + 1.0;
	rect.bottom = rect.top + Editor()->BarHeight() - 2.0;

	return rect;
}

long
CTrackEventRenderer::Pick(
	const Event &ev,
	BPoint pickPt,
	short &partCode) const
{
	BRect rect;
	rect.top = Editor()->VPosToViewCoords(ev.repeat.vPos) + 1.0;
	rect.bottom = rect.top + Editor()->BarHeight() - 2.0;

	return Editor()->PickDurationEvent(ev, rect.top, rect.bottom,
									   pickPt, partCode);
}

const BCursor *
CTrackEventRenderer::Cursor(
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
CTrackEventRenderer::QuantizeDragValue(
	const Event &ev,
	short partCode,
	BPoint clickPos,
	BPoint dragPos) const
{
	// Get the y position of the old note.
	long oldPos = ev.repeat.vPos;
	float oldYPos = Editor()->VPosToViewCoords(oldPos);
	long newPos = Editor()->ViewCoordsToVPos(oldYPos + dragPos.y - clickPos.y
											 + Editor()->BarHeight() / 2,
											 false);

	return newPos - oldPos;
}

EventOp *
CTrackEventRenderer::CreateDragOp(
	const Event &ev,
	short partCode,
	long timeDelta,
	long valueDelta) const
{
	if (partCode == 0)
		return new VPosOffsetOp(valueDelta);

	return NULL;
}

EventOp *
CTrackEventRenderer::CreateTimeOp(
	const Event &ev,
	short partCode,
	long timeDelta,
	long valueDelta) const
{
	if (partCode == 1)
	{
		return new DurationOffsetOp(timeDelta);
	}

	return CEventRenderer::CreateTimeOp(ev, partCode, timeDelta, valueDelta);
}

// ---------------------------------------------------------------------------
// Event renderer class for repeats

class CRepeatEventRenderer
	:	public CTrackEventRenderer
{

public:							// Constructor/Destructor

								CRepeatEventRenderer(
									CEventEditor * const editor)
									:	CTrackEventRenderer(editor)
								{ }

public:							// CTrackEventRenderer Implementation

	/** Draw the event (or an echo) */
	void						Draw(
									const Event &ev,
									bool shadowed) const;
};

void
CRepeatEventRenderer::Draw(
	const Event &ev,
	bool shadowed) const
{
	BRect r;
	r.left = Editor()->TimeToViewCoords(ev.Start());
	r.right = Editor()->TimeToViewCoords(ev.Stop());
	r.top = Editor()->VPosToViewCoords(ev.repeat.vPos) + 1.0;
	r.bottom = r.top + Editor()->BarHeight() - 2.0;

	const rgb_color *color;
	if (shadowed && (Editor()->DragOperation() != NULL))
	{
		Editor()->SetDrawingMode(B_OP_BLEND);
		color = BLUE_PALETTE;
	}
	else if (ev.IsSelected() && Editor()->IsSelectionVisible())
	{
		Editor()->SetDrawingMode(B_OP_COPY);
		color = BLUE_PALETTE;
	}
	else
	{
		Editor()->SetDrawingMode(B_OP_COPY);
		color = GREY_PALETTE;
	}

	Editor()->BeginLineArray(8);
	Editor()->AddLine(r.RightBottom(), r.LeftBottom(), color[5]);
	Editor()->AddLine(r.RightBottom(), r.RightTop(), color[5]);
	Editor()->AddLine(r.LeftTop(), r.LeftBottom(), color[5]);
	Editor()->AddLine(r.LeftTop(), r.RightTop(), color[5]);
	r.InsetBy(1.0, 1.0);
	Editor()->AddLine(r.RightBottom(), r.LeftBottom(), color[4]);
	Editor()->AddLine(r.RightBottom(), r.RightTop(), color[4]);
	Editor()->AddLine(r.LeftTop(), r.LeftBottom(), color[0]);
	Editor()->AddLine(r.LeftTop(), r.RightTop(), color[0]);
	Editor()->EndLineArray();

	r.InsetBy(1.0, 1.0);
	Editor()->SetHighColor(color[shadowed ? 3 : 2 ]);
	Editor()->FillRect(r);

	// Now, for the little dots...
	if (r.Width() > 6)
	{
		BString repeatText = "";
		if (ev.repeat.repeatCount == 0)
			repeatText << "oo";
		else
			repeatText << ev.repeat.repeatCount;
	
		Editor()->SetDrawingMode(B_OP_OVER);
		if (shadowed)
			Editor()->SetHighColor(128, 128, 128, 255);
		else
			Editor()->SetHighColor(0, 0, 0, 255);
		font_height fh;
		be_plain_font->GetHeight(&fh);
		float textWidth = be_plain_font->StringWidth(repeatText.String());
		Editor()->MovePenTo((r.left + r.right - textWidth) / 2.0,
						 (r.top + r.bottom - fh.descent + fh.ascent) / 2 );
		Editor()->DrawString(repeatText.String());

		BPoint offset;
		Editor()->BeginLineArray(20);
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
			Editor()->AddLine(offset, offset + BPoint(1.0, 0.0), color[0]);
			Editor()->AddLine(offset, offset + BPoint(0.0, 1.0), color[0]);
			offset += BPoint(1.0, 1.0);
			Editor()->AddLine(offset, offset, color[2]);
			offset += BPoint(0.0, 1.0);
			Editor()->AddLine(offset, offset + BPoint(1.0, 0.0), color[5]);
			Editor()->AddLine(offset + BPoint(1.0, 0.0),
							  offset + BPoint(1.0, -1.0), color[5]);
		}
		Editor()->EndLineArray();
	}
}

// ---------------------------------------------------------------------------
// Event renderer class for parts

class CSequenceEventRenderer
	:	public CTrackEventRenderer
{

public:							// Constructor/Destructor

								CSequenceEventRenderer(
									CEventEditor * const editor)
									:	CTrackEventRenderer(editor)
								{ }

public:							// CTrackEventRenderer Implementation

	/** Draw the event (or an echo) */
	void						Draw(
									const Event &ev,
									bool shadowed) const;
};

void
CSequenceEventRenderer::Draw(
	const Event &ev,
	bool shadowed) const
{
	BRect r;
	r.left = Editor()->TimeToViewCoords(ev.Start());
	r.right = Editor()->TimeToViewCoords(ev.Stop());
	r.top = Editor()->VPosToViewCoords(ev.repeat.vPos) + 1.0;
	r.bottom = r.top + Editor()->BarHeight() - 2.0;

	const rgb_color *color;
	if (shadowed && (Editor()->DragOperation() != NULL))
	{
		Editor()->SetDrawingMode(B_OP_BLEND);
		color = BLUE_PALETTE;
	}
	else if (ev.IsSelected() && Editor()->IsSelectionVisible())
	{
		Editor()->SetDrawingMode(B_OP_COPY);
		color = BLUE_PALETTE;
	}
	else
	{
		Editor()->SetDrawingMode(B_OP_COPY);
		color = GREY_PALETTE;
	}

	Editor()->BeginLineArray(8);
	Editor()->AddLine(r.RightBottom(), r.LeftBottom(), color[5]);
	Editor()->AddLine(r.RightBottom(), r.RightTop(), color[5]);
	Editor()->AddLine(r.LeftTop(), r.LeftBottom(), color[5]);
	Editor()->AddLine(r.LeftTop(), r.RightTop(), color[5]);
	r.InsetBy(1.0, 1.0);
	Editor()->AddLine(r.RightBottom(), r.LeftBottom(), color[4]);
	Editor()->AddLine(r.RightBottom(), r.RightTop(), color[4]);
	Editor()->AddLine(r.LeftTop(), r.LeftBottom(), color[0]);
	Editor()->AddLine(r.LeftTop(), r.RightTop(), color[0]);
	Editor()->EndLineArray();

	r.InsetBy(1.0, 1.0);
	Editor()->SetHighColor(color[shadowed ? 3 : 2 ]);
	Editor()->FillRect(r);
	
	CTrack *track = Document()->FindTrack(ev.sequence.sequence);
	if (track != NULL)
	{
		int32 length = track->LogicalLength();
		for (int32 t = ev.Start() + length; t < ev.Stop(); t += length)
		{
			float x = Editor()->TimeToViewCoords(t);
			if (x <= r.left + 4.0)
				break;
			Editor()->SetHighColor(color[4]);
			Editor()->StrokeLine(BPoint(x, r.top), BPoint(x, r.bottom),
								 B_SOLID_HIGH);
			Editor()->SetHighColor(color[0]); 
			x += 1.0;
			Editor()->StrokeLine(BPoint(x, r.top), BPoint(x, r.bottom),
								 B_SOLID_HIGH);
		} 
	} 

	if (r.Width() >= 6.0)
	{
		CTrack *track = Editor()->Track()->Document().FindTrack(ev.sequence.sequence);
		BString trackName;
		if (track)
			trackName = track->Name();
		else
			trackName = "(None)";
	
		be_plain_font->TruncateString(&trackName, B_TRUNCATE_END,
									  r.Width() - 8.0);

		Editor()->SetDrawingMode(B_OP_OVER);
		if ((track == NULL) || shadowed
		 || track->Muted() || track->MutedFromSolo())
			Editor()->SetHighColor(128, 128, 128, 255);
		else
			Editor()->SetHighColor(0, 0, 0, 255);
		font_height fh;
		be_plain_font->GetHeight(&fh);
		Editor()->MovePenTo(r.left + 4,
							(r.top + r.bottom - fh.descent + fh.ascent) / 2 );
		Editor()->DrawString(trackName.String());
	}
}

// ---------------------------------------------------------------------------
// Event renderer class for time signatures

class CTimeSigEventRenderer
	:	public CEventRenderer
{

public:							// Constructor/Destructor

								CTimeSigEventRenderer(
									CEventEditor * const editor)
									:	CEventRenderer(editor)
								{ }

public:							// CEventRenderer Implementation

	// Invalidate the event
	void						Invalidate(
									const Event &ev) const;

	// Draw the event (or an echo)
	void						Draw(
									const Event &ev,
									bool shadowed) const;

	// Invalidate the event
	BRect						Extent(
									const Event &ev) const;

	// Pick a single event and returns the distance.
	long						Pick(
									const Event &ev,
									BPoint pickPt,
									short &partCode) const;

	// Quantize the vertical position of the mouse based
	// on the event type and return a value delta.
	long						QuantizeDragValue(
									const Event &ev,
									short partCode,
									BPoint clickPos,
									BPoint dragPos) const;

	// Make a drag op for dragging notes...
	EventOp *					CreateDragOp(
									const Event &ev,
									short partCode,
									long timeDelta,
									long valueDelta) const;

public:							// Accessors

	CTrackCtlStrip *			Editor() const
								{ return (CTrackCtlStrip *)CEventRenderer::Editor(); }
};

void
CTimeSigEventRenderer::Invalidate(
	const Event &ev) const
{
	Editor()->Invalidate(Extent(ev));
}

void
CTimeSigEventRenderer::Draw(
	const Event &ev,
	bool shadowed) const
{
	BString sigText;
	sigText << ev.sigChange.numerator << "/" << (1 << ev.sigChange.denominator);
	float textWidth = Editor()->StringWidth(sigText.String());

	BRect r;
	r.left = Editor()->TimeToViewCoords(ev.Start());
	r.right	= r.left + textWidth + 2.0;
	r.top = Editor()->VPosToViewCoords(ev.repeat.vPos) + 1.0;
	r.bottom = r.top + Editor()->BarHeight() - 2.0;

	if (shadowed)
	{
		Editor()->SetDrawingMode(B_OP_BLEND);
		Editor()->SetHighColor(128, 0, 128, 255);
	}
	else if (ev.IsSelected() && Editor()->IsSelectionVisible())
	{
		Editor()->SetDrawingMode(B_OP_OVER);
		Editor()->SetHighColor(64, 64, 255, 255);
	}
	else
	{
		Editor()->SetDrawingMode(B_OP_OVER);
		Editor()->SetHighColor(0, 0, 0, 255);
	}

	font_height fh;
	be_plain_font->GetHeight(&fh);
	Editor()->MovePenTo(r.left + 1.0,
						(r.top + r.bottom - fh.descent + fh.ascent) / 2.0);
	Editor()->DrawString(sigText.String());
}

BRect
CTimeSigEventRenderer::Extent(
	const Event &ev) const
{
	char text[32];
	sprintf(text, "%d/%d", ev.sigChange.numerator, 1 << (ev.sigChange.denominator));
	float pWidth = Editor()->StringWidth(text);

	BRect r;
	r.left = Editor()->TimeToViewCoords(ev.Start());
	r.right	= r.left + pWidth + 2;
	r.top = Editor()->VPosToViewCoords(ev.repeat.vPos) + 1.0;
	r.bottom = r.top + Editor()->BarHeight() - 2.0;

	return r;
}

long
CTimeSigEventRenderer::Pick(
	const Event &ev,
	BPoint pickPt,
	short &partCode) const
{
	BRect r(Extent(ev));
	
	if (r.Contains(pickPt))
	{
		partCode = 0;
		return static_cast<long>(fabs((r.top + r.bottom) - pickPt.y * 2));
	}

	return LONG_MAX;
}

long
CTimeSigEventRenderer::QuantizeDragValue(
	const Event &ev,
	short partCode,
	BPoint clickPos,
	BPoint dragPos) const
{
	// Get the y position of the old note.
	long oldPos = ev.repeat.vPos;
	float oldYPos = Editor()->VPosToViewCoords(oldPos);
	long newPos = Editor()->ViewCoordsToVPos(oldYPos + dragPos.y - clickPos.y
											 + Editor()->BarHeight() / 2,
											 false);

	return newPos - oldPos;
}

EventOp *
CTimeSigEventRenderer::CreateDragOp(
	const Event &ev,
	short partCode,
	long timeDelta,
	long valueDelta) const
{
	return new VPosOffsetOp(valueDelta);
}

// ---------------------------------------------------------------------------
// Event renderer class for program changes

class CProgramChangeEventRenderer
	:	public CEventRenderer
{

public:							// Constructor/Destructor

	/** Load the program change icon from resources. */
								CProgramChangeEventRenderer(
									CEventEditor * const editor);

	/** Free the icon. */
	virtual						~CProgramChangeEventRenderer();

public:							// CAbstractEventRenderer Implementation

	/** Invalidate the event. */
	virtual void				Invalidate(
									const Event &ev) const ;

	/** Draw the event (or an echo). */
	virtual void				Draw(
									const Event &ev,
									bool shadowed) const;

	/** Invalidate the event. */
	virtual BRect				Extent(
									const Event &ev) const;

	/** Pick a single event and returns the distance. */
	virtual long				Pick(
									const Event &ev,
									BPoint pickPt,
									short &partCode) const;

	/** Quantize the vertical position of the mouse based
		on the event type and return a value delta. */
	virtual long				QuantizeDragValue(
									const Event &ev,
									short partCode,
									BPoint clickPos,
									BPoint dragPos) const;

	/** Make a drag op for dragging notes...
		@param timeDelta The horizontal drag delta
	*/
	virtual EventOp *			CreateDragOp(
									const Event &ev,
									short partCode,
									long timeDelta,
									long valueDelta) const;

protected:						// Accessors

	CTrackCtlStrip *			Editor() const
								{ return (CTrackCtlStrip *)CEventRenderer::Editor(); }

private:						// Internal Operations

	/** Acquires the patch name of the given program change 
		event and stuffs the name into outName. */
	void						GetProgramName(
									const Event &ev,
									char *outName) const;

private:						// Instance Data

	BBitmap *					m_icon;

	float						m_maxWidth;
};

CProgramChangeEventRenderer::CProgramChangeEventRenderer(
	CEventEditor *editor)
	:	CEventRenderer(editor),
		m_icon(NULL)
{
	m_icon = ResourceUtils::LoadImage("ProgramTool");
	char maxProgramName[PROGRAM_NAME_LENGTH];
	for (int i = 0; i < PROGRAM_NAME_LENGTH; i++)
		maxProgramName[i] = 'M';
	m_maxWidth = m_icon->Bounds().Width() + 4.0;
	m_maxWidth += be_plain_font->StringWidth(maxProgramName);
}

CProgramChangeEventRenderer::~CProgramChangeEventRenderer()
{
	delete m_icon;
}

void
CProgramChangeEventRenderer::Invalidate(
	const Event &ev) const
{
	BRect r;
	r.left = Editor()->TimeToViewCoords(ev.Start());
	r.top = Editor()->VPosToViewCoords(ev.repeat.vPos);
	r.right = r.left + m_maxWidth;

	font_height fh;
	be_plain_font->GetHeight(&fh);
	if ((fh.ascent + fh.descent) > Editor()->BarHeight())
		r.bottom = r.top + fh.ascent + fh.descent;
	if (m_icon->Bounds().Height() > r.Height())
		r.bottom = r.top + m_icon->Bounds().Height();
	r.InsetBy(-1.0, -2.0);

	Editor()->Invalidate(r);
}

void
CProgramChangeEventRenderer::Draw(
	const Event &ev,
	bool shadowed) const
{
	BRect extent;
	extent.left = Editor()->TimeToViewCoords(ev.Start());
	extent.right = Editor()->TimeToViewCoords(ev.Stop()) - 1.0;
	extent.top = Editor()->VPosToViewCoords(ev.repeat.vPos) + 1.0;
	extent.bottom = extent.top + m_icon->Bounds().Height();

	BRect iconRect(extent.LeftTop(),
				   extent.LeftTop() + m_icon->Bounds().RightBottom());

	// acquire patch name
	char programName[PROGRAM_NAME_LENGTH];
	GetProgramName(ev, programName);

	font_height fh;
	be_plain_font->GetHeight(&fh);
	BRect textRect;
	textRect.left = iconRect.right + 4.0;
	textRect.bottom = (extent.top + extent.bottom - fh.descent + fh.ascent)
					  / 2.0;
	textRect.right = textRect.left
					 + be_plain_font->StringWidth(programName);
	textRect.top = textRect.bottom - fh.ascent;

	BRect frameRect(textRect.InsetByCopy(-1.0, -2.0));
	frameRect.top += 1.0;
	frameRect.right -= 1.0;

	CDestination *dest = Document()->FindDestination(ev.GetVChannel());
	rgb_color fillColor = dest->Color();
	if (ev.IsSelected())
		fillColor = tint_color(fillColor, B_DARKEN_2_TINT);
	rgb_color textColor = {0, 0, 0, 255};
	if ((fillColor.red + fillColor.green + fillColor.blue) < 384)
		textColor = tint_color(textColor, B_LIGHTEN_MAX_TINT);

	if (shadowed)
	{
		Editor()->SetDrawingMode(B_OP_ALPHA);
		Editor()->SetHighColor(0, 0, 0, 128);
		Editor()->SetBlendingMode(B_CONSTANT_ALPHA, B_ALPHA_COMPOSITE);
		Editor()->DrawBitmap(m_icon, iconRect.LeftTop());
		Editor()->SetDrawingMode(B_OP_BLEND);
	}
	else if (ev.IsSelected() && Editor()->IsSelectionVisible())
	{
		Editor()->SetDrawingMode(B_OP_INVERT);
		Editor()->DrawBitmap(m_icon, iconRect.LeftTop());
		Editor()->SetDrawingMode(B_OP_ALPHA);
		Editor()->SetHighColor(0, 0, 0, 180);
		Editor()->SetBlendingMode(B_CONSTANT_ALPHA, B_ALPHA_COMPOSITE);
		Editor()->DrawBitmap(m_icon, iconRect.LeftTop());
		Editor()->SetDrawingMode(B_OP_OVER);
	}
	else
	{
		Editor()->SetDrawingMode(B_OP_OVER);
		Editor()->DrawBitmap(m_icon, iconRect.LeftTop());
	}

	Editor()->SetHighColor(fillColor);
	Editor()->FillRect(frameRect);

	Editor()->SetDrawingMode(B_OP_OVER);
	Editor()->SetHighColor(textColor);
	Editor()->DrawString(programName, textRect.LeftBottom());
}

BRect
CProgramChangeEventRenderer::Extent(
	const Event &ev) const
{
	char programName[PROGRAM_NAME_LENGTH];
	GetProgramName(ev, programName);
	float textWidth = be_plain_font->StringWidth(programName);

	BRect r;
	r.left = Editor()->TimeToViewCoords(ev.Start());
	r.top = Editor()->VPosToViewCoords(ev.repeat.vPos);
	r.right = r.left + m_icon->Bounds().Width() + 4.0 + textWidth;

	font_height fh;
	be_plain_font->GetHeight(&fh);
	if ((fh.ascent + fh.descent) > Editor()->BarHeight())
		r.bottom = r.top + fh.ascent + fh.descent;
	if (m_icon->Bounds().Height() > r.Height())
		r.bottom = r.top + m_icon->Bounds().Height();
	r.InsetBy(-1.0, -2.0);

	return r;
}

long
CProgramChangeEventRenderer::Pick(
	const Event &ev,
	BPoint pickPt,
	short &partCode) const
{
	BRect r(Extent(ev));
	
	if (r.Contains(pickPt))
	{
		partCode = 0;
		return static_cast<long>(fabs((r.top + r.bottom) - pickPt.y * 2));
	}

	return LONG_MAX;
}

long
CProgramChangeEventRenderer::QuantizeDragValue(
	const Event &ev,
	short partCode,
	BPoint clickPos,
	BPoint dragPos ) const
{
	// Get the y position of the old note.
	long oldPos = ev.repeat.vPos;
	float oldYPos = Editor()->VPosToViewCoords(oldPos);
	long newPos = Editor()->ViewCoordsToVPos(oldYPos + dragPos.y - clickPos.y 
											 + Editor()->BarHeight() / 2,
											 false);

	return newPos - oldPos;
}

EventOp *
CProgramChangeEventRenderer::CreateDragOp(
	const Event &ev,
	short partCode,
	long timeDelta,
	long valueDelta) const
{
	return new VPosOffsetOp(valueDelta);
}

void
CProgramChangeEventRenderer::GetProgramName(
	const Event &ev,
	char *outName) const
{
	using namespace Midi;
	CMidiDestination *dest = (CMidiDestination *)Document()->FindDestination(ev.GetVChannel());
	uint16 bank = (ev.programChange.bankMSB << 7) | ev.programChange.bankLSB;
	if (!dest->GetProgramName(bank, ev.programChange.program, outName))
		sprintf(outName, "Program %d", ev.programChange.program);
}

// ---------------------------------------------------------------------------
// Event renderer class for tempo events

class CTempoEventRenderer
	:	public CEventRenderer
{

public:							// Constructor/Destructor

	/** Load the program change icon from resources. */
								CTempoEventRenderer(
									CEventEditor * const editor);

	/** Free the icon. */
	virtual						~CTempoEventRenderer();

public:							// CAbstractEventRenderer Implementation

	/** Invalidate the event. */
	virtual void				Invalidate(
									const Event &ev) const ;

	/** Draw the event (or an echo). */
	virtual void				Draw(
									const Event &ev,
									bool shadowed) const;

	/** Invalidate the event. */
	virtual BRect				Extent(
									const Event &ev) const;

	/** Pick a single event and returns the distance. */
	virtual long				Pick(
									const Event &ev,
									BPoint pickPt,
									short &partCode) const;

	/** For a part code returned earlier, return a cursor
		image. */
	virtual const BCursor *		Cursor(
									short partCode,
									int32 editMode,
									bool dragging = false) const;

	/** Quantize the vertical position of the mouse based
		on the event type and return a value delta. */
	virtual long				QuantizeDragValue(
									const Event &inClickEvent,
									short partCode,
									BPoint inClickPos,
									BPoint inDragPos) const;

	/** Make a drag op for dragging notes...
		@param timeDelta The horizontal drag delta */
	virtual EventOp *			CreateDragOp(
									const Event &ev,
									short partCode,
									long timeDelta,
									long valueDelta) const;

	/**
		@param timeDelta The horizontal drag delta
	*/
	virtual EventOp *			CreateTimeOp(
									const Event &ev,
									short partCode,
									long timeDelta,
									long valueDelta) const;

public:							// Accessors

	CTrackCtlStrip *			Editor() const
								{ return (CTrackCtlStrip *)CEventRenderer::Editor(); }

private:						// Instance Data

	BBitmap *					m_icon;
};

CTempoEventRenderer::CTempoEventRenderer(
	CEventEditor *editor)
	:	CEventRenderer(editor),
		m_icon(NULL)
{
	m_icon = ResourceUtils::LoadImage("MetroTool");
}

CTempoEventRenderer::~CTempoEventRenderer()
{
	delete m_icon;
}

void
CTempoEventRenderer::Invalidate(
	const Event &ev) const
{
	Editor()->Invalidate(Extent(ev));
}

void
CTempoEventRenderer::Draw(
	const Event &ev,
	bool shadowed) const
{
	BRect extent;
	extent.left = Editor()->TimeToViewCoords(ev.Start());
	extent.right = Editor()->TimeToViewCoords(ev.Stop()) - 1.0;
	extent.top = Editor()->VPosToViewCoords(ev.repeat.vPos) + 1.0;
	extent.bottom = extent.top + m_icon->Bounds().Height();

	BRect iconRect(extent.LeftTop(),
				   extent.LeftTop() + m_icon->Bounds().RightBottom());

	BString tempoText;
	tempoText << static_cast<float>(ev.tempo.newTempo / 1000.0) << " bpm";

	font_height fh;
	be_plain_font->GetHeight(&fh);
	BRect textRect;
	textRect.left = iconRect.right + 4.0;
	textRect.bottom = (extent.top + extent.bottom - fh.descent + fh.ascent)
					  / 2.0;
	textRect.right = textRect.left
					 + be_plain_font->StringWidth(tempoText.String());
	textRect.top = textRect.bottom - fh.ascent;

	if (shadowed)
	{
		Editor()->SetDrawingMode(B_OP_ALPHA);
		Editor()->SetHighColor(0.0, 0.0, 0.0, 128);
		Editor()->SetBlendingMode(B_CONSTANT_ALPHA, B_ALPHA_COMPOSITE);
		Editor()->DrawBitmap(m_icon, iconRect.LeftTop());
		Editor()->SetHighColor(128, 128, 128, 255);
	}
	else if (ev.IsSelected() && Editor()->IsSelectionVisible())
	{
		Editor()->SetDrawingMode(B_OP_INVERT);
		Editor()->DrawBitmap(m_icon, iconRect.LeftTop());
		Editor()->SetDrawingMode(B_OP_ALPHA);
		Editor()->SetHighColor(0, 0, 0, 180);
		Editor()->SetBlendingMode(B_CONSTANT_ALPHA, B_ALPHA_COMPOSITE);
		Editor()->DrawBitmap(m_icon, iconRect.LeftTop());
		Editor()->SetHighColor(255, 255, 255, 255);
		Editor()->SetLowColor(0, 0, 0, 255);
		Editor()->SetDrawingMode(B_OP_COPY);
		BRect frameRect(textRect.InsetByCopy(-1.0, -2.0));
		frameRect.top += 1.0;
		frameRect.right -= 1.0;
		Editor()->FillRect(frameRect, B_SOLID_LOW);
		Editor()->SetDrawingMode(B_OP_OVER);
	}
	else
	{
		Editor()->SetDrawingMode(B_OP_OVER);
		Editor()->DrawBitmap(m_icon, iconRect.LeftTop());
		Editor()->SetHighColor(0, 0, 0, 255);
	}
	Editor()->DrawString(tempoText.String(), textRect.LeftBottom());
}

BRect
CTempoEventRenderer::Extent(
	const Event &ev) const
{
	BString tempoText;
	tempoText << static_cast<float>(ev.tempo.newTempo / 1000.0) << " bpm";
	float textWidth = be_plain_font->StringWidth(tempoText.String());

	BRect r;
	r.left = Editor()->TimeToViewCoords(ev.Start());
	r.top = Editor()->VPosToViewCoords(ev.repeat.vPos);
	r.right = r.left + m_icon->Bounds().Width() + 4.0 + textWidth;

	font_height fh;
	be_plain_font->GetHeight(&fh);
	if ((fh.ascent + fh.descent) > Editor()->BarHeight())
		r.bottom = r.top + fh.ascent + fh.descent;
	if (m_icon->Bounds().Height() > r.Height())
		r.bottom = r.top + m_icon->Bounds().Height();
	r.InsetBy(-1.0, -2.0);

	return r;
}

long
CTempoEventRenderer::Pick(
	const Event &ev,
	BPoint pickPt,
	short &partCode) const
{
	BRect r;
	r.top = Editor()->VPosToViewCoords(ev.tempo.vPos) + 1.0;
	r.bottom = r.top + Editor()->BarHeight() - 2;

	int32 result = Editor()->PickDurationEvent(ev, r.top, r.bottom, pickPt, partCode);
	if (result == LONG_MAX)
	{
		float left = Editor()->TimeToViewCoords(ev.Start());
		float right = Editor()->TimeToViewCoords(ev.Stop());
		
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

const BCursor *
CTempoEventRenderer::Cursor(
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
CTempoEventRenderer::QuantizeDragValue(
	const Event &ev,
	short partCode,
	BPoint clickPos,
	BPoint dragPos) const
{
	// Get the y position of the old note.
	long oldPos = ev.repeat.vPos;
	float oldYPos = Editor()->VPosToViewCoords(oldPos);
	long newPos = Editor()->ViewCoordsToVPos(oldYPos + dragPos.y - clickPos.y 
										  + Editor()->BarHeight() / 2,
										  false);

	return newPos - oldPos;
}

EventOp *
CTempoEventRenderer::CreateDragOp(
	const Event &ev,
	short partCode,
	long timeDelta,
	long valueDelta) const
{
	if (partCode == 0)
		return new VPosOffsetOp(valueDelta);

	return NULL;
}

EventOp *
CTempoEventRenderer::CreateTimeOp(
	const Event &ev,
	short partCode,
	long timeDelta,
	long valueDelta) const
{
	if (partCode == 1)
	{
		return new DurationOffsetOp(timeDelta);
	}

	return CEventRenderer::CreateTimeOp(ev, partCode, timeDelta, valueDelta);
}

// ---------------------------------------------------------------------------
// Constructor/Destructor

CTrackCtlStrip::CTrackCtlStrip(
	CStripFrameView	&frame,
	BRect rect,
	CEventTrack *track,
	char *name)
	:	CEventEditor(frame, rect, track, name, true, true),
		m_barHeight(16)
{
	SetRendererFor(EvtType_ProgramChange, new CProgramChangeEventRenderer(this));
	SetRendererFor(EvtType_Repeat, new CRepeatEventRenderer(this));
	SetRendererFor(EvtType_Sequence, new CSequenceEventRenderer(this));
	SetRendererFor(EvtType_TimeSig, new CTimeSigEventRenderer(this));
	SetRendererFor(EvtType_Tempo, new CTempoEventRenderer(this));

	CalcZoom();

	// Make the label view on the left-hand side
	SetLabelView(new CStripLabelView(BRect(-1.0, 0.0, 20.0, rect.Height()),
									 name, B_FOLLOW_TOP_BOTTOM,
									 B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE));
}

CTrackCtlStrip::~CTrackCtlStrip()
{
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
CTrackCtlStrip::Draw(
	BRect updateRect)
{
	long startTime = ViewCoordsToTime(updateRect.left - 128.0);
	long stopTime = ViewCoordsToTime(updateRect.right + 1.0);

	SetHighColor(255, 255, 255, 255);
	FillRect(updateRect);

	DrawGridLines(updateRect);

	// Initialize an event marker for this track.
	StSubjectLock trackLock(*Track(), Lock_Shared);
	EventMarker marker(Track()->Events());

	// For each event that overlaps the current view, draw it.
	for (const Event *ev = marker.FirstItemInRange(startTime, stopTime);
		 ev != NULL;
		 ev = marker.NextItemInRange(startTime, stopTime))
	{
		RendererFor(*ev)->Draw(*ev, false);
	}
	
	EventOp	*echoOp = PendingOperation();
	if (echoOp == NULL)
		echoOp = DragOperation();

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
CTrackCtlStrip::CalcZoom()
{
	m_stripLogicalHeight = static_cast<int32>(BarHeight()) * 64 - 1;
}

void
CTrackCtlStrip::AttachedToWindow()
{
	SetViewColor(B_TRANSPARENT_32_BIT);
	SetScrollRange(scrollRange.x, scrollValue.x, m_stripLogicalHeight, 0.0);

	SetFont(be_plain_font);
}

void
CTrackCtlStrip::MessageReceived(
	BMessage *message)
{
	switch (message->what)
	{
		case MeVDragMsg_ID:
		{
			if (m_dragType == DragType_DropTarget)
			{
				// Initialize an event marker for this track.
				StSubjectLock trackLock(*Track(), Lock_Exclusive);
				long prevTrackDuration = Track()->LastEventTime();
					
				// Creating a new event
				Track()->DeselectAll(this);
				RendererFor(m_newEv)->Invalidate(m_newEv);
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

				if (ConstructEvent(point, evtType) == false)
					return;
				
				// Initialize an event marker for this track.
				StSubjectLock trackLock(*Track(), Lock_Exclusive);

				// Invalidate the new event and insert it into the track.
				Track()->DeselectAll(this);
				RendererFor(m_newEv)->Invalidate(m_newEv);
				Track()->CreateEvent(this, m_newEv, "Create Event");
			}

			m_dragType = DragType_None;
			Window()->Activate();
			break;
		}	
		default:
		{
			CEventEditor::MessageReceived(message);
		}
	}
}

void
CTrackCtlStrip::MouseMoved(
	BPoint point,
	uint32 transit,
	const BMessage *message)
{
	CEventEditor::MouseMoved(point, transit, message);

	if (Window()->IsActive())
	{
		if ((transit == B_EXITED_VIEW) || (transit == B_OUTSIDE_VIEW))
		{
			if (m_dragType == DragType_DropTarget)
			{
				RendererFor(m_newEv)->Invalidate(m_newEv);
				m_dragType = DragType_None;
			}
		
			TrackWindow()->SetHorizontalPositionInfo(NULL, 0);
			return;
		}
		else
		{
			TrackWindow()->SetHorizontalPositionInfo(Track(), ViewCoordsToTime(point.x));	
		}
	}

	// If there's a drag message, and we're not already doing another kind of
	// dragging...
	if ((message != NULL)
	 &&	(m_dragType == DragType_None || m_dragType == DragType_DropTarget))
	{
		// Check the message type to see if the message is acceptable.
		int32 msgType;
		if ((message->what == MeVDragMsg_ID)
		 &&	(message->FindInt32("Type", 0, &msgType) == B_OK))
		{
			switch (msgType)
			{
				case DragTrack_ID:
				{
					int32 trackID;
					void *dragDoc;
					if ((message->FindInt32("TrackID", 0, &trackID) == B_OK)
					 && (message->FindPointer("Document", 0, &dragDoc) == B_OK)
					 &&	(dragDoc == TrackWindow()->Document()))
					{
						CTrack *track = TrackWindow()->Document()->FindTrack(trackID);
						if ((track == NULL)
						 || (track->GetID() == Track()->GetID()))
							return;

						// Initialize a new event.
						Event dragEv;
						dragEv.SetCommand(EvtType_Sequence);
						int32 time = RendererFor(dragEv)->QuantizeDragTime(dragEv,
																		  0, BPoint(16.0, 0.0),
																		  point, true);
						if (time < 0)
							time = 0;
						dragEv.SetStart(time);
						dragEv.SetVChannel(0);
						dragEv.sequence.vPos = static_cast<uint8>(point.y / BarHeight());
						dragEv.sequence.transposition = TrackWindow()->Document()->GetDefaultAttribute(EvAttr_Transposition);
						dragEv.sequence.sequence = trackID;
						// Rem: Change this to the logical length of the track we are ADDING. */
						dragEv.SetDuration(track->LogicalLength());
					
						if ((m_dragType != DragType_DropTarget)
						 || (memcmp(&dragEv, &m_newEv, sizeof(m_newEv)) != 0))
						{
							if (m_dragType == DragType_DropTarget)
								RendererFor(m_newEv)->Invalidate(m_newEv);
							m_newEv = dragEv;
							RendererFor(m_newEv)->Invalidate(m_newEv);

							if (Window()->IsActive())
								TrackWindow()->SetHorizontalPositionInfo(Track(),
																		 time);
							m_dragType = DragType_DropTarget;
						}
						return;
					}
					break;
				}
			}
		}
	}
}

void
CTrackCtlStrip::SubjectUpdated(
	BMessage *message)
{
	BRect r(Bounds());

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
						   CTrack::Update_TempoMap | CTrack::Update_Name |
						   CTrack::Update_Flags)))
		{
			CEventEditor::SubjectUpdated(message);
			return;
		}
	}

	int32 minTime;
	if (message->FindInt32("MinTime", 0, &minTime) != B_OK)
		minTime = ViewCoordsToTime(Bounds().left);
	r.left = TimeToViewCoords(minTime) - 1.0;

	int32 maxTime;
	if (message->FindInt32("MaxTime", 0, &maxTime) != B_OK)
		maxTime = ViewCoordsToTime(Bounds().right);
	r.right = TimeToViewCoords(maxTime) + 1.0;

	int32 trackID;
	if ((message->FindInt32("TrackID", 0, &trackID) == B_OK)
	 && (trackID != Track()->GetID()))
	{
		// change on some other track; dispatch change to every track
		// instance in the current bounds
		StSubjectLock trackLock(*Track(), Lock_Shared);
		EventMarker	marker(Track()->Events());
		for (const Event *ev = marker.FirstItemInRange(minTime, maxTime);
			 ev;
			 ev = marker.NextItemInRange(minTime, maxTime))
		{
			if ((ev->Command() == EvtType_Sequence)
			 && (ev->sequence.sequence == trackID))
			{
				RendererFor(*ev)->Invalidate(*ev);
			}
		}

		// nothing else to do with this update
		return;
	}

	if (trackHint & CTrack::Update_Duration)
		RecalcScrollRangeH();

	uint8 channel;
	if (trackHint & (CTrack::Update_SigMap | CTrack::Update_TempoMap))
	{
		// Invalidate everything if signature map changed
		Invalidate();
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
				RendererFor(*ev)->Invalidate(*ev);
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
			RendererFor(*ev)->Invalidate(*ev);
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
			RendererFor(*ev)->Invalidate(*ev);
		}
		Invalidate(r);
	}
}

bool
CTrackCtlStrip::ConstructEvent(
	BPoint point)
{
	return ConstructEvent(point,
						  TrackWindow()->NewEventType(EvtType_Sequence));
}

void
CTrackCtlStrip::ZoomChanged(
	int32 diff)
{
	m_barHeight += 2 * diff;
	if (m_barHeight > 32)
		m_barHeight = 32;
	else if (m_barHeight < 10)
		m_barHeight = 10;

	CalcZoom();
	SetScrollRange(scrollRange.x, scrollValue.x,
				   m_stripLogicalHeight, scrollValue.y);
	Invalidate();
}

bool
CTrackCtlStrip::ConstructEvent(
	BPoint point,
	event_type type)
{
	int32 time;

	// Initialize a new event.
	m_newEv.SetCommand(type);

	// Compute the difference between the original
	// time and the new time we're dragging the events to.
	time = RendererFor(m_newEv)->QuantizeDragTime(m_newEv, 0,
												 BPoint(0.0, 0.0), point, true);

	TrackWindow()->SetHorizontalPositionInfo(Track(), time);
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
			CTrack *track = NULL;
			int32 sequence = TrackWindow()->Document()->GetDefaultAttribute(EvAttr_SequenceNumber);
			if (sequence == Track()->GetID())
			{
				// can't nest the track within itself
				// try the next higher track
				track = TrackWindow()->Document()->FindNextHigherTrackID(sequence);
				if ((track == NULL) || (track->GetID() == sequence))
					return false;
				sequence = track->GetID();
				TrackWindow()->Document()->SetDefaultAttribute(EvAttr_SequenceNumber,
															   sequence);
			}
			else
			{
				track = TrackWindow()->Document()->FindTrack(sequence);
			}
			m_newEv.sequence.vPos = static_cast<uint8>(ViewCoordsToVPos(point.y));
			m_newEv.sequence.transposition = TrackWindow()->Document()->GetDefaultAttribute(EvAttr_Transposition);
			m_newEv.sequence.sequence = sequence;
			m_newEv.sequence.flags = 0;
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
			if (TrackWindow()->Document()->FindDestination(destination) == NULL)
				return false;
			m_newEv.SetVChannel(TrackWindow()->Document()->GetDefaultAttribute( EvAttr_Channel ) );
			m_newEv.programChange.vPos = static_cast<uint8>(ViewCoordsToVPos(point.y));
			m_newEv.programChange.program = TrackWindow()->Document()->GetDefaultAttribute( EvAttr_Program );
			m_newEv.SetAttribute(EvAttr_ProgramBank, TrackWindow()->Document()->GetDefaultAttribute( EvAttr_ProgramBank ) );
			break;
		}
		case EvtType_Tempo:
		{
			if (Track()->GetID() > 1)
				// don't support tempo changes anywhere but in the master 
				// tracks
				return false;
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
