/* ===================================================================== *
 * Sylvan Technical Arts Class Library, Copyright © 1997 Talin.
 * Writer.h -- Defines abstract base for writer classes.
 * ---------------------------------------------------------------------
 * $NoKeywords: $
 * ===================================================================== */
 
#ifndef WRITER_H
#define WRITER_H

// Kernel Kit
#include <OS.h>

	/**	An abstract class representing a writeable stream of bytes.
		You make subclasses for files, strings, packets, etc.
	*/
class CAbstractWriter {
protected:

		// This flag is TRUE if we want to convert all bytes to network order
	bool				convertByteOrder;

public:

		/**	Default constructor. */
	CAbstractWriter()
	{
		convertByteOrder = true;
	}

		/**	Virtual destructors are always a good idea, even if they don't
			do anything.
		*/
	virtual ~CAbstractWriter() {}

		/**	Indicate if we want the stream to automatically convert bytes
			to network byte order upon writing. Defaults to true. This setting
			has no effect on systems that are already big-endian.
		*/
	void ConvertByteOrder( bool inConvert ) { convertByteOrder = inConvert; }

		/**	Main writing function. This is pure virtual, and must be
			implemented by subclasses.
		*/
	virtual bool Write( void *buffer, int32 inLength ) = 0;

		/**	Write exactly 'inLength' bytes, or toss an exception. */
	virtual void MustWrite( void *buffer, int32 inLength )
	{
		Write( buffer, inLength );
	}
	
		/**	Writes a 16-bit integer, or throws an exception.
			Also byte-swaps the integer if requested.
		*/
	void MustWriteInt16( int16 inValue )
	{
#if _LITTLE_ENDIAN
		if (convertByteOrder) inValue = SwapByteOrder( (uint16)inValue );
#endif
		MustWrite( &inValue, sizeof inValue );
	}

		/**	Writes a 32-bit integer, or throws an exception.
			Also byte-swaps the integer if requested.
		*/
	void MustWriteInt32( int32 inValue )
	{
#if _LITTLE_ENDIAN
		if (convertByteOrder) inValue = SwapByteOrder( (uint32)inValue );
#endif
		MustWrite( &inValue, sizeof inValue );
	}

private:
	void MustSwapAndWrite( void *ptr, long bytes )
	{
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
		MustWrite( ptr, bytes );
	}
public:
	
		/**	Returns the current write position. */
	virtual uint32 Position() = 0;

		/**	Seek to a specific position in the file.
			Not supported by all stream types.
		*/
	virtual bool Seek( uint32 inFilePos ) = 0;

		/**	Stream output operator to write an int8 */
	CAbstractWriter &operator<<( int8    d ) { MustWrite( &d, sizeof d ); return *this; }

		/**	Stream output operator to write a uint8 */
	CAbstractWriter &operator<<( uint8   d ) { MustWrite( &d, sizeof d ); return *this; }

		/**	Stream output operator to write an int16 */
	CAbstractWriter &operator<<( int16   d ) { MustWriteInt16( d ); return *this; }

		/**	Stream output operator to write a uint16 */
	CAbstractWriter &operator<<( uint16  d ) { MustWriteInt16( (int16)d ); return *this; }

		/**	Stream output operator to write an int32 */
	CAbstractWriter &operator<<( int32 d ) { MustWriteInt32( d ); return *this; }

		/**	Stream output operator to write a uint32 */
	CAbstractWriter &operator<<( uint32 d ) { MustWriteInt32( (int32)d ); return *this; }

		/**	Stream output operator to write a float */
	CAbstractWriter &operator<<( float d ) { MustSwapAndWrite( &d, sizeof d ); return *this; }

		/**	Stream output operator to write a double */
	CAbstractWriter &operator<<( double d ) { MustSwapAndWrite( &d, sizeof d ); return *this; }
};

	/**	A Writer which writes to a fixed array of bytes. */
class CFixedByteArrayWriter : public CAbstractWriter {
	uint8			*byteArray;
	int32			pos;
	int32			len;

public:

		/**	Constructor. */
	CFixedByteArrayWriter( void *inByteArray, int32 inLen )
	{
		byteArray = (uint8 *)inByteArray;
		pos = 0;
		len = inLen;
	}

		/**	Main writing function. */
	bool Write( void *buffer, int32 inLength )
	{
		if (inLength > len - pos) return false;

		memcpy( &byteArray[ pos ], buffer, inLength );
		pos += inLength;
		return inLength;
	}

		/**	Returns the current write position. */
	uint32 Position() { return pos; }

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

	/**	A Writer which writes to a dynamic array of bytes. */
class CDynamicByteArrayWriter {
	uint8			*byteArray;
	int32			pos;
	int32			len;

public:

		/**	Constructor. */
	CDynamicByteArrayWriter( int32 inInitialLen = 0 )
	{
		pos = 0;

		if (inInitialLen > 0)
		{
			byteArray = new uint8[ inInitialLen ];
			len = inInitialLen;
		}

		byteArray = NULL;
		len = inInitialLen;
	}
	
		/** Destructor. Deletes the internal array of data. */
	~CDynamicByteArrayWriter() { delete byteArray; }

		/**	Main writing function. */
	bool Write( void *buffer, int32 inLength )
	{
		if (inLength > len - pos)
		{
			int32	newSize = len + len / 4 + 256;
			uint8	*newBuf = new uint8[ newSize ];
			
			if (newBuf)
			{
				memcpy( newBuf, byteArray, len );
				delete byteArray;
			}
			byteArray = newBuf;
			len = newSize;
		}

		memcpy( &byteArray[ pos ], buffer, inLength );
		pos += inLength;
		return inLength;
	}

		/**	Returns the current write position. */
	uint32 Position() { return pos; }

		/**	Seek to a specific position in the file.
			Not supported by all stream types.
		*/
	bool Seek( uint32 inFilePos )
	{
		if (inFilePos < 0 || inFilePos > len) return false;
		pos = inFilePos;
		return true;
	}
	
		/** Returns the array of bytes written. Pointer is not stable between writes. */
	void *Buffer() { return byteArray; }
};

#endif /* WRITER_H */