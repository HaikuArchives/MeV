/* ===================================================================== *
 * Time.cpp (MeV/Engine)
 * ===================================================================== */

#include "Time.h"

#include <limits.h>

// ---------------------------------------------------------------------------
// Constants Initialization

const CTime
CTime::MAX = LONG_MAX;

const CTime
CTime::MIN = LONG_MIN;

// ---------------------------------------------------------------------------
// Operations

void
CTime::Decode(
	long *outHours,
	long *outMinutes,
	long *outSeconds,
	long *outFrames,
	short framesPerSec) const
{
	*outHours = Hours();
	*outMinutes = Minutes() - (*outHours * 60);
	*outSeconds = Seconds() - (*outMinutes * 60)
							- (*outHours * 60 * 60);
	*outFrames = Frames(framesPerSec) - (*outSeconds * framesPerSec)
									  - (*outMinutes * framesPerSec * 60)
									  - (*outHours * framesPerSec * 60 * 60);
}

void
CTime::Encode(
	long hours,
	long minutes,
	long seconds,
	long frames,
	short framesPerSec)
{
	m_time = frames * 1000L / framesPerSec;
	m_time += seconds * 1000L;
	m_time += minutes * 1000L * 60;
	m_time += hours * 1000L * 60;
}

// END -- Time.cpp
