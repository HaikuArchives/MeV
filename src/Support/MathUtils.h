/* ===================================================================== *
 * MathUtils.h (MeV/Support)
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
 *  
 * ---------------------------------------------------------------------
 * History:
 *	04/16/2000	cell
 *		Initial version
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __U_MathUtils_H__
#define __U_MathUtils_H__

#include <Point.h>

#define CLAMP(min, val, max)	val < min ? min : val > max ? max : val

namespace MathUtils
{								
	float						DistanceFromPointToLine(
									BPoint point,
									BPoint lineStart,
									BPoint lineEnd);
};

#endif /* __U_MathUtils_H__ */
