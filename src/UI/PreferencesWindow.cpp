/* ===================================================================== *
 * PreferencesWindow.cpp (MeV/UI)
 * ===================================================================== */

#include "PreferencesWindow.h"

#include "ScreenUtils.h"
#include "TextSlider.h"

// Gnu C Library
#include <stdio.h>
// Interface Kit
#include <Box.h>
#include <Button.h>
#include <CheckBox.h>
#include <ListView.h>
#include <ScrollView.h>
#include <StringView.h>
// Support Kit
#include <Debug.h>

// ---------------------------------------------------------------------------
// Constructor/Destructor

CPreferencesWindow::CPreferencesWindow(
	CWindowState &state)
	:	CAppWindow(state, state.Rect(), "Preferences", B_TITLED_WINDOW,
				   B_NOT_RESIZABLE | B_NOT_ZOOMABLE),
		m_panelCount(0),
		m_currentPanel(-1)
{
	BRect r(state.Rect());
	r.OffsetTo(B_ORIGIN);

	m_bgView = new BView(r, NULL, B_FOLLOW_ALL_SIDES, B_WILL_DRAW);
	AddChild(m_bgView);
	m_bgView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	r.InsetBy(5.0, 5.0);
	r.bottom -= 35;

	BButton *button;

	button = new BButton(BRect(r.right - 90, r.bottom + 9,
							   r.right - 4,	r.bottom + 34),
							   "Defaults", "Defaults",
							   new BMessage(DEFAULTS_REQUESTED),
							   B_FOLLOW_LEFT | B_FOLLOW_BOTTOM);
	button->SetEnabled(false);
	m_bgView->AddChild(button);

	button = new BButton(BRect(r.right - 176, r.bottom + 9,
							   r.right - 102, r.bottom + 34),
							   "Revert", "Revert",
							   new BMessage(REVERT_REQUESTED),
							   B_FOLLOW_LEFT | B_FOLLOW_BOTTOM);
	button->MakeDefault(true);
	m_bgView->AddChild(button);

	m_listView = new BListView(BRect(r.left + 2, r.top + 7,
									 110 - B_V_SCROLL_BAR_WIDTH, r.bottom),
									 NULL, B_SINGLE_SELECTION_LIST);
	m_listView->SetSelectionMessage(new BMessage(PANEL_SELECTED));

	BScrollView *sv = new BScrollView(NULL, m_listView,
									  B_FOLLOW_TOP_BOTTOM | B_FOLLOW_LEFT,
									  0, false, true, B_PLAIN_BORDER);
	m_bgView->AddChild(sv);

	AddFeedbackPanel();
	AddEditingPanel();

	m_prefs = gPrefs;
	ReadPrefs();
	SetCurrentPanel(m_prefs.appPrefsPanel);
}

CPreferencesWindow::~CPreferencesWindow()
{
	while (m_listView->CountItems() > 0)
	{
		BListItem *item = m_listView->RemoveItem((int32)0);
		if (item)
			delete item;
	}
}

// ---------------------------------------------------------------------------
// CAppWindow Implementation

void
CPreferencesWindow::MessageReceived(
	BMessage *message)
{
	switch (message->what)
	{
		case DEFAULTS_REQUESTED:
		{
			// +++ nyi
			break;
		}
		case REVERT_REQUESTED:
		{
			gPrefs = m_prefs;
			ReadPrefs();
			break;
		}
		case VALUE_CHANGED:
		{
			WritePrefs();
			break;
		}
		case PANEL_SELECTED:
		{
			SetCurrentPanel(m_listView->CurrentSelection());
			break;
		}
		default:
		{
			CAppWindow::MessageReceived(message);
		}
	}
}

// ---------------------------------------------------------------------------
// Internal Operations

BView *
CPreferencesWindow::AddPanel(
	char *name)
{
	BRect r(m_bgView->Frame());

	if (m_panelCount >= MAX_PREFERENCES_PANELS)
		return NULL;
	
	BBox *box = new BBox(BRect(118, r.top + 5.0,
							   r.right - 5.0,
							   r.bottom - 38.0),
						 NULL, B_FOLLOW_ALL);
	box->Hide();
	box->SetLabel(name);
	m_bgView->AddChild(box);
	m_panels[m_panelCount++] = box;
	m_listView->AddItem(new BStringItem(name));

	return box;
}

void
CPreferencesWindow::SetCurrentPanel(
	int index)
{ 
	if (index <= 0)
		index = 0; 
	else if (index >= m_panelCount)
		index = m_panelCount - 1;

	if (index != m_currentPanel) 
	{
		if (m_currentPanel >= 0)
			m_panels[m_currentPanel]->Hide(); 
		m_currentPanel = index; 
		m_listView->Select(index); 
		m_panels[m_currentPanel]->Show(); 
	}
}

void
CPreferencesWindow::ReadPrefs()
{	
	for (int i = 0; i < 4; i++)
	{
		m_cb[i][0]->SetValue((gPrefs.feedbackDragMask & (1 << i)) ? true : false);
		m_cb[i][1]->SetValue((gPrefs.feedbackAdjustMask & (1 << i)) ? true : false);
	}
	m_chan_cb->SetValue((gPrefs.feedbackAdjustMask & CGlobalPrefs::FB_Channel) ? true : false);
	m_rect_cb->SetValue(gPrefs.inclusiveSelection);
	m_fbDelay->SetValue(gPrefs.feedbackDelay);
}

void
CPreferencesWindow::WritePrefs()
{
	for (int i = 0; i < 4; i++)
	{
		uint32 mask = 1 << i;
		if (m_cb[i][0]->Value())
			gPrefs.feedbackDragMask |= mask;
		else
			gPrefs.feedbackDragMask &= ~mask;
			
		if (m_cb[i][1]->Value())
			gPrefs.feedbackAdjustMask |= mask;
		else
			gPrefs.feedbackAdjustMask &= ~mask;
	}

	if (m_chan_cb->Value())
		gPrefs.feedbackAdjustMask |=  CGlobalPrefs::FB_Channel;
	else
		gPrefs.feedbackAdjustMask &= ~CGlobalPrefs::FB_Channel;

	gPrefs.inclusiveSelection = m_rect_cb->Value();
	gPrefs.feedbackDelay = m_fbDelay->Value();
	gPrefs.appPrefsPanel = CurrentPanel();
}

void
CPreferencesWindow::AddFeedbackPanel()
{
	static char	*labels[] = {"Note Pitch", "Note Velocity",
							 "Program Change", "Controller Value"};

	float y = 20.0;

	// Construct the Audible feedback panel
	BView *panel = AddPanel("Audible Feedback");
	for (int i = 0; i < 4; i++)
	{
		BStringView *stringView = new BStringView(BRect(20, y, 100, y + 16),
												  NULL, labels[i]);
		stringView->SetAlignment(B_ALIGN_RIGHT);
		panel->AddChild(stringView);

		m_cb[i][0] = new BCheckBox(BRect(110, y, 120, y + 10 ), NULL,
								   "While dragging", new BMessage(VALUE_CHANGED));
		panel->AddChild(m_cb[i][0]);
		m_cb[i][0]->ResizeToPreferred();

		m_cb[i][1] = new BCheckBox(BRect(220, y, 230, y + 10), NULL,
								   "While adjusting", new BMessage(VALUE_CHANGED));
		panel->AddChild(m_cb[i][1]);
		m_cb[i][1]->ResizeToPreferred();

		y = m_cb[i][1]->Frame().bottom + 2.0;
	}

	y += 13;

	m_chan_cb = new BCheckBox(BRect(110, y, 120, y + 10), NULL,
							  "Channel Change", new BMessage(VALUE_CHANGED));
	panel->AddChild(m_chan_cb);
	m_chan_cb->ResizeToPreferred();

	y = m_chan_cb->Frame().bottom + 15.0;

	BStringView *stringView = new BStringView(BRect(10, y, 100, y + 13),
											  NULL, "Delay (ms)");
	stringView->SetAlignment(B_ALIGN_RIGHT);
	panel->AddChild(stringView);
	
	m_fbDelay = new CTextSlider(BRect(110, y, 320, y + 13),
							  new BMessage(VALUE_CHANGED), NULL);
	panel->AddChild(m_fbDelay);
	m_fbDelay->SetRange(0, 1000);
}

void
CPreferencesWindow::AddEditingPanel()
{
	// Construct the Editing options panel
	BView *panel = AddPanel("Editing");

	float y = 20.0;
	m_rect_cb = new BCheckBox(BRect(110.0, y, 120.0, y + 10.0), NULL,
							  "Inclusive Rectangle Selection",
							  new BMessage(VALUE_CHANGED));
	panel->AddChild(m_rect_cb);
	m_rect_cb->ResizeToPreferred();
	y = m_rect_cb->Frame().bottom + 2.0;
}

// END - PreferencesWindow.cpp
