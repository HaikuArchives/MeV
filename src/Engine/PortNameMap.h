/*
not fully documented yet.
PortNameMap lets us give good names to ugly ones.
SoundBlaster AWE64 1
instead of /dev/midi/awe/01 ...
the way we handle multiple sound cards needs to be fixed...
Also needs to be documented.
Dan
6/25/2000
*/

#include "String.h"
#include <List.h>
#ifndef __C_PortNameMap_H__
#define __C_PortNameMap_H__
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
#endif