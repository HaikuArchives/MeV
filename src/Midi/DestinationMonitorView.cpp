/* ===================================================================== *
 * DestinationMonitorView.cpp (MeV/Midi)
 * ===================================================================== */

#include "DestinationMonitorView.h"

#include "MidiDestination.h"
#include "MidiModule.h"

// Application Kit
#include <MessageRunner.h>
// Interface Kit
#include <Bitmap.h>
#include <Region.h>
// Midi Kit
#include <MidiProducer.h>
// Support Kit
#include <Debug.h>
#include <String.h>

// Debugging Macros
#define D_ALLOC(x) //PRINT(x)		// Constructor/Destructor
#define D_HOOK(x) //PRINT(x)		// BView Implementation
#define D_MESSAGE(x) //PRINT(x)		// MessageReceived()

// ---------------------------------------------------------------------------
// CNoteMonitorView

namespace Midi {

class CNoteMonitorView
	:	public BView
{

public:							// Constructor/Destructor

								CNoteMonitorView(
									BRect frame);

public:							// Operations

	void						NoteOn(
									unsigned char note,
									unsigned char velocity);
	void						NoteOff(
									unsigned char note,
									unsigned char velocity);

	void						Tick();

public:							// BView Implementation

	virtual void				Draw(
									BRect updateRect);

	virtual void				GetPreferredSize(
									float *width,
									float *height);

private:						// Instance Data

	unsigned char				m_notes[128];
	unsigned char *				m_max;
	unsigned char				m_current;

	BRect						m_labelRect;
	BPoint						m_labelOffset;
};

};

// ---------------------------------------------------------------------------
// CProgramChangeMonitorView

namespace Midi {

class CProgramChangeMonitorView
	:	public BView
{

public:							// Constructor/Destructor

								CProgramChangeMonitorView(
									BRect frame);

public:							// Operations

	void						ProgramChange(
									BString program);

	void						Tick();

public:							// BView Implementation

	virtual void				Draw(
									BRect updateRect);

private:						// Instance Data

	BString						m_program;
	unsigned char				m_fresh;

	BRect						m_labelRect;
	BPoint						m_labelOffset;
};

};

// ---------------------------------------------------------------------------
// ControlChangeMonitorView

namespace Midi {

class CControlChangeMonitorView
	:	public BView
{

public:							// Constructor/Destructor

								CControlChangeMonitorView(
									BRect frame);

public:							// Operations

	void						ControlChange(
									BString controllerName,
									unsigned char value);

	void						Tick();

public:							// BView Implementation

	virtual void				Draw(
									BRect updateRect);

private:						// Instance Data

	BString						m_controllerName;
	unsigned char				m_freshController;
	unsigned char				m_value;
	unsigned char				m_freshValue;
	bool						m_set;

	BRect						m_labelRect;
	BPoint						m_labelOffset;
};

};

// ---------------------------------------------------------------------------
// CPitchBendMonitorView

namespace Midi {

class CPitchBendMonitorView
	:	public BView
{

public:							// Constructor/Destructor

								CPitchBendMonitorView(
									BRect frame);

public:							// Operations

	void						PitchBend(
									short pitch);

	void						Tick();

public:							// BView Implementation

	virtual void				Draw(
									BRect updateRect);

private:						// Instance Data

	short						m_pitch;
	unsigned char				m_fresh;
	bool						m_set;

	BRect						m_labelRect;
	BPoint						m_labelOffset;
};

};

using namespace Midi;

// ---------------------------------------------------------------------------
// Constructor/Destructor

CDestinationMonitorView::CDestinationMonitorView(
	BRect frame,
	CMidiDestination *destination)
	:	CConsoleView(frame, "Monitor"),
		m_destination(destination),
		m_noteMeter(NULL),
		m_programMeter(NULL),
		m_pitchBendMeter(NULL),
		m_controllerMeter(NULL)
{
	D_ALLOC(("CDestinationMonitorView::CDestinationMonitorView()\n"));

	MakeExpandable(true);
	MakeSelectable(false);

	Destination()->AddObserver(this);

	BRect rect(Bounds());
	rect.InsetBy(6.0, 6.0);

	// add note meter
	rect.InsetBy(5.0, 5.0);
	m_noteMeter = new CNoteMonitorView(rect);
	m_noteMeter->ResizeToPreferred();
	AddChild(m_noteMeter);

	Expanded(true);
}

CDestinationMonitorView::~CDestinationMonitorView()
{
	D_ALLOC(("CDestinationMonitorView::~CDestinationMonitorView()\n"));

	delete m_messageRunner;

	if (m_destination != NULL)
	{
		m_destination->RemoveObserver(this);
		m_destination = NULL;
	}
}

// ---------------------------------------------------------------------------
// CConsoleView Implementation

void
CDestinationMonitorView::AttachedToWindow()
{
	CConsoleView::AttachedToWindow();

	BMessenger messenger(this, Window());
	BMessage message(TICK);
	m_messageRunner = new BMessageRunner(messenger, &message, 50000);

	Destination()->MonitorOpened(messenger);
}

void
CDestinationMonitorView::DetachedFromWindow()
{
	CConsoleView::DetachedFromWindow();

	BMessenger messenger(this, Window());
	Destination()->MonitorClosed(messenger);	
}

void
CDestinationMonitorView::Draw(
	BRect updateRect)
{
	CConsoleView::Draw(updateRect);

	BRect rect(Bounds());
	rect.InsetBy(5.0, 5.0);
	rgb_color lightBorder = tint_color(Parent()->ViewColor(),
									   B_LIGHTEN_2_TINT);
	rgb_color darkBorder = tint_color(Parent()->ViewColor(),
									  B_DARKEN_2_TINT);
	BeginLineArray(4);
	AddLine(rect.LeftBottom(), rect.RightBottom(), lightBorder);
	AddLine(rect.RightBottom(), rect.RightTop(), lightBorder);
	AddLine(rect.RightTop(), rect.LeftTop(), darkBorder);
	AddLine(rect.LeftTop(), rect.LeftBottom(), darkBorder);
	EndLineArray();

	rect.InsetBy(1.0, 1.0);
	SetHighColor(0, 0, 0, 255);
	FillRect(rect, B_SOLID_HIGH);
}

void
CDestinationMonitorView::Expanded(
	bool expanded)
{
	if (expanded)
	{
		BRect rect(m_noteMeter->Frame());

		// add program meter
		if (m_programMeter == NULL)
		{
			rect.left = rect.right + 5.0;
			rect.right = Bounds().right - 11.0;
			rect.bottom = rect.top + floor((Bounds().Height() - 30.0) / 3);
			m_programMeter = new CProgramChangeMonitorView(rect);
		}
		AddChild(m_programMeter);

		// add pitch bend meter
		if (m_pitchBendMeter == NULL)
		{
			rect.OffsetBy(0.0, rect.Height() + 5.0);
			m_pitchBendMeter = new CPitchBendMonitorView(rect);
		}
		AddChild(m_pitchBendMeter);

		// add controller meter
		if (m_controllerMeter == NULL)
		{
			rect.OffsetBy(0.0, rect.Height() + 5.0);
			rect.bottom = m_noteMeter->Frame().bottom;
			m_controllerMeter = new CControlChangeMonitorView(rect);
		}
		AddChild(m_controllerMeter);
	}
	else
	{
		RemoveChild(m_programMeter);
		RemoveChild(m_pitchBendMeter);
		RemoveChild(m_controllerMeter);
	}
}

void
CDestinationMonitorView::GetPreferredSize(
	float *width,
	float *height)
{
	if (IsExpanded())
	{
		*width = 11.0 + m_noteMeter->Frame().Width();
		*width += 5.0 + m_programMeter->Frame().Width() + 11.0;
	}
	else
	{
		*width = 11.0 + m_noteMeter->Frame().Width() + 11.0;
	}
}

void
CDestinationMonitorView::MessageReceived(
	BMessage *message)
{
	switch (message->what)
	{
		case TICK:
		{
			m_noteMeter->Tick();
			if (IsExpanded())
			{
				m_programMeter->Tick();
				m_controllerMeter->Tick();
				m_pitchBendMeter->Tick();
			}
			break;
		}
		case NOTE_ON:
		{
			unsigned char note;
			unsigned char velocity;
			if ((message->FindInt8("mev:note", (int8 *)&note) != B_OK)
			 || (message->FindInt8("mev:velocity", (int8 *)&velocity) != B_OK))
				return;
			m_noteMeter->NoteOn(note, velocity);
			break;
		}
		case NOTE_OFF:
		{
			unsigned char note;
			unsigned char velocity;
			if ((message->FindInt8("mev:note", (int8 *)&note) != B_OK)
			 || (message->FindInt8("mev:velocity", (int8 *)&velocity) != B_OK))
				return;
			m_noteMeter->NoteOff(note, velocity);
			break;
		}
		case PROGRAM_CHANGE:
		{
			unsigned short bank;
			unsigned char program;
			if ((message->FindInt16("mev:bank", (int16 *)&bank) != B_OK)
			 || (message->FindInt8("mev:program", (int8 *)&program) != B_OK))
				return;
			// try to acquire program name from the destination
			char name[PROGRAM_NAME_LENGTH];
			if (!Destination()->GetProgramName(bank, program, name))
				sprintf(name, "%d", program);
			m_programMeter->ProgramChange(name);
			if (IsExpanded())
				m_programMeter->Invalidate();
			break;
		}
		case PITCH_BEND:
		{
			unsigned short pitch;
			if (message->FindInt16("mev:pitch", (int16 *)&pitch) != B_OK)
				return;
			m_pitchBendMeter->PitchBend(pitch);
			if (IsExpanded())
				m_pitchBendMeter->Invalidate();
			break;
		}
		case CONTROL_CHANGE:
		{
			unsigned char control;
			if (message->FindInt8("mev:control", (int8 *)&control) != B_OK)
				return;
			// try to acquire controller name from the destination
			char name[CONTROLLER_NAME_LENGTH];
			if (!Destination()->GetControllerName(control, name))
				sprintf(name, "controller %d", control);
			unsigned char value1;
			unsigned short value2;
			if (message->FindInt8("mev:value", (int8 *)&value1) == B_OK)
				m_controllerMeter->ControlChange(name, value1);
			else if (message->FindInt16("mev:value", (int16 *)&value2) == B_OK)
				m_controllerMeter->ControlChange(name,
												 (unsigned char)(value2 / 128));
			if (IsExpanded())
				m_controllerMeter->Invalidate();
			break;
		}
		default:
		{
			CConsoleView::MessageReceived(message);
		}
	}
}

bool
CDestinationMonitorView::SubjectReleased(
	CObservable *subject)
{
	D_OBSERVE(("CDestinationMonitorView::SubjectReleased()\n"));

	if (subject == m_destination)
	{
		Destination()->RemoveObserver(this);
		m_destination = NULL;
		return true;
	}

	return false;
}

void
CDestinationMonitorView::SubjectUpdated(
	BMessage *message)
{
	D_OBSERVE(("CDestinationMonitorView::SubjectUpdated()\n"));

	int32 destAttrs;
	if (message->FindInt32("DestAttrs", &destAttrs) != B_OK)
		return;
}

// ---------------------------------------------------------------------------
// CNoteMonitorView Implementation

CNoteMonitorView::CNoteMonitorView(
	BRect frame)
	:	BView(frame, "", B_FOLLOW_NONE, B_WILL_DRAW),
		m_current(0)
{
	memset((void *)m_notes, 0, sizeof(m_notes));
	m_max = m_notes;

	SetViewColor(B_TRANSPARENT_COLOR);

	BFont font(be_plain_font);
	font.SetSize(9.0);
	font_height fh;
	font.GetHeight(&fh);
	SetFont(&font);
	m_labelRect = Bounds();
	m_labelRect.top = m_labelRect.bottom - (fh.ascent + 2.0);
	SetLowColor(0, 0, 0, 255);

	m_labelOffset = m_labelRect.LeftBottom();
	m_labelOffset.y -= fh.descent;
}

void
CNoteMonitorView::NoteOn(
	unsigned char note,
	unsigned char velocity)
{
	m_notes[note] = velocity;
	m_current = velocity;

	if (velocity > *m_max)
	{
		// a new max velocity
		m_max = &m_notes[note];
		Invalidate();
	}
	else if (&m_notes[note] == m_max)
	{
		// look for a max among other playing notes
		for (int i = 0; i < 127; i++)
		{
			if (m_notes[note] > *m_max)
				m_max = &m_max[note];
		}
		Invalidate();
	}
}

void
CNoteMonitorView::NoteOff(
	unsigned char note,
	unsigned char velocity)
{
	m_notes[note] = 0;

	// if this is the current max velocity note ...
	if (&m_notes[note] == m_max)
	{
		// ... look for a max among other playing notes
		for (int i = 0; i < 127; i++)
		{
			if (m_notes[note] > *m_max)
				m_max = &m_max[note];
		}
		Invalidate();
	}
}

void
CNoteMonitorView::Tick()
{
	if (m_current == 0)
		return;

	if (10 > m_current)
		m_current = 0;
	else
		m_current -= 5;
	Invalidate();
}

void
CNoteMonitorView::Draw(
	BRect updateRect)
{
	BRect rect(Bounds());
	
	FillRect(m_labelRect, B_SOLID_LOW);
	SetHighColor(160, 160, 160, 255);
	DrawString(" note ", m_labelOffset);

	rect.bottom = m_labelRect.top - 1.0;

	float val;
	val = (float)*m_max / 127.f;
	int peak = (int)rect.bottom - (int)(val * rect.Height());
	val = (float)m_current / 127.f;
	int current = (int)rect.bottom - (int)(val * rect.Height());
	for (int i = (int)rect.bottom; i >= (int)rect.top; i--)
	{
		if (i % 2)
		{
			SetHighColor(0, 0, 0, 255);
		}
		else
		{
			if ((i > peak) || (i > current))
			{
				if (i == current)
					SetHighColor(30, 255, 30, 255);
				else
					SetHighColor(30, 180, 30, 255);
			}
			else
			{
				SetHighColor(96, 96, 96, 255);
			}
		}
		BRect line(rect);
		line.top = line.bottom = i;
		StrokeLine(line.LeftTop(), line.RightBottom(), B_SOLID_HIGH);
	}
}

void
CNoteMonitorView::GetPreferredSize(
	float *width,
	float *height)
{
	BFont font;
	GetFont(&font);
	*width = font.StringWidth(" note ");
}

// ---------------------------------------------------------------------------
// CProgramChangeMonitorView Implementation

CProgramChangeMonitorView::CProgramChangeMonitorView(
	BRect frame)
	:	BView(frame, "", B_FOLLOW_NONE, B_WILL_DRAW),
		m_program("???"),
		m_fresh(15)
{
	SetViewColor(B_TRANSPARENT_COLOR);

	BFont font(be_plain_font);
	font.SetSize(9.0);
	font_height fh;
	font.GetHeight(&fh);
	SetFont(&font);
	m_labelRect = Bounds();
	m_labelRect.top = m_labelRect.bottom - (fh.ascent + 2.0);
	m_labelOffset = m_labelRect.LeftBottom();
	m_labelOffset.y -= fh.descent;

	SetLowColor(0, 0, 0, 255);
}

void
CProgramChangeMonitorView::ProgramChange(
	BString program)
{
	BFont font;
	GetFont(&font);
	font.TruncateString(&program, B_TRUNCATE_END, Bounds().Width());

	m_program = program;
	
	m_fresh = 15;
}

void
CProgramChangeMonitorView::Tick()
{
	if (m_fresh > 0)
	{
		m_fresh--;
		Invalidate();
	}
}

void
CProgramChangeMonitorView::Draw(
	BRect updateRect)
{
	BRect rect(Bounds());
	
	FillRect(rect, B_SOLID_LOW);

	SetHighColor(160, 160, 160, 255);
	DrawString("program", m_labelOffset);

	rect.bottom = m_labelRect.top - 1.0;

	SetHighColor(30 + 5 * m_fresh, 180 + 5 * m_fresh, 30 + 5 * m_fresh, 255);
	DrawString(m_program.String(), rect.LeftBottom());
}

// ---------------------------------------------------------------------------
// CControlChangeMonitorView Implementation

CControlChangeMonitorView::CControlChangeMonitorView(
	BRect frame)
	:	BView(frame, "", B_FOLLOW_NONE, B_WILL_DRAW),
		m_controllerName(""),
		m_freshController(false),
		m_value(0),
		m_freshValue(0),
		m_set(false)
{
	SetViewColor(B_TRANSPARENT_COLOR);

	BFont font(be_plain_font);
	font.SetSize(9.0);
	font_height fh;
	font.GetHeight(&fh);
	SetFont(&font);
	m_labelRect = Bounds();
	m_labelRect.top = m_labelRect.bottom - (fh.ascent + 2.0);
	m_labelOffset = m_labelRect.LeftBottom();
	m_labelOffset.y -= fh.descent;

	SetLowColor(0, 0, 0, 255);
}

void
CControlChangeMonitorView::ControlChange(
	BString controllerName,
	unsigned char value)
{
	if (controllerName != m_controllerName)
	{
		// truncate controller name to max width
		BFont font;
		GetFont(&font);
		font.TruncateString(&controllerName, B_TRUNCATE_END, Bounds().Width());

		m_controllerName = controllerName;
		m_freshController = 15;
	}
	m_value = value;
	m_freshValue = 15;

	m_set = true;
}

void
CControlChangeMonitorView::Tick()
{
	bool invalidate = false;
	if (m_freshController > 0)
	{
		m_freshController--;
		invalidate = true;
	}
	if (m_freshValue > 0)
	{
		m_freshValue--;
		invalidate = true;
	}

	if (invalidate)
		Invalidate();
}

void
CControlChangeMonitorView::Draw(
	BRect updateRect)
{
	BRect rect(Bounds());
	
	FillRect(rect, B_SOLID_LOW);

	SetHighColor(160 + 5 * m_freshController, 160 + 5 * m_freshController,
				 160 + 5 * m_freshController, 255);
	if (m_set)
		DrawString(m_controllerName.String(), m_labelOffset);
	else
		DrawString("controller", m_labelOffset);

	rect.bottom = m_labelRect.top - 1.0;

	float val = (float)m_value / 127.f;
	int current = (int)rect.left + (int)(val * rect.Width());
	for (int i = (int)rect.left; i <= (int)rect.right; i++)
	{
		if (i % 2)
		{
			SetHighColor(0, 0, 0, 255);
		}
		else
		{
			if ((i < ((int)rect.left + 2)) || (i > (int)rect.right - 2))
				SetHighColor(255, 255, 255, 255);
			else if (m_set && (i == current))
				SetHighColor(30 + 5 * m_freshValue, 180 + 5 * m_freshValue,
							 30 + 5 * m_freshValue, 255);
			else if (m_set && (i < current))
				SetHighColor(30, 180, 30, 255);
			else
				SetHighColor(96, 96, 96, 255);
		}
		BRect line(rect);
		line.left = line.right = i;
		StrokeLine(line.LeftTop(), line.RightBottom(), B_SOLID_HIGH);
	}
}

// ---------------------------------------------------------------------------
// CPitchBendMonitorView Implementation

CPitchBendMonitorView::CPitchBendMonitorView(
	BRect frame)
	:	BView(frame, "", B_FOLLOW_NONE, B_WILL_DRAW),
		m_pitch(0),
		m_fresh(0),
		m_set(false)
{
	SetViewColor(B_TRANSPARENT_COLOR);

	BFont font(be_plain_font);
	font.SetSize(9.0);
	font_height fh;
	font.GetHeight(&fh);
	SetFont(&font);
	m_labelRect = Bounds();
	m_labelRect.top = m_labelRect.bottom - (fh.ascent + 2.0);
	m_labelOffset = m_labelRect.LeftBottom();
	m_labelOffset.y -= fh.descent;

	SetLowColor(0, 0, 0, 255);
}

void
CPitchBendMonitorView::PitchBend(
	short pitch)
{
	m_pitch = pitch;
	
	m_fresh = 15;
	m_set = true;
}

void
CPitchBendMonitorView::Tick()
{
	if (m_fresh > 0)
	{
		m_fresh--;
		Invalidate();
	}
}

void
CPitchBendMonitorView::Draw(
	BRect updateRect)
{
	BRect rect(Bounds());
	
	FillRect(rect, B_SOLID_LOW);

	SetHighColor(160, 160, 160, 255);
	DrawString("pitch bend", m_labelOffset);

	rect.bottom = m_labelRect.top - 1.0;

	float val = (float)(m_pitch + 8192) / 16384.f;
	int current = (int)rect.left + (int)(val * rect.Width());
	int center = (int)rect.right - rect.IntegerWidth() / 2;
	for (int i = (int)rect.left; i <= (int)rect.right; i++)
	{
		if (i % 2)
		{
			SetHighColor(0, 0, 0, 255);
		}
		else
		{
			if (m_set && (i == current))
				SetHighColor(30 + 5 * m_fresh, 180 + 5 * m_fresh,
							 30 + 5 * m_fresh, 255);
			else if (i == center)
				SetHighColor(255, 255, 255, 255);
			else if (m_set && ((i > current) && (i < center))
				  || ((i < current) && (i > center)))
				SetHighColor(30, 180, 30, 255);
			else
				SetHighColor(96, 96, 96, 255);
		}
		BRect line(rect);
		line.left = line.right = i;
		StrokeLine(line.LeftTop(), line.RightBottom(), B_SOLID_HIGH);
	}
}

// END - DestinationMonitorView.cpp
