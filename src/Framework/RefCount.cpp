/* ===================================================================== *
 * RefCount.cpp (MeV/Application Framework)
 * ===================================================================== */

#include "RefCount.h"

#ifdef __POWERPC__
#pragma export on
#endif

CRefCountObject *CRefCountObject::Acquire()
{
	atomic_add( &count, 1 );
	return this;
}

void CRefCountObject::Release( CRefCountObject *inObj )
{
	if (inObj)
	{
		if (atomic_add( &inObj->count, -1 ) == 1)
		{
			delete inObj;
		}
	}
}

#ifdef __POWERPC__
#pragma export off
#endif
