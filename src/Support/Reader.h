/* ===================================================================== *
 * Sylvan Technical Arts Class Library, Copyright © 1997 Talin.
 * Reader.h -- Defines abstract base for reader classes.
 * ---------------------------------------------------------------------
 * $NoKeywords: $
 * ===================================================================== */
 
#ifndef STREAMLIB_READER_H
#define STREAMLIB_READER_H

// Kernel Kit
#include <OS.h>

	/**	An abstract class representing a readable stream of bytes.
		You make subclasses for files, strings, packets, etc.
	*/
class CAbstractReader {
protected:

		//	This flag is TRUE if we want to convert all bytes to network order
	bool				convertByteOrder;

public:

		/**	Default constructor. */
	CAbstractReader()
	{
		convertByteOrder = true;
	}

		/**	Virtual destructors are always a good idea, even if they don't
			do anything.
		*/
	virtual ~CAbstractReader() {}
	
		/**	Indicate if we want the stream to automatically convert bytes
			from network byte order upon load. Defaults to true. This setting
			has no effect on systems that are already big-endian.
		*/
	void ConvertByteOrder( bool inConvert ) { convertByteOrder = inConvert; }

		/**	Main reading function. This is pure virtual, and must be
			implemented by subclasses. If "inPartialOK" is false, then this is
			an "all or nothing" read.
		*/
	virtual int32 Read( void *buffer, int32 inLength, bool inPartialOK = true ) = 0;
	
		/**	Read exactly 'inLength' bytes, or toss an exception. */
	virtual void MustRead( void *buffer, int32 inLength )
	{
		Read( buffer, inLength, false );
	}
	
		/**	Returns a 16-bit integer, or throws an exception.
			Also byte-swaps the integer if requested.
		*/
	int16 MustReadInt16()
	{
		uint16		v;
		
		MustRead( &v, sizeof v );

#if _LITTLE_ENDIAN
		if (convertByteOrder) v = SwapByteOrder( v );
#endif
		return v;
	}

		/**	Returns a 32-bit integer, or throws an exception.
			Also byte-swaps the integer if requested.
		*/
	int32 MustReadInt32()
	{
		uint32		v;
		
		MustRead( &v, sizeof v );

#if _LITTLE_ENDIAN
		if (convertByteOrder) v = SwapByteOrder( v );
#endif
		return v;
	}

private:
	void MustReadAndSwap( void *ptr, long bytes )
	{
		MustRead( ptr, bytes );
#if _LITTLE_ENDIAN
		if (convertByteOrder)
		{
			for (int i = 0, j = bytes - 1; i < j; i++, j--)
			{
				uint8		t = ((uint8*)ptr)[ i ];
				((uint8*)ptr)[ i ] = ((uint8*)ptr)[ j ];
				((uint8*)ptr)[ j ] = t;
			}
		}
#endif
	}
public:
	
		/**	Returns the number of bytes remaining to be read in this stream.
			Note that in the case of pipes or other dynamic stream types,
			this number may change even when not reading from the stream.
		*/
	virtual uint32 BytesAvailable() = 0;
	
		/**	Returns the current read position. */
	virtual uint32 Position() = 0;
	
		/**	Skip over data in the stream. Won't go past end of file. */
	virtual uint32 Skip( uint32 inSkipBytes ) = 0;

		/**	Seek to a specific position in the file.
			Not supported by all stream types.
			Returns true if it was successful.
		*/
	virtual bool Seek( uint32 inFilePos ) = 0;

		/**	Stream read operator to read an int8 */
	CAbstractReader &operator>>( int8 &d   ) { MustRead( &d, sizeof d ); return *this; }

		/**	Stream read operator to read a uint8 */
	CAbstractReader &operator>>( uint8 &d  ) { MustRead( &d, sizeof d ); return *this; }

		/**	Stream read operator to read an int16 */
	CAbstractReader &operator>>( int16  &d ) { d = MustReadInt16(); return *this; }

		/**	Stream read operator to read a uint16 */
	CAbstractReader &operator>>( uint16 &d ) { d = (uint16)MustReadInt16(); return *this; }

		/**	Stream read operator to read an int32 */
	CAbstractReader &operator>>( int32  &d ) { d = MustReadInt32(); return *this; }

		/**	Stream read operator to read a uint32 */
	CAbstractReader &operator>>( uint32 &d ) { d = (uint32)MustReadInt32(); return *this; }

		/**	Stream read operator to read a float */
	CAbstractReader &operator>>( float &d ) { MustReadAndSwap( &d, sizeof d); return *this; }

		/**	Stream read operator to read a double */
	CAbstractReader &operator>>( double &d ) { MustReadAndSwap( &d, sizeof d); return *this; }
};

	/**	A reader which reads from a fixed array of bytes, supplied at
		construction time.
	*/
class CByteArrayReader : public CAbstractReader {

	uint8			*byteArray;
	int32			pos;
	int32			len;

public:

		/**	Constructor. */
	CByteArrayReader( void *inByteArray, int32 inLen )
	{
		byteArray = (uint8 *)inByteArray;
		pos = 0;
		len = inLen;
	}

		/**	Main reading function.
		*/
	int32 Read( void *buffer, int32 inLength, bool inPartialOK )
	{
		int32		avail = len - pos;
		
		if (inLength > avail)
		{
				//	REM: Should we toss instead?
			if (inPartialOK == false) return -1;
			inLength = avail;
		}
		memcpy( buffer, &byteArray[ pos ], inLength );
		pos += inLength;
		return inLength;
	}
	
		/**	Returns the number of bytes remaining to be read in this stream.
			Note that in the case of pipes or other dynamic stream types,
			this number may change even when not reading from the stream.
		*/
	uint32 BytesAvailable()
	{
		return len - pos;
	}
	
		/**	Returns the current read position. */
	uint32 Position()
	{
		return pos;
	}
	
		/**	Skip over data in the stream. */
	uint32 Skip( uint32 inSkipBytes )
	{
		int32		sPos = pos;
		
		pos += inSkipBytes;
		if (pos >= len) pos = len;
		
		return len - sPos;
	}

		/**	Seek to a specific position in the file.
			Not supported by all stream types.
		*/
	bool Seek( uint32 inFilePos )
	{
		if (inFilePos < 0 || inFilePos > len) return false;
		pos = inFilePos;
		return true;
	}
};

#endif
