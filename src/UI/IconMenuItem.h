/* ===================================================================== *
 * IconMenuItem.h (MeV/User Interface)
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
 * History:
 *	04/30/2000	cell
 *		Original implementation
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __C_IconMenuItem_H__
#define __C_IconMenuItem_H__

// Interfacem Kit
#include <MenuItem.h>

 /**
 *		Simply adds (icon sized) bitmaps to the left of the MenuItems	text.
 *		@author	Christoper Lenz.  
 */

class CIconMenuItem :
	public BMenuItem
{

	public:						// Constructor/Destructor

								/**	Constructor.	*/
								CIconMenuItem(
									const char *label,
									BMessage *message,
									BBitmap *bitmap = 0,
									char shortcut = 0,
									uint32 modifiers = 0);

								CIconMenuItem(
									BMenu *subMenu,
									BMessage *message = NULL,
									BBitmap *bitmap = 0);

								/**	Destructor.	*/	
			virtual 			~CIconMenuItem();

	public:						// BMenuItem Implementation

		virtual void			DrawContent();
		
		virtual void			GetContentSize(
									float *width,
									float *height);

	private:					// Instance Data

		BBitmap *				m_bitmap;
};

#endif /* __C_IconMenuItem_H__ */
