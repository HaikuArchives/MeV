/* ===================================================================== *
 * TempoMap.cpp (MeV/Engine)
 * ===================================================================== */

#include "TempoMap.h"

// Gnu C Library
#include <math.h>

// ---------------------------------------------------------------------------
// Sets up the playback task's initial tempo

void CTempoMapEntry::SetInitialTempo( double inPeriod )
{
	rOrigin			= mOrigin		= 0;
	rDuration			= mDuration		= 0;
	rEnd				= mEnd			= 0;
	
	initialPeriod		= finalPeriod		= inPeriod / (double)cTempoFactor;

		// Compute floating-point (double) values.
	periodRatio		= finalPeriod / initialPeriod;
	periodLog			= log( periodRatio );
}
 
// ---------------------------------------------------------------------------
// Calculate the tempo period for the given clock time.

double CTempoMapEntry::InterpolatePeriod( long time, TClockType inClockType ) const
{
	if (inClockType == ClockType_Real)
	{
			// If it's a simple tempo change, then compute it simply
		if (time <= rOrigin)						return initialPeriod;
		else if (time >= rEnd)					return finalPeriod;
		else if (initialPeriod == finalPeriod)		return finalPeriod;
		else
		{
			return initialPeriod * pow( periodRatio,
					log( 1.0 + time * (initialPeriod - finalPeriod)
						/ (finalPeriod * rDuration) ) / periodLog );
			
//			ASSERT(FALSE);
//			return 0;
		}
	}
	else
	{
			// If it's a simple tempo change, then compute it simply
		if (time <= mOrigin)					return initialPeriod;
		else if (time >= mEnd)					return finalPeriod;
		else if (initialPeriod == finalPeriod)		return finalPeriod;
		else
		{
				// Compute complex tempo change.
			return initialPeriod * pow( periodRatio, (time - mOrigin) / mDuration );
		}
	}
}

// ---------------------------------------------------------------------------
// converts metered time to real time, taking accelerando and such into account.

long CTempoMapEntry::ConvertMeteredToReal( long mTime ) const
{
		// Convert relative to last tempo change
	mTime -= mOrigin;

		// If it's a simple tempo change, then compute it simply
	if (initialPeriod == finalPeriod || mDuration <= 0 || mTime >= mDuration)
	{
		return (double)(mTime - mDuration) * finalPeriod + rEnd;
	}
	else
	{
		double		t = (double)mTime / (double)mDuration;
		return (long)(mDuration * initialPeriod * (pow( periodRatio, t ) - 1.0) / periodLog) + rOrigin;
	}
}

// ---------------------------------------------------------------------------
// converts real time to metered time, taking accelerando and such into account.

long CTempoMapEntry::ConvertRealToMetered( long rTime ) const
{
		// Convert relative to last tempo change
	rTime -= rOrigin;

		// If it's a simple tempo change, then compute it simply
	if (		initialPeriod == finalPeriod
		||	rDuration <= 0
		||	rTime >= rDuration)
	{
		return (rTime - rDuration) / finalPeriod + mEnd;
	}
	else
	{
		return (long)(mDuration
				* log( 1.0 + rTime * periodLog / (mDuration * initialPeriod) )
				/ periodLog) + mOrigin;
	}
}

// ---------------------------------------------------------------------------
// Set up a tempo change

	// Set up a tempo entry based off the previous one.
void CTempoMapEntry::SetTempo(
	CTempoMapEntry	prevEntry,		// A copy of previous entry
	double			newPeriod,
	long				start,
	long				duration,
	TClockType		clockType )
{
	double			oldPeriod = prevEntry.InterpolatePeriod( start, clockType );

	initialPeriod		= oldPeriod;
	finalPeriod		= newPeriod / (double)cTempoFactor;

		// Compute floating-point (double) values.
	periodRatio		= finalPeriod / initialPeriod;
	periodLog			= log( periodRatio );

		// Set the origin times for both real and metered.
	if (clockType == ClockType_Real)
	{
		mOrigin		= prevEntry.ConvertRealToMetered( start );
		rOrigin		= start;
		
		if (periodLog == 0.0) rDuration  = mDuration = 0.0;
		else
		{
			rDuration		= duration;
			mDuration	= (long)(rDuration * periodLog / (finalPeriod - initialPeriod));
		}

	}
	else
	{
		rOrigin		= prevEntry.ConvertMeteredToReal( start );
		mOrigin		= start;

		if (periodLog == 0.0) rDuration  = mDuration = 0.0;
		else
		{
			mDuration	= duration;
			rDuration		= (long)(mDuration * initialPeriod * (periodRatio - 1.0) / periodLog);
		}
	}
	
	if (rDuration <= 0.0 || mDuration <= 0.0) rDuration = mDuration = 0.0;
	
	rEnd				= rOrigin + rDuration;
	mEnd			= mOrigin + mDuration;
}

// ---------------------------------------------------------------------------
// Binary search list of tempo items

CTempoMapEntry *CTempoMap::Find( int32 time, TClockType inClockType ) const
{
	int32				high = count - 1,
						low = 0,
						middle;
	CTempoMapEntry		*entry;

	if (inClockType == ClockType_Metered)
	{
		while (high > low)
		{
			middle = (high + low + 1) / 2;
			entry = &list[ middle ];
			if (entry->mOrigin > time)	high = middle - 1;
			else							low = middle;
		}
	}
	else
	{
		while (high > low)
		{
			middle = (high + low + 1) / 2;
			entry = &list[ middle ];
			if (entry->rOrigin > time)	high = middle - 1;
			else							low = middle;
		}
		
	}
	return &list[ low ];
}

// ---------------------------------------------------------------------------
// converts metered time to real time, taking accelerando and such into account.

int32 CTempoMap::ConvertMeteredToReal( int32 mTime ) const
{
	CTempoMapEntry		*entry = Find( mTime, ClockType_Metered );
	
	return entry->ConvertMeteredToReal( mTime );
}

// ---------------------------------------------------------------------------
// converts real time to metered time, taking accelerando and such into account.

int32 CTempoMap::ConvertRealToMetered( int32 rTime ) const
{
	CTempoMapEntry		*entry = Find( rTime, ClockType_Real );

	return entry->ConvertRealToMetered( rTime );
}
