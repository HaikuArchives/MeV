/* ===================================================================== *
 * Sylvan Technical Arts Class Library, Copyright © 1997 Talin.
 * IFFReader.cp -- Reader that reads IFF files.
 * ---------------------------------------------------------------------
 * $NoKeywords: $
 * ===================================================================== */
 
#include "IFFReader.h"

CIFFReader::CIFFReader( CAbstractReader &inReader )
	: reader( inReader )
{
	pos = 0;
	stack = NULL;

	Push();						// Push chunk representing overall form
	
	Push();						// Push next chunk to be parsed.
}

CIFFReader::~CIFFReader()
{
	while (stack != NULL)
	{
		ChunkState	*cs = stack;
		
		stack = stack->parent;
		delete cs;
	}
}
	
int32 CIFFReader::ChunkLength( int32 parentLevel )
{
	ChunkState	*cs = stack;
	while (parentLevel-- && cs != NULL)
	{
		cs = cs->parent;
	}		
	
	return cs->size;
}

int32 CIFFReader::ChunkPos( int32 parentLevel )
{
	ChunkState	*cs = stack;
	while (parentLevel-- && cs != NULL)
	{
		cs = cs->parent;
	}		
	
	return pos - cs->filePos;
}

int32 CIFFReader::ChunkID( int32 parentLevel, int32 *subID )
{
	ChunkState	*cs = stack;
	while (parentLevel-- && cs != NULL)
	{
		cs = cs->parent;
	}		
	
	if (subID) *subID = cs->subID;
	return cs->id;
}

bool CIFFReader::NextChunk()
{
		//	Skip over the rest of the current chunk, including pad byte.
	int32 newPos = stack->filePos + (stack->size + 1) & ~1;
	if (pos < newPos)
	{
		reader.Seek( newPos );
	}
	pos = newPos;
	
		// Check to see if there are any more chunks in this container
	if (pos > 0)
	{
		ChunkState	*parent = stack->parent;
		if (parent == NULL) return false;
		if (pos >= parent->filePos + parent->size) return false;
	}
	
		// Read the chunk header of the next chunk.
	uint32 chunkHeader[ 2 ];
	
	reader.MustRead( chunkHeader, 8 );
	pos += 8;
	
	stack->id = chunkHeader[ 0 ]; //NToHL( chunkHeader[ 0 ] );
	stack->size = chunkHeader[ 1 ]; //NToHL( chunkHeader[ 1 ] );
	stack->filePos = pos;
	stack->subID = 0;
	
	if (stack->id == 'FORM')
	{
		int32		id;
	
		reader.MustRead( &id, 4 );
		pos += 4;
		stack->subID = id; //NToHL( id );
	}
	return true;
}

void CIFFReader::Push()
{
	ChunkState		*cs = new ChunkState;
	
	cs->parent = stack;
	cs->id = 0;
	cs->subID = 0;
	cs->filePos = pos;
	cs->size = 0;
	stack = cs;
}

bool CIFFReader::Pop()
{
	if (stack != NULL)
	{
		ChunkState		*cs = stack;
		
		stack = stack->parent;
		delete cs;
		
		if (stack == NULL) return false;
		return true;
	}
	return false;
}

int32 CIFFReader::Read( void *buffer, int32 length, bool inPartialOK )
{
	if (length > stack->filePos + stack->size - pos)
		length = stack->filePos + stack->size - pos;
		
	if (length <= 0) return 0;

	length = reader.Read( buffer, length );
	if (length >= 0) pos += length;
	return length;
}

void CIFFReader::MustRead( void *buffer, int32 inLength )
{
	if (inLength > 0)
	{
		reader.MustRead( buffer, inLength );
		pos += inLength;
	}
}

uint32 CIFFReader::Skip( uint32 length )
{
	return Seek( pos + length );
}

bool CIFFReader::Seek( uint32 inFilePos )
{
	if (inFilePos < 0) inFilePos = 0;
	if (inFilePos > stack->size) inFilePos = stack->size;
	
	inFilePos += stack->filePos;
	if (reader.Seek( inFilePos ) == false) return false;
		
	pos = inFilePos;
	return true;
}
