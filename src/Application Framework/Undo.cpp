/* ===================================================================== *
 * Undo.cpp (MeV/Application Framework)
 * ===================================================================== */

#include "Undo.h"

void UndoHistory::TrimUndo()
{
	while (currentUndoSize > maxUndoSize)
	{
		DNode		*d = undoList.Last();
		UndoAction	*ua = (UndoAction *)d;
		
		if (d == NULL || d == undoList.First() || ua == undoPos) break;
		
		currentUndoSize -= ua->Size();
		ua->Remove();
		
		delete ua;
	}
}

	//	Return true if there is an undoable action.
bool UndoHistory::CanUndo()
{
	return undoPos != NULL;
}
	
	//	Return true if there is a redoable action.
bool UndoHistory::CanRedo()
{
	return (undoList.First() != NULL && undoPos != undoList.First()) ;
}

	//	Return the description of the current undo action. Returns NULL
	//	if current undo action has no description, or there is no current
	//	undo action.
const char *UndoHistory::UndoDescription() const
{
	return undoPos != NULL ? undoPos->Description() : NULL;
}	

	//	Return the description of the current redo action. Returns NULL
	//	if current redo action has no description, or there is no current
	//	redo action.
const char *UndoHistory::RedoDescription() const
{
	DNode		*d;
	
	if (undoPos) d = undoPos->Prev();
	else d = undoList.Last();
	
	return d != NULL ? ((UndoAction *)d)->Description() : NULL;
}

	//	Undo the undoable action, if any.
bool UndoHistory::Undo()
{
	if (undoPos)
	{
		undoPos->Undo();
		if (undoPos->Next()) undoPos = (UndoAction *)undoPos->Next();
		else undoPos = NULL;
		return true;
	}
	return false;
}

	//	Redo the redoable action, if any.
bool UndoHistory::Redo()
{
	DNode		*d;
	
	if (undoPos) d = undoPos->Prev();
	else d = undoList.Last();
	
	if (d != NULL)
	{
		UndoAction	*ua = (UndoAction *)d;
		
		ua->Redo();
		undoPos = ua;
		return true;
	}
	return false;
}	

	//	Add a new action to be undone to the list. This discards any
	//	pending re-do actions. It is assumed that the action was just
	//	performed
void UndoHistory::Add( UndoAction *inAction )
{
	DNode		*d;

		//	First, get rid of all redo actions
	while ((d = undoList.First()) != NULL)
	{
		UndoAction	*ua = (UndoAction *)d;
		
		if (ua == undoPos) break;

		currentUndoSize -= ua->Size();
		ua->Remove();
		
		delete ua;
	}
	
	undoList.AddHead( inAction );
	undoPos = inAction;
	TrimUndo();
}

	//	Set how much undo information we wish to keep.
void UndoHistory::SetMaxUndoSize( int32 inMaxUndo )
{
	maxUndoSize = inMaxUndo;
	TrimUndo();
}

bool UndoHistory::IsMostRecent( UndoAction *inAction )
{
	return (		undoPos == inAction
			&&	undoPos == (UndoAction *)undoList.First() );
}
