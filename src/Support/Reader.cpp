/* ===================================================================== *
 * Reader.cpp (MeV/Support)
 * ===================================================================== */

#include "Reader.h"

#include "TimeSpan.h"

// ---------------------------------------------------------------------------
// CReader Implementation

int32
CReader::ReadStr255(
	char *outBuffer,
	int32 maxLength)
{
	uint8 stringLength;
	int32 actualLength;

	MustRead(&stringLength, sizeof(stringLength));
	actualLength = stringLength < (maxLength - 1) ?
				   stringLength :
				   maxLength - 1;
	MustRead(outBuffer, actualLength);
	outBuffer[actualLength] = 0;
	if (actualLength < stringLength)
		Skip(stringLength - actualLength);
	return actualLength;
}

int16
CReader::MustReadInt16()
{
	int16 v;
	MustRead(&v, sizeof(v));
#if __LITTLE_ENDIAN
		if (m_convertByteOrder)
			v = ntohs(v);
#endif
	return v;
}

int32
CReader::MustReadInt32()
{
	int32 v;

	MustRead(&v, sizeof(v));
#if __LITTLE_ENDIAN
		if (m_convertByteOrder)
			v = ntohl(v);
#endif
	return v;
}

int64
CReader::MustReadInt64()
{
	int64 v;

	MustRead(&v, sizeof(v));
#if __LITTLE_ENDIAN
		if (m_convertByteOrder)
			v = ntohl(v);
#endif
	return v;
}

void
CReader::MustReadAndSwap(
	void *ptr,
	long bytes)
{
	MustRead(ptr, bytes);
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
}

// ---------------------------------------------------------------------------
// CReader Implementation: Overloaded Operators

CReader &
CReader::operator>>(
	rgb_color &color)
{
	MustRead(&color.red, sizeof(color.red));
	MustRead(&color.green, sizeof(color.green));
	MustRead(&color.blue, sizeof(color.blue));
	MustRead(&color.alpha, sizeof(color.alpha));

	return *this;
}

CReader &
CReader::operator>>(
	CTime &time)
{
	time = MustReadInt64();

	return *this;
}

CReader &
CReader::operator>>(
	CTimeSpan &span)
{
	CTime start = MustReadInt64();
	CTime duration = MustReadInt64();
	span.SetTo(start, duration);

	return *this;
}

// ---------------------------------------------------------------------------
// CByteArrayReader Implementation

int32
CByteArrayReader::Read(
	void *buffer,
	int32 length,
	bool partialOK)
{
	uint32 avail = m_len - m_pos;

	if (length < 0)
	{
		return -1;
	}
	else if (uint32(length) > avail)
	{
		//	REM: Should we toss instead?
		if (partialOK == false)
			return -1;
		length = avail;
	}
	memcpy(buffer, &m_bytes[m_pos], length);
	m_pos += length;
	return length;
}
	
uint32
CByteArrayReader::Skip(
	uint32 skipBytes)
{
	int32 sPos = m_pos;

	m_pos += skipBytes;
	if (m_pos >= m_len)
		m_pos = m_len;

	return m_len - sPos;
}

bool
CByteArrayReader::Seek(
	uint32 filePos)
{
	if (filePos > m_len)
		return false;
	m_pos = filePos;
	return true;
}

// END - Reader.cpp
