/* ===================================================================== *
 * TrackListItem.cpp (MeV/User Interface)
 * ===================================================================== */

#include "TrackListItem.h"
#include "StringEditView.h"
#include "Track.h"

// Interface Kit
#include <Bitmap.h>
#include <ScrollView.h>
#include <TextView.h>
// Storage Kit
#include <Mime.h>
// Support Kit
#include <Debug.h>

// Debugging Macros
#define D_ALLOC(x) PRINT (x)		// Constructor/Destructor
#define D_ACCESS(x) PRINT (x)		// Accessors
#define D_OPERATION(x) PRINT(x)		// Operations
#define D_HOOK(x) PRINT (x)			// BListItem Implementation
#define D_MESSAGE(x) PRINT (x)		// MessageReceived()

// ---------------------------------------------------------------------------
// Constants

const unsigned char MIDI_TRACK_ICON_BITS [] = {
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xff,
	0xff,0x00,0x3f,0x3f,0x3f,0x3f,0x3f,0x3f,0x3f,0x3f,0x3f,0x3f,0x1b,0x00,0xff,0xff,
	0xff,0x00,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x3f,0x3f,0x3f,0x1b,0x00,0x0f,0xff,
	0xff,0x00,0x0f,0x1b,0x0f,0x00,0x00,0x00,0x00,0x0f,0x1b,0x1b,0x1b,0x00,0x0f,0xff,
	0xff,0x00,0x0f,0x0f,0x00,0x00,0x00,0x00,0x00,0x0f,0x00,0x00,0x00,0x0f,0x0f,0xff,
	0xff,0x00,0x0f,0x00,0x00,0x00,0x00,0x00,0x00,0x0f,0x3f,0x3f,0x1b,0x00,0x0f,0xff,
	0xff,0x00,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x3f,0x3f,0x1b,0x00,0x0f,0xff,
	0xff,0x00,0x0f,0x1b,0x0f,0x00,0x00,0x00,0x00,0x0f,0x1b,0x1b,0x1b,0x00,0x0f,0xff,
	0xff,0x00,0x0f,0x0f,0x00,0x00,0x00,0x00,0x00,0x0f,0x00,0x00,0x00,0x0f,0x0f,0xff,
	0xff,0x00,0x0f,0x00,0x00,0x00,0x00,0x00,0x00,0x0f,0x3f,0x3f,0x1b,0x00,0x0f,0xff,
	0xff,0x00,0x3f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x3f,0x3f,0x1b,0x00,0x0f,0xff,
	0xff,0x00,0x1b,0x1b,0x1b,0x1b,0x1b,0x1b,0x1b,0x1b,0x1b,0x1b,0x1b,0x00,0x0f,0xff,
	0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0f,0x0f,0xff,
	0xff,0xff,0xff,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff
};

// ---------------------------------------------------------------------------
// Constructor/Destructor

CTrackListItem::CTrackListItem(
	CTrack *track)
	:	BListItem(),
		m_track(track),
		m_editing(false)
{
	D_ALLOC(("CTrackListItem::CTrackListItem()\n"));

	BRect rect(0.0, 0.0, B_MINI_ICON - 1.0, B_MINI_ICON - 1.0);
	m_icon = new BBitmap(rect, B_CMAP8);
	m_icon->SetBits(MIDI_TRACK_ICON_BITS, 256, 0, B_CMAP8);
}
	
CTrackListItem::~CTrackListItem()
{
	D_ALLOC(("CTrackListItem::~CTrackListItem()\n"));

}

// ---------------------------------------------------------------------------
// Accessors

BBitmap *
CTrackListItem::GetDragBitmap() const
{
	D_ACCESS(("CTrackListItem::GetDragBitmap()\n"));

	// Prepare Bitmap for Drag & Drop
	BRect rect(0.0, 0.0, Width(), Height());
	BBitmap *dragBitmap = new BBitmap(rect, B_RGBA32, true); 
	if (dragBitmap->Lock())
	{
		BView *dragView = new BView(dragBitmap->Bounds(), "", B_FOLLOW_NONE, 0); 
		dragBitmap->AddChild(dragView); 
		dragView->SetOrigin(0.0, 0.0);
		dragView->SetHighColor(0.0, 0.0, 0.0, 0.0);
		dragView->FillRect(dragView->Bounds()); 
		dragView->SetDrawingMode(B_OP_ALPHA); 
		dragView->SetHighColor(0.0, 0.0, 0.0, 128.0);       
	    dragView->SetBlendingMode(B_CONSTANT_ALPHA, B_ALPHA_COMPOSITE); 
		if (m_icon)
		{
			BRect iconRect = dragView->Bounds();
			iconRect.left += 3.0;
			iconRect.top += (Height() / 2.0) - (B_MINI_ICON / 2.0);
			iconRect.right = iconRect.left + B_MINI_ICON - 1.0;
			iconRect.bottom = iconRect.top + B_MINI_ICON - 1.0;
			dragView->DrawBitmap(m_icon, iconRect.LeftTop());
		}
		BRect labelRect = dragView->Bounds();
		labelRect.left += B_MINI_ICON + 6.0;
		labelRect.top += (Height() / 2.0) - (m_fh.ascent / 2.0);
		labelRect.right = labelRect.left + Width();
		labelRect.bottom = labelRect.top + m_fh.ascent + m_fh.descent;		
		BPoint labelOffset(labelRect.left + 1.0, labelRect.bottom - m_fh.descent);
		dragView->DrawString(m_track->Name(), labelOffset);
		dragView->Sync();
	}
	dragBitmap->Unlock();

	return dragBitmap;
}

int32
CTrackListItem::GetTrackID() const
{
	D_ACCESS(("CTrackListItem::GetTrackID()\n"));

	if (!m_track) {
		return -1;
	}
	
	return m_track->GetID();
}

// ---------------------------------------------------------------------------
// Operations

void
CTrackListItem::StartEdit(
	BView *owner,
	int32 index,
	BRect itemRect)
{
	D_OPERATION(("CTrackListItem::StartEdit()\n"));

	itemRect.left += B_MINI_ICON + 6.0;
	itemRect.top += (itemRect.Height() / 2.0) - (m_fh.ascent / 2.0);

	BMessage *message = new BMessage(TRACK_NAME_EDITED);
	message->AddInt32("index", index);
	BMessenger messenger(owner, owner->Window());
	CStringEditView *view = new CStringEditView(itemRect, m_track->Name(),
												message, messenger);
	owner->AddChild(view);

	m_editing = true;

//	owner->Window()->UpdateIfNeeded();
}

void
CTrackListItem::StopEdit()
{
	D_OPERATION(("CTrackListItem::StopEdit()\n"));

	m_editing = false;
}

// ---------------------------------------------------------------------------
// BListItem Implementation

void
CTrackListItem::DrawItem(
	BView *owner,
	BRect itemRect,
	bool drawEverything)
{
	D_HOOK(("CTrackListItem::DrawItem()\n"));

	itemRect.PrintToStream();
	owner->Bounds().PrintToStream();

	owner->SetDrawingMode(B_OP_OVER);

	// Draw icon
	if (m_icon) {
		BRect rect = itemRect;
		rect.left += 3.0;
		rect.top += (itemRect.Height() / 2.0) - (B_MINI_ICON / 2.0);
		rect.right = rect.left + B_MINI_ICON - 1.0;
		rect.bottom = rect.top + B_MINI_ICON - 1.0;
		if (m_track->Muted() || m_track->MutedFromSolo())
		{
			owner->SetHighColor(255.0, 255.0, 255.0, 255.0);
			owner->FillRect(rect);
			owner->SetDrawingMode(B_OP_ALPHA);
			owner->SetHighColor(0.0, 0.0, 0.0, 150.0);
			owner->SetBlendingMode(B_CONSTANT_ALPHA, B_ALPHA_COMPOSITE);
			owner->DrawBitmap(m_icon, rect.LeftTop());
			owner->SetDrawingMode(B_OP_OVER);
		}
		else if (IsSelected())
		{
			owner->SetHighColor(255.0, 255.0, 255.0, 255.0);
			owner->FillRect(rect);
			owner->SetDrawingMode(B_OP_INVERT);
			owner->DrawBitmap(m_icon, rect.LeftTop());
			owner->SetDrawingMode(B_OP_ALPHA);
			owner->SetHighColor(0.0, 0.0, 0.0, 180.0);
			owner->SetBlendingMode(B_CONSTANT_ALPHA, B_ALPHA_COMPOSITE);
			owner->DrawBitmap(m_icon, rect.LeftTop());
			owner->SetDrawingMode(B_OP_OVER);
		}
		else
		{
			owner->SetDrawingMode(B_OP_OVER);
			owner->DrawBitmap(m_icon, rect.LeftTop());
		}
	}

	if (!m_editing)
	{
		// Draw label
		BRect rect(itemRect);
		rect.OffsetTo(itemRect.LeftTop());
		rect.left += B_MINI_ICON + 6.0;
		rect.top += (itemRect.Height() / 2.0) - (m_fh.ascent / 2.0);
		rect.right = rect.left + be_plain_font->StringWidth(m_track->Name());
		rect.bottom = rect.top + m_fh.ascent + m_fh.descent;		
		if (IsSelected() || drawEverything)
		{
			if (IsSelected())
			{
				if (m_track->Muted() || m_track->MutedFromSolo())
					owner->SetHighColor(128, 128, 128, 255);
				else
					owner->SetHighColor(0, 0, 0, 255);
			}
			else
			{
				owner->SetHighColor(owner->ViewColor());
			}
			owner->FillRect(rect, B_SOLID_HIGH);
		}
		if (m_track->Muted() || m_track->MutedFromSolo())
		{
			if (IsSelected())
				owner->SetHighColor(200, 200, 200, 200);
			else
				owner->SetHighColor(128, 128, 128, 255);
		}
		else
		{
			if (IsSelected())
				owner->SetHighColor(255, 255, 255, 255);
			else
				owner->SetHighColor(0, 0, 0, 0);
		}
		BPoint labelOffset(rect.left + 1.0, rect.bottom - m_fh.descent);
		owner->DrawString(m_track->Name(), labelOffset);
	}
}

void
CTrackListItem::Update(
	BView *owner,
	const BFont *font)
{
	BListItem::Update(owner, font);

	SetWidth(B_MINI_ICON + 6.0 + font->StringWidth(m_track->Name()));
	font->GetHeight(&m_fh);
	float newHeight = m_fh.ascent + m_fh.descent + m_fh.leading;
	if (newHeight < B_MINI_ICON) {
		newHeight = B_MINI_ICON;
	}
	newHeight += 2.0;
	if (Height() < newHeight) {
		SetHeight(newHeight);
	}

	font->GetHeight(&m_fh);
}

// END - TrackListItem.cpp
