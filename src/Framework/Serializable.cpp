/* ===================================================================== *
 * Serializable.cpp (MeV/Framework)
 * ===================================================================== */

#include "Serializable.h"

// Support Kit
#include <Debug.h>

#define D_ALLOC(x) //PRINT(x)		// Constructor/Destructor
#define D_HOOK(x) //PRINT(x)		// Hook Functions

// ---------------------------------------------------------------------------
// Hook Functions

void
CSerializable::ReadChunk(
	CIFFReader &reader)
{
	D_HOOK(("CSerializable::ReadChunk()\n"));
}

// END - Serializable.cpp
