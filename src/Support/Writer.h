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
 *	10/16/2000	cell
 *		Cleaned up style & added ReadStr255() method
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */
  
#ifndef __C_Writer_H__
#define __C_Writer_H__

// Kernel Kit
#include <OS.h>

// Interface Kit
#include <InterfaceDefs.h>
// Support Kit
#include <ByteOrder.h>
#include <String.h>

/**	An abstract class representing a writeable stream of bytes.
	You make subclasses for files, strings, packets, etc.
*/
class CWriter
{

public:							// Constructor/Destructor

	/**	Default constructor. */
								CWriter()
									:	m_convertByteOrder(true)
								{ }

	/**	Virtual destructors are always a good idea, even if they don't
		do anything.
	*/
	virtual						~CWriter() {}

public:							// Accessors

	/**	Indicate if we want the stream to automatically convert bytes
		to network byte order upon writing. Defaults to true. This setting
		has no effect on systems that are already big-endian.
	*/
	void						ConvertByteOrder(
									bool convert)
								{ m_convertByteOrder = convert; }

	/**	Returns the current write position. */
	virtual uint32				Position() const = 0;

public:							// Operations

	/**	Main writing function. This is pure virtual, and must be
		implemented by subclasses.
	*/
	virtual bool				Write(
									const void *buffer,
									int32 length) = 0;

	void						WriteStr255(
									const char *buffer,
									int32 length);

	/**	Write exactly 'inLength' bytes, or toss an exception. */
	virtual void				MustWrite(
									const void *buffer,
									int32 length)
								{ Write(buffer, length); }

	/**	Writes a 16-bit integer, or throws an exception.
		Also byte-swaps the integer if requested.
	*/
	void						MustWriteInt16(
									int16 value);

	/**	Writes a 32-bit integer, or throws an exception.
		Also byte-swaps the integer if requested.
	*/
	void						MustWriteInt32(
									int32 value);

	/**	Seek to a specific position in the file.
		Not supported by all stream types.
	*/
	virtual bool				Seek(
									uint32 filePos) = 0;

public:							// Overloaded Operators

	/**	Stream output operator to write a bool */
	CWriter &					operator<<(
									bool d)
								{ MustWrite(&d, sizeof(d)); return *this; }

	/**	Stream output operator to write an int8 */
	CWriter &					operator<<(
									int8 d)
								{ MustWrite(&d, sizeof(d)); return *this; }

	/**	Stream output operator to write a uint8 */
	CWriter &					operator<<(
									uint8 d)
								{ MustWrite(&d, sizeof(d)); return *this; }

	/**	Stream output operator to write an int16 */
	CWriter &					operator<<(
									int16 d)
								{ MustWriteInt16(d); return *this; }

	/**	Stream output operator to write a uint16 */
	CWriter &					operator<<(
									uint16 d)
								{ MustWriteInt16((int16)d); return *this; }

	/**	Stream output operator to write an int32 */
	CWriter &					operator<<(
									int32 d)
								{ MustWriteInt32(d); return *this; }

	/**	Stream output operator to write a uint32 */
	CWriter &					operator<<(
									uint32 d)
								{ MustWriteInt32((int32)d); return *this; }

	/**	Stream output operator to write an int64 */
	CWriter &					operator<<(
									int64 d)
								{ MustWrite(&d, sizeof(uint64)); return *this; }

	/**	Stream output operator to write a uint64 */
	CWriter &					operator<<(
									uint64 d)
								{ MustWrite(&d, sizeof(uint64)); return *this; }

	/**	Stream output operator to write a float */
	CWriter &					operator<<(
									float d)
								{ MustSwapAndWrite(&d, sizeof(d)); return *this; }

	/**	Stream output operator to write a double */
	CWriter &					operator<<(
									double d)
								{ MustSwapAndWrite(&d, sizeof(d)); return *this; }

	/** Stream output operator to write a rgb_color */
	CWriter &					operator<<(
									rgb_color color);

	/** Stream output operator to write a BString */
	CWriter &					operator<<(
									BString d)
								{ MustWrite(&d, d.Length()); return *this; }

private:						// Internal Operations

	void						MustSwapAndWrite(
									void *ptr,
									long bytes);

protected:						// Instance Data

	/** This flag is true if we want to convert all bytes to network order. */
	bool						m_convertByteOrder;
};

/**	A Writer which writes to a fixed array of bytes. */
class CFixedByteArrayWriter
	:	public CWriter
{

public:							// Constructor/Destructor

	/**	Constructor. */
								CFixedByteArrayWriter(
									void *byteArray,
									int32 length)
									:	m_bytes((uint8 *)byteArray),
										m_pos(0),
										m_len(length)
								{ }

public:							// CWriter Implementation

	/**	Main writing function. */
	
	bool						Write(
									void *buffer,
									int32 length);

	/**	Returns the current write position. */
	uint32						Position()
								{ return m_pos; }

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

/**	A Writer which writes to a dynamic array of bytes. */
class CDynamicByteArrayWriter
	:	public CWriter
{

public:							// Constructor/Destructor

	/**	Constructor. */
								CDynamicByteArrayWriter(
									uint32 initialLength = 0);

	/** Destructor. Deletes the internal array of data. */
								~CDynamicByteArrayWriter()
								{ delete [] m_bytes; }

	/** Returns the array of bytes written. 
		Pointer is not stable between writes.
	 */
	void *						Buffer() const
								{ return m_bytes; }

	/**	Main writing function. */
	bool						Write(
									void *buffer,
									int32 length);

	/**	Returns the current write position. */
	uint32						Position() const
								{ return m_pos; }

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

#endif /* WRITER_H */