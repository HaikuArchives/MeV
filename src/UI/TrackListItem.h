/* ===================================================================== *
 * TrackListItem.h (MeV/User Interface)
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
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __C_TrackListItem_H__
#define __C_TrackListItem_H__

// Interface Kit
#include <Font.h>
#include <ListItem.h>

class CTrack;

class CTrackListItem
	:	public BListItem
{

public:							// Constants

	enum messages
	{
								EDIT_TRACK = 'tliE',

								MUTE_TRACK = 'tliM',

								SOLO_TRACK = 'tliS',

								DELETE_TRACK = 'tliD',

								RENAME_TRACK = 'tliR',

								TRACK_NAME_EDITED = 'tliN'
	};

public:							// Constructor/Destructor

								CTrackListItem(
									CTrack *track);

	virtual						~CTrackListItem();

public:							// Accessors

	BBitmap *					GetDragBitmap() const;

	CTrack *					GetTrack() const
								{ return m_track; }

	int32						GetTrackID() const;

public:							// Operations

	void						StartEdit(
									BView *owner,
									int32 index,
									BRect itemRect);

	void						StopEdit();

public:							// BListItem Implementation

	virtual void				DrawItem(
									BView *owner,
									BRect frame,
									bool drawEverything = false);

	virtual void				Update(
									BView *owner,
									const BFont *font);

private:						// Instance Data

	CTrack *					m_track;

	BBitmap *					m_icon;

	font_height					m_fh;

	bool						m_editing;
};

#endif /* __C_AssemblyWindow_H__ */
