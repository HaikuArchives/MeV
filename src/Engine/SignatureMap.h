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
 * History:
 *	1997		Talin
 *		Original implementation
 *	04/08/2000	cell
 *		General cleanup in preparation for initial SourceForge checkin
 * ===================================================================== */

#ifndef __C_SignatureMap_H__
#define __C_SignatureMap_H__

#include "TimeUnits.h"

// Support Kit
#include <OS.h>

/**	A class which manages time-signature changes.
 *	Note that this class is only a summary of the actual time-signature 
 *	events within the event list.
 */
class CSignatureMap
{

public:

	/** Encapsulates structure type for storing precomputed
	 *	time-signature changes.
	 */
	struct SigChange
	{
		/** Start time of this signature event. */
		long			sigTime;

		/** Duration in ticks of minor unit. */
		long			sigMinorUnitDur;
		/** Duration in ticks of major unit. */
		long			sigMajorUnitDur;
	};

	/** List of signature changes. */
	SigChange			*entries;

	/** Number of signature entries. */
	long				numEntries;

	/** Type of clock. */
	int16				clockType;

	/** An iterator which iterates over each major and minor unit. */
	class Iterator
	{

	public:						// Constructor/Destructor

		/** Constructor. */
								Iterator(
									long startTime,
									SigChange *list,
									long numEntries);

		/** Constructor. */
								Iterator(
									CSignatureMap &map,
									long startTime);

	public:						// Accessors

		// Iteration function
		long					First(
									bool &major)
								{
									major = (minorCount == 0);
									return minorTime;
								}

		long					Next(
									bool &major);

		int32					MajorCount() const
								{ return majorCount; }
		int32					MinorCount() const
								{ return minorCount; }
		int32					MajorUnitTime() const
								{ return sig->sigMajorUnitDur; }
		int32					MinorUnitTime() const
								{ return sig->sigMinorUnitDur; }

	private:					// Internal Operations

		/**
		 *	@param startTime	start time of iteration
		 *	@param list			list of sig changes
		 *	@param numEntries	number of sig changes
		 */
		void					Init(
									long startTime,
									SigChange *list,
									long numEntries);

	private:					// Instance Data

		/** Time of minor time unit. */
		long					minorTime;

		/** Time of major time unit. */
		long					majorTime;

		/** Count of major time units. */
		long					minorCount;

		/** Count of minor time units. */
		long					majorCount;

		/** Current time signature. */
		SigChange *				sig;

		/** Number of sig changes left. */
		long					numEntries;
	};

	/** Given a start time, locates the previous major unit in time. */
	SigChange *					PreviousMajorUnit(
									long startTime,
									long &majorUnitCount,
									long &majorStartTime);

	SigChange *					DecomposeTime(
									long startTime,
									long &majorUnit,
									long &minorUnit,
									long &extraTime);

	SigChange *					DecomposeTime(
									long startTime,
									long &majorUnit,
									long &extraTime);
};

#endif /* __C_SignatureMap_H__ */
