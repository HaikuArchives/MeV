/* ===================================================================== *
 * TimeUnits.h (MeV/Engine)
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
 *  Types dealing with time
 * ---------------------------------------------------------------------
 * History:
 *	1997		Joe Pearce
 *		Original implementation
 *	04/08/2000	cell
 *		General cleanup in preparation for initial SourceForge checkin
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __TimeUnits_H__
#define __TimeUnits_H__

	// The different units of time.
enum E_ClockTypes {

		// Real time (constant rate).
		// 	All units are in milliseconds.
	ClockType_Real = 0,

		// 	Musical time, changes with tempo
		// 	All units are 1/480th of a quarter note. (double or long)
	ClockType_Metered,

		// 	Audio time, changes with sample rate
		// 	All units are in samples.
	ClockType_Audio
};

typedef enum E_ClockTypes		TClockType;

	// What the resolution of the various time units are.
const long			Ticks_Per_QtrNote = 480;	// 480 ticks per quarter note

	// There are two ways that time is used in the program. First of all, a
	// timestamp is stored with individual events in a sequence. These are
	// stored as longs. Secondly, clock counters within the program compute
	// tempo values with fairly high precision. These are stored as doubles.
	// (Extended-precision longs would be better).
//typedef double		T_BigTime;

//long MeteredToLinear( TempoMapEntry *tm, long metered );
//long LinearToMetered( TempoMapEntry *tm, long metered );

//long DeltaMeteredToLinear( TempoMapEntry *tm, long linearBase, long meteredDelta );
//long DeltaLinearToMetered( TempoMapEntry *tm, long meteredBase, long linearDelta );

const long MicroSeconds_Per_Minute = 60000000L;

// ---------------------------------------------------------------------------
// Divide microseconds per minute by quarter notes per minute
// to get microseconds per quarter note. (Extended-precision)

// Tempos are now stored as doubles...

inline long RateToPeriod( double tempoRate )
{
	return (long)( MicroSeconds_Per_Minute / tempoRate );
}

// ---------------------------------------------------------------------------
// Divide microseconds per minute by microseconds per quarter note
// to get microseconds per minute
inline double long PeriodToRate( long tempoPeriod )
{
	return MicroSeconds_Per_Minute / (float)tempoPeriod;
}

#endif /* __C_TimeUnits_H__ */
