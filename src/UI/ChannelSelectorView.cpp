/* ===================================================================== *
 * ChannelSelectorView (MeV/User Interface)
 * ===================================================================== */

#include "TextDisplay.h"
#include "MeVDoc.h"
#include "ChannelSelectorView.h"
#include "MathUtils.h"
#include "MidiDeviceInfo.h"

#include <Message.h>
//Interface kit
#include <View.h>
#include <Window.h>
#include <Rect.h>


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
