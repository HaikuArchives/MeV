/* ===================================================================== *
 * Serializable.h (MeV/Framework)
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
 *  History:
 *  11/10/2000	cell
 *		Original implementation
 * ===================================================================== */

#ifndef __C_Serializable_H__
#define __C_Serializable_H__

#include "IFFReader.h"
#include "IFFWriter.h"

// Kernel Kit
#include <OS.h>

/**
 *  Defines a common interface for classes that can be serialized to
 *	IFF chunks.
 *
 *	A concrete subclass of CSerializable must either provide a 
 *	Deserialization constructor that accepts a CIFFReader as argument,
 *	or implement the ReadChunk() method.
 *
 *	@author		Christopher Lenz
 */
class CSerializable
{

public:							// Hook Functions

	/** Subclasses implement this method to store the object into an IFF
	 *	chunk.
	 */
	virtual void				Serialize(
									CIFFWriter &writer) = 0;

	virtual void				ReadChunk(
									CIFFReader &reader);
};

#endif /* __C_Serializable_H__ */
