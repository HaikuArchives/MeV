/* ===================================================================== *
 * TimeEditControl.h (MeV/TransportWindow)
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
 *	time editing control 
 * ---------------------------------------------------------------------
 * History:
 *	1997		Talin
 *		Original implementation
 *	04/08/2000	cell
 *		General cleanup in preparation for initial SourceForge checkin
 *	07/09/2000	cell
 *		Separated from Junk
 * ---------------------------------------------------------------------
 * To Do:
 *	- locating doesn't work yet; ie the time values are editable, but
 *	  editing them has no effect
 * ===================================================================== */

#ifndef __C_TimeEditControl_H__
#define __C_TimeEditControl_H__

#include "TimeUnits.h"

// Interface Kit
#include <Control.h>
// Media Kit
#include <TimeCode.h>

class CMeVDoc;
//class CTrack;

class BMessageRunner;
class BPopUpMenu;

class CTimeEditControl
	:	public BControl
{

public:							// Constants

	enum messages
	{
								UPDATE_TIME = 'tecA',

								CLOCK_TYPE_CHANGED
	};

	static const bigtime_t		UPDATE_TIME_PERIOD;

	static const rgb_color		LIGHT_GRAY_COLOR;
	static const rgb_color		DARK_GRAY_COLOR;

public:							// Constructor/Destructor

								CTimeEditControl(
									BRect frame,
									BMessage *message,
									uint32 resizingMode = B_FOLLOW_LEFT | B_FOLLOW_RIGHT,
									uint32 flags = B_WILL_DRAW);

	virtual						~CTimeEditControl();

public:							// Accessors

	bool						IsEditable() const
								{ return m_editable; }
	void						SetEditable(
									bool editable)
								{ m_editable = editable; }

	void						SetClockType(
									TClockType type);

	void						SetDocument(
									CMeVDoc *doc);

	void						SetTime(
									bigtime_t realTime,
									int32 meteredTime);

public:							// Operations

	void						Started();

	void						Stopped();

public:							// BControl Implementation

	virtual void				AttachedToWindow();

	virtual void				Draw(
									BRect updateRect);

	virtual void				GetPreferredSize(
									float *width,
									float *height);

	virtual void				MessageReceived(
									BMessage *message);

	virtual void				MouseDown(
									BPoint point);

	virtual void				MouseMoved(
									BPoint point,
									uint32 transit,
									const BMessage *message);

	virtual void				MouseUp(
									BPoint point);

protected:						// Internal Operations

	uint32						ColumnAt(
									BPoint point);

	void						DrawInto(
									BView *view,
									BRect updateRect);

	void						GetMeteredTime(
									int32 *bars,
									int32 *beats,
									int32 *ticks);
	void						SetMeteredTime(
									int32 bars,
									int32 beats,
									int32 ticks);

	void						GetRealTime(
									int32 *hours,
									int32 *minutes,
									int32 *seconds,
									int32 *frames);
	void						SetRealTime(
									int32 hours,
									int32 minutes,
									int32 seconds,
									int32 frames);

	void						ShowContextMenu(
									BPoint point);

	void						UpdateOffscreenBitmap();

private:						// Instance Data

	CMeVDoc *					m_document;

	BTimeCode					m_realTime;
	int32						m_meteredTime;

	TClockType					m_clockType;
	int32						m_measureBase,
								m_measureSize,
								m_beatSize;

	BFont						m_digitFont;
	font_height					m_digitFontHeight;
	BFont						m_labelFont;
	font_height					m_labelFontHeight;

	BMessageRunner *			m_messageRunner;

	BBitmap *					m_backBitmap;
	BView *						m_backView;
	bool						m_dirty;

	bool						m_editable;

	BPopUpMenu *				m_contextMenu;

	int32						m_draggingColumn;
	BPoint						m_dragOffset;
};
	
#endif /* __C_TimeEditControl_H__ */
