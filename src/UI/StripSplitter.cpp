/* ===================================================================== *
 * StripSplitter.cpp (MeV/UI)
 * ===================================================================== */

#include "StripSplitter.h"

#include "StripFrameView.h"
#include "StripView.h"

// Interface Kit
#include <Window.h>
// Support Kit
#include <Debug.h>

// ---------------------------------------------------------------------------
// Constructor/Destructor

CStripSplitter::CStripSplitter(
	BRect frame,
	CStripView *primaryStrip,
	CStripView *secondaryStrip)
	:	CSplitter(frame, primaryStrip->TopView(), secondaryStrip->TopView(),
				  B_HORIZONTAL, B_FOLLOW_LEFT_RIGHT),
		m_primaryStrip(primaryStrip),
		m_secondaryStrip(secondaryStrip)
{
}

CStripSplitter::~CStripSplitter()
{
}

// ---------------------------------------------------------------------------
// Accessors

void
CStripSplitter::SetPrimaryStrip(
	CStripView *strip)
{
	m_primaryStrip = strip;
	CSplitter::SetPrimaryTarget(strip->TopView());
}

void
CStripSplitter::SetSecondaryStrip(
	CStripView *strip)
{
	m_secondaryStrip = strip;
	CSplitter::SetSecondaryTarget(strip->TopView());
}

// ---------------------------------------------------------------------------
// CSplitter Implementation

void
CStripSplitter::MoveRequested(
	float diff)
{
	switch (Posture())
	{
		case B_HORIZONTAL:
		{
			float primaryHeight = PrimaryTarget()->Bounds().Height();
			float secondaryHeight = SecondaryTarget()->Bounds().Height();
			
			if ((primaryHeight < m_primaryStrip->MinimumHeight())
			 || (secondaryHeight < m_secondaryStrip->MinimumHeight()))
			{
				// if either strip is already smaller than its minimal size
				// don't stop the user to enlarge them
				CSplitter::MoveRequested(diff);
			}
			else
			{
				// else make sure the strips don't get smaller than their
				// minimal size
				if (primaryHeight + diff < m_primaryStrip->MinimumHeight())
					diff = m_primaryStrip->MinimumHeight() - primaryHeight;
				else if (secondaryHeight - diff < m_secondaryStrip->MinimumHeight())
					diff = secondaryHeight - m_secondaryStrip->MinimumHeight();
				CSplitter::MoveRequested(diff);
			}

			// notify the frame view
			BMessage message(CStripView::PROPORTIONS_CHANGED);
			Window()->PostMessage(&message, m_primaryStrip->FrameView());
			break;
		}
		case B_VERTICAL:
		{
			// only horizontal strips currently
			break;
		}
	}
}

// END - StripSplitter.cpp
