/* ===================================================================== *
 * StripFrameView.cpp (MeV/UI)
 * ===================================================================== */

#include "StripFrameView.h"

#include "IFFReader.h"
#include "IFFWriter.h"
#include "StripSplitter.h"
#include "StripView.h"
#include "StWindowUtils.h"
#include "Track.h"
#include "TrackWindow.h"

// Interface Kit
#include <Window.h>
// Support Kit
#include <Autolock.h>
#include <Debug.h>

// Debugging Macros
#define D_INTERNAL(x) //PRINT(x)		// Internal Operations

// ---------------------------------------------------------------------------
// Constants

const int32
CStripFrameView::FILE_CHUNK_ID = 'strf';

// ---------------------------------------------------------------------------
// Constructor/Destructor

CStripFrameView::CStripFrameView(
	BRect frame,
	char *name,
	CTrack *track,
	ulong resizingMode)
	:	CScrollerTarget(frame, name, resizingMode,
						B_FRAME_EVENTS),
		m_track(track),
		m_ruler(NULL),
		m_clockType(track->ClockType()),
		m_horizontalZoom(0)
{
	SetViewColor(B_TRANSPARENT_COLOR);

	ZoomBy(16);
}

CStripFrameView::~CStripFrameView()
{
	while (m_strips.CountItems() > 0)
	{
		strip_info *info = (strip_info *)m_strips.RemoveItem((int32)0);
		if (info)
			delete info;
	}
}

// ---------------------------------------------------------------------------
// Accessors

void
CStripFrameView::SetClockType(
	TClockType type)
{
	m_clockType = type;
	Ruler()->Invalidate();
}

// ---------------------------------------------------------------------------
// Operations

void
CStripFrameView::AddType(
	BString name)
{
	strip_type *type = new strip_type;
	type->name = name;
	m_types.AddItem(reinterpret_cast<void *>(type));
}

BString
CStripFrameView::TypeAt(
	int32 index) const
{
	if (index < CountTypes())
	{
		strip_type *type = (strip_type *)m_types.ItemAt(index);
		if (type)
			return type->name;
	}

	return "";
}

bool
CStripFrameView::AddStrip(
	CStripView *view,
	float proportion,
	int32 index,
	bool fixedSize)
{
	// make entry for the strips list
	strip_info *info = new strip_info;
	info->strip = view;
	info->container = view->TopView();
	info->proportion = proportion > 0.0
					   ? proportion
					   : info->container->Bounds().Height() / Bounds().Height();
	info->fixed_size = fixedSize;

	info->container->SetResizingMode(B_FOLLOW_LEFT_RIGHT);

	bool first = m_strips.IsEmpty();

	if (index < 0)
		index = m_strips.CountItems();
	if (!m_strips.AddItem(reinterpret_cast<void *>(info), index))
		return false;
	AddChild(info->container);

	if (!first)
	{
		info->splitter = new CStripSplitter(Bounds(), StripAt(index - 1),
											info->strip);
		AddChild(info->splitter);
	}
	else
	{
		info->splitter = NULL;
	}

	ArrangeViews();

	return true;
}

int32
CStripFrameView::IndexOf(
	CStripView *view) const
{
	for (int32 i = 0; i < CountStrips(); i++)
	{
		strip_info *info = (strip_info *)m_strips.ItemAt(i);
		if (info->strip == view)
			return i;
	}

	return -1;
}

void
CStripFrameView::PackStrips()
{
	float totalProportion = 0.0;
	for (int32 i = 0; i < CountStrips(); i++)
	{
		strip_info *info = (strip_info *)m_strips.ItemAt(i);
		D_INTERNAL((" -> proportion of '%s' = %f\n",
					info->strip->Name(), info->proportion));
		totalProportion += info->proportion;
	}

	D_INTERNAL((" -> total proportion = %f\n", totalProportion));

	if (totalProportion == 1.0)
		return;

	for (int32 i = 0; i < CountStrips(); i++)
	{
		strip_info *info = (strip_info *)m_strips.ItemAt(i);
		if (!info->fixed_size)
		{
			info->proportion = info->proportion / totalProportion;
		}
	}

	ArrangeViews();
}

bool
CStripFrameView::RemoveStrip(
	CStripView *view)
{
	strip_info *previousStrip = NULL;

	// make entry for the strips list
	for (int32 i = 0; i < CountStrips(); i++)
	{
		strip_info *info = (strip_info *)m_strips.ItemAt(i);
		if (info->strip == view)
		{
			if (info->splitter)
			{
				strip_info *nextStrip = (strip_info *)m_strips.ItemAt(i + 1);
				if (nextStrip)
					nextStrip->splitter->SetPrimaryStrip(previousStrip->strip);
				RemoveChild(info->splitter);
			}
			RemoveChild(info->container);
			m_strips.RemoveItem(info);

			UpdateSplitters();
			ArrangeViews();
			return false;
		}
		previousStrip = info;
	}

	return false;
}

CStripView *
CStripFrameView::StripAt(
	int32 index) const
{
	strip_info *info = (strip_info *)m_strips.ItemAt(index);
	if (!info)
		return NULL;

	return info->strip;
}

void
CStripFrameView::SwapStrips(
	CStripView *strip1,
	CStripView *strip2)
{
	m_strips.SwapItems(IndexOf(strip1), IndexOf(strip2));
	UpdateSplitters();
	ArrangeViews();
}

float
CStripFrameView::TimeToViewCoords(
	long time,
	TClockType clockType) const
{
	if (clockType != ClockType())
	{
		const CTempoMap &tMap(Track()->Document().TempoMap());

		if (clockType == ClockType_Metered)
			time = tMap.ConvertMeteredToReal(time);
		else
			time = tMap.ConvertRealToMetered(time);
	}
	return floor(m_pixelsPerTimeUnit * time);
}

long
CStripFrameView::ViewCoordsToTime(
	float x,
	TClockType clockType) const
{
	long time;

	time = static_cast<long>(x / m_pixelsPerTimeUnit);
	if (clockType != ClockType())
	{
		const CTempoMap &tMap(Track()->Document().TempoMap());
		if (clockType == ClockType_Metered)
			time = tMap.ConvertRealToMetered(time);
		else
			time = tMap.ConvertMeteredToReal(time);
	}

	return time;
}

void
CStripFrameView::ZoomBy(
	int32 diff)
{
	m_horizontalZoom += diff;
	if (m_horizontalZoom > 24)
		m_horizontalZoom = 24;
	else if (m_horizontalZoom < 8)
		m_horizontalZoom = 8;

	int32 time = ViewCoordsToTime(scrollValue.x, ClockType());
	m_pixelsPerTimeUnit = (double)(1 << m_horizontalZoom)
						  / (256.0 * 256.0 * 16.0);
	Hide();
	SetScrollRange(TimeToViewCoords(Track()->LastEventTime(),
									ClockType()) + FrameSize().x,
				   TimeToViewCoords(time, ClockType()),
				   0.0, 0.0);
	Show();

	if (Ruler())
		Ruler()->Invalidate();
}

// ---------------------------------------------------------------------------
// Serialization

void
CStripFrameView::ExportSettings(
	BMessage *settings) const
{
	settings->AddInt32("zoom_value", m_horizontalZoom);
	settings->AddFloat("scroll_value", ScrollValue(B_HORIZONTAL));
	for (int32 i = 0; i < CountStrips(); i++)
	{
		strip_info *info = (strip_info *)m_strips.ItemAt(i);
		BMessage stripMsg;
		stripMsg.AddString("type", info->strip->Name());
		stripMsg.AddFloat("proportion", info->proportion);
		stripMsg.AddBool("fixed_size", info->fixed_size);
		info->strip->ExportSettings(&stripMsg);
		settings->AddMessage("strip", &stripMsg);
	}
}

void
CStripFrameView::ImportSettings(
	const BMessage *settings)
{
	CTrackWindow *window = static_cast<CTrackWindow *>(Window());

	int32 zoom;
	settings->FindInt32("zoom_value", &zoom);
	float scroll;
	settings->FindFloat("scroll_value", &scroll);
	BMessage stripMsg;
	int32 i = 0;
	while (settings->FindMessage("strip", i, &stripMsg) == B_OK)
	{
		BString type;
		stripMsg.FindString("type", &type);
		float proportion = 0.3;
		stripMsg.FindFloat("proportion", &proportion);
		if (window->AddStrip(type, proportion))
		{
			strip_info *info = (strip_info *)m_strips.ItemAt(i);
			if (info)
				stripMsg.FindBool("fixed_size", &info->fixed_size);
			info->strip->ImportSettings(&stripMsg);
		}
		i++;
	}

	PackStrips();

	ZoomBy(zoom - ZoomValue());
	SetScrollValue(scroll, B_HORIZONTAL);
}

void
CStripFrameView::ReadState(
	CIFFReader &reader,
	BMessage *settings)
{
	int32 zoom;
	reader >> zoom;
	settings->AddInt32("zoom_value", zoom);
	float scroll;
	reader >> scroll;
	settings->AddFloat("scroll_value", scroll);

	while (reader.BytesAvailable() > 0)
	{
		BMessage stripMsg;

		// read type string
		char buffer[256];
		BString str;
		int32 length = 0;
		reader >> length;
		reader.MustRead(buffer, length);
		buffer[length] = '\0';
		str.SetTo(buffer);
		stripMsg.AddString("type", str);

		// read proportion
		float proportion;
		reader >> proportion;
		stripMsg.AddFloat("proportion", proportion);

		// read fixed_size
		int8 fixedSize;
		reader >> fixedSize;
		stripMsg.AddBool("fixed_size", (bool)fixedSize);

		CStripView::ReadState(reader, &stripMsg);

		settings->AddMessage("strip", &stripMsg);
	}
}

void
CStripFrameView::WriteState(
	CIFFWriter &writer,
	const BMessage *settings)
{
	int32 zoom;
	settings->FindInt32("zoom_value", &zoom);
	writer << zoom;
	float scroll;
	settings->FindFloat("scroll_value", &scroll);
	writer << scroll;

	int32 i = 0;
	BMessage stripMsg;
	while (settings->FindMessage("strip", i, &stripMsg) == B_OK)
	{
		// write type string
		BString str;
		stripMsg.FindString("type", &str);
		writer << str.CountChars();
		char buffer[256];
		str.CopyInto(buffer, 0, str.CountChars());
		buffer[str.CountChars()] = '\0';
		writer.MustWrite(buffer, str.CountChars());

		// write proportion
		float proportion;
		stripMsg.FindFloat("proportion", &proportion);
		writer << proportion;

		// read fixed_size
		int8 fixedSize;
		stripMsg.FindBool("fixed_size", (bool *)&fixedSize);
		writer << fixedSize;

		CStripView::WriteState(writer, &stripMsg);

		i++;
	}
}

// ---------------------------------------------------------------------------
// CScrollerTarget Implementation

void
CStripFrameView::AttachedToWindow()
{
	AdjustScrollers();
	CScrollerTarget::AttachedToWindow();
}

void
CStripFrameView::FrameResized(
	float width,
	float height)
{
	BAutolock autoLock(Window());
	if (!autoLock.IsLocked())
		return;

	RecalcScrollRange();
	ArrangeViews();
	AdjustScrollers();
}

void
CStripFrameView::MessageReceived(
	BMessage *message)
{
	switch (message->what)
	{
		case CStripView::PROPORTIONS_CHANGED:
		{
			UpdateProportions();
			break;
		}
		default:
		{
			CScrollerTarget::MessageReceived(message);
		}
	}
}

void
CStripFrameView::SetScrollValue(
	float position,
	orientation posture)
{
	if (posture == B_HORIZONTAL)
	{
		float scrollDelta = position - scrollValue.x;
		scrollValue.x = position;

		if (m_ruler)
			m_ruler->SetScrollValue(position, B_HORIZONTAL);

		for (int32 i = 0; i < m_strips.CountItems(); i++)
		{
			strip_info *info = (strip_info *)m_strips.ItemAt(i);
			CScrollerTarget	*st = dynamic_cast<CScrollerTarget *>(info->container);
			if (st)
				st->SetScrollValue(position, B_HORIZONTAL);
			else
				info->container->ScrollBy(scrollDelta, 0.0);
		}
	}
}

// ---------------------------------------------------------------------------
// Internal Operations

void
CStripFrameView::ArrangeViews()
{
	D_INTERNAL(("CStripFrameView::ArrangeViews()\n"));

	float width = Bounds().Width();
	float height = Bounds().Height();

	float y = 0.0;
	for (int32 i = 0; i < CountStrips(); i++)
	{
		strip_info *info = (strip_info *)m_strips.ItemAt(i);
		info->height = floor(height * info->proportion);
		info->vertical_offset = y;
		y += info->height + 1.0;
	}

	for (int32 i = 0; i < CountStrips(); i++)
	{
		strip_info *info = (strip_info *)m_strips.ItemAt(i);
		float offset = info->vertical_offset;
		float height = info->height;
		if (info->splitter)
		{
			info->splitter->MoveTo(0.0, offset);
			offset += CSplitter::H_SPLITTER_HEIGHT + 1.0;
			height -= CSplitter::H_SPLITTER_HEIGHT + 1.0;
		}
		info->container->ResizeTo(width, height);
		info->container->MoveTo(0.0, offset);
		if (i == CountStrips() - 1)
		{
			// limit the last strip to the bottom pixel, to 
			float pad = info->container->Frame().bottom
						- Bounds().bottom;
			if (pad != 0.0)
			{
				info->container->ResizeBy(0.0, - pad);
			}
		}
	}
}

void
CStripFrameView::RecalcScrollRange()
{
	SetScrollRange(TimeToViewCoords(Track()->LastEventTime(),
									Track()->ClockType()) + FrameSize().x,
				   scrollValue.x, 0.0, 0.0);
}

void
CStripFrameView::UpdateProportions()
{
	D_INTERNAL(("CStripFrameView::UpdateProportions()\n"));

	float totalHeight = Bounds().Height();
	float totalProportion = 0.0;
	for (int32 i = 0; i < CountStrips(); i++)
	{
		strip_info *info = (strip_info *)m_strips.ItemAt(i);
		if (!info->fixed_size)
		{
			float stripHeight = info->container->Bounds().Height();
			if (info->splitter)
				stripHeight += CSplitter::H_SPLITTER_HEIGHT;
			info->proportion = stripHeight / totalHeight;
			D_INTERNAL((" -> proportion of '%s' = %f\n",
						info->strip->Name(), info->proportion));
			totalProportion += info->proportion;
		}
	}

	D_INTERNAL((" -> total proportion = %f\n", totalProportion));

	if (totalProportion != 1.0)
		return;

	for (int32 i = 0; i < CountStrips(); i++)
	{
		strip_info *info = (strip_info *)m_strips.ItemAt(i);
		if (!info->fixed_size)
		{
			info->proportion = info->proportion / totalProportion;
		}
	}
}

void
CStripFrameView::UpdateSplitters()
{
	strip_info *previousStrip = NULL;
	for (int32 i = 0; i < CountStrips(); i++)
	{
		strip_info *info = (strip_info *)m_strips.ItemAt(i);
		if (info->splitter)
		{
			if (!previousStrip)
			{
				RemoveChild(info->splitter);
				delete info->splitter;
				info->splitter = NULL;
			}
			else
			{
				info->splitter->SetPrimaryStrip(previousStrip->strip);
				info->splitter->SetSecondaryStrip(info->strip);
			}
		}
		else
		{
			if (previousStrip)
			{
				info->splitter = new CStripSplitter(Bounds(),
													previousStrip->strip,
													info->strip);
				AddChild(info->splitter);
			}
		}
		previousStrip = info;
	}
}

// END - StripFrameView.cpp
