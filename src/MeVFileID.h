/* ===================================================================== *
 * MeVFileID.h (MeV)
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
 *  Identifiers for file saving
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

#ifndef __MeVFileID_H__
#define __MeVFileID_H__

// File format IDs
enum MeVFileIDs
{
	// General IFF headers
	Form_ID						= 'FORM',
	Body_ID						= 'BODY',

	// MeV Document type
	MeV_ID						= 'MeV ',		// Idenfies a MeV file

	// Document chunk headers
	DOC_HEADER_CHUNK			= 'dTem',		// Document header
	ENVIRONMENT_CHUNK			= 'envi',		// Environment
	FMasterRealTrack			= 'mrTk',		// Master real track embedded form
	FMasterMeteredTrack			= 'mmTk',		// Master metered track embedded form

	// Track chunk headers
	TRACK_HEADER_CHUNK			= 'head',		// Track header chunk
	TRACK_NAME_CHUNK			= 'name',		// Track name chunk
	TRACK_SECTION_CHUNK 		= 'sect',		// Section markers chunk
	TRACK_GRID_CHUNK			= 'grid',		// Gridsnap chunk
	TRACK_WINDOW_CHUNK			= 'tkwd',		// Track window chunk

	// Environment chunk headers
	DESTINATION_CHUNK			= 'dst ',		// Destination
	SOURCE_CHUNK				= 'src ',		// Source

	// Destination chunks header
	DESTINATION_HEADER_CHUNK 	= 'dsth',
	DESTINATION_NAME_CHUNK 		= 'dstn',
};

#endif /* __MeVFileID_H__ */
