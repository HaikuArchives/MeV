/* ===================================================================== *
 * Tool.h (MeV/UI)
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
 *  CTool is complementary to the CToolBar class, defining the individual
 *	items displayed in a tool bar
 * ---------------------------------------------------------------------
 * History:
 *	1997		Talin
 *		Original implementation
 *	04/08/2000	cell
 *		General cleanup in preparation for initial SourceForge checkin
 *	04/21/2000	cell
 *		Separated functionality from CToolBar class
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __C_Tool_H__
#define __C_Tool_H__

// Interface Kit
#include <Rect.h>
// Support Kit
#include <Archivable.h>

class CTool :
	public BArchivable
{

	friend class CToolBar;

public:									// Constants

	enum tool_mode {
										TOGGLE_MODE = 0,

										TRIGGER_MODE,

										RADIO_MODE
	};

	enum tool_flags {
										FORCE_SELECTION = 0x1
	};										

	static const size_t					TOOL_NAME_LENGTH;

public:									// Constructor/Destructor

										CTool(
											const char *name,
											BMessage *message,
											int32 mode = TOGGLE_MODE,
											uint32 flags = 0);

										~CTool();

public:									// Hook Functions

	virtual void						DrawTool(
											BView *owner,
											BRect toolRect) = 0;

	virtual void						GetContentSize(
											float *width,
											float *height) const = 0;

	// is called by the tool bar when the tool has been clicked
	virtual void						Clicked(
											BPoint point,
											uint32 buttons)
										{ }

	virtual void						ValueChanged()
										{ }

public:									// Accessors

	// Returns the position of this tool in the tool bar
	BPoint								ContentLocation() const;

	uint32								Flags() const
										{ return m_flags; }

	// Returns the tools frame rectangle
	BRect								Frame() const;

	// Returns whether the tool is currently selected or not
	bool								IsSelected() const
										{ return m_selected; }

	// Returns whether the tool is currently enabled or not
	bool								IsEnabled() const
										{ return m_enabled;	}

	BMessage *							Message() const
										{ return m_message; }

	int32								Mode() const
										{ return m_mode; }

	// Returns the tools name
	const char *						Name() const
										{ return m_name; }

	CToolBar *							ToolBar() const
										{ return m_toolBar;	}

	int32								Value() const
										{ return m_value; }

	CTool *								NextTool() const;

	CTool *								PreviousTool() const;

public:									// Operations

	// Select/deselect the tool
	void								Select(
											bool selected = true)
										{ m_selected = selected; }

	// Enable or disable the tool
	void								SetEnabled(
											bool enabled = true)
										{ m_enabled = enabled; }
	
	void								SetFlags(
											uint32 flags)
										{ m_flags = flags; }

	void								SetMode(
											int32 mode)
										{ m_mode = mode; }

	void								SetValue(
											int32 value);

private:								// Instance Data

	char *								m_name;

	BMessage *							m_message;

	CToolBar *							m_toolBar;

	uint32								m_flags;

	int32								m_mode;

	int32								m_value;

	bool								m_enabled;

	bool								m_selected;

protected:								// Class Data

	static float						BORDER_HEIGHT;

	static float						BORDER_WIDTH;
};

#endif /* __C_Tool_H__ */
