/* ===================================================================== *
 * Sylvan Technical Arts Class Library, Copyright © 1997 Talin.
 * BeFileWriter.h -- Writer class which writes to BeOS file.
 * ---------------------------------------------------------------------
 * $NoKeywords: $
 * ===================================================================== */
 
#ifndef __C_BeFileWriter_H__
#define __C_BeFileWriter_H__

#include "Writer.h"

// Storage Kit
#include "File.h"

extern void CheckBeError( status_t errCode );

	/**	An abstract class representing a writeable stream of bytes.
		You make subclasses for files, strings, packets, etc.
	*/
class CBeFileWriter : public CAbstractWriter {
	BFile			&file;

public:
		/**	Constructor taking a file name. */
	CBeFileWriter( BFile &inFile ) : file( inFile )
	{
		CheckBeError( file.InitCheck() );
	}

		/**	Main writing function. This is pure virtual, and must be
			implemented by subclasses.
		*/
	bool Write( void *buffer, int32 inLength )
	{
		CheckBeError( file.Write( buffer, inLength ) );
		return true;
	}

		/**	Write exactly 'inLength' bytes, or toss an exception. */
	void MustWrite( void *buffer, int32 inLength )
	{
		CheckBeError( file.Write( buffer, inLength ) );
	}
	
		/**	Returns the current write position. */
	uint32 Position()
	{
		return file.Position();
	}

		/**	Seek to a specific position in the file.
			Not supported by all stream types.
		*/
	bool Seek( uint32 inFilePos )
	{
		if (file.Seek( inFilePos, SEEK_SET ) < 0) return false;
		return true;
	}
};

#endif /* __C_BeFileWriter_H__ */
