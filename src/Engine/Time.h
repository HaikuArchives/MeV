/* ===================================================================== *
 * Time.h (MeV/Engine)
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

#ifndef __C_Time_H__
#define __C_Time_H__

class CTime
{

public:							// Constants

	static const CTime			MIN;

	static const CTime			MAX;

public:							// Constructors/Destructor

	/**	Default constructor. Time value is not initialized. */
								CTime()
								{ }

	/**	Constructor with initializer */
								CTime(
									long milliseconds)
								:	m_time(milliseconds)
								{ }

public:							// Overloaded Operators

	const CTime	&				operator+=(
									const CTime &other)
								{ m_time += other.m_time; return *this; }
	const CTime &				operator-=(
									const CTime &other)
								{ m_time -= other.m_time; return *this; }

	CTime						operator-(
									const CTime &other) const
								{ return m_time - other.m_time; }
	CTime						operator+(
									const CTime &other) const
								{ return m_time + other.m_time; }

	bool						operator==(
									const CTime &other) const
								{ return (other.m_time - m_time) == 0; }
	bool						operator!=(
									const CTime &other) const
								{ return (other.m_time - m_time) != 0; }
	bool						operator>(
									const CTime &other) const
								{ return (m_time - other.m_time) > 0; }
	bool						operator>=(
									const CTime &other) const
								{ return (m_time - other.m_time) >= 0; }
	bool						operator<(
									const CTime &other) const
								{ return (other.m_time - m_time) > 0; }
	bool						operator<=(
									const CTime &other) const
								{ return (other.m_time - m_time) >= 0; }

public:							// Accessors

	/**	Returns the time in milliseconds. */
	long long					Microseconds() const
								{ return (long long)Milliseconds() * 1000LL; }

	/**	Returns the time in milliseconds. */
	long						Milliseconds() const
								{ return m_time; }

	/**	Returns the time in seconds. */
	long						Seconds() const
								{ return (Milliseconds() / 1000); }

	/**	Returns the time in minutes. */
	long						Minutes() const
								{ return (Seconds() / 60); }

	/**	Returns the time in hours. */
	long						Hours() const
								{ return (Minutes() / 60); }

	/**	Returns the time in frames.
	 *	@param	framesPerSec	The number of frames per second.
	 */
	long						Frames(
									short framesPerSec) const
								{ return Milliseconds() * framesPerSec / 1000L; }

public:							// Operations

	void						Decode(
									long *outHours,
									long *outMinutes,
									long *outSeconds,
									long *outFrames,
									short framesPerSec) const;
	void						Encode(
									long hours,
									long minutes,
									long seconds,
									long frames,
									short framesPerSec);

private:

	long						m_time;
};

#endif /* __C_Time_H__ */
