/* ===================================================================== *
 * Sylvan Technical Arts Class Library, Copyright © 1997 Talin.
 * BeFileReader.h -- Reader class which reads from BeOS file.
 * ---------------------------------------------------------------------
 * $NoKeywords: $
 * ===================================================================== */
 
#ifndef BE_FILE_READER_H
#define BE_FILE_READER_H

#include "Reader.h"

// Gnu C Library
#include <stdio.h>
// Storage Kit
#include <File.h>

extern void CheckBeError( status_t errCode );

	/**	An abstract class representing a readable stream of bytes.
		You make subclasses for files, strings, packets, etc.
	*/
class CBeFileReader : public CAbstractReader {
	BFile			&file;
	bool			sizeOK;
	uint32		size;
	
public:

		/**	Constructor taking a file name. */
	CBeFileReader( BFile &inFile ) : file( inFile )
	{
		CheckBeError( file.InitCheck() );
		sizeOK = false;
	}

		/**	Main reading function. This is pure virtual, and must be
			implemented by subclasses.
		*/
	int32 Read( void *buffer, int32 inLength, bool inPartialOK = true );
	
		/**	Read exactly 'inLength' bytes, or toss an exception. */
	void MustRead( void *buffer, int32 inLength );
	
		/**	Returns the number of bytes remaining to be read in this stream.
			Note that in the case of pipes or other dynamic stream types,
			this number may change even when not reading from the stream.
		*/
	uint32 BytesAvailable()
	{
		off_t		sz;
	
		if (!sizeOK)
		{
			CheckBeError( file.GetSize( &sz ) );
			size = sz;
		}
		return size - file.Position();
	}
	
		/**	Returns the current write position. */
	uint32 Position()
	{
		return file.Position();
	}
	
		/**	Skip over data in the stream. Won't go past end of file. */
	uint32 Skip( uint32 inSkipBytes )
	{
		CheckBeError( file.Seek( inSkipBytes, SEEK_CUR ) );
		return Position();
	}

		/**	Seek to a specific position in the file.
			Not supported by all stream types.
			Returns true if it was successful.
		*/
	bool Seek( uint32 inFilePos )
	{
		if ( file.Seek( inFilePos, SEEK_SET ) < 0 ) return false;
		return true;
	}
};

#endif /* BE_FILE_READER_H */