/* ===================================================================== *
 * MidiDeviceInfo.h (MeV/Midi)
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
 *  Information to track MIDI devices
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

#ifndef __C_MidiDeviceInfo_H__
#define __C_MidiDeviceInfo_H__

#include "DocApp.h"
#include "Dictionary.h"

// Gnu C Library
#include <string.h>

// ---------------------------------------------------------------------------
// A device definition

	/** Defines a single patch. Right now, all that means is the name. */
class PatchInfo {
	char			*name;
	
public:
	PatchInfo() { name = NULL; }
	PatchInfo( char *inName ) { name = strdup( inName ); }
	~PatchInfo() { if (name) free( name ); }
	PatchInfo( const PatchInfo &inPatch ) { name = strdup( inPatch.name ); }
	
	const char *Name() const { return name; }
	void SetName( const char *inName )
		{ if (name) free( name ); name = strdup( inName ); }
};

const int32		Max_Device_Name = 48;

	/** A class which can hold data about a MIDI instrument. */
class MIDIDeviceInfo {
	friend class	CMeVApp;

public:
	int8			portNum,
				lowChannel,
				highChannel,
				baseChannel;
	char			name[ Max_Device_Name ];

		// This device supports the "bank select" function.
	bool			bankSelect;

		// A list of banks associated with this instrument.
	CDictionary<uint32,PatchInfo> patches;
	
private:
		/** Constructor. */
	MIDIDeviceInfo()
	{
		portNum = 0;
		lowChannel = highChannel = baseChannel = 0;
		name[ 0 ] = 0;
		bankSelect = false;
	}
	
		/** Destructor. */
	~MIDIDeviceInfo();
	
public:
		/** Set the name of this instrument. */
	void SetName( char *inName )
	{
		strncpy( name, inName, sizeof name );
		name[ sizeof name - 1 ] = '\0';
	}
	
		/** Set which port this is associated with. */
	void SetPort( int8 inPortNum ) { portNum = inPortNum; }

		/** Set the range of channels for this instrument. */
	void SetChannels( int8 inLowChannel, int8 inHighChannel, int8 inBaseChannel )
	{
		lowChannel = inLowChannel;
		highChannel = inHighChannel;
		baseChannel = inBaseChannel;
	}
	
		/** Assign a patch bank to this instrument. */
//	void SetPatchBank( int32 inBankIndex, PatchBank *inBank );

		/** Retrieve a patch bank associated with this instrument. */
//	PatchBank *GetPatchBank( int32 inBankIndex );

		/** Retrive the patch associated with this bank and channel */
	PatchInfo *GetPatch( int32 inBankIndex, uint8 inPatchIndex );

		/** Set the patch associated with this bank and channel. */
	void SetPatch( int32 inBankIndex, uint8 inPatchIndex, const PatchInfo &patch );

		/** Get a particular patch name */
	const char *GetPatchName( int32 inBankIndex, uint8 inPatchIndex );

		/** Get a particular patch name */
	void SetPatchName( int32 inBankIndex, uint8 inPatchIndex, const char *name );
	
		/** Delete a patch. */
	void DeletePatch( int32 inBankIndex, uint8 inPatchIndex );
	
		/** Returns TRUE if bank-select is supported. */
	bool SupportsProgramBanks() { return bankSelect; }
};

#endif /* __MidiDeviceInfo_H__ */
