/* ===================================================================== *
 * Writer.cpp (MeV/Support)
 * ===================================================================== */

#include "Writer.h"

// ---------------------------------------------------------------------------
// CWriter Implementation

void
CWriter::WriteStr255(
	const char *buffer,
	int32 length)
{
	if (length > 255)
		length = 255;
	MustWrite(&length, sizeof(uint8));
	MustWrite(buffer, length);
}

void
CWriter::MustWriteInt16(
	int16 value)
{
#if __LITTLE_ENDIAN
	if (m_convertByteOrder)
		value = htons(value);
#endif
	MustWrite(&value, sizeof(value));
}

void
CWriter::MustWriteInt32(
	int32 value)
{
#if __LITTLE_ENDIAN
	if (m_convertByteOrder)
		value = htonl(value);
#endif
	MustWrite(&value, sizeof(value));
}

void
CWriter::MustSwapAndWrite(
	void *ptr,
	long bytes)
{
#if __LITTLE_ENDIAN
	if (m_convertByteOrder)
	{
		for (int i = 0, j = bytes - 1; i < j; i++, j--)
		{
			uint8 t = ((uint8*)ptr)[i];
			((uint8*)ptr)[i] = ((uint8*)ptr)[j];
			((uint8*)ptr)[j] = t;
		}
	}
#endif

	MustWrite(ptr, bytes);
}

CWriter &
CWriter::operator<<(
	rgb_color color)
{
	MustWrite(&color.red, sizeof(color.red));
	MustWrite(&color.green, sizeof(color.green));
	MustWrite(&color.blue, sizeof(color.blue));
	MustWrite(&color.alpha, sizeof(color.alpha));

	return *this;
}

// ---------------------------------------------------------------------------
// CFixedByteArrayWriter Implementation

bool
CFixedByteArrayWriter::Write(
	void *buffer,
	int32 length)
{
	if ((length < 0) || (uint32(length) > (m_len - m_pos)))
		return false;

	memcpy(&m_bytes[m_pos], buffer, length);
	m_pos += length;
	return length;
}

bool
CFixedByteArrayWriter::Seek(
	uint32 filePos)
{
	if (filePos > m_len)
		return false;
	m_pos = filePos;
	return true;
}

// ---------------------------------------------------------------------------
// CDynamicByteArrayWriter Implementation

CDynamicByteArrayWriter::CDynamicByteArrayWriter(
	uint32 initialLength)
	:	m_pos(0),
		m_len(initialLength)
{
	if (m_len > 0)
		m_bytes = new uint8[m_len];
	else
		m_bytes = NULL;
}
	
bool
CDynamicByteArrayWriter::Write(
	void *buffer,
	int32 length)
{
	if (length < 0)
	{
		return false;
	}
	else if (uint32(length) > (m_len - m_pos))
	{
		int32 newSize = m_len + (m_len / 4) + 256;
		uint8 *newBuffer = new uint8[newSize];

		memcpy(newBuffer, m_bytes, m_len);
		delete [] m_bytes;
		m_bytes = newBuffer;
		m_len = newSize;
	}

	memcpy(&m_bytes[m_pos], buffer, length);
	m_pos += length;
	return length;
}

bool
CDynamicByteArrayWriter::Seek(
	uint32 filePos)
{
	if (filePos > m_len)
		return false;
	m_pos = filePos;
	return true;
}
	
// END - Writer.cpp
