/* ===================================================================== *
 * IFFReader.h (MeV/Support)
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
 *  Reader that reads IFF files.
 * ---------------------------------------------------------------------
 * History:
 *	1997		Talin
 *		Original implementation
 *	05/15/2000	malouin
 *		Chunk headers always stored big endian
 *	06/05/2000	malouin
 *		Constructor reads top-level FORM chunk properly
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */
 
#ifndef IFF_READER_H
#define IFF_READER_H

#ifndef STREAMLIB_READER_H
#include "Reader.h"
#endif

	/**	An input stream handle for IFF files which is also an AbstractReader (i.e.
		it can use the stream operators.)
	*/
class CIFFReader : public CAbstractReader {

	struct ChunkState {
		ChunkState	*parent;
		int32		id;
		int32		subID;
		int32		filePos;
		int32		size;
		uint8		*tempBuffer;			//	For chunks who's size is known.
	};
	
	ChunkState		*stack;
	CAbstractReader	&reader;
	int32			pos;
	
public:

		/**	Constructor
			@param inReader Stream handle to read data from.
		*/
	CIFFReader( CAbstractReader &inReader );
	~CIFFReader();
	
		/**	Returns the length of the chunk being read */
	int32 ChunkLength( int32 parentLevel = 0 );
	
		/**	Returns how many bytes of chunk data have been read so far. */
	int32 ChunkPos( int32 parentLevel = 0 );
	
		/**	Returns the ID of the current chunk being read. */
	int32 ChunkID( int32 parentLevel = 0, int32 *subID = NULL );

		/**	Skips to the next chunk.
		*/
	bool NextChunk();

		/**	Call this function after NextChunk to instruct the reader to parse the current
			chunk recursively (i.e. embedded forms and such).
		*/
	void Push();
	
		/**	Call this function after NextChunk returns false to instruct the reader to
			continue parsing at the level enclosing the current chunk. Returns false
			if there is no enclosing chunk.
		*/
	bool Pop();
	
		/**	Read data from chunk. Will not read past the logical end of the chunk.
			Overrides the Read() function from CAbstractReader.
		*/
	int32 Read( void *buffer, int32 length, bool inPartialOK );
	
		/**	Read exactly 'inLength' bytes, or toss an exception. */
	void MustRead( void *buffer, int32 inLength );

		/**	Skip over chunk data. Won't skip past end of chunk. */
	uint32 Skip( uint32 length );

		/**	Seek to a specific position in the chunk. Won't seek past end of chunk.
		*/
	bool Seek( uint32 inChunkPos );

		/**	Returns the current read position in the chunk. */
	virtual uint32 Position() { return ChunkPos(); }

		/** Returns the number of bytes left in the current chunk. */
	virtual uint32 BytesAvailable() { return ChunkLength() - ChunkPos(); }
};

#endif /* IFF_READER_H */