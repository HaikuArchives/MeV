/* ===================================================================== *
 * GeneralMidi.h (MeV/Midi)
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
 *  GeneralMidi definitions
 * ---------------------------------------------------------------------
 * History:
 *	1997		Talin
 *		Original implementation
 *	08/06/2000	cell
 *		Separated from MeVApp.cpp
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __GeneralMidi_H__
#define __GeneralMidi_H__

#include <SupportDefs.h>

namespace GeneralMidi
{
	const unsigned char			DRUM_KIT_CHANNEL = 9;

	const char *				GetProgramNameFor(
									uint8 program);

	const char *				GetDrumSoundNameFor(
									uint8 note);
};

#endif /* __GeneralMidi_H__ */
