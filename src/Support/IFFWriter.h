/* ===================================================================== *
 * IFFWriter.h (MeV/Support)
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
 *		Curt Malouin (malouin)
 *
 * ---------------------------------------------------------------------
 * Purpose:
 *  Writer that writes IFF files.
 * ---------------------------------------------------------------------
 * History:
 *	1997		Talin
 *		Original implementation
 *	05/15/2000	malouin
 *		Chunk headers always stored big endian
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef IFF_WRITER_H
#define IFF_WRITER_H

#ifndef WRITER_H
#include "Writer.h"
#endif

	/**	An output stream handle for IFF files which is also an AbstractWriter (i.e.
		it can use the stream operators.)
	*/
class CIFFWriter : public CAbstractWriter {

	struct ChunkState {
		ChunkState	*parent;		// pointer to parent chunk.
		int32		id;			// chunk ID
		int32		startPos;		// position in file of start of chunk
		int32		maxSize;			// size of chunk.
	};
	
	ChunkState		*stack;
	CAbstractWriter	&writer;
	int32			limit;		// File position beyond which no writing occurs.
	int32			pos;

	void CalcLimit();

public:

		/**	Constructor
			@param inWriter Stream handle to write data to.
		*/
	CIFFWriter( CAbstractWriter &inWriter );
	~CIFFWriter();

		/**	Begin a new chunk nested within the current one. */
	bool Push( int32 chunkID, int32 length = -1 );

		/**	Finish the chunk we were working on. */
	bool Pop();
	
		/**	Returns the length in bytes of the current chunk. */
	int32 ChunkLength();

		/**	Returns the how many bytes of chunk data we have written. */
	int32 ChunkPos();

		/**	Returns the ID of the current chunk. */
	int32 ChunkID();

		/**	Write out data into a chunk (Overrides the CAbstractWriter function).
		*/
	bool Write( void *buffer, int32 length );

		/**	Write exactly 'inLength' bytes, or toss an exception. */
	void MustWrite( void *buffer, int32 inLength );

		/**	Push, write, and pop and entire chunk all at once. */
	bool WriteChunk( int32 chunkID, void *buffer, int32 length );
	
		/** Seek not supported. */
	bool Seek( uint32 inFilePos ) { return false; }
	
		/** Position does same as chunk pos( 0 ) */
	uint32 Position() { return ChunkPos(); }
};

#endif /* IFF_WRITER_H */