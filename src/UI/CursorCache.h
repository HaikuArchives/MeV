/* ===================================================================== *
 * CursorCache.h (MeV/UI)
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
 * ---------------------------------------------------------------------
 * History:
 *	09/05/2000	cell
 *		Initial implementation
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __C_CursorCache_H__
#define __C_CursorCache_H__

class BCursor;

#include <SupportDefs.h>

class CCursorCache
{

public:							// Constants

	enum cursors
	{
								DEFAULT = 0,

								I_BEAM,

								CROSS_HAIR,

								PENCIL,

								ERASER,

								DRAGGABLE,

								DRAGGING,

								HORIZONTAL_MOVE,

								VERTICAL_MOVE,

								HORIZONTAL_RESIZE,

								VERTICAL_RESIZE,

								__CURSOR_COUNT__
	};

protected:						// Constructor/Destructor

								CCursorCache();

								~CCursorCache();

public:							// Operations

	/** Returns the cursor corresponding to the which constant */
	static const BCursor *		GetCursor(
									int32 which);

	/** Frees the singleton instance of the cache;
		Call this when app quits
	 */
	static void					Release();

private:						// Class Data

	static CCursorCache *		m_instance;

private:						// Instance Data

	const BCursor *				m_cursors[__CURSOR_COUNT__];
};

#endif /* __C_CursorCache_H__ */
