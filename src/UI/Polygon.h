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

class CPolygon
{

public:							// Constructor/Destructor

								CPolygon(
									const BPoint *ptArray,
									int32 numPoints);

								CPolygon();

								CPolygon(
									const CPolygon &other);

	virtual						~CPolygon();

public:							// Operators

	CPolygon &					operator=(
									const CPolygon &other);

public:							// Accessors

	BRect						Frame() const
								{ return m_frame; }

	void						AddPoints(
									const BPoint *points,
									int32 numPoints);

	int32						CountPoints() const
								{ return m_count; }

	BPoint *					Points()
								{ return (BPoint *)m_points; }

	bool						Contains(
									const BPoint &point);

	bool						Contains(
									const BRect &rect,
									bool exlcude);

private:						// Internal Operations

	void						SetFrame(
									BPoint point);

	void						ExpandFrame(
									const BPoint *points,
									int32 numPoints);

private:						// Instance Data

	uint8 *						m_points;

	BRect						m_frame;

	int32						m_count;
	int32						m_alloc;
};

#endif /* __C_Polygon_H__ */
