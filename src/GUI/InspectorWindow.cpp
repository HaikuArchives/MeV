/* ===================================================================== *
 * InspectorWindow.cpp (MeV/User Interface)
 * ===================================================================== */

#include "InspectorWindow.h"
#include "TextSlider.h"
#include "MeVDoc.h"
#include "StWindowUtils.h"
#include "EventOp.h"
#include "StdEventOps.h"
#include "PlayerControl.h"
#include "MidiDeviceInfo.h"
#include "MathUtils.h"

#include "Junk.h"

// Gnu C Library
#include <stdio.h>
// Interface Kit
#include <StringView.h>

// ---------------------------------------------------------------------------
// Constants Initialization

const BRect
CInspectorWindow::DEFAULT_DIMENSIONS(0.0, 0.0, 640.0, 64.0);





const int					channelBoxWidth = 14,
						channelBoxHeight = 14;

	// Default attributes -- where...?
	// 	track, doc, app?
	// attribute table

	// Channel selection
	// Channel locking

enum EInspectorControlIDs {
	ChannelControl_ID	= 'chan',
	Slider1_ID			= 'sld1',
	Slider2_ID			= 'sld2',
	Slider3_ID			= 'sld3',
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

#if 0
class CNoteNameTextHook : public CTextSlider::CTextHook {
public:
		/**	Return the width in pixels of the largest possible knob text. */
	int32 Largest( BView *inView, int32 inMin, int32 inMax )
	{
		return inView->StringWidth( "C#00" );
	}

		/**	Format the text for the text slider knob */
	void FormatText( char *outText, int32 inValue, int32 inMaxLen )
	{
		static char	notes[] = "CCDDEFFGGAAB",
					accid[] = " # #  # # # ";
					
		int32		k = inValue % 12;
	
		*outText++ = notes[ k ];
		if (accid[ k ] == '#') *outText++ = '#';
		
		sprintf( outText, "%d", inValue/12 - 4 );
	}
};
#endif

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
						 B_WILL_ACCEPT_FIRST_CLICK | B_NOT_RESIZABLE | B_NOT_ZOOMABLE,
						 B_CURRENT_WORKSPACE),
		CObserver(*this, NULL),
		m_track(NULL),
		m_previousValue(-1)
{
	BStringView	*stringView;

	CBorderView *bgView = new CBorderView(Bounds(), "", B_FOLLOW_ALL_SIDES,
										  B_WILL_DRAW, 0, CBorderView::BEVEL_BORDER);
	AddChild(bgView);

	// Event type view
	stringView = new BStringView(BRect(18.0, 4.0, 58.0, 17.0), "", "Event");
	stringView->SetAlignment( B_ALIGN_RIGHT );
	bgView->AddChild(stringView);

	m_eventTypeView = new CTextDisplay(BRect(60.0, 4.0, 191.0, 17.0), "");
	bgView->AddChild(m_eventTypeView);

	// Channel name
	stringView = new BStringView(BRect(18.0, 20.0, 58.0, 33.0), "", "Channel");
	bgView->AddChild(stringView);
	stringView->SetAlignment(B_ALIGN_RIGHT);

	m_channelNameView = new CTextDisplay(BRect(60.0, 20.0, 191.0, 33.0), "");
	bgView->AddChild(m_channelNameView);

	m_channelControl = new CChannelSelectorView(BRect(410, 3,
													  410 + channelBoxWidth * 16 + 2,
													  3 + channelBoxHeight * 4 + 2 ),
												 new BMessage(ChannelControl_ID),
												 m_channelNameView);
	bgView->AddChild(m_channelControl);

	for(int i = 0; i < 3; i++)
	{
		static int32 ids[3] = { Slider1_ID, Slider2_ID, Slider3_ID };
		float y = 8.0 + 18.0 * i;
		m_vLabel[i] = new BStringView(BRect(192.0, y, 253.0, y + 13.0), "", "Pitch");
		bgView->AddChild(m_vLabel[i]);
		m_vLabel[i]->SetAlignment(B_ALIGN_RIGHT);
		m_vSlider[i] = new CTextSlider(BRect(253.0, y, 408.0, y + 13.0),
									   new BMessage(ids[i]), "");
		bgView->AddChild(m_vSlider[i]);
		m_vSlider[i]->SetEnabled(false);
	}
	
	CTimeEditControl *tec;
	tec = new CTimeEditControl(BRect(16.0, 37.0, 16.0 + 75.0 + 11.0, 61.0), 
							   new BMessage('strt'));
	bgView->AddChild(tec);
	tec = new CTimeEditControl(BRect(16.0 + 75.0 + 3.0 + 11.0, 37.0,
									 16.0 + 75.0 + 3.0 + 75.0 + 22.0, 61.0),
							   new BMessage('strt'));
	tec->SetClockType(ClockType_Real);
	bgView->AddChild(tec);

		// REM: Get window position from preferences.
		// REM: When we close inspector, then save position to
		// 		preferences.
		// REM: Also save the fact whether the inspector was open or closed.
}

// ---------------------------------------------------------------------------
// CObserver Implementation

void
CInspectorWindow::OnDeleteRequested(
	BMessage *message)
{
	WatchTrack(NULL);
}

void
CInspectorWindow::OnUpdate(
	BMessage *message)
{
	if(message->HasInt8("channel"))
	{
		m_channelControl->Invalidate();
	}

	if((m_track == NULL)
		|| (m_track->SelectionType() == CTrack::Select_None))
	{
		Clear();
	}
	else
	{
		StSubjectLock trackLock(*m_track, Lock_Shared);
		const Event *event = m_track->CurrentEvent();
		int	channel = event->GetVChannel();
		StWindowLocker lck(this);
	
		// Set the event name
		m_eventTypeView->SetText(event->NameText());
		if(event->HasProperty(Event::Prop_Channel))
		{
			if (channel != m_doc->GetDefaultAttribute(EvAttr_Channel))
			{
				m_doc->SetDefaultAttribute(EvAttr_Channel, channel);
			}
			m_channelControl->SetChannel(channel);
		}

		// Set up the data values for all three sliders
		for(int i = 0; i < 3; i++)
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

void
CInspectorWindow::MessageReceived(
	BMessage *message)
{
	switch(message->what)
	{
		case ChannelControl_ID:
		{
			if(m_doc)
			{
				int8 channel;
				char vcName[Max_Device_Name + 16];
				if(message->FindInt8("channel", &channel) != B_OK)
				{
					return;
				}
				m_doc->VirtualChannelName(channel, vcName);
				m_channelNameView->SetText(vcName);
				if((channel >= 0) && (channel <= Max_VChannels) && m_track)
				{
					// Set attribute for newly created events
					m_doc->SetDefaultAttribute(EvAttr_Channel, channel);
					// Do audio feedback
					if (m_track->SelectionType() != CEventTrack::Select_None)
					{
						EventOp *op = new ChannelOp(static_cast<uint8>(channel));
						// Modify any selected events
						m_track->ModifySelectedEvents(NULL, *op, "Change channel");
						CRefCountObject::Release(op);
						if (gPrefs.feedbackAdjustMask & CGlobalPrefs::FB_Channel)
						{
							CPlayerControl::DoAudioFeedback(m_doc, EvAttr_Channel,
															channel,
															m_track->CurrentEvent());
						}
					}
				}
			}
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
					&& m_doc)
				{
					CPlayerControl::DoAudioFeedback(m_doc, attr, val, 
													m_track->CurrentEvent());
				}
				window->PostMessage(&msg, window);
				m_doc->SetDefaultAttribute(attr, val);
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
		case Update_ID:
		case Delete_ID:
		{
			CObserver::MessageReceived(message);
			break;
		}
		default:
		{
			BWindow::MessageReceived(message);
			break;
		}
	}
}

// ---------------------------------------------------------------------------
// Operations

void
CInspectorWindow::WatchTrack(
	CEventTrack *track)
{
	if (track != m_track)
	{
		m_track = track;
		m_channelControl->SetTrack(m_track);
		SetSubject(m_track);
		if (m_track)
		{
			m_doc = &(m_track->Document());
		}
		else
		{
			m_doc = NULL;
		}
		CUpdateHint hint(Update_ID);
		PostMessage(&hint);
	}
}

void CInspectorWindow::Clear()
{
	// Set the event name
	m_eventTypeView->SetText("");
	for (int i = 0; i < 3; i++)
	{
		m_vSlider[i]->SetRange(0.0, 0.0);
		m_vSlider[i]->SetEnabled(false);
		m_vLabel[i]->SetText("");
	}
}

// =============================================================================

CChannelSelectorView::CChannelSelectorView(
	BRect 		inFrame,
	BMessage		*inMessage,
	CTextDisplay	*inNameView,
	uint32		inResizingMode,
	uint32		inFlags )
	: BControl( inFrame, NULL, NULL, inMessage, inResizingMode, inFlags )
{
	channel = 0;
	track = NULL;
	nameView = inNameView;
	SetFontSize( 9.0 );
}

void CChannelSelectorView::Draw( BRect r )
{
	BRect		fr( Bounds() );
	
	fr.OffsetTo( B_ORIGIN );

	int32		cWidth = static_cast<int32>((fr.Width() - 2) / 16),
				cHeight = static_cast<int32>((fr.Height() - 2) / 4),
				x,
				y;

	SetHighColor( 190, 190, 190 );
	FillRect( BRect( 0.0, 0.0, fr.right - 1, 0.0 ) );
	FillRect( BRect( 0.0, 0.0, 0.0, fr.bottom - 1 ) );
	
	SetHighColor( 255, 255, 255 );
	FillRect( BRect( 1.0, fr.bottom, fr.right, fr.bottom ) );
	FillRect( BRect( fr.right, 1.0, fr.right, fr.bottom ) );
	
	if (track == NULL)
	{
		SetHighColor( 200, 200, 200 );
		FillRect( BRect( 2.0, 2.0, fr.right - 2, fr.bottom - 2 ) );
	}

	SetHighColor( 0, 0, 0 );
	for (x = 1; x < fr.right; x += cWidth)
		StrokeRect( BRect( x, 1.0, x, fr.bottom - 1 ) );

	for (y = 1; y < fr.bottom; y += cHeight)
		StrokeRect( BRect( 1.0, y, fr.right - 1, y ) );

	if (track)
	{
		CMeVDoc	&doc = track->Document();
		int		ch = 0;
		int		sx = 0, sy = 0;
		
			// REM: Add code to display hidden / locked channels
	
		for (y = 2; y < fr.bottom; y += cHeight)
		{
			for (x = 2; x < fr.right; x += cWidth, ch++)
			{
				VChannelEntry	&vc = doc.GetVChannel( ch );
				char				text[ 3 ];
				
				if (track && track->IsChannelLocked( ch ))
				{
					SetHighColor( 192, 192, 192 );
					SetLowColor( 128, 128, 128 );
					FillRect( BRect( x, y, x + cWidth - 2, y + cHeight - 2 ), B_MIXED_COLORS );
				}
				else
				{
					SetHighColor( vc.fillColor );
					FillRect( BRect( x + 1, y + 1, x + cWidth - 2, y + cHeight - 2 ) );
					SetHighColor( vc.highlightColor );
					FillRect( BRect( x, y, x + cWidth - 2, y ) );
					FillRect( BRect( x, y, x, y + cHeight - 2 ) );
					SetLowColor( vc.fillColor );
				}
				
				text[ 0 ] = '0' + (ch + 1) / 10;
				text[ 1 ] = '0' + (ch + 1) % 10;
				text[ 2 ] = '\0';

					// Set text color based on brightness of background
				if (vc.fillColor.red*vc.fillColor.red + vc.fillColor.green*vc.fillColor.green * 2 + vc.fillColor.blue*vc.fillColor.blue < 160*160)
					SetHighColor( 255, 255, 255 );
				else SetHighColor( 0, 0, 0 );
				MovePenTo( x + 2, y + 10 );
				DrawString( text );
				
				if (ch == channel)
				{
					sx = x - 1; sy = y - 1;
				}
			}
		}
		
		if (channel < Max_VChannels)
		{
			SetLowColor( ViewColor() );
			SetHighColor( 0, 0, 0 );
			StrokeRect( BRect( sx - 1, sy - 1, sx + cWidth + 1, sy + cHeight + 1 ) );
			SetHighColor( 255, 255, 255 );
			StrokeRect( BRect( sx, sy, sx + cWidth, sy + cHeight ) );
			StrokeRect( BRect( sx + 1, sy + 1, sx + cWidth - 1, sy + cHeight - 1 ) );
		}
	}
}

	// Invalidate the rectangle surrounding a particular channel
void CChannelSelectorView::InvalidateChannel( int32 inChannel )
{
	BRect		fr( Bounds() );
	
	fr.OffsetTo( B_ORIGIN );
	
//	if (channel < 0 || channel >= Max_VChannels) return;

	int32		cWidth = static_cast<int32>((fr.Width() - 2) / 16),
				cHeight = static_cast<int32>((fr.Height() - 2) / 4),
				x,
				y;
				
	x = (inChannel % 16) * cWidth;
	y = (inChannel / 16) * cHeight;
	
	Invalidate( BRect( x, y, x + cWidth + 2, y + cHeight + 2 ) );
}

void CChannelSelectorView::MouseDown( BPoint point )
{
	uint8		prevChannel = channel;
	uint32		buttons;
//	BPoint		pos;
	BRect		fr( Bounds() );
	int32		cWidth = static_cast<int32>((fr.Width() - 2) / 16),
				cHeight = static_cast<int32>((fr.Height() - 2) / 4);
	bool			lockDrag = false,
				lockTest = false,
				lockState = false;

		// Can't edit channel name when there's no track
	if (track == NULL) return;

	GetMouse( &point, &buttons, TRUE );
	
	if (buttons & B_SECONDARY_MOUSE_BUTTON) lockDrag = true;
	
	do
	{
			// If both buttons are held down, it means cancel the drag
		if ((~buttons & (B_PRIMARY_MOUSE_BUTTON|B_SECONDARY_MOUSE_BUTTON)) == 0)
		{
			SetChannel( prevChannel );
			return;
		}
	
		int32	x, y;
		int32	newValue;
	
		x = CLAMP( 0, (int32)((point.x - 1) / cWidth), 15 );
		y = CLAMP( 0, (int32)((point.y - 1) / cHeight), 3 );
		
		newValue = y * 16 + x;
		
		if (lockDrag)
		{
			if (!lockTest)
			{
				lockTest = true;
				lockState = !track->IsChannelLocked( newValue );
			}

			if (track->IsChannelLocked( newValue ) != lockState)
			{
				track->LockChannel( newValue, lockState );
				InvalidateChannel( newValue );
			}
		}
		else
		{
			if (channel != newValue) SetChannel( newValue );
		}
		
		Window()->UpdateIfNeeded();
		snooze(20 * 1000);
		GetMouse( &point, &buttons, TRUE );
	}
	while (buttons) ;
	
	if (channel != prevChannel)
	{
		BMessage		msg(*Message());
		
		msg.AddInt8( "channel", channel );
		Window()->PostMessage( &msg );
	}
}

		/**	Set which channel is selected. */
void CChannelSelectorView::SetChannel( uint8 inChannel )
{
	if (channel != inChannel)
	{
		Window()->Lock();
		InvalidateChannel( channel );
		channel = inChannel;

		if (nameView)
		{
			CMeVDoc	&doc = track->Document();
			if (channel < Max_VChannels)
			{
				char			vcName[ Max_Device_Name + 16 ];
			
				doc.VirtualChannelName( channel, vcName );
//				nameView->SetText( doc.GetVChannel( channel ).name );
				nameView->SetText( vcName );
			}
			else nameView->SetText( NULL );
		}

		InvalidateChannel( channel );
		Window()->Unlock();
	}
}
