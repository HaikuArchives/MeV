/* ===================================================================== *
 * GridWindow.cpp (MeV/User Interface)
 * ===================================================================== */

#include "GridWindow.h"
#include "BorderView.h"
#include "EventTrack.h"
#include "TimeIntervalEditor.h"

// ---------------------------------------------------------------------------
// Constants Initialization

const BRect
CGridWindow::DEFAULT_DIMENSIONS(0.0, 0.0, 210.0, 50.0);

// ---------------------------------------------------------------------------
// Constructor/Destructor

CGridWindow::CGridWindow(
	BPoint position,
	CWindowState &state)
	:	CAppWindow(state,
				   BRect(position.x, position.y,
						 position.x + DEFAULT_DIMENSIONS.Width(),
						 position.y + DEFAULT_DIMENSIONS.Height()),
				   "Grid",
				   B_FLOATING_WINDOW,
				   B_WILL_ACCEPT_FIRST_CLICK | B_NOT_RESIZABLE | B_NOT_ZOOMABLE,
				   B_CURRENT_WORKSPACE),
		CObserver(*this, NULL),
		m_intervalEditor(NULL),
		m_track(NULL)
{
	CBorderView *bgView = new CBorderView(Bounds(), "", B_FOLLOW_ALL_SIDES,
										  B_WILL_DRAW, 0, CBorderView::BEVEL_BORDER);
	AddChild(bgView);

	m_intervalEditor = new CTimeIntervalEditor(Bounds().InsetByCopy(5.0, 5.0),
											   "GridEdit",
											   new BMessage(GRID_INTERVAL_CHANGED));
	bgView->AddChild(m_intervalEditor);
	m_intervalEditor->SetTarget(dynamic_cast<BWindow *>(this));
}

// ---------------------------------------------------------------------------
// CObserver Implementation

void
CGridWindow::MessageReceived(
	BMessage *message)
{
	switch(message->what)
	{
		case GRID_INTERVAL_CHANGED:
		{
			if(m_track)
			{
				m_track->SetTimeGridSize(m_intervalEditor->Value());
			}
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
			BWindow::MessageReceived(message);
		}
	}
}

void
CGridWindow::OnDeleteRequested(
	BMessage *message)
{
	WatchTrack(NULL);
}

void
CGridWindow::OnUpdate(
	BMessage *message)
{
	if(m_track)
	{
		m_intervalEditor->SetValue(m_track->TimeGridSize());
	}
}

// ---------------------------------------------------------------------------
// Operations

void
CGridWindow::WatchTrack(
	CEventTrack *track)
{
	if(track != m_track)
	{
		m_track = track;
		if(m_track && Lock())
		{
			m_intervalEditor->SetValue(m_track->TimeGridSize());
			Unlock();
		}
		SetSubject(m_track);
	}
}

// END - GridWindow.cpp
