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
enum MeVFileIDs {

		// General IFF headers
	Form_ID				= 'FORM',
	Body_ID				= 'BODY',

		// MeV Document type
	MeV_ID				= 'MeV ',			// Idenfies a MeV file

		// Document chunk headers
	VCTable_ID			= 'vctb',			// Virtual channel table
	FMasterRealTrack		= 'mrTk',			// Master real track embedded form
	FMasterMeteredTrack	= 'mmTk',		// Master metered track embedded form
	DocTempo_ID			= 'dTem',			// Overall document tempo

		// Track chunk headers
	Track_Header_ID		= 'head',			// Track header chunk
	Track_Name_ID		= 'name',			// Track name chunk
	Track_Section_ID		= 'sect',			// Section markers chunk
	Track_Grid_ID			= 'grid',			// Gridsnap chunk
};

#endif /* __MeVFileID_H__ */
