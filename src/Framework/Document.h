/* ===================================================================== *
 * Document.h (MeV/Application Framework)
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
 *  Document framework classes
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

#ifndef __C_Document_H__
#define __C_Document_H__

#include "Observer.h"
#include <AppKit.h>
#include <Resources.h>

class CDocApp;
class CDocWindow;

class CDocument : public CObservableSubject {

	friend class		CDocWindow;
	friend class		CDocApp;

		//	List of open windows relating to this document
	BList				windows;
	CDocApp			&app;
	BFilePanel		*savePanel;

	bool				modified;				// The document has been modified
	bool				named;				// The document has been given a file name
	
	BEntry			docLocation;			//	Location of this doc in hierarchy

	static int32		newDocCount;

	long GetUniqueWindowNum();
	
	void AddWindow   ( CDocWindow *inWindow );
	void RemoveWindow( CDocWindow *inWindow );

protected:
	bool				valid;				// The constructor initialized OK

public:
	//	CDocument();
	CDocument( CDocApp &inApp );
	CDocument( CDocApp &inApp, entry_ref &ref );
	virtual ~CDocument();
	
		// ---------- Window list control

	void BuildWindowMenu( BMenu *inMenu, CDocWindow *inSelected );
	
		// ---------- Getters

	int32 WindowCount() { return windows.CountItems(); }
	bool GetName( char *outName );	//	outName must be at least B_FILE_NAME_LENGTH
	bool InitCheck() { return valid; }
	
		// ---------- Setters

		/** Used to set the modification state of the document. */
	void SetModified( bool inModified = true )
		{ modified = inModified; }
		
		/** Returns true if document has unsaved modifications. */
	bool Modified() { return modified; }
	
		/** Returns TRUE if document has ever been saved. */
	bool Named() { return named; }

		// ---------- Document saving
		
	const BEntry &DocLocation() { return docLocation; }

		/** Low-level function to actually write the document data. */
	virtual void SaveDocument() = 0;

		/** Call this to save the document to it's current location */
	void Save();
		
		/** Call this to save the document to a new location. */
	void SaveAs();
	
		/** Override this to change the way the save file panel is created. */
	virtual BFilePanel *CreateSavePanel();
};

#endif /* __C_Document_H__ */
