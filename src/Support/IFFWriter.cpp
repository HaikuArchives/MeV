/* ===================================================================== *
 * Sylvan Technical Arts Class Library, Copyright © 1997 Talin.
 * IFFWriter.cp -- Writer that writes IFF files.
 * ---------------------------------------------------------------------
 * $NoKeywords: $
 * ===================================================================== */
 
#include "IFFWriter.h"

CIFFWriter::CIFFWriter( CAbstractWriter &inWriter )
	: writer( inWriter )
{
		// Initialize stack and position counter to 0.
	stack = NULL;
	pos = 0;
	limit = 0;
}

CIFFWriter::~CIFFWriter()
{
		// Pop all unpopped chunks.
	while (stack != NULL) Pop();
}

void CIFFWriter::CalcLimit()
{
		// Look through the stack of chunks for chunks which have a predetermined
		// size. Find the chunk who's predetermined end is the lowest in the file.
	limit = LONG_MAX;
	for (ChunkState *st = stack; st != NULL; st = st->parent)
	{
		if (st->maxSize >= 0 && limit > st->startPos + st->maxSize)
			limit = st->startPos + st->maxSize;
	}
}

bool CIFFWriter::Push( int32 chunkID, int32 length )
{
	ChunkState		*newState = new ChunkState;
	uint32			chunkHeader[ 2 ];
	
		// Create a new chunk record and fill it in.
	newState->parent		= stack;
	newState->id			= chunkID;
	newState->startPos		= pos + 8;
	newState->maxSize	= length;
	stack = newState;
	
		// Write out the header, and keep track of file position.
	chunkHeader[ 0 ] = chunkID; //HToNL( chunkID );
	chunkHeader[ 1 ] = length; //HToNL( length );
	writer.MustWrite( chunkHeader, 8 );
	pos += 8;
	
		// Calculate the limit beyond which we cannot write without exceeding constraints
	CalcLimit();
}

bool CIFFWriter::Pop()
{
	int32		currentLength,
				idealLength;

		// Compute the current size of the chunk, and the fixed size, if any.
	currentLength = pos - stack->startPos;
	idealLength = stack->maxSize;
	if (idealLength < 0)
	{
			// Go back and fix chunk length in chunk header.
		writer.Seek( stack->startPos - 4 );
		int32	len = currentLength; //HToNL( currentLength );
		writer.MustWrite( &len, 4 );
		
			// Seek to current file position.
		writer.Seek( pos );

			// Write pad byte at end of chunk
		if (currentLength & 1)
		{
			writer.MustWrite( (void *)"\0",  1 );
			pos ++;
			currentLength++;
		}
	}
	else
	{
			// Add pad bytes to end of chunk until fixed length is reached.
		idealLength = (idealLength + 1) & ~1;
		while (currentLength < idealLength)
		{
			int32	len = MIN( 16, idealLength - currentLength );

			writer.MustWrite( (void *)"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0",  len );
			pos += len;
			currentLength += len;
		}
	}
	
		// Pop a chunk state off of the stack.
	ChunkState *parent = stack->parent;
	delete stack;
	stack = parent;

		// Now that the stack has been popped, calculate the limit beyond
		// which we cannot write without exceeding constraints
	CalcLimit();
}

int32 CIFFWriter::ChunkLength()
{
	return pos - stack->startPos;
}

int32 CIFFWriter::ChunkPos()
{
	return pos - stack->startPos;
}

int32 CIFFWriter::ChunkID()
{
	return pos - stack->id;
}

bool CIFFWriter::Write( void *buffer, int32 length )
{
		// Clip length to maximum chunk length
	if (length > limit - pos) return false;

		// Write chunk data
	writer.MustWrite( buffer, length );
	pos += length;
	return true;
}

void CIFFWriter::MustWrite( void *buffer, int32 inLength )
{
		// Write chunk data
	writer.MustWrite( buffer, inLength );
	pos += inLength;
}

bool CIFFWriter::WriteChunk( int32 chunkID, void *buffer, int32 length )
{
	uint32			chunkHeader[ 2 ];
	
		// If chunk would exceed limit, then it can't be written
	if (length + 8 > limit - pos) return false;

		// Write out the header and entire chunk body. Don't bother to push/pop.
	chunkHeader[ 0 ] = chunkID; //HToNL( chunkID );
	chunkHeader[ 1 ] = length; //HToNL( length );
	writer.MustWrite( chunkHeader, 8 );
	writer.MustWrite( buffer, length );
	if (length & 1)
	{
		length++;
		writer.MustWrite( (void *)"\0", 1 );
	}
	pos += length + 8;
	return true;
}
