/* ===================================================================== *
 * CursorCache.cpp (MeV/UI)
 * ===================================================================== */

#include "CursorCache.h"

#include "ResourceUtils.h"

// Application Kit
#include <AppDefs.h>
#include <Cursor.h>
// Support Kit
#include <Debug.h>

// Debugging Macros
#define D_ALLOC(x) //PRINT(x)			// Constructor/Destructor

// ---------------------------------------------------------------------------
// Class Data Initialization

CCursorCache *
CCursorCache::m_instance = NULL;

// ---------------------------------------------------------------------------
// Constructor/Destructor

CCursorCache::CCursorCache()
{
	m_cursors[DEFAULT] = B_CURSOR_SYSTEM_DEFAULT;
	m_cursors[I_BEAM] = B_CURSOR_I_BEAM;

	for (int32 i = I_BEAM + 1; i < __CURSOR_COUNT__; i++)
		m_cursors[i] = NULL;
}

CCursorCache::~CCursorCache()
{
	for (int32 i = I_BEAM + 1; i < __CURSOR_COUNT__; i++)
	{
		if (m_cursors[i] != NULL)
		{
			delete m_cursors[i];
			m_cursors[i] = NULL;
		}
	}
}

// ---------------------------------------------------------------------------
// Operations

const BCursor *
CCursorCache::GetCursor(
	int32 which)
{
	ASSERT(which < __CURSOR_COUNT__);

	if (m_instance == NULL)
		m_instance = new CCursorCache();

	if (m_instance->m_cursors[which] == NULL)
		m_instance->m_cursors[which] = new BCursor(ResourceUtils::LoadCursor(which));

	return m_instance->m_cursors[which];
}

void
CCursorCache::Release()
{
	if (m_instance != NULL)
	{
		delete m_instance;
		m_instance = NULL;
	}
}

// END - CCursorCache.cpp
