/* ===================================================================== *
 * QuickKeyMenuItem.cpp (MeV/User Interface)
 * ===================================================================== */
 
#include "QuickKeyMenuItem.h"

// Support Kit
#include <Debug.h>

// Debugging Macros
#define D_ALLOC(x) //PRINT(x)		// Constructor/Destructor
#define D_HOOK(x) //PRINT(x)		// BMenuItem Implementation
#define D_OPERATION(x) //PRINT(x)	// Operations

// ---------------------------------------------------------------------------
// Constructor/Destructor

CQuickKeyMenuItem::CQuickKeyMenuItem(
	const char *label,
	BMessage *message,
	char shortcutKey,
	const char *shortcutLabel)
	:	BMenuItem(label, message, 0, 0),
		m_shortcutKey(shortcutKey),
		m_shortcutLabel(shortcutLabel)
{
	D_ALLOC(("CQuickKeyMenuItem::CQuickKeyMenuItem()\n"));

}

// ---------------------------------------------------------------------------
// Constructor/Destructor

void
CQuickKeyMenuItem::DrawContent()
{
	D_HOOK(("CQuickKeyMenuItem::DrawContent()\n"));

	BRect rect(Frame());
	BFont font;
	font_height	fh;

	Menu()->GetFont(&font);
	font.GetHeight(&fh);

	BMenuItem::DrawContent();
	Menu()->MovePenTo(rect.right - 2.0 - font.StringWidth(m_shortcutLabel.String()),
					  rect.bottom - fh.descent - fh.leading);
	Menu()->SetDrawingMode(B_OP_OVER);
	Menu()->DrawString(m_shortcutLabel.String());
}

void
CQuickKeyMenuItem::GetContentSize(
	float *width,
	float *height)
{
	D_HOOK(("CQuickKeyMenuItem::GetContentSize()\n"));

	BMenuItem::GetContentSize(width, height);
	*width += 20.0 + Menu()->StringWidth(m_shortcutLabel.String());
}
	
// ---------------------------------------------------------------------------
// Operations

bool
CQuickKeyMenuItem::TriggerShortcutMenu(
	BMenu *menu, char key)
{
	D_OPERATION(("CQuickKeyMenuItem::TriggerShortcutMenu()\n"));

	int32 count = menu->CountItems();
	BMenu *subMenu;

	// Search all menu items.
	for (int32 i = 0; i < count; i++)
	{
		BMenuItem *item = menu->ItemAt(i);
		CQuickKeyMenuItem *kqmi;	
		
		if (!item->IsEnabled()) continue;

		// If it's one of ours, and it has a matching key...
		if ((kqmi = dynamic_cast<CQuickKeyMenuItem *>(item)) != NULL)
		{
			if (kqmi->m_shortcutKey == key)
			{
				// Unfortunately, BMenuItem makes Invoke() a private
				// function, so we're going to have to do without
				// it's added functionality.
				kqmi->BInvoker::Invoke();
				return true;
			}
		}
		
		// Search submenus
		if ((subMenu = item->Submenu()) != NULL)
		{
			// Recursive search
			if (TriggerShortcutMenu(subMenu, key)) return true;
		}
	}
	return false;
}

// END - QuickKeyMenuItem.cpp
