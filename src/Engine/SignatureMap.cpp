/* ===================================================================== *
 * SignatureMap.cpp (MeV/Engine)
 * ===================================================================== */

#include "SignatureMap.h"

CSignatureMap::SigChange *CSignatureMap::PreviousMajorUnit(
	long			startTime,
	long			&majorUnitCount,
	long			&majorStartTime )
{
	long			t;
	long			majorUnits;
	SigChange		*sig = entries;
	long			numSigChanges = numEntries;

	majorUnitCount = 0;
	majorStartTime = 0;

		// Search for the last time signature before the start time.
		// Start by checking to see of the _next_ time signature is also
		// before the start time. If it is, then the current one couldn't
		// be the last one, now could it?
	while (sig[ 1 ].sigTime <= startTime && numSigChanges > 1)
	{
			// Compute how much time has elapsed since the last time the major
			// unit time was updated, up to the time of the timesig change
		t = sig[ 1 ].sigTime - majorStartTime;

			// Add an even number of major time units to the major time.
		majorUnits = t / sig->sigMajorUnitDur;
		majorUnitCount += majorUnits;
		majorStartTime += majorUnits * sig->sigMajorUnitDur;
		numSigChanges--;
		sig++;
	}

		// Compute how much time has elapsed since the last time the major
		// unit time was updated, up to the time of the start time.
	t = startTime - majorStartTime;

		// Add an even number of major time units to the major time.
	if (t < 0)
		majorUnits = (t - sig->sigMajorUnitDur + 1) / sig->sigMajorUnitDur;
	else majorUnits = t / sig->sigMajorUnitDur;
	majorUnitCount += majorUnits;
	majorStartTime += majorUnits * sig->sigMajorUnitDur;

	return sig;
}

CSignatureMap::SigChange *CSignatureMap::DecomposeTime(
	long startTime,
	long &majorUnit,
	long &minorUnit,
	long &extraTime )
{
	long			majorUnitStart;
	SigChange		*sig = PreviousMajorUnit( 	startTime,
												majorUnit,
												majorUnitStart );

	startTime -= majorUnitStart;
	minorUnit = startTime / sig->sigMinorUnitDur;
	extraTime = startTime % sig->sigMinorUnitDur;

	return sig;
}

CSignatureMap::SigChange *CSignatureMap::DecomposeTime(
	long startTime,
	long &majorUnit,
	long &extraTime )
{
	long			majorUnitStart;
	SigChange		*sig = PreviousMajorUnit( 	startTime,
												majorUnit,
												majorUnitStart );

	startTime -= majorUnitStart;
	extraTime = startTime;

	return sig;
}

#if 0
void CSignatureMap::TimeToString( long time, CString &str )
{
	if (clockType == clockTypeReal)
	{
		long		hours,
					minutes,
					seconds,
					frames,
					sign = 1;

			// For real-time, a negative time simply counts the minutes
			// backwards.

		if (time < 0) { sign = -1; time = -time; }

		hours	= time / (1000*60*60);
		minutes = (time / (1000*60)) % 60;
		seconds = (time / 1000) % 60;
		frames	= (time % 1000) * 30 / 1000;

		str.Format( "%03.3d:%02.2d:%02.2d.%02.2d",
			hours * sign, minutes, seconds, frames );
	}
	else
	{
		long		majorUnitNum,
					majorUnitStart;
		SignatureMapEntry	*sig;

			// For metered time, a negative time will cause
			// the majorUnitNum to be negative, and other units
			// to be positive.
		
		sig = PreviousMajorUnit( 	time,
									majorUnitNum,
									majorUnitStart );

		time -= majorUnitStart;

		str.Format(	"%04.4d:%02.2d:%03.3d",
					majorUnitNum >= 0 ? majorUnitNum + 1 : majorUnitNum,
					time / sig->sigMinorUnitDur + 1,
					time % sig->sigMinorUnitDur );
	}
}
#endif

	// Constructor
void CSignatureMap::Iterator::Init(
	long				startTime,				// start time of iteration
	SigChange			*list,					// list of sig changes
	long				numSigChanges )			// number of sig changes
{
	long				t;
	long				units;

	majorTime	= 0;
	majorCount	= 0;
	minorCount	= 0;
	numEntries	= numSigChanges;
	sig			= list;

		// Search for the last time signature before the start time.
		// Start by checking to see of the _next_ time signature is also
		// before the start time. If it is, then the current one couldn't
		// be the last one, now could it?
	while (startTime >= sig[ 1 ].sigTime && numEntries > 1)
	{
			// Compute how much time has elapsed since the last time the major
			// unit time was updated, up to the time of the timesig change
		t = sig[ 1 ].sigTime - majorTime;
		units = (t / sig->sigMajorUnitDur);

			// Add an even number of major time units to the major time.
		majorTime	+= units * sig->sigMajorUnitDur;
		majorCount	+= units;
		numEntries--;
		sig++;
	}

		// Compute how much time has elapsed since the last time the major
		// unit time was updated, up to the time of the start time.
	t = startTime - majorTime;
	if (t >= 0)
	{
		units = (t / sig->sigMajorUnitDur);
	}
	else
	{
		units = (t - sig->sigMajorUnitDur + 1) / sig->sigMajorUnitDur;
	}

		// Add an even number of major time units to the major time.
	majorTime	+= units * sig->sigMajorUnitDur;
	majorCount	+= units;

		// Minor time units are reset at each major time unit.
	minorCount = (startTime - majorTime) / sig->sigMinorUnitDur;
	minorTime  = majorTime + minorCount * sig->sigMinorUnitDur;
}

CSignatureMap::Iterator::Iterator(
	CSignatureMap		&map,
	long				startTime )
{
	Init( startTime, map.entries, map.numEntries );
}

CSignatureMap::Iterator::Iterator(
	long				startTime,				// start time of iteration
	SigChange			*list,					// list of sig changes
	long				numSigChanges )			// number of sig changes
{
	Init( startTime, list, numSigChanges );
}

	// Iteration function
long CSignatureMap::Iterator::Next( bool &major )
{
		// Compute the time of the next major time gridline, and the
		// time of the minor time gridline.
	long			nextMajor = majorTime + sig->sigMajorUnitDur,
					nextMinor = minorTime + sig->sigMinorUnitDur;

		// See which one is going to come first.
	if ((nextMajor - nextMinor) < 0)			// If major is coming first
	{
		majorTime = minorTime = nextMajor;		// Update current timeline unit.
		major = true;							// Draw a major timeline
		majorCount++;
		minorCount = 0;

			// Check signature items. If there are more than 1 signature
			// items left, and the start time of the very next signature
			// item is within the current major time unit (the major time
			// unit as defined by that signature item) then change to that
			// signature item.
		while (	numEntries > 1
			&&	sig[ 1 ].sigTime < majorTime + sig[ 0 ].sigMajorUnitDur)
		{
			sig++;
			numEntries--;
		}

		return nextMajor;						// return time of timeline
	}
	else
	{
		minorTime = nextMinor;					// Update minor time only
		minorCount++;
		major = false;							// Tell caller it's not major
		return nextMinor;						// return time of timeline
	}
}
