/* ===================================================================== *
 * DestinationModifier.h (MeV/UI)
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
 *		Dan Walton (dwalton)
 *		Christopher Lenz (cell)
 *
 * History:
 *	6/21/2000		dwalton
 *	Original implementation.
 *	8/01/2000		dwalton
 * Name change, many improvements.
 * ---------------------------------------------------------------------
 * To Do:
 * 
 * ===================================================================== */

#ifndef __C_VChannelModifier_H__
#define __C_VChannelModifier_H__

#include "AppWindow.h"

/**
 *		A class that manages the m_destinations. 
 *		@author	Christoper Lenz, Dan Walton.    
 */
 
class CConsoleView;
class CDestination;
class CDestinationList;
class CMeVDoc;

class CDestinationModifier
	:	public CAppWindow
{

public:							// Constants

	enum messages
	{
								ADD_ID,

								WINDOW_CLOSED
	};

public:							// Constructor/Destructor
								
								/**	Constructor.	*/
								CDestinationModifier(
									BRect frame,
									int32 id,
									CMeVDoc *doc,
									BHandler *parent);
								
								/**	Destructor.	*/
	virtual 					~CDestinationModifier();

public:							// CAppWindow Implementation

	virtual bool				QuitRequested();

	virtual bool				SubjectReleased(
									CObservable *subject);

	virtual void				SubjectUpdated(
									BMessage *message);

private:						// Internal Operations

	void						_updateName();

private:						// Instance Data

	/**	Pointer to the currently selected dest.	*/
	CDestination *				m_dest;

	int32						m_id;

	BHandler *					m_parent;
};

#endif /* __C_DestinationModifier_H__ */
