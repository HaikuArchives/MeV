/* ===================================================================== *
 * Reader.h (MeV/Support)
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
 *  Defines abstract base for writer classes.
 * ---------------------------------------------------------------------
 * History:
 *	1997		Talin
 *		Original implementation
 *	05/15/2000	malouin
 *		Updated byte-swapping code, fixed bugs, and cleaned up warnings
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */
  
#ifndef WRITER_H
#define WRITER_H

// Kernel Kit
#include <OS.h>

// Support Kit
#include <ByteOrder.h>

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
#if __LITTLE_ENDIAN
		if (convertByteOrder)
			inValue = htons( inValue );
#endif
		MustWrite( &inValue, sizeof inValue );
	}

		/**	Writes a 32-bit integer, or throws an exception.
			Also byte-swaps the integer if requested.
		*/
	void MustWriteInt32( int32 inValue )
	{
#if __LITTLE_ENDIAN
		if (convertByteOrder)
			inValue = htonl( inValue );
#endif
		MustWrite( &inValue, sizeof inValue );
	}

private:
	void MustSwapAndWrite( void *ptr, long bytes )
	{
#if __LITTLE_ENDIAN
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
	uint32			pos;
	uint32			len;

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
		if (inLength < 0 || uint32(inLength) > len - pos)
			return false;

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
		if (inFilePos > len)
			return false;
		pos = inFilePos;
		return true;
	}
};

	/**	A Writer which writes to a dynamic array of bytes. */
class CDynamicByteArrayWriter {
	uint8			*byteArray;
	uint32			pos;
	uint32			len;

public:

		/**	Constructor. */
	CDynamicByteArrayWriter( uint32 inInitialLen = 0 )
	{
		pos = 0;

		if (inInitialLen > 0)
			byteArray = new uint8[ inInitialLen ];
		else
			byteArray = NULL;

		len = inInitialLen;
	}
	
		/** Destructor. Deletes the internal array of data. */
	~CDynamicByteArrayWriter() { delete[] byteArray; }

		/**	Main writing function. */
	bool Write( void *buffer, int32 inLength )
	{
		if (inLength < 0)
		{
			return false;
		}
		else if (uint32(inLength) > len - pos)
		{
			int32	newSize = len + len / 4 + 256;
			uint8	*newBuf = new uint8[ newSize ];
			
			memcpy( newBuf, byteArray, len );
			delete[] byteArray;
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
		if (inFilePos > len)
			return false;
		pos = inFilePos;
		return true;
	}
	
		/** Returns the array of bytes written. Pointer is not stable between writes. */
	void *Buffer() { return byteArray; }
};

#endif /* WRITER_H */