/* ===================================================================== *
 * Sylvan Technical Arts Class Library, Copyright © 1997 Talin.
 * IFFRead.h -- Reader that reads IFF files.
 * ---------------------------------------------------------------------
 * $NoKeywords: $
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