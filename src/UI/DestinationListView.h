/* ===================================================================== * 
 * DestinationListView.h (MeV/UI) 
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
 *              Dan Walton (dwalton) 
 *				Christopher Lenz (cell)
 * 
 * History: 
 * 6/21/00 Original Implementation: Dan Walton 
 * 
 * --------------------------------------------------------------------- 
 * To Do: 
 * 
 * ===================================================================== */ 

#ifndef __C_DestinationListView_H__ 
#define __C_DestinationListView_H__ 

#include "Observer.h"

// Interface Kit
#include <View.h>
// Standard Template Library
#include <map.h> 

class CDestinationModifier;
class CMeVDoc;
class CEventTrack;

/**
 *		@author	Christoper Lenz, Dan Walton.  
 */ 
 
class CDestinationListView
	:	public BView,
		public CObserver
{

public:							// Constants

	enum
	{
								DESTINATION_SELECTED = 'dliA',

								CREATE_DESTINATION,

								EDIT_DESTINATION,

								DELETE_DESTINATION
	};

public:							// Constructor/Destructor

	/** Constructor. */ 
								CDestinationListView(
									BRect frame,
                             		uint32 resizingMode = B_FOLLOW_LEFT | B_FOLLOW_RIGHT, 
                             		uint32 flags = B_WILL_DRAW);
                                                        
	/** Destructor. */ 
        						~CDestinationListView();

public:							// Accessors

	/**	Select which track we are looking at, so we can draw channel 
       	array properly.*/                     
	void						SetDocument(
									CMeVDoc *doc);
	CMeVDoc *					Document() const
								{ return m_doc; }

	void						SetTrack(
									CEventTrack *track);

public:							// Operations

	void						SubjectUpdated(
									BMessage *message);

public:							// BView Implementation

	/** Update the info on selected channel. */
	virtual void				AttachedToWindow(); 
        
	virtual void				MessageReceived(
									BMessage *message);

public:							// CObserver Implementation

	virtual bool				Released(
									CObservable *subject);

	virtual void				Updated(
									BMessage *message);

private:						// Internal Operations

	void						_destinationAdded(
									int32 id);

	void						_destinationChanged(
									int32 id);

	void						_destinationRemoved(
									int32 originalIndex);

private:						// Instance Data

	CMeVDoc *					m_doc;

	CEventTrack *				m_track; 

	BPopUpMenu *				m_destMenu; 

	BButton *					m_editButton; 

	BButton *					m_deleteButton; 

	BMenuField *				m_destField;

	map<int, CDestinationModifier *> m_modifierMap;  
}; 

#endif /* __C_DestinationListView_H__ */
