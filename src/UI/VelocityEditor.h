/* ===================================================================== *
 * VelocityEditor.h (MeV/UI)
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
 *  
 * ---------------------------------------------------------------------
 * History:
 *	1997		Talin
 *		Original implementation
 *	04/08/2000	cell
 *		General cleanup in preparation for initial SourceForge checkin
 *	09/12/2000	cell
 *		Added option to couple the control of attack & release velocity
 *		Added StripLabelView descendant to extend the context menu
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __C_VelocityEditor_H__
#define __C_VelocityEditor_H__

#include "DocWindow.h"
#include "EventEditor.h"
#include "StripFrameView.h"

class EventListUndoAction;

// ---------------------------------------------------------------------------
// Linear editor strip view

class CVelocityEditor
	:	public CEventEditor
{
	friend class CVelocityNoteEventRenderer;
	friend class CVelocityStripLabelView;

public:							// Constants

	enum messages
	{
								COUPLE_ATTACK_RELEASE = 'veeA'
	};

public:							// Constructor/Destructor

								CVelocityEditor(
									CStripFrameView &frame,
									BRect rect);

public:							// CEventEditor Implementation

	void						AttachedToWindow()
								{ SetViewColor(B_TRANSPARENT_32_BIT); }

	const BCursor *				CursorFor(
									int32 editMode) const;

	void						Draw(
									BRect updateRect);

	void						MessageReceived(
									BMessage *message);

	void						MouseMoved(
									BPoint point,
									uint32 transit,
									const BMessage *message);

	virtual void				Pulse();

	void						StartDrag(
									BPoint point,
									ulong buttons);
	bool						DoDrag(
									BPoint point,
									ulong buttons);
	void						FinishDrag(
									BPoint point,
									ulong buttons,
									bool commit);

	/**	Called when the window activates to tell this view
		to make the selection visible.
	*/
	virtual void				OnGainSelection()
								{ InvalidateSelection(); }

		/**	Called when some other window activates to tell this view
			to hide the selection.
		*/
	virtual void				OnLoseSelection()
								{ InvalidateSelection(); }

private:						// Instance Data

	int32						m_dragTime;

	int32						m_smallestTime;

	int32						m_largestTime;

	int32						m_dragVelocity;

	EventListUndoAction *		m_dragAction;

	bool						m_coupleAttackRelease;
};

#endif /* __C_VelocityEditor_H__ */
