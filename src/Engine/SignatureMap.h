/* ===================================================================== *
 * SignatureMap.h (MeV/Engine)
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
 *  Defines time signatures and structures time
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

#ifndef __C_SignatureMap_H__
#define __C_SignatureMap_H__

#include "TimeUnits.h"

// Support Kit
#include <OS.h>

// ---------------------------------------------------------------------------
// A structure which manages time-signature changes

// Note that this structure is (I believe) only a summary of the
// actual time-signature events within the event list.

class CSignatureMap {
public:

		// Encapsulates structure type for storing precomputed
		// time-signature changes.
		
	struct SigChange {
		long			sigTime;			// start time of this signature event
		long			sigMinorUnitDur,	// Duration in ticks of minor unit
						sigMajorUnitDur;	// Duration in ticks of major unit
	};

	SigChange			*entries;			// list of signature changes
	long				numEntries;			// number of signature entries
	int16				clockType;			// type of clock

		// An iterator which iterates over each major and minor unit.
	class Iterator {
	private:
		long			minorTime,			// time of minor time unit
						majorTime;			// time of major time unit
		long			minorCount,			// count of major time units
						majorCount;			// count of minor time units
		SigChange		*sig;				// current time signature
		long			numEntries;			// number of sig changes left

			// Constructor
		void Init(	long		startTime,	// start time of iteration
					SigChange	*list,		// list of sig changes
					long		numEntries );// number of sig changes

	public:
			// Constructor
		Iterator(	long		startTime,	// start time of iteration
					SigChange	*list,		// list of sig changes
					long		numEntries );// number of sig changes

			// Constructor
		Iterator(	CSignatureMap	&map,
					long			startTime );

			// Iteration function
		long Next( bool &major );
		long First( bool &major )
		{
			major = (minorCount == 0);
			return minorTime;
		}
		
		int32 MajorCount() { return majorCount; }
		int32 MinorCount() { return minorCount; }
		
		int32 MajorUnitTime() { return sig->sigMajorUnitDur; }
		int32 MinorUnitTime() { return sig->sigMinorUnitDur; }
	};

		// Given a start time, locates the previous major unit in time
	SigChange *PreviousMajorUnit(	long	startTime,
									long	&majorUnitCount,
									long	&majorStartTime );

	SigChange *DecomposeTime(		long startTime,
									long &majorUnit,
									long &minorUnit,
									long &extraTime );

	SigChange *DecomposeTime(			long startTime,
									long &majorUnit,
									long &extraTime );

// void TimeToString( long time, CString &str );
};

#endif /* __C_SignatureMap_H__ */
