/* ===================================================================== *
 * PositionInfoBar.h (MeV/UI)
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
 *	11/08/2000	cell
 *		Original implementation
 * ===================================================================== */

#ifndef __C_PositionInfoBar_H__
#define __C_PositionInfoBar_H__

#include "StatusBar.h"

// Support Kit
#include <String.h>

/** 
 *		@author	Christoper Lenz.
 */
   
class CPositionInfoBar :
	public CStatusBar
{

public:							// Constructor/Destructor

								CPositionInfoBar(
									BRect frame,
									BScrollBar *scrollBar = NULL);

	virtual						~CPositionInfoBar();

public:							// Operations

	void						SetText(
									orientation which,
									BString text);

public:							// CStatusBar Implementation

	virtual void				DrawInto(
									BView *view,
									BRect updateRect);

	virtual void				FrameResized(
									float width,
									float height);

private:						// Instance Data

	BBitmap *					m_verticalIcon;
	BString						m_verticalText;

	BBitmap *					m_horizontalIcon;
	BString						m_horizontalText;
};

#endif /* __C_PositionInfoBar_H__ */
