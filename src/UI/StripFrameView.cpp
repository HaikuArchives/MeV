/* ===================================================================== *
 * StripFrameView.cpp (MeV/User Interface)
 * ===================================================================== */

#include "StripFrameView.h"

#include "StripSplitter.h"
#include "StripView.h"
#include "StWindowUtils.h"

// Interface Kit
#include <Window.h>
// Support Kit
#include <Autolock.h>
#include <Debug.h>

// Debugging Macros
#define D_INTERNAL(x) //PRINT(x)		// Internal Operations

// ---------------------------------------------------------------------------
// Constructor/Destructor

CStripFrameView::CStripFrameView(
	BRect frame,
	char *name,
	ulong resizingMode)
	:	CScrollerTarget(frame, name, resizingMode,
						B_FRAME_EVENTS),
		m_ruler(NULL)
{
	SetViewColor(B_TRANSPARENT_COLOR);
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
