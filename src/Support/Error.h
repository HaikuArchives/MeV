/* ===================================================================== *
 * Error.h (MeV/Support)
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
 *  Contributor(s): 
 *		Curt Malouin (malouin)
 *
 * ---------------------------------------------------------------------
 * Purpose:
 *  Defines abstract base for MeV exceptions.
 * ---------------------------------------------------------------------
 * History:
 *	09/13/2000	malouin
 *		Original implementation based on fragments in Talin's code
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */
#ifndef __I_Error_H__
#define __I_Error_H__

class IError
{
public:
	virtual					~IError() { /* no op */ }

	// Implemented by derived classes to return a human-readable
	// description of the error.
	virtual	const char*		Description() const = 0;
};

#endif /* __I_Error_H__ */