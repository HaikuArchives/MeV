/* ===================================================================== *
 * Document.h (MeV/Framework)
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

#ifndef __C_Document_H__
#define __C_Document_H__

#include "Observable.h"

// Storage Kit
#include <Entry.h>

class CDocApp;
class CDocWindow;

/**	Document framework class.
 *		@author		Talin, Christopher Lenz
 *		@package	Framework
 */
class CDocument
	:	public CObservable
{
	friend class CDocApp;
								
public:							// Constants

	enum messages
	{
								NAME_CHANGED = 'docA'
	};

public:							// Constructor/Destructor

								CDocument(
									CDocApp *app);

								CDocument(
									CDocApp *app,
									entry_ref &ref);

	virtual						~CDocument();

public:							// Hook Functions

	/**	Override this to change the way the save file panel is created.	*/
	virtual BFilePanel *		CreateSavePanel();

	/**	Low-level function to actually write the document data.	*/
	virtual void				SaveDocument() = 0;

public:							// Accessors

	virtual CDocApp *			Application() const
								{ return m_app; }

	const BEntry &				DocLocation()
								{ return m_entry; }

	/**	Returns true if document is correctly initialized.	*/
	bool						InitCheck()
								{ return m_valid; }

	void						CancelSaving()
								{ m_saving = false; }

	/**	Returns true if the document is currently being saved
	 *		i.e., if it has an open save panel.
	 */
	bool						IsSaving() const
								{ return m_saving; }

	/**	Name must be at least B_FILE_NAME_LENGTH.	*/
	status_t					GetName(
									char *name) const;
	status_t					GetEntry(
									BEntry *entry) const;
	status_t					SetEntry(
									const BEntry *entry);

	/** Returns true if the document has unsaved modifications. */
	bool						Modified()
								{ return m_modified; }
	/**	Used to set the modification state of the document.	*/
	void						SetModified(
									bool modified = true)
								{ m_modified = modified; }

	/**	Returns true if document has ever been saved.	*/
	bool						Named()
								{ return m_named; }
	void						SetNamed();

	void						SetValid(
									bool valid = true)
								{ m_valid = valid; }

public:							// Operations

	void						AddWindow(
									CDocWindow *window);
	int32						CountWindows() const
								{ return m_windows.CountItems(); }
	CDocWindow *				MasterWindow() const
								{ return m_masterWindow; }
	void						RemoveWindow(
									CDocWindow *window);
	CDocWindow *				WindowAt(
									int32 index) const
								{ return (CDocWindow *)m_windows.ItemAt(index); }

	/**	Call this to save the document to it's current location.	*/
	void						Save();

	/**	Call this to save the document to a new location.	*/
	void						SaveAs();

private:						//	Instance Data

	/**	Location of this doc in hierarchy.	*/
	BEntry						m_entry;

	CDocApp *					m_app;

	/**	List of open windows relating to this document.	*/
	BList						m_windows;

	/**	Pointer to the main document window.	*/
	CDocWindow *				m_masterWindow;

	/**	true if the document has been modified.	*/
	bool						m_modified;				

	/**	true if the document has been given a file name.		*/
	bool						m_named;
	
	/**	true if the constructor initialized properly.	*/
	bool						m_valid;

	/**	The file panel for saving this document.	*/
	BFilePanel *				m_savePanel;

	/**	true if the a save panel is open for the document.	*/
	bool						m_saving;

private:						//	Class Data
	
	/**	Adds a new document to the list of documents already open.	*/
	static int32				s_newDocCount;
};

#endif /* __C_Document_H__ */
