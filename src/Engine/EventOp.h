/* ===================================================================== *
 * EventOp.h (MeV/Engine)
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
 *  Defines function objects for operations on events
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

#ifndef __C_EventOp_H__
#define __C_EventOp_H__

#include "MeVSpec.h"
#include "Event.h"
#include "RefCount.h"
#include "TimeUnits.h"

/* ============================================================================ *
   Event Operators -- small "function" objects to operate on events
 * ============================================================================ */

/** ---------------------------------------------------------------------------
	Base class of event operators.
*/

class MeVSpec MeVPlugIn;

#ifdef __POWERPC__
#pragma export on
#endif

class MeVSpec EventOp : public CRefCountObject {
protected:

	MeVPlugIn			*creator;

public:
		/**	Default constructor for builtin operators. */
	EventOp() { creator = NULL; }

		/**	Constructor for operators created by plug-ins.
			The creator field is used for archiving / dearchiving the
			operator to the document's save file.
		*/
	EventOp( MeVPlugIn *inCreator );

		/**	The function call operater applies the function to the event. */
	virtual void operator()( Event &, TClockType ) = 0;
	
		/**	Returns the name of the plug-in that created this event operator.
			This is used for archiving the state of the operator.
		*/
	const char *CreatorName() const;

		/**	Returns the name of the plug-in that created this event operator.
			This is used for archiving the state of the operator.
		*/
	MeVPlugIn *Creator() const { return creator; }

		/**	Returns TRUE if this function can modify the order in the sequence
			to an event upn which it is applied. This is needed so that iterators
			which apply operators can know whether events need to be resorted.
		*/
	virtual bool CanModifyOrder() const { return false; }
	
		/**	Returns TRUE if this function is fast enough to be performed in
			real-time. If this returns FALSE, it can only be applied to
			a sequence destructively, and the user will not be able to
			hear the changes "live".
			
			(Plug-in implementers please be honest about this!)
		*/
	virtual bool RealTimeOK() const { return true; }
	
		/**	Return a text string describing the action. Used for setting the
			Undo menu.
		*/
	virtual const char *UndoDescription() const = 0;

		/**	For external operations created by plug-ins, return the user-readable
			name of this operation. (Internal operators do not need this).
		*/
	virtual const char *Name() const { return NULL; }
};

#ifdef __POWERPC__
#pragma export off
#endif

#endif /* __C_EventOp_H__ */
