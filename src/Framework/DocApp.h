/* ===================================================================== *
 * DocApp.h (MeV/Framework)
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
 *  History:
 *  1997		Talin
 *		Original implementation
 *  04/08/2000	cell
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

/**
 *	Handles document creation, file modification and file saving routines.
 *	@author		Talin, Christopher Lenz
 */
class CDocApp : 
	public BApplication
{

public:							// Constructor/Destructor
								CDocApp(
									const char *signature);

								~CDocApp();
	
public:							// Hook Functions

	virtual	CDocument *			NewDocument(
									bool showWindow = true,
									entry_ref *ref = NULL) = 0;

	// Override this to change the way the open file panel is created
	virtual BFilePanel *		CreateOpenPanel();

public:							// Operations

	void						AddDocument(
									CDocument *doc);
	int32						CountDocuments() const
								{ return m_documents.CountItems(); }
	virtual CDocument *			DocumentAt(
									int32 index) const
								{ return static_cast<CDocument *>(m_documents.ItemAt(index)); }
	int32						IndexOf(
									CDocument *doc) const
								{ return m_documents.IndexOf(doc); }
	void						OpenDocument();
	
	/**	Remove a document. If the last document is removed, then
	*		quit the application.
	*/
	void						RemoveDocument(
									CDocument *doc);

	/**	Global error message function.	*/
	static void					Error(
									char *errorMsg);

public:							// BApplication Implementation

	virtual void				MessageReceived(
									BMessage *message);

	/**	Checks that all open documents have been properly closed.	*/
	virtual bool				QuitRequested();

	/**	If no documents are open create a new, blank one.	*/
	virtual void				ReadyToRun();

	/**	Process icons which are dropped on the app.	*/
	virtual void				RefsReceived(
									BMessage *message);

private:

	/**	The list of open documents.	*/
	BList						m_documents;

	/**	The file panel used to open documents.	*/
	BFilePanel *				m_openPanel;
};

#endif /* __C_DocApp_H__ */
