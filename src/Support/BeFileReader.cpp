/* ===================================================================== *
 * Sylvan Technical Arts Class Library, Copyright © 1997 Talin.
 * IFFReader.cp -- Reader that reads IFF files.
 * ---------------------------------------------------------------------
 * $NoKeywords: $
 * ===================================================================== */
 
#include "BeFileReader.h"
#include "Error.h"

	/**	Main reading function. This is pure virtual, and must be
		implemented by subclasses.
	*/
int32 CBeFileReader::Read( void *buffer, int32 inLength, bool inPartialOK )
{
	if (inPartialOK)
	{
		int32 result = file.Read( buffer, inLength );
		CheckBeError( result );
		return result;
	}
	else
	{
		int32	pos = Position();
		
		int32 result = file.Read( buffer, inLength );
		CheckBeError( result );
		
		if (result != inLength)
		{
			file.Seek( pos, SEEK_SET );
			return 0;
		}
		return result;
	}
}

	/**	Read exactly 'inLength' bytes, or toss an exception. */
void CBeFileReader::MustRead( void *buffer, int32 inLength )
{
	class EndOfFile :
		public IError
	{
	public:
		virtual	const char*		Description() const
									{ return "File is incomplete or corrupt"; }
	};

	int32 bytesRead = file.Read( buffer, inLength );
	CheckBeError( bytesRead );
	if (bytesRead < inLength)
		throw EndOfFile();
}
