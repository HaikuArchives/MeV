/* ===================================================================== *
 * ConsoleContainerView.h (MeV/UI)
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
 *	11/15/2000	cell
 *		Original implementation
 * ===================================================================== */

#ifndef __C_ConsoleContainerView_H__
#define __C_ConsoleContainerView_H__

// Interface Kit
#include <View.h>

class CConsoleView;

/**
 *		@author	Christoper Lenz.  
 */

class CConsoleContainerView
	:	public BView
{

public:							// Constructor/Destructor

								CConsoleContainerView(
									BRect frame,
									const char *name);

public:							// Accessors

	CConsoleView *				GetNextSelected(
									long *index) const;

public:							// Operations

	void						AddSlot(
									CConsoleView *view,
									long atIndex = -1);

	void						DeselectAll();

	int32						CountSlots() const
								{ return CountChildren(); }

	CConsoleView *				FindSlot(
									const char *name) const;
	CConsoleView *				SlotAt(
									long index) const;

	void						RemoveSlot(
									CConsoleView *view);
	CConsoleView *				RemoveSlot(
									long index);

	void						Pack();

	void						SelectAll();

public:							// BView Implementation

	virtual void				AttachedToWindow();

	virtual void				GetPreferredSize(
									float *width,
									float *height);

	virtual void				MouseDown(
									BPoint point);

private:						// Instance Data

	BScrollBar *				m_hScrollBar;

	BScrollBar *				m_vScrollBar;
};

#endif /* __C_ConsoleContainerView_H__ */
