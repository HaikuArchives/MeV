/* ===================================================================== *
 * InspectorWindow.cpp (MeV/UI)
 * ===================================================================== */

#include "InspectorWindow.h"

#include "BorderView.h"
#include "DestinationListView.h"
#include "DocWindow.h"
#include "EventOp.h"
#include "EventTrack.h"
#include "MathUtils.h"
#include "MeVDoc.h"
#include "PlayerControl.h"
#include "StWindowUtils.h"
#include "StdEventOps.h"
#include "TextSlider.h"

// Gnu C Library
#include <stdio.h>
// Interface Kit
#include <StringView.h>
// Support Kit
#include <Autolock.h>
#include <Debug.h>

// Debugging Macros
#define D_ALLOC(x) //PRINT(x)		// Constructor/Destructor
#define D_HOOK(x) //PRINT(x)		// CAppWindow Implementation
#define D_MESSAGE(x) //PRINT(x)		// Messaging
#define D_INTERNAL(x) //PRINT(x)	// Internal Operations

// ---------------------------------------------------------------------------
// Constants Initialization

const BRect
CInspectorWindow::DEFAULT_DIMENSIONS(0.0, 0.0, 470.0, 64.0);

const int					channelBoxWidth = 14,
						channelBoxHeight = 14;

	// Default attributes -- where...?
	// 	track, doc, app?
	// attribute table

	// Channel selection
	// Channel locking

enum EInspectorControlIDs {
	Slider1_ID			= 'sld1',
	Slider2_ID			= 'sld2',
	Slider3_ID			= 'sld3',
		EDIT_ID			= 'butt',
	NEW_ID				= 'nwid',
	DELETE_ID			= 'dtid'
};


class CBeatSizeTextHook : public CTextSlider::CTextHook {
public:
		/**	Return the width in pixels of the largest possible knob text. */
	int32 Largest( BView *inView, int32 inMin, int32 inMax )
	{
		return static_cast<int32>(inView->StringWidth("1/64"));
	}

		/**	Format the text for the text slider knob */
	void FormatText( char *outText, int32 inValue, int32 inMaxLen )
	{
		sprintf( outText, "1/%d", 1 << inValue );
	}
};

class CFixedPointHook : public CTextSlider::CTextHook {

	int			ct;
	double		fracScale;
	char			fmt[ 16 ];

public:
	CFixedPointHook( int inInt, int inFrac )
	{
		fracScale = 1.0;
		ct = inFrac + inInt + 1;
		for (int i = 0; i < inFrac; i++) fracScale /= 10.0;
		sprintf( fmt, "%%%d.%df", ct, inFrac );
	}

		/**	Return the width in pixels of the largest possible knob text. */
	int32 Largest( BView *inView, int32 inMin, int32 inMax )
	{
		return static_cast<int32>(inView->StringWidth(".00000000000000000000000000", ct));
	}

		/**	Format the text for the text slider knob */
	void FormatText( char *outText, int32 inValue, int32 inMaxLen )
	{
		sprintf( outText, fmt, (double)inValue * fracScale );
	}
};

static CBeatSizeTextHook	bsHook;
class CFixedPointHook		fpHook22( 3, 3 );
/* static CNoteNameTextHook	nnHook; */

// ---------------------------------------------------------------------------
// Constructor/Destructor

CInspectorWindow::CInspectorWindow(
	BPoint position,
	CWindowState &state)
	:	CAppWindow(state,
				   BRect(position.x, position.y,
						 position.x + DEFAULT_DIMENSIONS.Width(),
						 position.y + DEFAULT_DIMENSIONS.Height()),
						 "Inspector", B_FLOATING_WINDOW,
						 B_WILL_ACCEPT_FIRST_CLICK | B_AVOID_FOCUS
						 | B_NOT_RESIZABLE | B_NOT_ZOOMABLE,
						 B_CURRENT_WORKSPACE),
		m_track(NULL),
		m_previousValue(-1)
{
	D_ALLOC(("CInspectorWindow::CInspectorWindow()\n"));

	CBorderView *bgView = new CBorderView(Bounds(), "", B_FOLLOW_ALL_SIDES,
										  B_WILL_DRAW, false, NULL,
										  CBorderView::BEVEL_BORDER);
	AddChild(bgView);

	for (int i = 0; i < 3; i++)
	{
		static int32 ids[3] = { Slider1_ID, Slider2_ID, Slider3_ID };
		float y = 8.0 + 18.0 * i;
		m_vLabel[i] = new BStringView(BRect(5.0, y, 75.0, y + 13.0), "", "Pitch");
		bgView->AddChild(m_vLabel[i]);
		m_vLabel[i]->SetAlignment(B_ALIGN_RIGHT);
		m_vSlider[i] = new CTextSlider(BRect(80.0, y, 235.0, y + 13.0),
									   new BMessage(ids[i]), "");
		bgView->AddChild(m_vSlider[i]);
		m_vSlider[i]->SetEnabled(false);
	}

	BRect r(240.0, 3.0, 240.0 + channelBoxWidth * 16 + 2.0,
			3.0 + channelBoxHeight * 4 + 2.0);
	m_channelControl = new CDestinationListView(r);
	bgView->AddChild(m_channelControl);

	// REM: Get window position from preferences.
	// REM: When we close inspector, then save position to
	// 		preferences.
	// REM: Also save the fact whether the inspector was open or closed.

	_clear();
}

CInspectorWindow::~CInspectorWindow()
{
	D_ALLOC(("CInspectorWindow::~CInspectorWindow()\n"));

	if (m_track != NULL)
	{
		m_track->RemoveObserver(this);
		m_track = NULL;
	}
}

// ---------------------------------------------------------------------------
// CAppWindow Implementation

void
CInspectorWindow::MessageReceived(
	BMessage *message)
{
	D_MESSAGE(("CInspectorWindow::MessageReceived()\n"));

	switch (message->what)
	{
		case CMeVApp::WATCH_TRACK:
		{
			D_MESSAGE((" -> CMeVApp::WATCH_TRACK\n"));

			CEventTrack *track = NULL;
			if (message->FindPointer("mev:track", (void **)&track) != B_OK)
				return;
			_watchTrack(track);
			break;
		}
		case Slider1_ID:
		case Slider2_ID:
		case Slider3_ID:
		{
			CDocWindow *window;
			bool finalFlag,	cancelFlag;
			int32 val, index = 0;

			switch (message->what)
			{
				case Slider1_ID: index = 0; break;
				case Slider2_ID: index = 1; break;
				case Slider3_ID: index = 2; break;
			}
			
			if (m_editedAttr[index] == EvAttr_None)
			{
				break;
			}
	
			be_app->Lock();
	
			finalFlag		= message->HasBool( "final" );
			cancelFlag	= message->HasBool( "canceled" );
			
			if (m_previousValue == -1)
			{
				m_previousValue = m_baseValue[index];
			}
			
			val = m_vSlider[index]->Value();
			window = CDocWindow::ActiveDocWindow();
	
			if (window && (val != m_previousValue || finalFlag || cancelFlag))
			{
				enum E_EventAttribute attr = m_editedAttr[index];
				BMessage msg('echo');
				msg.AddInt32("attr", attr );
				msg.AddInt32("delta", val - m_baseValue[index]);
				msg.AddInt32("value", val);
	
				if (finalFlag)
				{
					msg.AddBool("final", true);
				}
				if (cancelFlag)
				{
					msg.AddBool("cancel", true);
				}
				if (!cancelFlag
					&& (val != m_previousValue)
					&& gPrefs.FeedbackEnabled(attr, false)
					&& m_track)
				{
					CPlayerControl::DoAudioFeedback(&m_track->Document(),
													attr, val, 
													m_track->CurrentEvent());
					m_track->Document().SetDefaultAttribute(attr, val);
				}
				window->PostMessage(&msg, window);
			}
	
			if (finalFlag || cancelFlag)
			{
				m_previousValue = -1;
			}
			else
			{
				m_previousValue = val;
			}
	
			be_app->Unlock();
			break;
		}
		default:
		{
			CAppWindow::MessageReceived(message);
			break;
		}
	}
}

// ---------------------------------------------------------------------------
// CAppWindow Implementation

bool
CInspectorWindow::SubjectReleased(
	CObservable *subject)
{
	D_HOOK(("CInspectorWindow::SubjectReleased()\n", this));

	if (subject == m_track)
	{
		_clear();
		m_track->RemoveObserver(this);
		m_track = NULL;
		return true;
	}

	return CAppWindow::SubjectReleased(subject);
}

void
CInspectorWindow::SubjectUpdated(
	BMessage *message)
{
	D_HOOK(("CInspectorWindow::SubjectUpdated()\n", this));

	if ((m_track == NULL)
	 || (m_track->SelectionType() == CTrack::Select_None))
	{
		_clear();
	}
	else
	{
		CReadLock lock(m_track);
		const CEvent *event = m_track->CurrentEvent();
		if (event == NULL)
		{
			_clear();
			return;
		}

		D_HOOK((" -> valid selection, current event name: %s\n",
				event->NameText()));

		// Set the event name
		BString title = "Inspector: ";
		title << event->NameText();
		SetTitle(title.String());

		// Set up the data values for all three sliders
		for (int i = 0; i < 3; i++)
		{
			enum E_EventAttribute attrType;
			attrType = event->QueryAttribute(i);
			m_editedAttr[i] = attrType;
			if(attrType == EvAttr_None)
			{
				m_vSlider[i]->SetRange(0.0, 0.0);
				m_vSlider[i]->SetEnabled(false);
				m_vSlider[i]->SetTextHook(NULL);
				m_vLabel[i]->SetText("");
			}
			else
			{
				int32	minVal, maxVal, offset;
				const char
						*text = UEventAttributeTable::Name(attrType),
						*oldText = m_vLabel[i]->Text();
						
				if (text == NULL || oldText == NULL || strcmp(text, oldText) != 0)
				{
					m_vLabel[i]->SetText(UEventAttributeTable::Name(attrType));
				}

				UEventAttributeTable::Range(attrType, minVal, maxVal);
				offset = UEventAttributeTable::Offset(attrType);
				m_vSlider[i]->SetRange(minVal + offset, maxVal + offset);
				m_vSlider[i]->SetEnabled(true);
				m_vSlider[i]->SetBodyIncrement(10);
				
				// Set the text formatting hook
				switch (attrType)
				{
					case EvAttr_TSigBeatSize:
					{
						m_vSlider[i]->SetTextHook(&bsHook);
						break;
					}
					case EvAttr_TempoValue:
					{
						// A kludge -- sliders simply don't work for that much dynamic range...
						m_vSlider[i]->SetBodyIncrement(1000);
						m_vSlider[i]->SetTextHook(&fpHook22);
						break;
					}
					default:
					{
						m_vSlider[i]->SetTextHook(NULL);
						break;
					}
				}

				m_baseValue[i] = event->GetAttribute(attrType);
				m_vSlider[i]->SetValue(m_baseValue[i]);
			}
		}
	}
}

// ---------------------------------------------------------------------------
// Internal Operations

void
CInspectorWindow::_clear()
{
	D_INTERNAL(("CInspectorWindow::_clear()\n"));

	// Set the event name
	SetTitle("Inspector: (None)");
	for (int i = 0; i < 3; i++)
	{
		m_vSlider[i]->SetRange(0.0, 0.0);
		m_vSlider[i]->SetEnabled(false);
		m_vLabel[i]->SetText("");
	}
}

void
CInspectorWindow::_watchTrack(
	CEventTrack *track)
{
	D_INTERNAL(("CInspectorWindow::_watchTrack()\n"));

	if (track != m_track)
	{
		if (m_track)
			m_track->RemoveObserver(this);

		m_track = track;
		if (m_track)
			m_track->AddObserver(this);

		m_channelControl->SetTrack(m_track);
	}
}

// END - InspectorWindow.cpp
