/* ===================================================================== *
 * DocApp.h (MeV/Application Framework)
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

#ifndef __C_DocApp_H__
#define __C_DocApp_H__

#include "Document.h"

// Application Kit
#include <Application.h>

class CDocWindow;

class BResources;

class CDocApp : 
	public BApplication
{
	
protected:

	BResources		*resFile;
	BMessenger		*messenger;
	BList				documents;
	BFilePanel		*openPanel;
	BEntry			appDir;

	void RefsReceived( BMessage *inMsg );
	void ReadyToRun();

	void MessageReceived( BMessage *inMsg );

public:
	CDocApp( const char *inSignature );
	~CDocApp();
	
	void AddDocument( CDocument *inDoc );
	void RemoveDocument( CDocument *inDoc );
	BMessenger *Messenger() { return messenger; }
	void OpenDocument();

	virtual CDocument *NewDocument( bool inShowWindow = true, entry_ref *inRef = NULL ) = 0;

		/**	Global error message function */
	static void Error( char *inErrMsg );

		/** Override this to change the way the open file panel is created. */
	virtual BFilePanel *CreateOpenPanel();
};

#endif /* __C_DocApp_H__ */
