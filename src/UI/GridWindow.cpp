/* ===================================================================== *
 * GridWindow.cpp (MeV/UI)
 * ===================================================================== */

#include "GridWindow.h"

#include "BorderView.h"
#include "EventTrack.h"
#include "MeVApp.h"
#include "TimeIntervalControl.h"

// Support Kit
#include <Autolock.h>
#include <Debug.h>

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
				   B_WILL_ACCEPT_FIRST_CLICK | B_AVOID_FOCUS
				   | B_NOT_RESIZABLE | B_NOT_ZOOMABLE,
				   B_CURRENT_WORKSPACE),
		m_intervalControl(NULL),
		m_track(NULL)
{
	CBorderView *bgView = new CBorderView(Bounds(), "", B_FOLLOW_NONE,
										  B_WILL_DRAW, false, NULL,
										  CBorderView::BEVEL_BORDER);
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

CGridWindow::~CGridWindow()
{
	if (m_track != NULL)
	{
		m_track->RemoveObserver(this);
		m_track = NULL;
	}
}

// ---------------------------------------------------------------------------
// CObserver Implementation

void
CGridWindow::MessageReceived(
	BMessage *message)
{
	switch(message->what)
	{
		case CMeVApp::WATCH_TRACK:
		{
			CEventTrack *track = NULL;
			if (message->FindPointer("mev:track", (void **)&track) != B_OK)
				return;
			WatchTrack(track);
			break;
		}
		case GRID_INTERVAL_CHANGED:
		{
			if (m_track)
			{
				CWriteLock lock(m_track);
				m_track->SetTimeGridSize(m_intervalControl->Value());
			}
			break;
		}
		default:
		{
			BWindow::MessageReceived(message);
		}
	}
}

bool
CGridWindow::SubjectReleased(
	CObservable *subject)
{
	D_OBSERVE(("CGridWindow<%p>::SubjectReleased()\n", this));

	if (subject == m_track)
	{
		WatchTrack(NULL);
		return true;
	}

	return CAppWindow::SubjectReleased(subject);
}

void
CGridWindow::SubjectUpdated(
	BMessage *message)
{
	if (m_track)
		m_intervalControl->SetValue(m_track->TimeGridSize());
}

// ---------------------------------------------------------------------------
// Operations

void
CGridWindow::WatchTrack(
	CEventTrack *track)
{
	if (track != m_track)
	{
		if (m_track != NULL)
			m_track->RemoveObserver(this);

		m_track = track;
		if (m_track)
		{
			m_track->AddObserver(this);
			m_intervalControl->SetValue(m_track->TimeGridSize());
		}
	}
}

// END - GridWindow.cpp
