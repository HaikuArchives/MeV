/* ===================================================================== *
 * ToolBar.h (MeV/User Interface)
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
 *  A toolbar class.
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

#ifndef __C_ToolBar_H__
#define __C_ToolBar_H__

// Interface Kit
#include <Control.h>
// Support Kit
#include <List.h>

class CTool;

class CToolBar :
	public BControl {

	friend class CTool;

public:									// Constants

	static const float					H_TOOL_BAR_SEPARATOR_WIDTH;

	static const float					V_TOOL_BAR_SEPARATOR_HEIGHT;

	static const float					H_TOOL_BAR_BORDER;

	static const float					V_TOOL_BAR_BORDER;

public:									// Constructor/Destructor

										CToolBar(
											BRect frame,
											const char *name,
											orientation posture = B_HORIZONTAL,
											uint32 resizingMode = B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP,
											uint32 flags = B_WILL_DRAW | B_FRAME_EVENTS);

										~CToolBar();

public:									// Accessors

	// return the number of tools and seperators
	int32								CountTools()
										{
											return m_toolList.CountItems();
										}
	
	CTool *								FindTool(
											const char *name) const;

	int32								IndexOf(
											const CTool *tool) const;
	int32								IndexOf(
											BPoint point) const;

	CTool *								ToolAt(
											int32 index) const;

public:									// Operations

	// Add a separator
	bool								AddSeparator();

	// Add a tool to the array. (Doesn't redraw)
	bool								AddTool(
											CTool *tool);

	// Delete a tool
	CTool *								RemoveTool(
											int32 index);

	// Will set the items from 'fromItem' to 'toItem' to radio mode,
	// ie only one of them can be turned on at once; if 'toItem' is
	// NULL (default), the radio group will either extend until the
	// separator or the end of the toolbar; the 'forceSelection' arg
	// determines whether the group _must_ have one tool turned on
	void								MakeRadioGroup(
											const char *fromItem,
											const char *toItem = NULL,
											bool forceSelection = true);

public:									// BControl Implementation

	virtual void						AttachedToWindow();

	// Redraw the toolbar.
	virtual void						Draw(
											BRect updateRect);
	
	virtual void						GetPreferredSize(
											float *width,
											float *height);

	virtual void						MouseDown(
											BPoint point);

	virtual void						MouseMoved(
											BPoint point,
											uint32 transit,
											const BMessage *message);

	virtual void						MouseUp(
											BPoint point);

protected:								// Internal Methods

	BPoint								ContentLocationFor(
											const CTool *whichTool) const;

private:								// Instance Data

	BList								m_toolList;

	orientation							m_posture;

	CTool *								m_lastSelectedTool;
};

#endif /* __C_ToolBar_H__ */
