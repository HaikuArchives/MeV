/* ===================================================================== *
 * Polygon.cpp (MeV/User Interface)
 * ===================================================================== */

#include "Polygon.h"

// Support Kit
#include <Debug.h>

CPolygon::CPolygon(const BPoint *ptArray, int32 numPoints)
{
	fPts = new uint8[ numPoints * sizeof (BPoint) ];
	memcpy( fPts, ptArray, numPoints * sizeof (BPoint) );
	fCount = fAlloc = numPoints;
	SetFrame( ptArray[ 0 ] );
	if (numPoints > 1) ExpandFrame( ptArray + 1, numPoints - 1 );
}

CPolygon::CPolygon()
{
	fPts = NULL;
	fCount = fAlloc = 0;
	fBounds.left = fBounds.right = fBounds.top = fBounds.bottom = 0;
}

CPolygon::CPolygon( const CPolygon *poly )
{
	fPts = new uint8[ poly->fCount  * sizeof (BPoint) ];
	memcpy( fPts, poly->fPts, fCount * sizeof (BPoint) );
	fCount = fAlloc = poly->fCount;
	fBounds = poly->fBounds;
}

CPolygon::~CPolygon()
{
	delete fPts;
}

void	 CPolygon::AddPoints(const BPoint *ptArray, int32 numPoints)
{
	ASSERT( numPoints > 0 );

	if (fCount + numPoints > fAlloc)
	{
		int32		n;
		uint8		*nPts;
		
		n = fCount + numPoints + 1 + fCount / 4;
		nPts = new uint8[ n * sizeof (BPoint) ];
		if (fPts)
		{
			memcpy( nPts, fPts, fCount * sizeof (BPoint) );
			delete fPts;
		}
		fPts = nPts;
		fAlloc = n;
	}
	
	ASSERT( fAlloc >= 0 );
	ASSERT( fCount >= 0 );
	ASSERT( fCount < fAlloc - numPoints );
	
	memcpy( &fPts[ fCount * sizeof (BPoint) ], ptArray, numPoints * sizeof (BPoint) );

	if (fCount == 0)
	{
		SetFrame( ptArray[ 0 ] );
		if (numPoints > 1) ExpandFrame( ptArray + 1, numPoints - 1 );
	}
	else
	{
		ExpandFrame( ptArray, numPoints );
	}
	
	fCount += numPoints;
}

bool CPolygon::PointInPoly( const BPoint &inPoint )
{
	int			winding = 0;
	BPoint		*pts = (BPoint *)fPts;
	BPoint		pt,
				prev;
	int			i;
	float		x = inPoint.x,
				y = inPoint.y;

	if (fPts == NULL || fCount < 3) return false;

		// we need to know if the point is to the left of an even number of lines.
	i = 0;										// start at last vertex
	pt = pts[ fCount - 1 ];

	for (;;)
	{
		if (pt.y <= y)
		{
				// Skip over any lines that are above the intersection line.
			do {
				if (i >= fCount)
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
				if (i >= fCount)
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

bool CPolygon::RectInPoly( const BRect &inRect, bool include )
{
	int			winding = 0;
	BPoint		*pts = (BPoint *)fPts;
	BPoint		pt,
				prev,
				top,
				bottom;
	float		x1, x2;
	int			i;
	float		y = inRect.top;

	if (fPts == NULL || fCount < 3) return false;

		// we need to know if the point is to the left of an even number of lines.
	i = 0;										// start at last vertex
	pt = pts[ fCount - 1 ];

	for (;;)
	{
		if (i >= fCount) return (winding & 1) != false;	// Quit if done

		prev = pt;								// set old point to current
		pt = pts[ i++ ];							// get new vertex
		
			// Ignore line segments completely above or below the rect.
		if (pt.y <= inRect.top && prev.y <= inRect.top) continue;
		if (pt.y >= inRect.bottom && prev.y >= inRect.bottom) continue;
		
			// sort the two points by height.
		if (pt.y < prev.y)	{ top = pt; bottom = prev; }
		else				{ top = prev; bottom = pt; }
	
			// Get the x-intersect of the top point.
		if (top.y >= inRect.top) x1 = top.x;
		else x1 = bottom.x + (top.x - bottom.x) * (inRect.top - bottom.y) / (top.y - bottom.y);

			// Get the x-intersect of the bottom point.
		if (bottom.y <= inRect.bottom) x2 = bottom.x;
		else x2 = bottom.x + (top.x - bottom.x) * (inRect.bottom - bottom.y) / (top.y - bottom.y);

			// If there is a rect edge between the two intersects, then we intersected the rect.
		if (		(x1 > inRect.left || x2 > inRect.left)
			&&	(x1 < inRect.right || x2 < inRect.right)) return include;

			// If there wasn't an intersect with the rect, then apply the winding rule.
		if (x1 > inRect.right)
		{
				// If line crossed upper part of rect downward, increment winding;
				// if it crossed upper part of rect upward, decrement winding
			if (prev.y <= y && pt.y > y) winding++;
			if (pt.y <= y && prev.y > y) winding--;
		}
	}
}
