/* ===================================================================== *
 * TempoEditControl.h (MeV/TransportWindow)
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
 *	BControl derived class for displaying & editing the tempo of a 
 *  metered track (in bpm)
 * ---------------------------------------------------------------------
 * History:
 *	07/10/2000	cell
 *		Original implementation
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __C_TempoEditControl_H__
#define __C_TempoEditControl_H__

// Interface Kit
#include <Control.h>

class CMeVDoc;

class CTempoEditControl
	:	public BControl
{

public:							// Constants

	enum messages
	{
								TEMPO_CHANGED = 'tmpA'
	};

	static const rgb_color		LIGHT_GRAY_COLOR;
	static const rgb_color		DARK_GRAY_COLOR;

public:							// Constructor/Destructor

								CTempoEditControl(
									BRect frame,
									BMessage *message,
									uint32 resizingMode = B_FOLLOW_LEFT | B_FOLLOW_RIGHT,
									uint32 flags = B_WILL_DRAW);

	virtual						~CTempoEditControl();

public:							// Accessors

	void						SetDocument(
									CMeVDoc *doc);

	void						SetTempo(
									double bpm);
	double						Tempo() const
								{ return m_bpm; }

public:							// Operations

	void						StartEdit();

	void						StopEdit();

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

	void						UpdateOffscreenBitmap();

private:						// Instance Data

	CMeVDoc *					m_document;

	double						m_bpm;

	BFont						m_digitFont;
	font_height					m_digitFontHeight;
	BFont						m_labelFont;
	font_height					m_labelFontHeight;

	BBitmap *					m_backBitmap;
	BView *						m_backView;
	bool						m_dirty;

	int32						m_draggingColumn;
	BPoint						m_dragOffset;

	bool						m_editing;
};
	
#endif /* __C_TempoEditControl_H__ */
