/* ===================================================================== *
 * Splitter.h (MeV/UI)
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
 *	04/21/2000	cell
 *		Initial version
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __C_Splitter_H__
#define __C_Splitter_H__

// Interface Kit
#include <View.h>

class CSplitterMessageFilter;

/**
 *		@author	Talin, Christoper Lenz.  
 */
 
class CSplitter
	:	public BView
{

public:							// Constants

	static const float			V_SPLITTER_WIDTH;

	static const float			H_SPLITTER_HEIGHT;

public:							// Constructor/Destructor

								CSplitter(
									BRect frame,
									BView *primaryTarget,
									BView *secondaryTarget,
									orientation posture = B_VERTICAL,
									uint32 resizingMode = B_FOLLOW_ALL_SIDES);

	virtual						~CSplitter();

public:							// Hook Functions

	virtual void				MoveRequested(
									float diff);

public:							// Accessors

	BView *						PrimaryTarget() const
								{ return m_primaryTarget; }
	void						SetPrimaryTarget(
									BView *target)
								{ m_primaryTarget = target; }

	BView *						SecondaryTarget() const
								{ return m_secondaryTarget; }
	void						SetSecondaryTarget(
									BView *target)
								{ m_secondaryTarget = target; }

	orientation					Posture() const
								{ return m_posture; }

public:							// Operations

	/**	Called by the B_MOUSE_MOVED message filter when the splitter
			bar is being dragged.	*/
	void						Dragged(
									BPoint point);

public:							// BView Implementation

	virtual void				Draw(
									BRect updateRect);

	virtual void				MouseDown(
									BPoint point);

	virtual void				MouseMoved(
									BPoint point,
									uint32 transit,
									const BMessage *message);

	virtual void				MouseUp(
									BPoint point);

private:						// Instance Data

	BView *						m_primaryTarget;
	BView *						m_secondaryTarget;

	orientation					m_posture;

	bool						m_dragging;
	float						m_offset;
	bigtime_t					m_lastDragTime;

	CSplitterMessageFilter *	m_messageFilter;

private:						// Class Data

	static const rgb_color		WHITE_COLOR;
	static const rgb_color		GRAY_COLOR;
	static const rgb_color		MEDIUM_GRAY_COLOR;
	static const rgb_color		DARK_GRAY_COLOR;

	static const bigtime_t		DRAG_LAG_TIME;
};

#endif /* __C_Splitter_H__ */
