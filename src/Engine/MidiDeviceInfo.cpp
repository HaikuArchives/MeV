/* ===================================================================== *
 * MidiDeviceInfo.cpp (MeV/Engine)
 * ===================================================================== */

#include "MidiDeviceInfo.h"
#include "MeVApp.h"

MIDIDeviceInfo::~MIDIDeviceInfo()
{
#if 0
	CDictionary<uint16,PatchBank *>::Iterator	iter( banks );
	uint16			*key;
	
		// Dereference all patch banks...
	while (key = iter.Next())
	{
		CRefCountObject::Release( *iter.Value() );
	}
#endif
}

PatchInfo *MIDIDeviceInfo::GetPatch( int32 inBankIndex, uint8 inPatchIndex )
{
	return patches[ (inBankIndex << 7) + (inPatchIndex & 0x7f) ];
}

void MIDIDeviceInfo::SetPatch( int32 inBankIndex, uint8 inPatchIndex, const PatchInfo &patch )
{
	uint32		index = (inBankIndex << 7) + (inPatchIndex & 0x7f);
	PatchInfo		*pi = patches[ index ];
	
	if (pi) *pi = patch;
	else patches.Add( index, patch );
}

const char *MIDIDeviceInfo::GetPatchName( int32 inBankIndex, uint8 inPatchIndex )
{
	PatchInfo		*pi = GetPatch( inBankIndex, inPatchIndex );
	
	if (pi) return pi->Name();
	return NULL;
}

void MIDIDeviceInfo::SetPatchName( int32 inBankIndex, uint8 inPatchIndex, const char *name )
{
	PatchInfo		*pi = GetPatch( inBankIndex, inPatchIndex );
	
	if (pi == NULL) pi = &patches.Add( (inBankIndex << 7) + (inPatchIndex & 0x7f) );
	
	pi->SetName( name );
}

void MIDIDeviceInfo::DeletePatch( int32 inBankIndex, uint8 inPatchIndex )
{
	patches.Clear( (inBankIndex << 7) + (inPatchIndex & 0x7f) );
}
