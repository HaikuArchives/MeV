/* ===================================================================== *
 * MixWindow.h (MeV/UI)
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
 * History:
 *	11/14/2000	cell
 *		Original implementation
 * ===================================================================== */

#ifndef __C_MixWindow_H__
#define __C_MixWindow_H__

#include "DocWindow.h"
#include "MeVDoc.h"

#include <map>

class CConsoleContainerView;
class CDestination;
class CDestinationView;

class CMixWindow
	:	public CDocWindow
{
	
public:							// Constants

	enum messages
	{
								NEW_DESTINATION = 'mixA'
	};

public:							// Constructor/Destructor

								CMixWindow(
									CWindowState &state,
									CMeVDoc *document,
									bool hasSettings = false);

								~CMixWindow();

public:							// CTrackWindow Implementation

	/** Returns a pointer to the current document. */
	virtual CMeVDoc *			Document()
								{ return static_cast<CMeVDoc *>
										 (CDocWindow::Document()); }

	virtual void				MenusBeginning();

	virtual void				MessageReceived(
									BMessage *message);

	virtual void				SubjectUpdated(
									BMessage *message);

private:						// Internal Operations

	void						_addMenuBar();

	void						_destinationAdded(
									CDestination *destination);

	void						_destinationRemoved(
									int32 destinationID);

private:						// Instance Data

	CConsoleContainerView *		m_containerView;

	float						m_consoleOffset;

	typedef map<int32, CDestinationView *> destination_view_map;
	destination_view_map		m_destinations;
};

#endif /* __C_MixWindow_H__ */
