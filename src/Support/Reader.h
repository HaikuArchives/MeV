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
 *		Christopher Lenz (cell)
 *
 * ---------------------------------------------------------------------
 * History:
 *	1997		Talin
 *		Original implementation
 *	05/15/2000	malouin
 *		Updated byte-swapping code and cleaned up warnings
 *	10/16/2000	cell
 *		Cleaned up style & added ReadStr255() method
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */
 
#ifndef __C_Reader_H__
#define __C_Reader_H__

// Kernel Kit
#include <OS.h>

// Interface Kit
#include <InterfaceDefs.h>
// Support Kit
#include <ByteOrder.h>
#include <String.h>

class CTime;
class CTimeSpan;

/**	An abstract class representing a readable stream of bytes.
	You make subclasses for files, strings, packets, etc.
*/
class CReader
{

public:							// Constructor/Destructor

	/**	Default constructor. */
								CReader()
									:	m_convertByteOrder(true)
								{ }

	/**	Virtual destructors are always a good idea, even if they don't
		do anything.
	*/
	virtual						~CReader()
								{ }

public:							// Accessors

	/**	Returns the number of bytes remaining to be read in this stream.
		Note that in the case of pipes or other dynamic stream types,
		this number may change even when not reading from the stream.
	 */
	virtual uint32				BytesAvailable() = 0;
	
	/**	Indicate if we want the stream to automatically convert bytes
		from network byte order upon load. Defaults to true. This setting
		has no effect on systems that are already big-endian.
	*/
	void						ConvertByteOrder(
									bool convert)
								{ m_convertByteOrder = convert; }

	/**	Returns the current read position. */
	virtual uint32				Position() const = 0;
	
public:							// Operations

	/**	Main reading function. This is pure virtual, and must be
		implemented by subclasses. If "inPartialOK" is false, then this is
		an "all or nothing" read.
	*/
	virtual int32				Read(
									void *buffer,
									int32 length,
									bool partialOK = true) = 0;

	int32						ReadStr255(
									char *outBuffer,
									int32 maxLength);

	/**	Read exactly 'inLength' bytes, or toss an exception. */
	virtual void				MustRead(
									void *buffer,
									int32 length)
								{ Read(buffer, length, false); }

	/**	Returns a 16-bit integer, or throws an exception.
	 *	Also byte-swaps the integer if requested. */
	int16						MustReadInt16();

	/**	Returns a 32-bit integer, or throws an exception.
	 *	Also byte-swaps the integer if requested. */
	int32						MustReadInt32();

	/**	Returns a 64-bit integer, or throws an exception.
	 *	Also byte-swaps the integer if requested. */
	int64						MustReadInt64();

public:
	
	/**	Skip over data in the stream. Won't go past end of file. */
	virtual uint32				Skip(
									uint32 skipBytes) = 0;

	/**	Seek to a specific position in the file.
		Not supported by all stream types.
		Returns true if it was successful.
	*/
	virtual bool				Seek(
									uint32 filePos) = 0;

public:							// Overloaded Operators

	/**	Stream read operator to read a bool */
	CReader &					operator>>(
									bool &d)
								{ MustRead(&d, sizeof(d)); return *this; }

	/**	Stream read operator to read an int8 */
	CReader &					operator>>(
									int8 &d)
								{ MustRead(&d, sizeof(d)); return *this; }

	/**	Stream read operator to read a uint8 */
	CReader &					operator>>(
									uint8 &d)
								{ MustRead(&d, sizeof(d)); return *this; }

	/**	Stream read operator to read an int16 */
	CReader &					operator>>(
									int16 &d)
								{ d = MustReadInt16(); return *this; }

	/**	Stream read operator to read a uint16 */
	CReader &					operator>>(
									uint16 &d)
								{ d = (uint16)MustReadInt16(); return *this; }

	/**	Stream read operator to read an int32 */
	CReader &					operator>>(
									int32 &d)
								{ d = MustReadInt32(); return *this; }

	/**	Stream read operator to read a uint32 */
	CReader &					operator>>(
									uint32 &d)
								{ d = (uint32)MustReadInt32(); return *this; }

	/**	Stream read operator to read an int32 */
	CReader &					operator>>(
									int64 &d)
								{ MustRead(&d, sizeof(int64)); return *this; }

	/**	Stream read operator to read a uint32 */
	CReader &					operator>>(
									uint64 &d)
								{ MustRead(&d, sizeof(uint64)); return *this; }

	/**	Stream read operator to read a float */
	CReader &					operator>>(
									float &d)
								{ MustReadAndSwap(&d, sizeof(d)); return *this; }

	/**	Stream read operator to read a double */
	CReader &					operator>>(
									double &d)
								{ MustReadAndSwap(&d, sizeof(d)); return *this; }

	/** Stream read operator to read a bstring*/
	CReader &					operator>>(
									rgb_color &color);

	/** Stream read operator to read a bstring*/
	CReader &					operator>>(
									BString &d)
								{ MustRead (&d, d.Length()); return *this;}

	/** Stream read operator to read a CTime. */
	CReader &					operator>>(
									CTime &time);

	/** Stream read operator to read a CTimeSpan. */
	CReader &					operator>>(
									CTimeSpan &span);

private:						// Internal Operations

	void						MustReadAndSwap(
									void *ptr,
									long bytes);

protected:						// Instance Data

	/**	This flag is true if we want to convert all bytes to network order. */
	bool						m_convertByteOrder;
};

/**	A reader which reads from a fixed array of bytes, supplied at
	construction time.
*/
class CByteArrayReader
	:	public CReader
{

public:							// Constructor/Destructor

	/**	Constructor. */
								CByteArrayReader(
									void *byteArray,
									int32 len)
									:	m_bytes((uint8 *)byteArray),
										m_pos(0),
										m_len(len)
								{ }

	/**	Main reading function. */
	int32						Read(
									void *buffer,
									int32 length,
									bool partialOK);

	/**	Returns the number of bytes remaining to be read in this stream.
		Note that in the case of pipes or other dynamic stream types,
		this number may change even when not reading from the stream.
	*/
	uint32						BytesAvailable()
								{ return m_len - m_pos; }
	
	/**	Returns the current read position. */
	uint32						Position()
								{ return m_pos; }

	/**	Skip over data in the stream. */
	uint32						Skip(
									uint32 skipBytes);

	/**	Seek to a specific position in the file.
		Not supported by all stream types.
	*/
	bool						Seek(
									uint32 filePos);

private:						// Instance Data

	uint8 *						m_bytes;

	uint32						m_pos;

	uint32						m_len;
};

#endif /* __C_Reader_H__ */
