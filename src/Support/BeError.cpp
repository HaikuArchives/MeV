/* ===================================================================== *
 * Sylvan Technical Arts Class Library, Copyright © 1997 Talin.
 * IFFReader.cp -- Reader that reads IFF files.
 * ---------------------------------------------------------------------
 * $NoKeywords: $
 * ===================================================================== */
 
// Storage Kit
#include <File.h>

void CheckBeError( status_t errCode );
void CheckBeError( status_t errCode )
{
	char			*msg;
	
	if (errCode >= 0) return;

	switch (errCode) {
	case B_NO_ERROR: return;
	case B_NO_INIT:	msg = "File object is uninitialized"; break;
	case B_BAD_VALUE: msg = "NULL path in dir/path, or some other argument is uninitialized"; break;
	case B_ENTRY_NOT_FOUND: msg = "File not found, or couldn't create the file"; break;
	case B_FILE_EXISTS: msg = "File already exists"; break;
	case B_PERMISSION_DENIED: msg = "Permission denied"; break;
	case B_NO_MEMORY: msg = "Couldn't allocate necessary memory to complete the operation"; break;
	case B_NOT_ALLOWED: msg = "Operation not allowed"; break;
	case B_DEVICE_FULL: msg = "Device full"; break;
//	case B_BAD_FILE: msg = "Uninitialized file"; break;
	default: msg = "Unknown error"; break;
	}
}
