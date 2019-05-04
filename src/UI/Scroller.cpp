/* ===================================================================== *
 * Scroller.cpp (MeV/User Interface)
 * ===================================================================== */

#include "Scroller.h"
#include "StWindowUtils.h"

// Gnu C Library
#include <algorithm>

// ---------------------------------------------------------------------------
// Constructor/Destructor

CScrollerTarget::CScrollerTarget(
	BRect frame,
	const char *name,
	uint32 resizingMode,
	uint32 flags)
	:	BView(frame, name, resizingMode, flags),
		scrollRange(0.0, 0.0),
		scrollValue(0.0, 0.0),
		redirect(NULL)
{
}

// ---------------------------------------------------------------------------
// Internal Operations

void
CScrollerTarget::AdjustScrollers()
{
	BScrollBar		*sBar;
	StWindowLocker	svLock   ( Window() );
	BPoint			frameSize( FrameSize() ),
					stepSize ( StepSize() );

	if ((sBar = ScrollBar( B_HORIZONTAL )) != NULL)
	{
		AdjustScroller(	sBar, scrollRange.x, &scrollValue.x,
						stepSize.x, frameSize.x );
	}
	if ((sBar = ScrollBar( B_VERTICAL )) != NULL)
	{

		AdjustScroller(	sBar, scrollRange.y, &scrollValue.y,
						stepSize.y, frameSize.y );
	}
}

void CScrollerTarget::AdjustScroller(
	BScrollBar		*sBar,
	float			range,
	float			*value,
	float			smallStep,
	float			frameSize )
{
	float			rgMin,
					rgMax,
					stSmall,
					stBig,
					v;

	sBar->GetRange(	&rgMin, &rgMax );
	sBar->GetSteps(	&stSmall, &stBig );

	range = std::max( (float)0.0, range - frameSize );
	v = *value = std::min( *value, range );

	if (stSmall != smallStep || stBig != frameSize)
		sBar->SetSteps( smallStep, frameSize );

	if (range != rgMax)		sBar->SetRange( 0.0, range );
	if (v != sBar->Value())	sBar->SetValue( v );
}

void CScrollerTarget::SetScrollRange(	float inHRange,
									float inHValue,
									float inVRange,
									float inVValue )
{
	scrollRange.x = inHRange;
	scrollValue.x = inHValue;
	scrollRange.y = inVRange;
	scrollValue.y = inVValue;

	AdjustScrollers();
}

void CScrollerTarget::SetScrollValue(
	float			inScrollValue,
	orientation		inOrient )
{
	if (inOrient == B_HORIZONTAL)
	{
// 	scrollValue.x = MIN( scrollRange.x - FrameSize().x, MAX( 0.0, inScrollValue ) );
		scrollValue.x = inScrollValue;
	}
	else if (inOrient == B_VERTICAL)
	{
// 	scrollValue.y = MIN( scrollRange.y - FrameSize().y, MAX( 0.0, inScrollValue ) );
		scrollValue.y = inScrollValue;
	}

	if (redirect) redirect->SetScrollValue( inScrollValue, inOrient );
}

	/**	Adjust the current scroll value by a delta amount. */

void CScrollerTarget::ScrollBy( float inAmount, orientation inOrient )
{
	BScrollBar		*sBar;

	if (inOrient == B_HORIZONTAL)
	{
		if ((sBar = ScrollBar( B_HORIZONTAL )) != NULL)
		{
			sBar->SetValue( scrollValue.x + inAmount );
		}
	}
	else
	{
		if ((sBar = ScrollBar( B_VERTICAL )) != NULL)
		{
			sBar->SetValue( scrollValue.y + inAmount );
		}
	}
}
