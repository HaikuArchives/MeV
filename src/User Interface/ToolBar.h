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

class CToolBar : public BControl {
	enum EToolFlags {
		Tool_Selected		= (1<<0),		// This tool is selected
		Tool_Enabled		= (1<<1),		// This tool is enabled
		Tool_Hidden		= (1<<2),		// This tool is not visible
		Tool_Seperator	= (1<<3),		// A seperator between tools (1/2 of tool button width)
		Tool_Toggle		= (1<<4),		// A toggle-state tool

// 	Tool_Diagonal		= (1<<3),		// A diagonal three-state tool (like DPaint)
// 	Tool_DiagSelect	= (1<<4),		// Set if diagonal state is selected.
	};

	struct CToolDef {
		int32		group;				// Exclusion group, or -1 if non-exclusive
		int32		toolID;				// Tool identifier code
		int32		flags;				// Various flags
		BBitmap		*bitmap[ 3 ];			// Bitmap for this tool
		BPopUpMenu	*menu;				// menu item attached to tool
	};

	bool			vertical;
	BList			toolList;
	BPoint		toolSize;
	BPicture		*toolPict,
				*selectPict,
				*dimPict;
	BPoint		lastToolPos;

	void SelectTool( CToolDef *def, bool inNotify = true, int32 inNewState = -1 );
	void DrawTool( CToolDef *tool, const BPoint &p );
	CToolDef *PickTool( BPoint &inMouse, BPoint &outPos );
	CToolDef *FindTool( int32 inToolID );
	BPoint NextToolPos( CToolDef *def, BPoint &outNext );
	BPoint ToolPos( CToolDef *def );
	void MouseDown( BPoint point );
	void AttachedToWindow();
	void FrameResized( float width, float height );

public:

		/**	Constructor */
	CToolBar(	BRect 		inFrame,
			BMessage		*inMessage,
			BPoint		inToolSize,
			bool			inVertical,
			uint32		inResizingMode,
			uint32		inFlags );
	
		/**	Destruct */
	~CToolBar();
	
		/**	Enable or disable a specific tool. */
	void EnableTool( int32 toolID, bool inEnabled = true );
	
		/**	Redraw the toolbar. */
	void Draw( BRect r );
	
		/**	Overrider keydown to deal with navigation. */
	void KeyDown( const char*, long );
	
		/**	Add a tool to the array. (Doesn't redraw) */
	void AddTool(	int32	inToolID,			// Tool id
				bool		inToggle,			// TRUE if tool should toggle
				BBitmap	*img = NULL,		// unselected image
				BBitmap	*selImg = NULL,	// selected image
				BBitmap	*altImg = NULL,	// alternate image
				BPopUpMenu *menu = NULL ); // menu for tool
				
		/** Allows changing of tool image. */
	void SetToolImage( int32 inToolID,
					BBitmap	*img = NULL,		// unselected image
					BBitmap	*selImg = NULL,	// selected image
					BBitmap	*altImg = NULL );	// alternate image

		/**	Add a specific tool to an exclusion group. */
	void ExcludeTool( int32 inToolID, int32 inGroupID );

		/**	Add a seperator */
	void AddSeperator( int32 toolID = -1 );
	
		/**	Delete a tool */
	void RemoveTool(	int32 inToolID );
	
		/**	return the number of tools and seperators */
	int32 ToolCount() { return toolList.CountItems(); }
	
		/**	Select the tool with the given tool ID as though the user had clicked on it. */
	void Select( int32 toolID, bool selected = true, bool inNotify = false );

		/**	Returns whether the indicated tool is currently selected or not. */
	bool IsSelected( int32 toolID );
};

#endif /* __C_ToolBar_H__ */
