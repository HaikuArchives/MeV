/* ===================================================================== *
 * VChannelWindow.h (MeV/User Interface)
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
 *  Editor window for Virtual Channel Table
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

#ifndef __C_VChannelWindow_H__
#define __C_VChannelWindow_H__

#include "MeVDoc.h"
#include "DocWindow.h"
#include "WindowState.h"

// Interface Kit
#include <MenuBar.h>
// Storage Kit
#include <FilePanel.h>			// for BRefFilter

class CMultiColumnListView;
class CTextSlider;

	/**	Virtual channel table editor window. */

class CVChannelWindow : public CDocWindow {
	CMultiColumnListView	*channelList;
	BColorControl			*channelColor;
	CTextSlider			*portSlider,
						*channelSlider;
	VChannelTable			saveTable;
	BMessenger			messenger;
	BRefFilter				*filter;
	
	enum EControlIDs {
		SelectChannel_ID = 'chan',
		Color_ID			= 'colr',
		ClickCheckbox_ID	= 'clck',
		PortSlider_ID		= 'port',
		ChannelSlider_ID	= 'chn#',
	};
	
	void MessageReceived( BMessage *msg );

public:
	CVChannelWindow( CWindowState &inState, CMeVDoc &inDocument );
	~CVChannelWindow() { delete filter; }
};

#endif /* __C_VChannelWindow_H__ */
