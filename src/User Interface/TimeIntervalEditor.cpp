/* ===================================================================== *
 * TimeIntervalEditor.cpp (MeV/User Interface)
 * ===================================================================== */

#include "TimeIntervalEditor.h"
#include "BitmapMenu.h"
#include "TextSlider.h"
#include "TextDisplay.h"
#include "Spinner.h"
#include "MeVDoc.h"
// User Interface
#include "BitmapTool.h"
#include "ToolBar.h"
// Support
#include "ResourceUtils.h"

// Gnu C Library
#include <stdio.h>

// ---------------------------------------------------------------------------
// Time Interval Editing Complex

BBitmap		*noteImages[ Note_IndexCount ];
bool			noteImageInit;

BBitmap		*tuplet3Image,
			*tuplet5Image,
			*tuplet7Image,
			*dot1Image,
			*dot2Image;

void LoadNoteImages()
{
	be_app->Lock();
	if (noteImageInit == false)
	{
		noteImageInit = true;
	
		for (int i = 0; i < Note_IndexCount; i++)
		{
			 noteImages[i] = ResourceUtils::LoadImage(200 + i);
		}
	}
	be_app->Unlock();
}

CTimeIntervalEditor::CTimeIntervalEditor(
	BRect		rect,
	const char	*name,
	BMessage		*msg,
	uint32		resizingMode,
	uint32		flags )
	: BControl( rect, name, "", msg, resizingMode, flags )
{
	BRect		r( rect );
	int32		x;
	int32		h;
	
//	SetViewColor(B_TRANSPARENT_COLOR);

	r.OffsetTo( B_ORIGIN );

	numerator = 1;
	denominator = 4;
	base = 3;
	
	h = ((int32)r.bottom - 16) & ~1;

	baseDuration = new CBitmapMenuButton(	BRect( 0.0, 0.0, 28.0, r.bottom ),
										"base duration",
										B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW );
	AddChild( baseDuration );

	x = 32;
						
	// Add tool bar
	modBar = new CToolBar(BRect(x, 0.0, x + 18 * 5 + 5, 26.0), "Modifiers");
	modBar->AddTool(new CBitmapTool("Tuplet3", ResourceUtils::LoadImage("Tuplet3"),
									NULL));
//	modBar->ExcludeTool( Tuplet3_ID, 0 );
	modBar->AddTool(new CBitmapTool("Tuplet5", ResourceUtils::LoadImage("Tuplet5"),
									NULL));
//	modBar->ExcludeTool( Tuplet5_ID, 0 );
	modBar->AddTool(new CBitmapTool("Tuplet7", ResourceUtils::LoadImage("Tuplet7"),
									NULL));
//	modBar->ExcludeTool( Tuplet7_ID, 0 );
	modBar->AddSeparator();
	modBar->AddTool(new CBitmapTool("Dot", ResourceUtils::LoadImage("Dot1"),
									NULL));
//	modBar->ExcludeTool( Dot_ID, 1 );
	modBar->AddTool(new CBitmapTool("DoubleDot", ResourceUtils::LoadImage("Dot2"),
									NULL));
//	modBar->ExcludeTool( DoubleDot_ID, 1 );
	AddChild(modBar);
//	modBar->SetViewColor( B_TRANSPARENT_32_BIT );
	
	x += 18*5 + 6;

	durationSlider = new CTextSlider( BRect( 31.0, r.bottom - 14, r.right, r.bottom ),
									new BMessage( DurationSlider_ID ),
									NULL, B_FOLLOW_LEFT | B_FOLLOW_BOTTOM, B_WILL_DRAW );
	durationSlider->SetRange( 1, Ticks_Per_QtrNote * 8 );
	AddChild( durationSlider );

	nSpinner = new CSpinner(	BRect( x, 0.0, x + 14, h ),
							"Numerator", new BMessage( Numerator_ID ) );
	AddChild( nSpinner );
	nSpinner->SetRange( 1, 64 );
	nSpinner->SetValue( 1 );
	x+= 16;
	
	ratio = new CTextDisplay( BRect( x, 0.0, x + 39, h ), "Ratio" );
	AddChild( ratio );
	ratio->SetFont( be_bold_font );
	ratio->SetFontSize( 14 );
	x += 40;

	dSpinner = new CSpinner(	BRect( x, 0.0, x + 14, h ),
							"Denominator", new BMessage( Denominator_ID ) );
	AddChild( dSpinner );
	dSpinner->SetRange( 1, 64 );
	dSpinner->SetValue( 4 );

	ShowRatio();
}

const int	MenuBorder_H = 2,
			ImageBorder_H = 4;

void CTimeIntervalEditor::AttachedToWindow()
{
	int		i;
	float	menuWidth = MenuBorder_H * 2;
	float	x;
	float	h = baseDuration->Frame().Height() - 4.0;

	LoadNoteImages();

	for (i = 0; i < Note_IndexCount; i++)
	{
		menuWidth += noteImages[ i ]->Bounds().Width() + ImageBorder_H*2;
	}
	
	menu = new CPopUpMatrixMenu( "Notes", menuWidth, h );
	menu->SetRadioMode( true );
	baseDuration->SetMenu( menu );

	x = MenuBorder_H;
	for (i = 0; i < Note_IndexCount; i++)
	{
		CBitmapMenuItem	*mi;
		BRect			r( noteImages[ i ]->Bounds() );
		
		r.OffsetTo( x, 2.0 );
		r.bottom = h - 2;
		r.right += ImageBorder_H * 2;

		mi = new CBitmapMenuItem(	noteImages[ i ], new BMessage( 2000 + i ) );
		menu->AddItem( mi, r );
		mi->SetTarget( this );
		
		if (i == 3) baseDuration->SelectItem( mi );
		x = r.right;
	}
	
	if (Parent()) SetViewColor( Parent()->ViewColor() );
	durationSlider->SetTarget( this );
	modBar->SetTarget( this );
	nSpinner->SetTarget( this );
	dSpinner->SetTarget( this );
}

void CTimeIntervalEditor::MessageReceived( BMessage *msg )
{
	switch (msg->what) {
	case 2000:
	case 2001:
	case 2002:
	case 2003:
	case 2004:
	case 2005:
	case 2006:
		base = msg->what - 2000;
		CalcInterval();
		break;

// case BaseDuration_ID:
// 	break;

	case ModBar_ID:
		CalcInterval();
		break;

	case DurationSlider_ID:
		bool				finalFlag;

		finalFlag	= msg->HasBool( "final" );
	
		BControl::SetValue( durationSlider->Value() );
		NoteLengthFromInterval();
		if (finalFlag) Notify();
		break;

	case Numerator_ID:
		numerator = nSpinner->Value();
		if (numerator > denominator * 2)
		{
			denominator = (numerator + 1) / 2;
			dSpinner->SetValue( denominator );
		}
		ShowRatio();
		NoteLengthFromRatio();
		break;

	case Denominator_ID:
		denominator = dSpinner->Value();
		if (numerator > denominator * 2)
		{
			numerator = denominator * 2;
			nSpinner->SetValue( numerator );
		}
		ShowRatio();
		NoteLengthFromRatio();
		break;

	default:
		BControl::MessageReceived( msg );
		break;
	}
}

void CTimeIntervalEditor::ShowRatio( bool inUpdateSlider )
{
	sprintf( ratioText, "%ld/%ld", numerator, denominator );
	
	ratio->SetText( ratioText );
	
	BControl::SetValue( Ticks_Per_QtrNote * 4 * numerator / denominator );
	
	if (inUpdateSlider)
	{
		durationSlider->SetValue( Value() );
	}
	
	Notify();
}

void CTimeIntervalEditor::Reduce( int32 &num, int32 &den )
{
		// Reduce fraction to lowest terms
	while ((num % 2) == 0 && (den % 2) == 0) { num /= 2; den /= 2; }
	while ((num % 3) == 0 && (den % 3) == 0) { num /= 3; den /= 3; }
	while ((num % 5) == 0 && (den % 5) == 0) { num /= 5; den /= 5; }
	while ((num % 7) == 0 && (den % 7) == 0) { num /= 7; den /= 7; }
}

void CTimeIntervalEditor::CalcInterval()
{
	int32			num, den;
	
	num = 2;
	den = 1;
	
	den *= (1 << base);

	if (base > 0) { num >>= 1; den >>= 1; }
	
//	if (modBar->IsSelected( Tuplet3_ID ))			{ num *= 2; den *= 3; }
//	else if (modBar->IsSelected( Tuplet5_ID ))		{ num *= 4; den *= 5; }
//	else if (modBar->IsSelected( Tuplet7_ID ))		{ num *= 4; den *= 7; }

//	if (modBar->IsSelected( Dot_ID ))				{ num *= 3; den *= 2; }
//	else if (modBar->IsSelected( DoubleDot_ID ))	{ num *= 7; den *= 4; }

	Reduce( num, den );
	
	numerator = num;
	denominator = den;

	nSpinner->SetValue( numerator );
	dSpinner->SetValue( denominator );
	
	ShowRatio();
}

void CTimeIntervalEditor::NoteLengthFromInterval()
{
	int32			b;
	uint8			tuplet,
					dots;
	float			r;
	int32			interval = Value();
	
	for (b = 0; b < Note_IndexCount - 1; b++)
	{
		r = (float)interval / (float)((Ticks_Per_QtrNote * 8) >> b);
	
		if (r >= 1.0) break;
	}
	
	tuplet = dots = 0;
	
	if (r < 1.3) ;
	else if (r < 1.4 && b > 0)
	{
		b--;
		tuplet = 1;
	}
	else if (r < 1.7)
	{
		dots = 1;
	}
	else if (r < 1.8 || b < 1)
	{
		dots = 2;
	}
	else
	{
		b--;
	}

	base = b;
	
	baseDuration->SelectItem( menu->ItemAt( b ) );
//	modBar->Select( Tuplet3_ID, tuplet == 1 );
//	modBar->Select( Tuplet5_ID, tuplet == 2 );
//	modBar->Select( Tuplet7_ID, tuplet == 3 );
//	modBar->Select( Dot_ID, dots == 1 );
//	modBar->Select( DoubleDot_ID, dots == 2 );
}

void CTimeIntervalEditor::NoteLengthFromRatio()
{
	uint8			tuplet = 0,
					dots = 0;
					
	int32			num = numerator,
					den = denominator;
					
	Reduce( num, den );
	
	if ((den % 3) == 0)		{ tuplet = 1; num *= 3; den *= 2; }
	else if ((den % 5) == 0)	{ tuplet = 2; num *= 5; den *= 4; }
	else if ((den % 7) == 0)	{ tuplet = 3; num *= 7; den *= 4; }

	Reduce( num, den );
	
	if ((num % 3) == 0)		{ dots = 1; num *= 2; den *= 3; }
	else if ((num % 7) == 0)	{ dots = 2; num *= 4; den *= 7; }

	Reduce( num, den );

	if (num == 2 && den == 1) base = 0;
	else if (num == 1 && den == 1) base = 1;
	else if (num == 1 && den == 2) base = 2;
	else if (num == 1 && den == 4) base = 3;
	else if (num == 1 && den == 8) base = 4;
	else if (num == 1 && den == 16) base = 5;
	else if (num == 1 && den == 32) base = 6;
	else
	{
		NoteLengthFromInterval();
		return;
	}

	baseDuration->SelectItem( menu->ItemAt( base ) );
//	modBar->Select( Tuplet3_ID, tuplet == 1 );
//	modBar->Select( Tuplet5_ID, tuplet == 2 );
//	modBar->Select( Tuplet7_ID, tuplet == 3 );
//	modBar->Select( Dot_ID, dots == 1 );
//	modBar->Select( DoubleDot_ID, dots == 2 );
}

void CTimeIntervalEditor::SetValue( int32 newValue )
{
	if (newValue != Value())
	{
		BControl::SetValue( newValue );
	}
	durationSlider->SetValue( newValue );
	NoteLengthFromInterval();

// sprintf( ratioText, "%d/%d", numerator, denominator );
// ratio->SetText( ratioText );
}

void CTimeIntervalEditor::Notify()
{
	if (Message())
	{
		BMessage		msg(*Message());
			
		msg.AddInt32( "value", Value() );
		Invoke( &msg );
	}
}
