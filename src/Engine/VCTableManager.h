/* ===================================================================== *
 * VCTableManager.h (MeV/engine)
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
 *
 * ---------------------------------------------------------------------
 * Purpose:
 * A class that wraps the vchannel array.  Allows psuedo dynamic management. 
 * Based on the cursor design patteren.
 * ---------------------------------------------------------------------
 * History:
 *	6/21/2000		dwalton
 *		Original implementation.
 *		
 * ---------------------------------------------------------------------
 * To Do:
 * 
 * ===================================================================== */
#ifndef __C_VChannelManager_H__
#define __C_VChannelManager_H__
#include "VChannel.h"
class CVCTableManager {
public:
	CVCTableManager(VChannelTable *table);
	void AddVC(VChannelEntry &vc);
	void RemoveVC(int id);
	
	bool IsDefined(int id);
	void First();
	bool IsDone();
	void Next();
	VChannelEntry * CurrentVC();
	int CurrentID();
private:
	VChannelTable *m_tablerep;
	int pos;
};
#endif /* __C_VChannelManager_H__ */