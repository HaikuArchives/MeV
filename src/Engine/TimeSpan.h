/* ===================================================================== *
 * TimeSpan.h (MeV/Engine)
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
 *	12/13/2000	cell
 *		Initial implementation
 * ===================================================================== */

#ifndef __C_TimeSpan_H__
#define __C_TimeSpan_H__

#include "Time.h"

/**
 *	A class that encapsulates a period of time (an interval with a start 
 *	time and duration) and provides various methods to simplify common 
 *	calculations.
 *	@author	Christopher Lenz
 */
class CTimeSpan
{

public:							// Constructors/Destructor

	/**	Default constructor. Time values are not initialized. */
								CTimeSpan()
								{ }

	/** Constructor with initialization */
								CTimeSpan(
									const CTime &start,
									const CTime &duration)
									:	m_start(start),
										m_duration(duration)
								{ }

public:							// Overloaded Operators

	const CTimeSpan &			operator=(
									const CTimeSpan &other);
	const CTimeSpan &			operator|=(
									const CTimeSpan &other);
	const CTimeSpan &			operator&=(
									const CTimeSpan &other);

	bool						operator==(
									const CTimeSpan &other) const;
	bool						operator!=(
									const CTimeSpan &other) const;

	/**	Returns a new CTimeSpan object that is the intersection of two
	 *	intervals (the interval that both operands cover).
	 */
	CTimeSpan					operator|(
									const CTimeSpan &other) const;

	/**	Returns a new CTimeSpan object that is the union of two
	 *	intervals (the minimal interval that contains both operands).
	 */
	CTimeSpan					operator&(
									const CTimeSpan &other) const;

public:							// Accessors

	/**	Returns true if the specified time is contained by this interval. */
	bool						Contains(
									const CTime &time) const;
	/**	Returns true if the specified interval is completely contained 
	 *	by this interval.
	 */
	bool						Contains(
									const CTimeSpan &interval) const;

	const CTime	&				Duration() const
								{ return m_duration; }
	CTime						End() const
								{ return m_start + m_duration; }
	const CTime &				Start() const
								{ return m_start; }

public:							// Operations

	/**	Shift entire interval by the given time. */
	void						OffsetBy(
									CTime time);

	/**	Move entire interval to start at the specified time. */
	void						OffsetTo(
									CTime time);

	/**	Adjust duration without changing the start time (effectively
	 *	changing the end time).
	 */
	void						SetDuration(
									CTime duration);

	/**	Adjust the end time without changing the start time (effectively
	 *	changing duration).
	 */
	void						SetEnd(
									CTime end);

	/**	Adjust the start time without changing the end time (effectively
	 *	changing duration).
	 */
	void						SetStart(
									CTime start);

	/**	Initializes the object to new start and duration values. */
	void						SetTo(
									CTime start,
									CTime duration);

private:

	CTime						m_start;

	CTime						m_duration;
};

inline const CTimeSpan &
CTimeSpan::operator=(
	const CTimeSpan &other)
{
	m_start = other.m_start;
	m_duration = other.m_duration;
	return *this;
}

inline const CTimeSpan &
CTimeSpan::operator|=(
	const CTimeSpan &other)
{
	if (m_start < other.m_start)
		m_start = other.m_start;
	if (m_duration > other.m_duration)
		m_duration = other.m_duration;

	return *this;
}

inline const CTimeSpan &
CTimeSpan::operator&=(
	const CTimeSpan &other)
{
	if (m_start > other.m_start)
		m_start = other.m_start;
	if (m_duration < other.m_duration)
		m_duration = other.m_duration;

	return *this;
}

inline bool
CTimeSpan::operator==(
	const CTimeSpan &other) const
{
	return (m_start == other.m_start) && (m_duration == other.m_duration);
}

inline bool
CTimeSpan::operator!=(
	const CTimeSpan &other) const
{
	return (m_start != other.m_start) || (m_duration != other.m_duration);
}

inline CTimeSpan
CTimeSpan::operator|(
	const CTimeSpan &other) const
{
	return CTimeSpan(m_start > other.m_start ? m_start
											 : other.m_start,
					 m_duration > other.m_duration ? other.m_duration
					 							   : m_duration);
}

inline CTimeSpan
CTimeSpan::operator&(
	const CTimeSpan &other) const
{
	return CTimeSpan(m_start > other.m_start ? other.m_start
											 : m_start,
					 m_duration > other.m_duration ? m_duration
					 							   : other.m_duration);
}

#endif /* __C_TimeSpan_H__ */
