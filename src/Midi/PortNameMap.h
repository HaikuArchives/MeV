/* ===================================================================== *
 * PortNameMap.h (MeV/Midi)
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
 *  
 *	PortNameMap lets us give good names to ugly ones.
 *	SoundBlaster AWE64 1
 *	instead of /dev/midi/awe/01 ...
 *	the way we handle multiple sound cards needs to be fixed...
 *
 * ---------------------------------------------------------------------
 * History:
 *	6/25/2000		dwalton
 *		Original implementation
 * ---------------------------------------------------------------------
 * To Do:
 * ===================================================================== */

#ifndef __C_PortNameMap_H__
#define __C_PortNameMap_H__

// Support Kit
#include <String.h>
#include <List.h>

class CStringTuple {
	public:
		BString *name;
		BString *map;
};
		
class CPortNameMap {
	public:
		CPortNameMap();
		void AddMap (CStringTuple *tuple);
		bool HasMap(BString *real_name);
		BString * Map(BString *real_name);
	private:
		BList *m_map;
};

#endif /* __C_PortNameMap_H__ */
