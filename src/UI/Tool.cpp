/* ===================================================================== *
 * Tool.cpp (MeV/User Interface)
 * ===================================================================== */

#include "Tool.h"
#include "ToolBar.h"

// Gnu C Library
#include <string.h>
// Support Kit
#include <Debug.h>

// Debugging Macros
#define D_ALLOC(x) //PRINT (x)		// Constructor/Destructor
#define D_ACCESS(x) //PRINT (x)		// Accessors
#define D_OPERATION(x) //PRINT (x)	// Operations

// ---------------------------------------------------------------------------
// Class Data Initialization

const size_t CTool::TOOL_NAME_LENGTH = 64;

// ---------------------------------------------------------------------------
// Constructor/Destructor

CTool::CTool(
	const char *name,
	BMessage *message,
	int32 mode,
	uint32 flags)
	:	BArchivable(),
		m_name(NULL),
		m_message(message),
		m_toolBar(NULL),
		m_flags(flags),
		m_mode(mode),
		m_value(B_CONTROL_OFF),
		m_enabled(true),
		m_selected(false)
{
	D_ALLOC(("CTool::CTool()\n"));

	if (name)
	{
		m_name = new char[TOOL_NAME_LENGTH];
		strncpy(m_name, name, TOOL_NAME_LENGTH);
	}
}

CTool::~CTool()
{
	D_ALLOC(("CTool::~CTool()\n"));

	if (m_name)
	{
		delete [] m_name;
		m_name = NULL;
	}

	if (m_message)
	{
		delete m_message;
		m_message = NULL;
	}
}

// ---------------------------------------------------------------------------
// Accessors

BPoint
CTool::ContentLocation() const
{
	D_ACCESS(("CTool::ContentLocation()\n"));

	if (m_toolBar)
	{
		return m_toolBar->ContentLocationFor(this);
	}

	return BPoint(0.0, 0.0);
}

BRect
CTool::Frame() const
{
	D_ACCESS(("CTool::Frame()\n"));

	BRect rect;
	float width, height;
	GetContentSize(&width, &height);
	rect.OffsetTo(ContentLocation());
	rect.right = rect.left + width;
	rect.bottom = rect.top + height;

	return rect;
}

CTool *
CTool::NextTool() const
{
	D_ACCESS(("CTool::NextTool()\n"));

	return ToolBar()->ToolAt(ToolBar()->IndexOf(this) + 1);
}

CTool *
CTool::PreviousTool() const
{
	D_ACCESS(("CTool::PreviousTool()\n"));

	return ToolBar()->ToolAt(ToolBar()->IndexOf(this) - 1);
}

// ---------------------------------------------------------------------------
// Operations

void
CTool::SetValue(
	int32 value)
{
	D_OPERATION(("CTool::SetValue(%ld)\n", value));

	if (value != m_value)
	{
		if (Mode() == TRIGGER_MODE)
		{
			// trigger tools don't change their value
			return;
		}

		if ((Mode() == RADIO_MODE) && (Flags() & FORCE_SELECTION)
		 && (Value() == B_CONTROL_ON))
		{
			// radio mode with the force-selection flag set does not
			// allow 'turning off' the tool directly
			return;
		}

		m_value = value;

		if ((Mode() == RADIO_MODE) && (Value() == B_CONTROL_ON))
		{
			// turn off other tools in radio group
			CTool *tool;
			tool = PreviousTool();
			while (tool)
			{
				if (tool->m_value == B_CONTROL_ON)
				{
					tool->m_value = B_CONTROL_OFF;
					tool->ValueChanged();
				}
				tool = tool->PreviousTool();
			}
			tool = NextTool();
			while (tool)
			{
				if (tool->m_value == B_CONTROL_ON)
				{
					tool->m_value = B_CONTROL_OFF;
					tool->ValueChanged();
				}
				tool = tool->NextTool();
			}
		}
		ValueChanged();
	}	
}

// END - Tool.cpp
