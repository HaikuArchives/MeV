/* ===================================================================== *
 * Math.cpp (MeV/Support)
 * ===================================================================== */

#include "MathUtils.h"

#include <math.h>

float
MathUtils::DistanceFromPointToLine(
	BPoint point,
	BPoint lineStart,
	BPoint lineEnd)
{
	double x1 = point.x  - lineStart.x;
	double y1 = point.y  - lineStart.y;

	double x2 = lineEnd.x - lineStart.x;
	double y2 = lineEnd.y - lineStart.y;

	if (x2 < 0)
	{
		x2 = -x2;
		x1 = -x1;
	}
	if (y2 < 0)
	{
		y2 = -y2;
		y1 = -y1;
	}

	//	If line is just a single point, then return maxint
	if ((x2 == 0) && (y2 == 0))
		return DBL_MAX;

	//	First, get the distance along the line
	double dist = x1 * x2 + y1 * y2;

	//	If closest point on line is not between the two points, then
	//	return maxint.
	if ((dist < 0.0) || (dist > (x2 * x2 + y2 * y2)))
		return DBL_MAX;

	if (x2 > y2)
		dist = y1 - y2 * x1 / x2;
	else
		dist = x1 - x2 * y1 / y2;

	return fabs(dist);
}

// END - MathUtils.cpp
