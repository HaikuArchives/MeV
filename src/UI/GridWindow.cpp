/* ===================================================================== *
 * GridWindow.cpp (MeV/User Interface)
 * ===================================================================== */

#include "GridWindow.h"
#include "BorderView.h"
#include "EventTrack.h"
#include "TimeIntervalControl.h"
//#include "TimeIntervalEditor.h"

// ---------------------------------------------------------------------------
// Constants Initialization

const BRect
CGridWindow::DEFAULT_DIMENSIONS(0.0, 0.0, 500.0, 100.0);

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
		m_intervalControl(NULL),
		m_track(NULL)
{
	CBorderView *bgView = new CBorderView(Bounds(), "", B_FOLLOW_NONE,
										  B_WILL_DRAW, 0, CBorderView::BEVEL_BORDER);
	AddChild(bgView);

	m_intervalControl = new CTimeIntervalControl(Bounds().InsetByCopy(5.0, 5.0),
												"GridEdit",
												new BMessage(GRID_INTERVAL_CHANGED),
												B_FOLLOW_ALL_SIDES);
	bgView->AddChild(m_intervalControl);

	// resize to preferred size
	float width, height;
	m_intervalControl->GetPreferredSize(&width, &height);
	ResizeTo(width + 10.0, height + 10.0);
	bgView->SetResizingMode(B_FOLLOW_ALL_SIDES);

	m_intervalControl->SetTarget(dynamic_cast<BWindow *>(this));
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
				m_track->SetTimeGridSize(m_intervalControl->Value());
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
		m_intervalControl->SetValue(m_track->TimeGridSize());
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
			m_intervalControl->SetValue(m_track->TimeGridSize());
			Unlock();
		}
		SetSubject(m_track);
	}
}

// END - GridWindow.cpp
