/* ===================================================================== *
 * Sylvan Technical Arts Class Library, Copyright © 1997 Talin.
 * IFFWriter.h -- Writer that writes IFF files.
 * ---------------------------------------------------------------------
 * $NoKeywords: $
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