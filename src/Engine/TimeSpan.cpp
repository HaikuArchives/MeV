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
	const CTimeSpan &other) const
{
	return (other.Start() >= Start()) && (other.End() <= End());
}

bool
CTimeSpan::Intersects(
	const CTimeSpan &other) const
{
	return (other.Start() <= End()) || (other.End() >= Start());
}

// ---------------------------------------------------------------------------
// Operations

void
CTimeSpan::OffsetBy(
	CTime time)
{
	m_start += time;
	m_end += time;
}

void
CTimeSpan::OffsetTo(
	CTime time)
{
	m_end += (time - m_start);
	m_start = time;
}

void
CTimeSpan::SetDuration(
	CTime duration)
{
	m_end = m_start + duration;
}

void
CTimeSpan::SetEnd(
	CTime end)
{
	// assign & normalize if necessary
	if (end < m_start)
	{
		m_end = m_start;
		m_start = end;
	}
	else
	{
		m_end = end;
	}
}

void
CTimeSpan::SetStart(
	CTime start)
{
	// assign & normalize if necessary
	if (start > m_end)
	{
		m_start = m_end;
		m_end = start;
	}
	else
	{
		m_start = start;
	}
}

void
CTimeSpan::SetTo(
	CTime start,
	CTime end)
{
	if (start > end)
	{
		m_start = end;
		m_end = start;
	}
	else
	{
		m_start = start;
		m_end = end;
	}
}

// END -- TimeSpan.cpp
