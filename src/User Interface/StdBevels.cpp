/* ===================================================================== *
 * StdBevels.cpp (MeV/User Interface)
 * ===================================================================== */

#include "StdBevels.h"

void DrawBorderBevel(	BView				&view,
					BRect				r,
					enum EBevelStates	state )
{
	rgb_color		*colors;

	static rgb_color	normal_greys[] = {	{ 128, 128, 128 },		// outline
										{ 255, 255, 255 },
										{ 190, 190, 190 },
										{ 220, 220, 220 } };

	static rgb_color	depressed_greys[] = {	{ 128, 128, 128 },		// outline
										{ 140, 140, 140 },
										{ 200, 200, 200 },
										{ 180, 180, 180 } };

	static rgb_color	dimmed_greys[] = {	{ 180, 180, 180 },		// outline
										{ 230, 230, 230 },
										{ 210, 210, 210 },
										{ 220, 220, 220 } };

	if (state == Bevel_Dimmed)			colors = dimmed_greys;
	else if (state == Bevel_Depressed)	colors = depressed_greys;
	else									colors = normal_greys;
	
	view.SetHighColor( 128, 128, 128 );
	view.StrokeRect( r );
	r.InsetBy( 1.0, 1.0 );

	view.SetHighColor( colors[ 1 ] );
	view.FillRect( BRect( r.left, r.top, r.left, r.bottom - 1 ) );
	view.FillRect( BRect( r.left, r.top, r.right - 1, r.top ) );
	
	view.SetHighColor( colors[ 2 ] );
	view.FillRect( BRect( r.right, r.top, r.right, r.bottom - 1 ) );
	view.FillRect( BRect( r.left + 1, r.bottom, r.right, r.bottom ) );
	
	view.SetHighColor( colors[ 3 ] );
	view.FillRect( BRect( r.left, r.bottom, r.left, r.bottom ) );
	view.FillRect( BRect( r.right, r.top, r.right, r.top ) );
	r.InsetBy( 1.0, 1.0 );
	view.FillRect( r );
}

void DrawButtonBevel(	BView				&view,
					BRect				r,
					enum EBevelStates	state )
{
	view.SetHighColor( 80, 80, 80 );
	view.FillRect( BRect( r.left + 1, r.top, r.right - 1, r.top ) );
	view.FillRect( BRect( r.left + 1, r.bottom, r.right - 1, r.bottom ) );
	view.FillRect( BRect( r.left, r.top + 1, r.left, r.bottom - 1 ) );
	view.FillRect( BRect( r.right, r.top + 1, r.right, r.bottom - 1 ) );
	r.InsetBy( 1.0, 1.0 );

	switch (state) {
	case Bevel_Normal:
			// 1st highlight
		view.SetHighColor( 235, 235, 235 );
		view.FillRect( BRect( r.left, r.top, r.right, r.top ) );
		view.FillRect( BRect( r.left, r.top, r.left, r.bottom ) );
			// Main highlight
		view.SetHighColor( 255, 255, 255 );
		view.FillRect( BRect( r.left + 1, r.top + 1, r.right - 1, r.top + 1 ) );
		view.FillRect( BRect( r.left + 1, r.top + 1, r.left + 1, r.bottom - 1 ) );
			// Deep shadow
		view.SetHighColor( 168, 168, 168 );
		view.FillRect( BRect( r.left + 1, r.bottom, r.right, r.bottom ) );
		view.FillRect( BRect( r.right, r.top + 1, r.right, r.bottom ) );
			// Mild shadow
		view.SetHighColor( 200, 200, 200 );
		view.FillRect( BRect( r.left + 2, r.bottom - 1, r.right - 1, r.bottom - 1 ) );
		view.FillRect( BRect( r.right - 1, r.top + 2, r.right - 1, r.bottom - 1 ) );

			// Fill
		view.SetHighColor( 235, 235, 235 );
		view.FillRect( BRect( r.left + 2, r.top + 2, r.right - 2, r.bottom - 2 ) );
		break;

	case Bevel_Depressed:
			// 1st highlight
		view.SetHighColor( 96, 96, 96 );
		view.FillRect( BRect( r.left, r.top, r.right, r.top ) );
		view.FillRect( BRect( r.left, r.top, r.left, r.bottom ) );
			// 2nd highlight
		view.SetHighColor( 120, 120, 120 );
		view.FillRect( BRect( r.left + 1, r.top + 1, r.right - 1, r.top + 1 ) );
		view.FillRect( BRect( r.left + 1, r.top + 1, r.left + 1, r.bottom - 1 ) );
			// Deep shadow
		view.SetHighColor( 184, 184, 184 );
		view.FillRect( BRect( r.left + 1, r.bottom, r.right, r.bottom ) );
		view.FillRect( BRect( r.right, r.top + 1, r.right, r.bottom ) );
			// Mild Shadow
		view.SetHighColor( 176, 176, 176 );
		view.FillRect( BRect( r.left + 2, r.bottom - 1, r.right - 1, r.bottom - 1 ) );
		view.FillRect( BRect( r.right - 1, r.top + 2, r.right - 1, r.bottom - 1 ) );
			// Fill
		view.SetHighColor( 168, 168, 168 );
		view.FillRect( BRect( r.left + 2, r.top + 2, r.right - 2, r.bottom - 2 ) );
		break;

	case Bevel_Dimmed:
		view.SetHighColor( 220, 220, 220 );
		view.FillRect( r );
		break;
	}
}

void DrawSquareBevel(	BView				&view,
					BRect				r,
					enum EBevelStates	state )
{
	view.SetHighColor( 80, 80, 80 );
	view.StrokeRect( r );
	r.InsetBy( 1.0, 1.0 );

	switch (state) {
	case Bevel_Normal:
			// 1st highlight
		view.SetHighColor( 235, 235, 235 );
		view.FillRect( BRect( r.left, r.top, r.right, r.top ) );
		view.FillRect( BRect( r.left, r.top, r.left, r.bottom ) );
			// Main highlight
		view.SetHighColor( 255, 255, 255 );
		view.FillRect( BRect( r.left + 1, r.top + 1, r.right - 1, r.top + 1 ) );
		view.FillRect( BRect( r.left + 1, r.top + 1, r.left + 1, r.bottom - 1 ) );
			// Deep shadow
		view.SetHighColor( 168, 168, 168 );
		view.FillRect( BRect( r.left + 1, r.bottom, r.right, r.bottom ) );
		view.FillRect( BRect( r.right, r.top + 1, r.right, r.bottom ) );
			// Mild shadow
		view.SetHighColor( 200, 200, 200 );
		view.FillRect( BRect( r.left + 2, r.bottom - 1, r.right - 1, r.bottom - 1 ) );
		view.FillRect( BRect( r.right - 1, r.top + 2, r.right - 1, r.bottom - 1 ) );

			// Fill
		view.SetHighColor( 235, 235, 235 );
		view.FillRect( BRect( r.left + 2, r.top + 2, r.right - 2, r.bottom - 2 ) );
		break;

	case Bevel_Depressed:
			// 1st highlight
		view.SetHighColor( 96, 96, 96 );
		view.FillRect( BRect( r.left, r.top, r.right, r.top ) );
		view.FillRect( BRect( r.left, r.top, r.left, r.bottom ) );
			// 2nd highlight
		view.SetHighColor( 120, 120, 120 );
		view.FillRect( BRect( r.left + 1, r.top + 1, r.right - 1, r.top + 1 ) );
		view.FillRect( BRect( r.left + 1, r.top + 1, r.left + 1, r.bottom - 1 ) );
			// Deep shadow
		view.SetHighColor( 184, 184, 184 );
		view.FillRect( BRect( r.left + 1, r.bottom, r.right, r.bottom ) );
		view.FillRect( BRect( r.right, r.top + 1, r.right, r.bottom ) );
			// Mild Shadow
		view.SetHighColor( 176, 176, 176 );
		view.FillRect( BRect( r.left + 2, r.bottom - 1, r.right - 1, r.bottom - 1 ) );
		view.FillRect( BRect( r.right - 1, r.top + 2, r.right - 1, r.bottom - 1 ) );
			// Fill
		view.SetHighColor( 168, 168, 168 );
		view.FillRect( BRect( r.left + 2, r.top + 2, r.right - 2, r.bottom - 2 ) );
		break;

	case Bevel_Dimmed:
		view.SetHighColor( 220, 220, 220 );
		view.FillRect( r );
		break;
	}
}
