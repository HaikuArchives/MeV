/* ===================================================================== *
 * BeFileReader.cpp (MeV/Support)
 * ===================================================================== */
 
#include "BeFileReader.h"

#include "Error.h"

// Storage Kit
#include <File.h>

// ---------------------------------------------------------------------------
// External Function

extern void
CheckBeError(
	status_t errCode);

// ---------------------------------------------------------------------------
// EndOfFile Implementation

class EndOfFile
	:	public IError
{

public:							// IError Implementation

	virtual	const char*			Description() const
								{ return "File is incomplete or corrupt"; }
};

// ---------------------------------------------------------------------------
// Constructor/Destructor

CBeFileReader::CBeFileReader(
	BFile &file)
	:	m_file(file)
{
	CheckBeError(file.InitCheck());
	m_sizeOK = false;
}

// ---------------------------------------------------------------------------
// CAbstractReader Implementation

int32
CBeFileReader::Read(
	void *buffer,
	int32 length,
	bool partialOK)
{
	if (partialOK)
	{
		int32 result = m_file.Read(buffer, length);
		CheckBeError(result);
		return result;
	}
	else
	{
		int32 pos = Position();
		int32 result = m_file.Read(buffer, length);
		CheckBeError(result);

		if (result != length)
		{
			m_file.Seek(pos, SEEK_SET);
			return 0;
		}

		return result;
	}
}

void
CBeFileReader::MustRead(
	void *buffer,
	int32 length)
{
	int32 bytesRead = m_file.Read(buffer, length);
	CheckBeError(bytesRead);
	if (bytesRead < length)
		throw EndOfFile();
}

uint32
CBeFileReader::BytesAvailable()
{
	off_t size;

	if (!m_sizeOK)
	{
		CheckBeError(m_file.GetSize(&size));
		m_size = size;
	}
	return m_size - m_file.Position();
}

uint32
CBeFileReader::Position() const
{
	return m_file.Position();
}
	
uint32
CBeFileReader::Skip(
	uint32 skipBytes)
{
	CheckBeError(m_file.Seek(skipBytes, SEEK_CUR));
	return Position();
}

bool
CBeFileReader::Seek(
	uint32 toPosition)
{
	if (m_file.Seek(toPosition, SEEK_SET) < 0)
		return false;

	return true;
}

// END - BeFileReader.cpp
