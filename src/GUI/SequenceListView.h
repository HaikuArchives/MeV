/* ===================================================================== *
 * SequenceListView.h (MeV/User Interface)
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
 *  Editor Window for song assembly
 * ---------------------------------------------------------------------
 * History:
 *	1997		Talin
 *		Original implementation
 *	04/08/2000	cell
 *		General cleanup in preparation for initial SourceForge checkin
 *  05/27/2000	cell
 *		Separated from AssemblyWindow and renamed from CTrackList
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __C_SequenceListView_H__
#define __C_SequenceListView_H__

#include "MultiColumnListView.h"

class CSequenceListView
	:	public CMultiColumnListView
{

public:						// Constructor/Destructor

							CSequenceListView(
								CMeVDoc *inDoc,
								BRect frame,
								const char *name,
								list_view_type	type = B_SINGLE_SELECTION_LIST,
								uint32 resizeMask = B_FOLLOW_LEFT | B_FOLLOW_TOP,
								uint32 flags = B_WILL_DRAW | B_FRAME_EVENTS);
	
	virtual bool			IsDragAcceptable(
								const BMessage *message);
	
	virtual void			OnDrop(
								BMessage *message,
								int32 index);

private:					// Instance Data

	CMeVDoc *				m_doc;
};

#endif /* __C_SequenceListView_H__ */
