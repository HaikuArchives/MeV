/* ===================================================================== *
 * ToolBar.cpp (MeV/UI)
 * ===================================================================== */

#include "ToolBar.h"
// User Interface
#include "StdBevels.h"
#include "Tool.h"

// Gnu C Library
#include <string.h>
// Application Kit
#include <Looper.h>
// Interface Kit
#include <Region.h>
#include <Window.h>
// Support Kit
#include <Debug.h>

// Debugging Macros
#define D_ALLOC(x) //PRINT (x)		// Constructor/Destructor
#define D_HOOK(x) //PRINT (x)		// BControl Implementation
#define D_ACCESS(x) //PRINT (x)		// Accessors
#define D_OPERATION(x) //PRINT (x)	// Operations
#define D_INTERNAL(x) //PRINT (x)	// Internal Methods

// ---------------------------------------------------------------------------
// Class Data Initialization

const float	CToolBar::H_TOOL_BAR_SEPARATOR_WIDTH = 9.0;

const float CToolBar::V_TOOL_BAR_SEPARATOR_HEIGHT = 6.0;

const float CToolBar::H_TOOL_BAR_BORDER = 3.0;

const float CToolBar::V_TOOL_BAR_BORDER = 3.0;

// ---------------------------------------------------------------------------
// Constructor/Destructor

CToolBar::CToolBar(
	BRect frame,
	const char *name,
	orientation posture,
	uint32 resizingMode,
	uint32 flags)
	:	BControl(frame, name, "", NULL, resizingMode, flags),
		m_posture(posture),
		m_lastSelectedTool(NULL)
{
	D_ALLOC(("CToolBar::CToolBar()\n"));
}

CToolBar::~CToolBar()
{
	D_ALLOC(("CToolBar::~CToolBar()\n"));

	while (m_toolList.CountItems() > 0)
	{
		CTool *tool = static_cast<CTool *>(m_toolList.RemoveItem((int32)0));
		if (tool)
		{
			delete tool;
		}
	}
}

// ---------------------------------------------------------------------------
// Accessors

int32
CToolBar::IndexOf(
	const CTool *whichTool) const
{
	D_ACCESS(("CToolBar::IndexOf(tool)\n"));

	for (int32 i = 0; i < m_toolList.CountItems(); i++)
	{
		CTool *tool = static_cast<CTool *>(m_toolList.ItemAt(i));
		if (tool && tool == whichTool)
		{
			return i;
		}
	}
	return -1;
}

int32
CToolBar::IndexOf(
	BPoint point) const
{
	D_ACCESS(("CToolBar::IndexOf(tool)\n"));

	BRect rect;
	for (int32 i = 0; i < m_toolList.CountItems(); i++)
	{
		CTool *tool = static_cast<CTool *>(m_toolList.ItemAt(i));
		if (tool)
		{
			float width, height;
			tool->GetContentSize(&width, &height);
			rect.OffsetTo(ContentLocationFor(tool));
			rect.right = rect.left + width;
			rect.bottom = rect.top + height;
			if (rect.Contains(point))
			{
				return i;
			}
		}
	}
	return -1;
}

CTool *
CToolBar::ToolAt(
	int32 index) const
{
	D_ACCESS(("CToolBar::ToolAt(%ld)\n", index));

	return static_cast<CTool *>(m_toolList.ItemAt(index));
}

CTool *
CToolBar::FindTool(
	const char *name) const
{
	D_ACCESS(("CToolBar::FindTool(%s)\n", name));

	for (int32 i = 0; i < m_toolList.CountItems(); i++)
	{
		CTool *tool = static_cast<CTool *>(m_toolList.ItemAt(i));
		if (tool && (strcmp(name, tool->Name()) == 0))
		{
			return tool;
		}
	}
	return NULL;
}

// ---------------------------------------------------------------------------
// Operations

bool
CToolBar::AddSeparator()
{
	D_OPERATION(("CToolBar::AddSeparator()\n"));

	return m_toolList.AddItem(NULL);
}

bool
CToolBar::AddTool(
	CTool *tool)
{
	D_OPERATION(("CToolBar::AddTool()\n"));

	bool success = m_toolList.AddItem(reinterpret_cast<void *>(tool));
	if (success)
	{
		tool->m_toolBar = this;
	}
	return success;
}

CTool *
CToolBar::RemoveTool(
	int32 index)
{
	D_OPERATION(("CToolBar::RemoveTool(%ld)\n", index));

	CTool *tool = static_cast<CTool *>(m_toolList.RemoveItem(index));
	return tool;
}

void
CToolBar::MakeRadioGroup(
	const char *fromItem,
	const char *toItem,
	bool forceSelection)
{
	D_OPERATION(("CToolBar::MakeRadioGroup(%s, %s)\n", fromItem, toItem));

	int32 index = IndexOf(FindTool(fromItem));
	CTool *tool = ToolAt(index);
	while (tool)
	{
		tool->SetMode(CTool::RADIO_MODE);
		tool->SetFlags(tool->Flags() | forceSelection ?
									   CTool::FORCE_SELECTION :
									   ~CTool::FORCE_SELECTION);
		if (strcmp(tool->Name(), toItem) == 0)
			break;
		tool = tool->NextTool();
	}
}

// ---------------------------------------------------------------------------
// BControl Implementation

void
CToolBar::AttachedToWindow()
{
	D_HOOK(("CToolBar::AttachedToWindow()\n"));

	BControl::AttachedToWindow();

	// resize to preferred dimensions
	float width, height;
	GetPreferredSize(&width, &height);
	ResizeTo(Bounds().Width(), height);

	if (Parent())
	{
		SetViewColor(Parent()->ViewColor());
	}
	else
	{
		SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	}
}

void
CToolBar::Draw(
	BRect updateRect)
{
	D_HOOK(("CToolBar::Draw()\n"));

	BRegion backgroundRegion(updateRect);

	float last = 0.0;
	for (int32 i = 0; i < m_toolList.CountItems(); i++)
	{
		CTool *tool = static_cast<CTool *>(m_toolList.ItemAt(i));
		if (tool)
		{
			float toolWidth, toolHeight;
			tool->GetContentSize(&toolWidth, &toolHeight);
			if (m_posture == B_HORIZONTAL)
			{
				last += H_TOOL_BAR_BORDER;
				BRect rect(last, V_TOOL_BAR_BORDER,
						   last + toolWidth, V_TOOL_BAR_BORDER + toolHeight);
				backgroundRegion.Exclude(rect);
				tool->DrawTool(this, rect);
				last += toolWidth;
			}
			else // m_posture == B_VERTICAL
			{
				last += V_TOOL_BAR_BORDER;
				BRect rect(H_TOOL_BAR_BORDER, last,
						   H_TOOL_BAR_BORDER + toolWidth, last + toolHeight);
				backgroundRegion.Exclude(rect);
				tool->DrawTool(this, rect);
				last += toolHeight;
			}
		}
		else // separator item
		{
			if (m_posture == B_HORIZONTAL)
			{
				last += H_TOOL_BAR_BORDER;
				BRect rect(last, 2 * V_TOOL_BAR_BORDER,
						   last + H_TOOL_BAR_SEPARATOR_WIDTH,
						   Bounds().bottom - 2 * V_TOOL_BAR_BORDER);
				rect.InsetBy(H_TOOL_BAR_SEPARATOR_WIDTH / 2.0 - 1.0, 0.0);
				backgroundRegion.Exclude(rect);
				BeginLineArray(2);
				AddLine(rect.LeftTop(), rect.LeftBottom(),
						tint_color(ui_color(B_PANEL_BACKGROUND_COLOR), B_DARKEN_2_TINT));
				AddLine(rect.RightTop(), rect.RightBottom(),
						tint_color(ui_color(B_PANEL_BACKGROUND_COLOR), B_LIGHTEN_2_TINT));
				EndLineArray();
				last += H_TOOL_BAR_SEPARATOR_WIDTH;
			}
			else
			{
				BRect rect(2 * H_TOOL_BAR_BORDER, last,
						   Bounds().right - 2 * H_TOOL_BAR_BORDER,
						   last + V_TOOL_BAR_SEPARATOR_HEIGHT);
				rect.InsetBy(0.0, V_TOOL_BAR_SEPARATOR_HEIGHT / 2.0 - 1.0);
				backgroundRegion.Exclude(rect);
				BeginLineArray(2);
				AddLine(rect.LeftTop(), rect.RightTop(),
						tint_color(ui_color(B_PANEL_BACKGROUND_COLOR), B_DARKEN_2_TINT));
				AddLine(rect.LeftBottom(), rect.RightBottom(),
						tint_color(ui_color(B_PANEL_BACKGROUND_COLOR), B_LIGHTEN_2_TINT));
				EndLineArray();
				last += V_TOOL_BAR_SEPARATOR_HEIGHT;
			}
		}
	}

	// fill with background color
	SetLowColor(ViewColor());
	FillRegion(&backgroundRegion, B_SOLID_LOW);

	BRect rect(Bounds());
	BeginLineArray(5);
	AddLine(rect.LeftTop(), rect.LeftBottom(),
			tint_color(ui_color(B_PANEL_BACKGROUND_COLOR), B_LIGHTEN_MAX_TINT));
	AddLine(rect.LeftTop(), rect.RightTop(),
			tint_color(ui_color(B_PANEL_BACKGROUND_COLOR), B_LIGHTEN_MAX_TINT));
	AddLine(rect.LeftBottom(), rect.RightBottom(),
			tint_color(ui_color(B_PANEL_BACKGROUND_COLOR), B_DARKEN_3_TINT));
	rect.bottom--;
	AddLine(rect.LeftBottom(), rect.RightBottom(),
			tint_color(ui_color(B_PANEL_BACKGROUND_COLOR), B_DARKEN_1_TINT));
	AddLine(rect.RightTop(), rect.RightBottom(),
			tint_color(ui_color(B_PANEL_BACKGROUND_COLOR), B_DARKEN_3_TINT));
	EndLineArray();
}

void
CToolBar::GetPreferredSize(
	float *width,
	float *height)
{
	D_HOOK(("CToolBar::GetPreferredSize()\n"));

	*width = *height = 0.0;

	for (int32 i = 0; i < m_toolList.CountItems(); i++)
	{
		CTool *tool = static_cast<CTool *>(m_toolList.ItemAt(i));
		if (tool)
		{
			float toolWidth, toolHeight;
			tool->GetContentSize(&toolWidth, &toolHeight);
			if (m_posture == B_HORIZONTAL)
			{
				*width += toolWidth + H_TOOL_BAR_BORDER;
				*height = MAX(toolHeight, *height);
			}
			else
			{
				*width = MAX(toolWidth, *width);
				*height += toolHeight + V_TOOL_BAR_BORDER;
			}
		}
		else // separator item
		{
			if (m_posture == B_HORIZONTAL)
			{
				*width += H_TOOL_BAR_SEPARATOR_WIDTH + 2 * H_TOOL_BAR_BORDER;
			}
			else
			{
				*height += V_TOOL_BAR_SEPARATOR_HEIGHT + 2 * V_TOOL_BAR_BORDER;
			}
		}
	}

	*width += 2 * H_TOOL_BAR_BORDER;
	*height += 2 * V_TOOL_BAR_BORDER;
}

void
CToolBar::MouseDown(
	BPoint point)
{
	D_HOOK(("CToolBar::MouseDown()\n"));

	int32 buttons = B_PRIMARY_MOUSE_BUTTON;
	Looper()->CurrentMessage()->FindInt32("buttons", &buttons);
	CTool *tool = ToolAt(IndexOf(point));
	if (tool)
	{
		if (!tool->IsEnabled())
		{
			return;
		}
		tool->Clicked(point, buttons);
		if (tool->Message())
		{
			BMessage message(*tool->Message());
			message.AddInt32("value", tool->Value());
			Invoke(&message);
		}
		else
		{
			Invoke(NULL);
		}
	}
}

void
CToolBar::MouseMoved(
	BPoint point,
	uint32 transit,
	const BMessage *message)
{
	D_HOOK(("CToolBar::MouseMoved()\n"));

	if (!Window()->IsActive() && !(Window()->Flags() & B_AVOID_FOCUS))
		return;

	CTool *tool = ToolAt(IndexOf(point));
	if (tool != m_lastSelectedTool)
	{
		if (m_lastSelectedTool)
		{
			m_lastSelectedTool->Select(false);
			Invalidate(m_lastSelectedTool->Frame());
		}
		if (tool)
		{
			tool->Select(true);
			Invalidate(tool->Frame());
		}
		m_lastSelectedTool = tool;
	}
}

void
CToolBar::MouseUp(
	BPoint point)
{
	D_HOOK(("CToolBar::MouseUp()\n"));
}

// ---------------------------------------------------------------------------
// Internal Methods

BPoint
CToolBar::ContentLocationFor(
	const CTool *whichTool) const
{
	D_INTERNAL(("CToolBar::ContentLocationFor()\n"));

	BPoint location(H_TOOL_BAR_BORDER, V_TOOL_BAR_BORDER);

	switch (m_posture)
	{
		case B_HORIZONTAL:
		{
			for (int32 i = 0; i < m_toolList.CountItems(); i++)
			{
				CTool *tool = static_cast<CTool *>(m_toolList.ItemAt(i));
				if (tool)
				{
					if (tool == whichTool)
					{
						return location;
					}
					float width, height;
					tool->GetContentSize(&width, &height);
					location.x += width + H_TOOL_BAR_BORDER;
				}
				else // separator
				{
					location.x += H_TOOL_BAR_SEPARATOR_WIDTH + H_TOOL_BAR_BORDER;
				}
			}
			break;
		}
		case B_VERTICAL:
		{
			for (int32 i = 0; i < m_toolList.CountItems(); i++)
			{
				CTool *tool = static_cast<CTool *>(m_toolList.ItemAt(i));
				if (tool)
				{
					if (tool == whichTool)
					{
						return location;
					}
					float width, height;
					tool->GetContentSize(&width, &height);
					location.y += height + V_TOOL_BAR_BORDER;
				}
				else // separator
				{
					location.y += V_TOOL_BAR_SEPARATOR_HEIGHT + V_TOOL_BAR_BORDER;
				}
			}
			break;
		}
	}
	return BPoint(0.0, 0.0);
}

// END - ToolBar.cpp
