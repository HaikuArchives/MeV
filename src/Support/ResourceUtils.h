/* ===================================================================== *
 * MeV.h (MeV)
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
 *  General MeV defines
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

#ifndef __U_ResourceUtils_H__
#define __U_ResourceUtils_H__

// Kernel Kit
#include <OS.h>

class BBitmap;

namespace ResourceUtils
{
	void *		LoadResource(
					type_code type,
					int32 resourceID,
					size_t *size);

	void *		LoadResource(
					type_code type,
					const char *resourceName,
					size_t *size);

	BBitmap *	LoadImage(
					int32 resourceID);

	BBitmap *	LoadImage(
					const char *resourceName);

	uint8 *		LoadCursor(
					int32 resourceID);

	uint8 *		LoadCursor(
					const char *resourceName);
};

#endif /* __U_ResourceUtils_H__ */
