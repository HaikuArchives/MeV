/* ===================================================================== *
 * StripSplitter.h (MeV/UI)
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
 *	Specializes CSplitter to take the minimum height of CStripViews into
 *  account.
 * ---------------------------------------------------------------------
 * History:
 *	07/23/2000	cell
 *		Initial version
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __C_StripSplitter_H__
#define __C_StripSplitter_H__

#include "Splitter.h"

class CStripView;

class CStripSplitter
	:	public CSplitter
{

public:							// Constructor/Destructor

								CStripSplitter(
									BRect frame,
									CStripView *primaryStrip,
									CStripView *secondaryStrip);

	virtual						~CStripSplitter();

public:							// Accessors

	CStripView *				PrimaryStrip() const
								{ return m_primaryStrip; }
	void						SetPrimaryStrip(
									CStripView *strip);

	CStripView *				SecondaryStrip() const
								{ return m_secondaryStrip; }
	void						SetSecondaryStrip(
									CStripView *strip);

public:							// CSplitter Implementation

	virtual void				MoveRequested(
									float diff);

private:						// Instance Data

	CStripView *				m_primaryStrip;
	CStripView *				m_secondaryStrip;
};

#endif /* __C_StripSplitter_H__ */
