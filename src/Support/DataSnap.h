/* ===================================================================== *
 * DataSnap.h (MeV/User Interface)
 * ---------------------------------------------------------------------
 * License:
 *  The contents of this file are subject to the Netscape Public
 *  License Version 1.1 (the "License"); you may not use this file
 *  except in compliance with the License. You may obtain a copy of
 *  the License at http://www.mozilla.org/NPL/
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
 *	Various routines that provide grid-snap and data-snap operations.
 *	These routines have been carefully constructed to work with negative
 *	numbers, so don't optimize them carelessly!
 *  
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

	/**	Given a grid of values defined by (inOrg + inModulus * n)
		return the next lower grid position from inVal. (Integer version)
	*/

inline long DataSnapLower( long inVal, long inOrg, long inModulus )
{
	inVal -= inOrg;
	
	long		q = inVal - (inVal / inModulus) * inModulus;
	
	if (q < 0) q += inModulus;

	return inVal + inOrg - q;
}

	/**	Given a grid of values defined by (inOrg + inModulus * n)
		return the next lower grid position from inVal.
		(Highly accurate floating-point version)
	*/

inline double DataSnapLower( double inVal, double inOrg, double inModulus )
{
	return floor( (inVal - inOrg) / inModulus ) * inModulus + inOrg;
}

	/**	Given a grid of values defined by (inOrg + inModulus * n)
		return the next higher grid position from inVal. (Integer version)
	*/

inline long DataSnapUpper( long inVal, long inOrg, long inModulus )
{
	inVal -= inOrg;
	
	long		q = inVal - (inVal / inModulus) * inModulus;
	
	if (q > 0) q -= inModulus;

	return inVal + inOrg - q;
}

	/**	Given a grid of values defined by (inOrg + inModulus * n)
		return the next higher grid position from inVal. (Highly accurate floating-point version)
	*/

inline double DataSnapUpper( double inVal, double inOrg, double inModulus )
{
	return ceil( (inVal - inOrg) / inModulus ) * inModulus + inOrg;
}

	/**	Given a grid of values defined by (inOrg + inModulus * n)
		return the nearest grid position from inVal. (Integer version)
	*/

inline long DataSnapNearest( long inVal, long inOrg, long inModulus )
{
	inVal -= inOrg;
	
	if (inVal >= 0)
	{
		return ((inVal + (inModulus/2)) / inModulus) * inModulus + inOrg;
	}
	else
	{
		return ((inVal - (inModulus/2)) / inModulus) * inModulus + inOrg;
	}
}

	/**	Given a grid of values defined by (inOrg + inModulus * n)
		return the nearest grid position from inVal. (Highly accurate floating-point version)
	*/

inline double DataSnapNearest( double inVal, double inOrg, double inModulus )
{
	return floor( (inVal - inOrg + 0.5) / inModulus ) * inModulus + inOrg;
}
