/* ===================================================================== *
 * Polygon.cpp (MeV/UI)
 * ===================================================================== */

#include "Polygon.h"

// Gnu C Library
#include <string.h>
// Support Kit
#include <Debug.h>

// ---------------------------------------------------------------------------
// Constructor/Destructor

CPolygon::CPolygon(
	const BPoint *points,
	int32 numPoints)
	:	m_points(NULL),
		m_count(numPoints),
		m_alloc(numPoints)
{
	m_points = new uint8[numPoints * sizeof (BPoint)];
	memcpy(m_points, points, numPoints * sizeof (BPoint));
	SetFrame(points[0]);
	if (numPoints > 1)
		ExpandFrame(points + 1, numPoints - 1);
}

CPolygon::CPolygon()
	:	m_points(NULL),
		m_frame(0.0, 0.0, 0.0, 0.0),
		m_count(0),
		m_alloc(0)
{
}

CPolygon::CPolygon(
	const CPolygon &other)
	:	m_points(0),
		m_frame(other.m_frame),
		m_count(other.m_count),
		m_alloc(other.m_alloc)
{
	m_points = new uint8[m_count  * sizeof (BPoint)];
	memcpy(m_points, other.m_points, m_count * sizeof (BPoint));
}

CPolygon::~CPolygon()
{
	delete [] m_points;
}

// ---------------------------------------------------------------------------
// Operators

CPolygon &
CPolygon::operator=(
	const CPolygon &other)
{
	delete m_points;
	m_points = new uint8[other.m_count * sizeof (BPoint)];
	memcpy(m_points, other.m_points, m_count * sizeof m_points[0]);
	m_count = m_alloc = other.m_count;
	m_frame = other.m_frame;
	return *this;
}

// ---------------------------------------------------------------------------
// Accessors

void
CPolygon::AddPoints(
	const BPoint *points,
	int32 numPoints)
{
	ASSERT(numPoints > 0);

	if ((m_count + numPoints) > m_alloc)
	{
		int32 n;
		uint8 *newPoints;
	
		n = m_count + numPoints + 1 + m_count / 4;
		newPoints = new uint8[n * sizeof (BPoint)];
		if (m_points)
		{
			memcpy(newPoints, m_points, m_count * sizeof (BPoint));
			delete m_points;
		}
		m_points = newPoints;
		m_alloc = n;
	}
	
	ASSERT(m_alloc >= 0 );
	ASSERT(m_count >= 0 );
	ASSERT(m_count <= (m_alloc - numPoints));
	
	memcpy(&m_points[m_count * sizeof (BPoint)], points,
		   numPoints * sizeof (BPoint));

	if (m_count == 0)
	{
		SetFrame(points[0]);
		if (numPoints > 1)
			ExpandFrame(points + 1, numPoints - 1);
	}
	else
	{
		ExpandFrame(points, numPoints);
	}

	m_count += numPoints;
}

bool
CPolygon::Contains(
	const BPoint &point)
{
	int			winding = 0;
	BPoint		*pts = (BPoint *)m_points;
	BPoint		pt,
				prev;
	int			i;
	float x = point.x, y = point.y;

	if (m_points == NULL || m_count < 3)
		return false;

		// we need to know if the point is to the left of an even number of lines.
	i = 0;										// start at last vertex
	pt = pts[ m_count - 1 ];

	for (;;)
	{
		if (pt.y <= y)
		{
				// Skip over any lines that are above the intersection line.
			do {
				if (i >= m_count)
					return (winding & 1) != false;	// Quit if done

				prev = pt;						// set old point to current
				pt = pts[ i++ ];					// get new vertex

			} while (pt.y <= y );

				// We know that the segment passes downward past the inPoint.
				// If both the previous point and the new point are to the left, then
				// increment the winding; Else if either of them are to the left, then
				// test the actual intersection point.
			if (pt.x < x)
			{
				if (	    (prev.x < x)
					|| (pt.x - prev.x)*(y - prev.y) < (pt.y - prev.y)*(x - prev.x) )
						winding++;
			}
			else
			{
				if (	(prev.x < x) &&
					(pt.x - prev.x)*(y - prev.y) < (pt.y - prev.y)*(x - prev.x) )
						winding++;
			}
		}
		else
		{
				//	Skip over any lines which are below the intersection line
			do {
				if (i >= m_count)
					return (winding & 1) != false;	// Quit if done

				prev = pt;						// set old point to current
				pt = pts[ i++ ];					// get new vertex

			} while (pt.y > y) ;

				// We know that the segment passes upward past the inPoint.
				// If both the previous point and the new point are to the left, then
				// increment the winding; Else if either of them are to the left, then
				// test the actual intersection point.
			if (pt.x < x)
			{
				if (	    (prev.x < x)
					|| (pt.x - prev.x)*(y - prev.y) > (pt.y - prev.y)*(x - prev.x) )
						winding--;
			}
			else	
			{
				if ((prev.x < x)
					&& (pt.x - prev.x)*(y - prev.y) > (pt.y - prev.y)*(x - prev.x) )
						winding--;
			}
		}
	}
}

bool
CPolygon::Contains(
	const BRect &rect,
	bool include)
{
	int			winding = 0;
	BPoint		*pts = (BPoint *)m_points;
	BPoint		pt,
				prev,
				top,
				bottom;
	float		x1, x2;
	int			i;
	float		y = rect.top;

	if (m_points == NULL || m_count < 3) return false;

		// we need to know if the point is to the left of an even number of lines.
	i = 0;										// start at last vertex
	pt = pts[ m_count - 1 ];

	for (;;)
	{
		if (i >= m_count) return (winding & 1) != false;	// Quit if done

		prev = pt;								// set old point to current
		pt = pts[ i++ ];							// get new vertex
		
			// Ignore line segments completely above or below the rect.
		if (pt.y <= rect.top && prev.y <= rect.top) continue;
		if (pt.y >= rect.bottom && prev.y >= rect.bottom) continue;
		
			// sort the two points by height.
		if (pt.y < prev.y)	{ top = pt; bottom = prev; }
		else				{ top = prev; bottom = pt; }
	
			// Get the x-intersect of the top point.
		if (top.y >= rect.top) x1 = top.x;
		else x1 = bottom.x + (top.x - bottom.x) * (rect.top - bottom.y) / (top.y - bottom.y);

			// Get the x-intersect of the bottom point.
		if (bottom.y <= rect.bottom) x2 = bottom.x;
		else x2 = bottom.x + (top.x - bottom.x) * (rect.bottom - bottom.y) / (top.y - bottom.y);

			// If there is a rect edge between the two intersects, then we intersected the rect.
		if ((x1 > rect.left || x2 > rect.left)
		 &&	(x1 < rect.right || x2 < rect.right))
			return include;

			// If there wasn't an intersect with the rect, then apply the winding rule.
		if (x1 > rect.right)
		{
				// If line crossed upper part of rect downward, increment winding;
				// if it crossed upper part of rect upward, decrement winding
			if (prev.y <= y && pt.y > y) winding++;
			if (pt.y <= y && prev.y > y) winding--;
		}
	}
}

// ---------------------------------------------------------------------------
// Internal Operations

void
CPolygon::SetFrame(
	BPoint point)
{
	m_frame.left = m_frame.right = point.x;
	m_frame.top = m_frame.bottom = point.y;
}

void
CPolygon::ExpandFrame(
	const BPoint *points,
	int32 numPoints)
{
	while (numPoints-- > 0)
	{
		if (points->x < m_frame.left)
			m_frame.left = points->x;
		if (points->x > m_frame.right)
			m_frame.right = points->x;
		if (points->y < m_frame.top)
			m_frame.top = points->y;
		if (points->y > m_frame.bottom)
			m_frame.bottom = points->y;
		points++;
	}
}

// END - Polygon.cpp
