/* ===================================================================== *
 * Polygon.h (MeV/User Interface)
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
 *  similar to BPolygon, but has more functions...
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

#ifndef __C_Polygon_H__
#define __C_Polygon_H__

// Interface Kit
#include <Point.h>
#include <Rect.h>

class CPolygon {
public:
	CPolygon(const BPoint *ptArray, int32 numPoints);
	CPolygon();
	CPolygon( const CPolygon *poly );
	virtual ~CPolygon();	

	CPolygon	&operator=(const CPolygon &from)
	{
		delete fPts;
		fPts = new uint8[ from.fCount * sizeof (BPoint) ];
		memcpy( fPts, from.fPts, fCount * sizeof fPts[ 0 ] );
		fCount = fAlloc = from.fCount;
		fBounds = from.fBounds;
		return *this;
	}

	BRect Frame() const { return fBounds; }
	void	 AddPoints(const BPoint *ptArray, int32 numPoints);
	int32 CountPoints() const { return fCount; }
	BPoint *Points() { return (BPoint *)fPts; }
	bool PointInPoly( const BPoint &inPoint );
	bool RectInPoly( const BRect &inRect, bool exlcude );

private:

	BRect	fBounds;
	int32	fCount, fAlloc;
	uint8	*fPts;

	void SetFrame( BPoint inPoint )
	{
		fBounds.left = fBounds.right = inPoint.x;
		fBounds.top = fBounds.bottom = inPoint.y;
	}

	void ExpandFrame( const BPoint *inPoints, int32 numPoints )
	{
		while (numPoints-- > 0)
		{
			if (inPoints->x < fBounds.left)	fBounds.left = inPoints->x;
			if (inPoints->x > fBounds.right)	fBounds.right = inPoints->x;
			if (inPoints->y < fBounds.top)	fBounds.top = inPoints->y;
			if (inPoints->y > fBounds.bottom)	fBounds.bottom = inPoints->y;
			inPoints++;
		}
	}

};

#endif /* __C_Polygon_H__ */
