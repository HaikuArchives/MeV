/* ===================================================================== *
 * TempoMap.h (MeV/Engine)
 * ---------------------------------------------------------------------
 * License:
 *  The contents of this file are subject to the Mozilla Public
 *  License Version 1.1 (the "License"); you may not use this file
 *  except in compliance with the License. You may obtain a copy of
 *  the License at http://www.mozilla.org/MPL/
 *
 *  Software distributed under the License is distributed on an "AS
 *  IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 *  implied. See the License for the specific language governing
 *  rights and limitations under the License.
 *
 *  The Original Code is MeV (Musical Environment) code.
 *
 *  The Initial Developer of the Original Code is Sylvan Technical 
 *  Arts. Portions created by Sylvan are Copyright (C) 1997 Sylvan 
 *  Technical Arts. All Rights Reserved.
 *
 *  Contributor(s): 
 *		Christopher Lenz (cell)
 *
 * ---------------------------------------------------------------------
 * Purpose:
 *  Defines a compiled tempo map entry
 * ---------------------------------------------------------------------
 * History:
 *	1997		Talin
 *		Original implementation
 *	04/08/2000	cell
 *		General cleanup in preparation for initial SourceForge checkin
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __C_TempoMap_H__
#define __C_TempoMap_H__

#include "TimeUnits.h"

#include <OS.h>

	// Conversion factor used in converting quarter notes to milliseconds
const int32			cTempoFactor = 480000;

	/**	A tempo map entry. Each document has a list of these which are
		compiled from the tempo change events in the sequence.
		
		REM: How do we handle the case of "immediate" tempo changes?
		Ans: We have a special "immediate" tempo map entry...
	*/
 
class CTempoMapEntry {
public:

		// The time of the last tempo change in both real and metered
	long				rOrigin,			// start time of tempo event (linear time)
					mOrigin;			// start time of tempo event (metered time)

		// The duration of the tempo change
	long				rDuration,		// duration (metered time) of tempo change
					mDuration;		// duration (real time) of tempo change

		// The end of the tempo change
	long				rEnd,			// end time of tempo event (linear time)
					mEnd;			// end time of tempo event (metered time)
					
		// The initial and final period of the tempo change (=cTempoFactor / rate)
	double			initialPeriod,
					finalPeriod;

		// Tempo management variables
	double			periodRatio,		// ratio of final to initial period
//					periodConversion,// initialRate * cTempoConversion
					periodLog;		// natural log of period ratio

	int32 ConvertMeteredToReal( int32 mTime ) const;
	int32 ConvertRealToMetered( int32 rTime ) const;
	double InterpolatePeriod( long time, TClockType clockType ) const;
	double CalcPeriodAtTime( long time, TClockType clockType ) const
	{
		return InterpolatePeriod( time, clockType ) * (double)cTempoFactor;
	}

		// Set up the tempo entry for the intial period
	void SetInitialTempo( double inPeriod );

		// Set up a tempo entry based off the previous one.
	void SetTempo(	CTempoMapEntry	prevEntry,
					double			newPeriod,
					long				startTime,
					long				duration,
					TClockType		clockType );
};

class CTempoMap {
public:
	CTempoMapEntry	*list;
	int32			count;
	
		// Binary search list of tempo items
	CTempoMapEntry *Find( int32 time, TClockType inClockType ) const;

		// REM: Add functions to binary search tempo map for a particular
		// clock time. Can also cache previous access...

	int32 ConvertMeteredToReal( int32 mTime ) const;
	int32 ConvertRealToMetered( int32 rTime ) const;
	double CalcPeriodAtTime( long time, TClockType clockType ) const;
};

class CTempoMapIterator {
	CTempoMap		*map;
	CTempoMapEntry	*current,
					*last;			// Last tempo entry in list

		// Binary search list of tempo items
	void Find( int32 time, TClockType inClockType );
public:

		/**	Increments the current tempo entry pointer. */
	void Update( long time, TClockType inClockType );

		/**	Calculate the tempo period at a given time. */
	double CalcPeriodAtTime( long time, TClockType inClockType );
};

#endif /* __C_TempoMap_H__ */
