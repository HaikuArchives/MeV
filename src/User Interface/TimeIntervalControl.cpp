/* ===================================================================== *
 * TimeIntervalControl.cpp (MeV/User Interface)
 * ===================================================================== */

#include "TimeIntervalControl.h"
// Application
#include "MeVDoc.h"
// User Interface
#include "BitmapTool.h"
#include "IconMenuItem.h"
#include "MenuTool.h"
#include "Spinner.h"
#include "TextDisplay.h"
#include "TextSlider.h"
#include "ToolBar.h"
// Support
#include "ResourceUtils.h"

// Gnu C Library
#include <stdio.h>
// Interface Kit
#include <PopUpMenu.h>
// Support Kit
#include <Debug.h>

// Debugging Macros
#define D_ALLOC(x) //PRINT (x)			// Constructor/Destructor
#define D_HOOK(x) PRINT (x)			// BControl Implementation
#define D_MESSAGE(x) PRINT (x)		// Messaging
#define D_OPERATION(x) PRINT (x)	// Operations

// ---------------------------------------------------------------------------
// Class Data Initialization

const float CTimeIntervalControl::MENU_BORDER = 2;
const float CTimeIntervalControl::IMAGE_BORDER = 4;

// ---------------------------------------------------------------------------
// Constructor/Destructor

CTimeIntervalControl::CTimeIntervalControl(
	BRect frame,
	const char *name,
	BMessage *message,
	uint32 resizingMode,
	uint32 flags)
	:	BControl(frame, name, "", message, resizingMode, flags),
		m_numerator(1),
		m_denominator(4),
		m_base(3),
		m_tupletModifier(0),
		m_dotModifier(0)
{
	D_ALLOC(("CTimeIntervalControl::CTimeIntervalControl()\n"));

	BMessage *message = NULL;

	// create note length menu
	BPopUpMenu *notesMenu = new BPopUpMenu("Notes", false, false);
	notesMenu->SetFont(be_plain_font);
	message = new BMessage(BASE_DURATION_CHANGED);
	message->AddInt32("base", DOUBLE_NOTE);
	notesMenu->AddItem(new CIconMenuItem("Double Note", message,
										 ResourceUtils::LoadImage("DoubleNote")));
	message = new BMessage(BASE_DURATION_CHANGED);
	message->AddInt32("base", WHOLE_NOTE);
	notesMenu->AddItem(new CIconMenuItem("Whole Note", message,
										 ResourceUtils::LoadImage("WholeNote")));
	message = new BMessage(BASE_DURATION_CHANGED);
	message->AddInt32("base", HALF_NOTE);
	notesMenu->AddItem(new CIconMenuItem("Half Note", message,
										 ResourceUtils::LoadImage("HalfNote")));
	message = new BMessage(BASE_DURATION_CHANGED);
	message->AddInt32("base", QUARTER_NOTE);
	notesMenu->AddItem(new CIconMenuItem("1/4 Note", message,
										 ResourceUtils::LoadImage("QuarterNote")));
	message = new BMessage(BASE_DURATION_CHANGED);
	message->AddInt32("base", EIGHTH_NOTE);
	notesMenu->AddItem(new CIconMenuItem("1/8 Note", message,
										 ResourceUtils::LoadImage("EighthNote")));
	message = new BMessage(BASE_DURATION_CHANGED);
	message->AddInt32("base", SIXTEENTH_NOTE);
	notesMenu->AddItem(new CIconMenuItem("1/16 Note", message,
										 ResourceUtils::LoadImage("SixteenthNote")));
	message = new BMessage(BASE_DURATION_CHANGED);
	message->AddInt32("base", THIRTY_SECOND_NOTE);
	notesMenu->AddItem(new CIconMenuItem("1/32 Note", message,
										 ResourceUtils::LoadImage("ThirtySecondNote")));

	m_toolBar = new CToolBar(Bounds(), "Time Interval");
	m_toolBar->AddTool(new CMenuTool("Base Duration", ResourceUtils::LoadImage("EighthNote"),
									 notesMenu, NULL));
	m_toolBar->AddSeparator();
	message = new BMessage(MODIFIERS_CHANGED);
	message->AddBool("tuplet", true);
	message->AddInt32("modifier", TUPLET_3);
	m_toolBar->AddTool(new CBitmapTool("Tuplet3", ResourceUtils::LoadImage("Tuplet3"),
									message));
	message = new BMessage(MODIFIERS_CHANGED);
	message->AddBool("tuplet", true);
	message->AddInt32("modifier", TUPLET_5);
	m_toolBar->AddTool(new CBitmapTool("Tuplet5", ResourceUtils::LoadImage("Tuplet5"),
									message));
	message = new BMessage(MODIFIERS_CHANGED);
	message->AddBool("tuplet", true);
	message->AddInt32("modifier", TUPLET_7);
	m_toolBar->AddTool(new CBitmapTool("Tuplet7", ResourceUtils::LoadImage("Tuplet7"),
									message));
	m_toolBar->MakeRadioGroup("Tuplet3", "Tuplet7", false);
	m_toolBar->AddSeparator();
	message = new BMessage(MODIFIERS_CHANGED);
	message->AddBool("dot", true);
	message->AddInt32("modifier", SINGLE_DOT);
	m_toolBar->AddTool(new CBitmapTool("Dot", ResourceUtils::LoadImage("Dot1"),
									message));
	message = new BMessage(MODIFIERS_CHANGED);
	message->AddBool("dot", true);
	message->AddInt32("modifier", DOUBLE_DOT);
	m_toolBar->AddTool(new CBitmapTool("DoubleDot", ResourceUtils::LoadImage("Dot2"),
									message));
	m_toolBar->MakeRadioGroup("Dot", "DoubleDot", false);
	AddChild(m_toolBar);

	float width, height;
	m_toolBar->GetPreferredSize(&width, &height);
	m_toolBar->ResizeTo(width, height);
	m_toolBar->Frame().PrintToStream();

	BRect rect(Bounds());
	rect.top += m_toolBar->Frame().bottom + 5.0;
	rect.right = m_toolBar->Frame().right;

	m_durationSlider = new CTextSlider(rect, new BMessage(DURATION_SLIDER_CHANGED),
									   "", B_FOLLOW_LEFT_RIGHT | B_FOLLOW_BOTTOM,
									   B_WILL_DRAW);
	font_height fh;
	m_durationSlider->GetFontHeight(&fh);
	m_durationSlider->ResizeTo(rect.Width(), fh.ascent + fh.descent + 4.0);
	m_durationSlider->SetRange(1, Ticks_Per_QtrNote * 8);
	AddChild(m_durationSlider);

	rect.top = m_toolBar->Frame().top;
	rect.left = m_toolBar->Frame().right + 10.0;
	rect.right = rect.left + 14.0;
	rect.bottom = m_durationSlider->Frame().bottom;
	m_numSpinner = new CSpinner(rect, "Numerator", new BMessage(NUMERATOR_CHANGED),
							  B_FOLLOW_RIGHT | B_FOLLOW_TOP);
	AddChild(m_numSpinner);
	m_numSpinner->SetRange(1, 64);
	m_numSpinner->SetValue(1);

	rect.left = rect.right + 2.0;
	rect.right = rect.left + 39.0;
	m_ratio = new CTextDisplay(rect, "Ratio", true, B_FOLLOW_RIGHT | B_FOLLOW_TOP);
	m_ratio->SetFont(be_bold_font);
	m_ratio->SetFontSize(14);
	AddChild(m_ratio);

	rect.left = rect.right + 1.0;
	rect.right = rect.left + 14.0;
	m_denSpinner = new CSpinner(rect, "Denominator", new BMessage(DENOMINATOR_CHANGED),
							  B_FOLLOW_RIGHT | B_FOLLOW_TOP);
	AddChild(m_denSpinner);
	m_denSpinner->SetRange(1, 64);
	m_denSpinner->SetValue(4);
}

// ---------------------------------------------------------------------------
// BControl Implementation

void
CTimeIntervalControl::AttachedToWindow()
{
	if (Parent())
	{
		SetViewColor(Parent()->ViewColor());
	}

	m_toolBar->SetTarget(this);
	CMenuTool *tool = dynamic_cast<CMenuTool *>(m_toolBar->FindTool("Base Duration"));
	if (tool)
	{
		tool->Menu()->SetTargetForItems(this);
	}
	m_durationSlider->SetTarget(this);
	m_numSpinner->SetTarget(this);
	m_denSpinner->SetTarget(this);

	ShowRatio();
}

void
CTimeIntervalControl::GetPreferredSize(
	float *width,
	float *height)
{
	D_HOOK(("CTimeIntervalControl::GetPreferredSize()\n"));

	m_toolBar->GetPreferredSize(width, height);

	*width = m_denSpinner->Frame().right + 5.0;
	*height += m_durationSlider->Frame().Height() + 5.0;
	D_HOOK((" -> returning width=%f, height=%f\n", *width, *height));
}

void
CTimeIntervalControl::MessageReceived(
	BMessage *message)
{
	D_MESSAGE(("CTimeIntervalControl::MessageReceived()\n"));

	switch (message->what)
	{
		case BASE_DURATION_CHANGED:
		{
			D_MESSAGE((" -> BASE_DURATION_CHANGED\n"));
			int32 base;
			if (message->FindInt32("base", &base) != B_OK)
			{
				return;
			}
			if (base != m_base)
			{
				m_base = base;
				CalcInterval();
				UpdateToolBar();
			}
			break;
		}
		case MODIFIERS_CHANGED:
		{
			D_MESSAGE((" -> MODIFIERS_CHANGED\n"));

			int32 modifier;
			if (message->FindInt32("modifier", &modifier) != B_OK)
			{
				return;
			}
			if (message->HasBool("tuplet"))
			{
				m_tupletModifier = modifier;
			}
			else if (message->HasBool("dot"))
			{
				m_dotModifier = modifier;
			}
			CalcInterval();
			break;
		}
		case DURATION_SLIDER_CHANGED:
		{
			D_MESSAGE((" -> DURATION_SLIDER_CHANGED\n"));

			BControl::SetValue(m_durationSlider->Value());
			NoteLengthFromInterval();
			if (message->HasBool("final"))
			{
				Notify();
			}
			break;
		}
		case NUMERATOR_CHANGED:
		{
			D_MESSAGE((" -> NUMERATOR_CHANGED\n"));

			m_numerator = m_numSpinner->Value();
			if (m_numerator > m_denominator * 2)
			{
				m_denominator = (m_numerator + 1) / 2;
				m_denSpinner->SetValue(m_denominator);
			}
			ShowRatio();
			NoteLengthFromRatio();
			break;
		}
		case DENOMINATOR_CHANGED:
		{
			D_MESSAGE((" -> DENOMINATOR_CHANGED\n"));

			m_denominator = m_denSpinner->Value();
			if (m_numerator > m_denominator * 2)
			{
				m_numerator = m_denominator * 2;
				m_numSpinner->SetValue(m_numerator);
			}
			ShowRatio();
			NoteLengthFromRatio();
			break;
		}
		default:
		{
			BControl::MessageReceived(message);
			break;
		}
	}
}

void
CTimeIntervalControl::SetValue(
	int32 value)
{
	if (value != Value())
	{
		BControl::SetValue(value);
	}
	m_durationSlider->SetValue(value);
	NoteLengthFromInterval();

	m_ratioText = "";
	m_ratioText << m_numerator << "/" << m_denominator;
	m_ratio->SetText(m_ratioText.String());
}

// ---------------------------------------------------------------------------
// Internal Methods

void
CTimeIntervalControl::CalcInterval()
{
	D_OPERATION(("CTimeIntervalControl::CalcInterval()\n"));

	int32 numerator = 2;
	int32 denominator  = 1;

	denominator *= (1 << m_base);
	if (m_base > 0)
	{
		numerator >>= 1;
		denominator >>= 1;
	}

	switch (m_tupletModifier)
	{
		case TUPLET_3:
		{
			D_OPERATION((" -> TUPLET_3\n"));
			numerator *= 2;
			denominator *= 3;
			break;
		}
		case TUPLET_5:
		{
			D_OPERATION((" -> TUPLET_5\n"));
			numerator *= 4;
			denominator *= 5;
			break;
		}
		case TUPLET_7:
		{
			D_OPERATION((" -> TUPLET_7\n"));
			numerator *= 4;
			denominator *= 7;
			break;
		}
	}

	switch (m_dotModifier)
	{
		case SINGLE_DOT:
		{
			D_OPERATION((" -> SINGLE_DOT\n"));
			numerator *= 3;
			denominator *= 2;
			break;
		}
		case DOUBLE_DOT:
		{
			D_OPERATION((" -> DOUBLE_DOT\n"));
			numerator *= 7;
			denominator *= 4;
			break;
		}
	}

	Reduce(&numerator, &denominator);
	
	m_numerator = numerator;
	m_denominator = denominator;

	m_numSpinner->SetValue(m_numerator);
	m_denSpinner->SetValue(m_denominator);
	
	ShowRatio();
}

void
CTimeIntervalControl::NoteLengthFromInterval()
{
	int32 base;
	float r;
	int32 interval = Value();
	
	// init modifiers
	m_tupletModifier = 0;
	m_dotModifier = 0;
					
	for (base = 0; base < BASE_DURATION_COUNT - 1; base++)
	{
		r = (float)interval / (float)((Ticks_Per_QtrNote * 8) >> base);
		if (r >= 1.0)
		{
			break;
		}
	}

	if (r < 1.3)
	{
		// do nothing
	}
	else if (r < 1.4 && base > 0)
	{
		base--;
		m_tupletModifier = TUPLET_3;
	}
	else if (r < 1.7)
	{
		m_dotModifier = SINGLE_DOT;
	}
	else if (r < 1.8 || base < 1)
	{
		m_dotModifier = DOUBLE_DOT;
	}
	else
	{
		base--;
	}

	m_base = base;

	UpdateToolBar();
}

void
CTimeIntervalControl::NoteLengthFromRatio()
{
	int32 numerator = m_numerator;
	int32 denominator = m_denominator;

	// init modifiers
	m_tupletModifier = 0;
	m_dotModifier = 0;
					
	Reduce(&numerator, &denominator);
	
	// calculate tuplet modifier
	if ((denominator % 3) == 0)
	{
		m_tupletModifier = TUPLET_3;
		numerator *= 3;
		denominator *= 2;
	}
	else if ((denominator % 5) == 0)
	{
		m_tupletModifier = TUPLET_5;
		numerator *= 5;
		denominator *= 4;
	}
	else if ((denominator % 7) == 0)
	{
		m_tupletModifier = TUPLET_7;
		numerator *= 7;
		denominator *= 4;
	}

	Reduce(&numerator, &denominator);

	// calculate dot modifier
	if ((numerator % 3) == 0)
	{
		m_dotModifier = SINGLE_DOT;
		numerator *= 2;
		denominator *= 3; }
	else if ((numerator % 7) == 0)
	{
		m_dotModifier = DOUBLE_DOT;
		numerator *= 4;
		denominator *= 7;
	}

	Reduce(&numerator, &denominator );

	// calculate base duration
	if (numerator == 2 && denominator == 1)
	{
		m_base = DOUBLE_NOTE;
	}
	else if (numerator == 1 && denominator == 1)
	{
		m_base = WHOLE_NOTE;
	}
	else if (numerator == 1 && denominator == 2)
	{
		m_base = HALF_NOTE;
	}
	else if (numerator == 1 && denominator == 4)
	{
		m_base = QUARTER_NOTE;
	}
	else if (numerator == 1 && denominator == 8)
	{
		m_base = EIGHTH_NOTE;
	}
	else if (numerator == 1 && denominator == 16)
	{
		m_base = SIXTEENTH_NOTE;
	}
	else if (numerator == 1 && denominator == 32)
	{
		m_base = THIRTY_SECOND_NOTE;
	}
	else
	{
		NoteLengthFromInterval();
	}

	UpdateToolBar();
}

void
CTimeIntervalControl::Notify()
{
	if (Message())
	{
		BMessage message(*Message());
			
		message.AddInt32("value", Value());
		Invoke(&message);
	}
}

void
CTimeIntervalControl::Reduce(
	int32 *numerator,
	int32 *denominator)
{
	while ((*numerator % 2) == 0 && (*denominator % 2) == 0)
	{
		*numerator /= 2;
		*denominator /= 2;
	}
	while ((*numerator % 3) == 0 && (*denominator % 3) == 0)
	{
		*numerator /= 3;
		*denominator /= 3;
	}
	while ((*numerator % 5) == 0 && (*denominator % 5) == 0)
	{
		*numerator /= 5;
		*denominator /= 5;
	}
	while ((*numerator % 7) == 0 && (*denominator % 7) == 0)
	{
		*numerator /= 7;
		*denominator /= 7;
	}
}

void
CTimeIntervalControl::ShowRatio(
	bool updateSlider)
{
	m_ratioText = "";
	m_ratioText << m_numerator << "/" << m_denominator;
	m_ratio->SetText(m_ratioText.String());

	BControl::SetValue(Ticks_Per_QtrNote * 4 * m_numerator / m_denominator);
	
	if (updateSlider)
	{
		m_durationSlider->SetValue( Value() );
	}
	
	Notify();
}

void
CTimeIntervalControl::UpdateToolBar()
{
	CMenuTool *menu = dynamic_cast<CMenuTool *>(m_toolBar->FindTool("Base Duration"));
	if (menu)
	{
		switch (m_base)
		{
			case DOUBLE_NOTE:
			{
				menu->SetBitmap(ResourceUtils::LoadImage("DoubleNote"));
				break;
			}
			case WHOLE_NOTE:
			{
				menu->SetBitmap(ResourceUtils::LoadImage("WholeNote"));
				break;
			}
			case HALF_NOTE:
			{
				menu->SetBitmap(ResourceUtils::LoadImage("HalfNote"));
				break;
			}
			case QUARTER_NOTE:
			{
				menu->SetBitmap(ResourceUtils::LoadImage("QuarterNote"));
				break;
			}
			case EIGHTH_NOTE:
			{
				menu->SetBitmap(ResourceUtils::LoadImage("EighthNote"));
				break;
			}
			case SIXTEENTH_NOTE:
			{
				menu->SetBitmap(ResourceUtils::LoadImage("SixteenthNote"));
				break;
			}
			case THIRTY_SECOND_NOTE:
			{
				menu->SetBitmap(ResourceUtils::LoadImage("ThirtySecondNote"));
				break;
			}
		}
	}
	
	CTool *tool = m_toolBar->FindTool("Tuplet3");
	if (tool)
	{
		switch (m_tupletModifier)
		{
			case TUPLET_3:
			{
				tool->SetValue(B_CONTROL_ON);
				break;
			}
			case TUPLET_5:
			{
				tool->NextTool()->SetValue(B_CONTROL_ON);
				break;
			}
			case TUPLET_7:
			{
				tool->NextTool()->NextTool()->SetValue(B_CONTROL_ON);
				break;
			}
			default: // no tuplet modifier
			{
				while (tool)
				{
					tool->SetValue(B_CONTROL_OFF);
					tool = tool->NextTool();
				}
			}
		}
	}

	tool = m_toolBar->FindTool("Dot");
	if (tool)
	{
		switch (m_dotModifier)
		{
			case SINGLE_DOT:
			{
				tool->SetValue(B_CONTROL_ON);
				break;
			}
			case DOUBLE_DOT:
			{
				tool->NextTool()->SetValue(B_CONTROL_ON);
				break;
			}
			default: // no dot modifier
			{
				while (tool)
				{
					tool->SetValue(B_CONTROL_OFF);
					tool = tool->NextTool();
				}
			}
		}
	}
}

// END - TimeIntervalControl.cpp
