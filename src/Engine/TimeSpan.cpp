/* ===================================================================== *
 * TimeSpan.cpp (MeV/Engine)
 * ===================================================================== */

#include "TimeSpan.h"

// ---------------------------------------------------------------------------
// Accessors

bool
CTimeSpan::Contains(
	const CTime &time) const
{
	return (time >= Start()) && (time <= End());
}

bool
CTimeSpan::Contains(
	const CTimeSpan &interval) const
{
	return (interval.Start() >= Start()) && (interval.End() <= End());
}

// ---------------------------------------------------------------------------
// Operations

void
CTimeSpan::OffsetBy(
	CTime time)
{
	m_start += time;
}

void
CTimeSpan::OffsetTo(
	CTime time)
{
	m_start = time;
}

void
CTimeSpan::SetDuration(
	CTime duration)
{
	m_duration = duration;
}

void
CTimeSpan::SetEnd(
	CTime end)
{
	if (m_start > end)
	{
		m_duration = m_start - end;
		m_start = end;
	}
	else
	{
		m_duration = end - m_start;
	}
}

void
CTimeSpan::SetStart(
	CTime start)
{
	CTime end = End();
	if (start > end)
	{
		m_duration = start - end;
		m_start = end;
	}
	else
	{
		m_duration = end - start;
		m_start = start;
	}
}

void
CTimeSpan::SetTo(
	CTime start,
	CTime duration)
{
	m_start = start;
	m_duration = duration;
}

// END -- TimeSpan.cpp
