/* ===================================================================== *
 * StdBevels.h (MeV/User Interface)
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
 *  Standardizes "bevel box" drawing routines.
 * ---------------------------------------------------------------------
 * History:
 *	1997		Talin
 *		Original implementation
 *	04/08/2000	cell
 *		General cleanup in preparation for initial SourceForge checkin
 *	04/18/2000	cell
 *		Updated DrawBorderBevel() to use BViews' LineArray API
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __StdBevels_H__
#define __StdBevels_H__

class BRect;
class BView;

namespace StdBevels
{
	// Constants
	enum bevel_state {
		NORMAL_BEVEL,
		DEPRESSED_BEVEL,
		DIMMED_BEVEL
	};

	// Bevels with coloration similar to Be Scrollbars, for placing in
	// window borders...
	void DrawBorderBevel(
		BView *view,
		BRect rect,
		bevel_state state);

	// Bevels with same coloration as Be buttons.
	void DrawButtonBevel(
		BView *view,
		BRect rect,
		bevel_state state);

	// Same as button bevel but with square corners.
	void DrawSquareBevel(
		BView *view,
		BRect rect,
		bevel_state state);
};

#endif /* __StdBevels_H__ */
