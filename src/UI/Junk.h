/* ===================================================================== *
 * Junk.h (MeV/User Interface)
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
 *  where we put classes that are still in flux
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

#ifndef __Junk_H__
#define __Junk_H__

#include "Observer.h"
#include "Track.h"
#include "TrackEditFrame.h"
#include "EventEditor.h"
#include "ToolBar.h"
#include "TrackWindow.h"
#include "MeVApp.h"
#include "MeVDoc.h"
#include "TextSlider.h"
#include "Idents.h"

class CTimeIntervalEditor;
class CMultiColumnListView;

//--------------

// ---------------------------------------------------------------------------
// Time editing control

class CTimeEditControl : public BControl {
	int16				numDigits;
	TClockType			clockType;
	BPoint				mousePos;
	char					text[ 16 ];
	int32				measureBase,
						measureSize,
						beatSize;
	BFont				digitFont;

	void MouseDown( BPoint point );
	static long drag_entry( void *arg );
	long Drag();
	void Draw( BRect r );

	void AttachedToWindow()
	{
		BControl::AttachedToWindow();
		SetViewColor( B_TRANSPARENT_32_BIT );
	}

public:
	CTimeEditControl(	BRect		inFrame,
						BMessage		*inMessage,
						uint32		inResizingMode = B_FOLLOW_LEFT | B_FOLLOW_RIGHT,
						uint32		inFlags = B_WILL_DRAW );
						
	void SetClockType( TClockType inClockType )
	{
		clockType = inClockType;
	}

	void SetValue( int32 value );
};
	
	// Generic preferences window
	
class CPrefsWindow : public CAppWindow {
protected:
	enum {
		Max_Panels = 32,
	};

	BListView		*panelList;

	enum {
		Select_ID	= 'sele',
	};
	
	BBox				*panels[ Max_Panels ];
	BView			*background;
	int32			panelCount;
	int32			currentPanel;

	void MessageReceived( BMessage* theMessage );
	BView *AddPanel( char *name );

public:
	int32 GetPanelNum();
	void SetPanelNum( int inPanelNum );
	CPrefsWindow( CWindowState &inState, char *name, uint32 flags = 0 );
	~CPrefsWindow();
};

class CDevListItem;

class CAppPrefsWindow : 
	public CPrefsWindow
{

protected:
	enum {
		Max_Panels = 32,
	};

	enum ETrackPrefsPanels {
		Panel_Feedback = 0,
		Panel_Editing,

		Panel_Count
	};
	
	CGlobalPrefs		prefs;
	BCheckBox		*cb[ 4 ][ 2 ],
					*chan_cb,
					*rect_cb;
	CTextSlider		*fbDelay;
	
	void MessageReceived( BMessage *msg );

	void ReadPrefs();
	void WritePrefs();
	bool QuitRequested();

public:
	CAppPrefsWindow( CWindowState &inState );
};

	// Generic preferences window
	
class CMiniDialog : public BWindow {
protected:
	BView		*background;
public:
	CMiniDialog( int32 inWidth, int32 inHeight, BWindow *parent, const char *title );
};
 
#endif /* __Junk_H__ */
