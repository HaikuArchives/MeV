/* ===================================================================== *
 * Sylvan Technical Arts Class Library, Copyright © 1997 Talin.
 * IFFReader.cp -- Reader that reads IFF files.
 * ---------------------------------------------------------------------
 * $NoKeywords: $
 * ===================================================================== */
 
#include <cstring>

#include <support/SupportDefs.h>

#include "Error.h"

void CheckBeError( status_t errCode );
void CheckBeError( status_t errCode )
{
	class Error :
		public IError
	{
	public:
		Error(status_t error) : m_error(error) { }
		virtual	const char*		Description() const
									{ return strerror(m_error); }
	private:
		status_t m_error;
	};

	if (errCode < B_OK)
		throw Error(errCode);
}
