/* ===================================================================== *
 * Undo.h (MeV/Application Framework)
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
 *  To Do:
 *
 * ===================================================================== */

#ifndef __C_Undo_H__
#define __C_Undo_H__

#include "DList.h"

/**
 *	Generalized undo framework.  	UndoAction is a single
 *	undoable user action. It should be subclassed
 *	to provide specific undo functionality.
 *	@author		Talin, Christopher Lenz
 *	@package	Framework
 */

class UndoAction : public DNode {
	friend class UndoHistory;
	
protected:

		/**	Virtual destructor will come in handy. */
	virtual ~UndoAction() {}

		/**	Return the estimated size of this undo action. */
	virtual int32 Size() = 0;
	
		/**	Apply this undo action. */
	virtual void Undo() = 0;
	
		/**	Apply this redo action. */
	virtual void Redo() = 0;
	
		/**	A textual description of this undo action. */
		//	REM: Should this be a string from a resource file?
	virtual const char *Description() const { return NULL; }
};

		/**	UndoHistory is a series of undoable actions.	*/

class UndoHistory {

		/**	List of undoable actions. The oldest actions are at the tail
		*		of the list, with newer actions being prepended to the head.
		*/

	DList		undoList;	
	int32		currentUndoSize,			//	Size of undo data
				maxUndoSize;				//	Limit size of undo data
				
		/**	Position of current item in undo list. This is the item
		*	which will be undone if an undo command is given; The
		*	predeccessor of this item is the current redo() item.
		*/
		
	UndoAction	*undoPos;

	void TrimUndo();

public:
		
		/**	Constructor. Optional parameter specifies how much undo
		*		information should be kept in RAM. (At least one is always
		*		kept).
		*/
		
	UndoHistory( int32 inMaxUndoSize = (1024 * 10) )
	{
		currentUndoSize	= 0;
		maxUndoSize		= inMaxUndoSize;
		undoPos			= NULL;
	}

		/**	Return true if there is an undoable action.	*/
	bool CanUndo();
	
		/**	Return true if there is a redoable action.	*/
	bool CanRedo();

		/**	Undo the undoable action, if any. */
	bool Undo();

		/**	Redo the redoable action, if any.	*/
	bool Redo();
	
		/**	Return the description of the current undo action. Returns NULL
		*		if current undo action has no description, or there is no current
		*		undo action.
		*/
		
	const char *UndoDescription() const;
	
		/**	Return the description of the current redo action. Returns NULL
		*		if current redo action has no description, or there is no current
		*		redo action.
		*/
		
	const char *RedoDescription() const;
	
		/**	Add a new action to be undone to the list. This discards any
		*		pending re-do actions.
		*/
		
	void Add( UndoAction *inAction );

		/**	Set how much undo information we wish to keep.	*/
	void SetMaxUndoSize( int32 inMaxUndo );
	
		/**	Query if this is the most recent undo action.
		*		Can be used to build up undo's cumulatively.
		*/
		
	bool IsMostRecent( UndoAction *inAction );
};

#endif /* __C_Undo_H__ */
