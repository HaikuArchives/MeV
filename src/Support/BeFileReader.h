/* ===================================================================== *
 * BeFileReader.h (MeV/Support)
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
 * History:
 *	1997		Talin
 *		Original implementation
 *	09/25/2000	cell
 *		Some cleanup
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */
 
#ifndef __C_BeFileReader_H__
#define __C_BeFileReader_H__

#include "Reader.h"

// Gnu C Library
#include <stdio.h>

/**	An abstract class representing a readable stream of bytes.
	You make subclasses for files, strings, packets, etc.
	@authors	Talin, Curt Malouin, Christopher Lenz
 */
class CBeFileReader
	:	public CAbstractReader
{

public:							// Constructor/Destructor

	/**	Constructor taking a BFile. */
								CBeFileReader(
									BFile &file);

public:							// CAbstractReader Implementation

	/**	Main reading function. This is pure virtual, and must be
		implemented by subclasses.
	*/
	int32						Read(
									void *buffer,
									int32 length,
									bool partialOK = true);
	
	/**	Read exactly 'inLength' bytes, or toss an exception. */
	void						MustRead(
									void *buffer,
									int32 length);

	/**	Returns the number of bytes remaining to be read in this stream.
		Note that in the case of pipes or other dynamic stream types,
		this number may change even when not reading from the stream.
	*/
	uint32						BytesAvailable();

	/**	Returns the current write position. */
	uint32						Position() const;

	/**	Skip over data in the stream. Won't go past end of file. */
	uint32						Skip(
									uint32 skipBytes);

	/**	Seek to a specific position in the file.
		Not supported by all stream types.
		Returns true if it was successful.
	*/
	bool						Seek(
									uint32 filePos);

private:						// Instance Data

	BFile &						m_file;

	uint32						m_size;
	
	bool						m_sizeOK;
};

#endif /* __C_BeFileReader_H__ */
