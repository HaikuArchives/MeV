/* ===================================================================== *
 * TransportButton.cpp (MeV/UI)
 * ===================================================================== */

#include "TransportButton.h"

// Application Kit
#include <MessageFilter.h>
#include <MessageRunner.h>
// Interface Kit
#include <Bitmap.h>
#include <Screen.h>
#include <Window.h>
// Support Kit
#include <Debug.h>
// Standard Template Library
#include <map>

// ---------------------------------------------------------------------------
// Internal Class: CBitmapStash
//
// Bitmap stash is a simple class to hold all the lazily-allocated
// bitmaps that the CTransportButton needs when rendering itself.
// signature is a combination of the different enabled, pressed, playing, etc.
// flavors of a bitmap. If the stash does not have a particular bitmap,
// it turns around to ask the button to create one and stores it for next time.
//
class CBitmapStash
{

public:							// Constructor/Destructor

								CBitmapStash(
									CTransportButton *button);

								~CBitmapStash();

public:							// Accessors

	BBitmap *					GetBitmap(
									uint32 signature);
	
private:						// Instance Data

	CTransportButton *			m_owner;

	map<uint32, BBitmap *> 		m_stash;
};

class SkipButtonKeypressFilter
	:	public BMessageFilter 
{

public:							// Constructor/Destructor

								SkipButtonKeypressFilter(
									uint32 shortcutKey,
									uint32 shortcutModifier,
									CTransportButton *target);

protected:						// BMessageFilter Implementation

	filter_result				Filter(
									BMessage *message,
									BHandler **handler);

private:						// Instance Data

	uint32						m_shortcutKey;

	uint32						m_shortcutModifier;

	CTransportButton *			m_target;
};

// ---------------------------------------------------------------------------
// Constructor/Destructor

CTransportButton::CTransportButton(
	BRect frame,
	const char *name,
	BBitmap *normalBitmap,
	BBitmap *disabledBitmap,
	BBitmap *pressedBitmap,
	BMessage *message,
	bigtime_t period,
	uint32 key,
	uint32 modifiers,
	uint32 resizingMode)
	:	BControl(frame, name, "", message, resizingMode,
				 B_WILL_DRAW | B_NAVIGABLE),
		m_bitmaps(new CBitmapStash(this)),
		m_normalBitmap(normalBitmap),
		m_disabledBitmap(disabledBitmap),
		m_pressedBitmap(pressedBitmap),
		m_startPressingMessage(NULL),
		m_pressingMessage(NULL),
		m_donePressingMessage(NULL),
		m_pressingPeriod(period),
		m_mouseDown(false),
		m_keyDown(false),
		m_keyPressFilter(0),
		m_messageSender(0)
{
	if (key)
		m_keyPressFilter = new SkipButtonKeypressFilter(key, modifiers, this);
}

CTransportButton::~CTransportButton()
{
	delete m_startPressingMessage;
	delete m_pressingMessage;
	delete m_donePressingMessage;
	delete m_bitmaps;
}

// ---------------------------------------------------------------------------
// Accessors

void 
CTransportButton::SetStartPressingMessage(
	BMessage *message)
{
	delete m_startPressingMessage;
	m_startPressingMessage = message;
}

void 
CTransportButton::SetPressingMessage(
	BMessage *message)
{
	delete m_pressingMessage;
	m_pressingMessage = message;
}

void 
CTransportButton::SetDonePressingMessage(
	BMessage *message)
{
	delete m_donePressingMessage;
	m_donePressingMessage = message;
}

void 
CTransportButton::SetPressingPeriod(
	bigtime_t period)
{
	m_pressingPeriod = period;
}

// ---------------------------------------------------------------------------
// BControl Implementation

void 
CTransportButton::AttachedToWindow()
{
	BControl::AttachedToWindow();

	if (m_keyPressFilter)
		Window()->AddCommonFilter(m_keyPressFilter);
	
	// transparent to reduce flicker
	SetViewColor(B_TRANSPARENT_COLOR);
}

void 
CTransportButton::DetachedFromWindow()
{
	if (m_keyPressFilter)
	{
		Window()->RemoveCommonFilter(m_keyPressFilter);
		delete m_keyPressFilter;
	}

	BControl::DetachedFromWindow();
}

void 
CTransportButton::Draw(BRect)
{
	DrawBitmapAsync(m_bitmaps->GetBitmap(ModeMask()));
}

void 
CTransportButton::MouseDown(
	BPoint point)
{
	if (!IsEnabled())
		return;

	ASSERT(Window()->Flags() & B_ASYNCHRONOUS_CONTROLS);
	SetTracking(true);
	SetMouseEventMask(B_POINTER_EVENTS, B_LOCK_WINDOW_FOCUS);
	MouseStartPressing();
}

void 
CTransportButton::MouseMoved(
	BPoint point,
	uint32 transit,
	const BMessage *message)
{
	if (IsTracking() && Bounds().Contains(point) != Value())
	{
		if (!Value())
			MouseStartPressing();
		else
			MouseCancelPressing();
	}
}

void 
CTransportButton::MouseUp(
	BPoint point)
{
	if (IsTracking())
	{
		if (Bounds().Contains(point))
			MouseDonePressing();
		else
			MouseCancelPressing();
		SetTracking(false);
	}
}

void 
CTransportButton::WindowActivated(
	bool active)
{
	BControl::WindowActivated(active);

	if (!active)
		ShortcutKeyUp();
}

void 
CTransportButton::SetEnabled(
	bool enabled)
{
	BControl::SetEnabled(enabled);

	if (!enabled)
		ShortcutKeyUp();	
}

// ---------------------------------------------------------------------------
// Operations

BBitmap *
CTransportButton::MakeBitmap(
	uint32 mask)
{
	switch (mask)
	{
		case DISABLED_MASK:
			return m_disabledBitmap;
		case PRESSED_MASK:
			return m_pressedBitmap;
		default:
			return m_normalBitmap;
	}
}

uint32 
CTransportButton::ModeMask() const
{
	return (IsEnabled() ? 0 : DISABLED_MASK) | (Value() ? PRESSED_MASK : 0);
}

void 
CTransportButton::StartPressing()
{
	SetValue(B_CONTROL_ON);
	if (m_startPressingMessage)
		Invoke(m_startPressingMessage);
	
	if (m_pressingMessage)
		m_messageSender = new BMessageRunner(Messenger(), m_pressingMessage,
											 m_pressingPeriod);
}

void 
CTransportButton::MouseCancelPressing()
{
	if (!m_mouseDown || m_keyDown)
		return;

	m_mouseDown = false;

	if (m_pressingMessage) {
		delete m_messageSender;
		m_messageSender = NULL;
	}

	if (m_donePressingMessage)
		Invoke(m_donePressingMessage);

	SetValue(B_CONTROL_OFF);
}

void 
CTransportButton::DonePressing()
{	
	if (m_pressingMessage) {
		delete m_messageSender;
		m_messageSender = NULL;
	}

	Invoke();
	SetValue(B_CONTROL_OFF);
}

void 
CTransportButton::MouseStartPressing()
{
	if (m_mouseDown)
		return;
	
	m_mouseDown = true;
	if (!m_keyDown)
		StartPressing();
}

void 
CTransportButton::MouseDonePressing()
{
	if (!m_mouseDown)
		return;
	
	m_mouseDown = false;
	if (!m_keyDown)
		DonePressing();
}

void 
CTransportButton::ShortcutKeyDown()
{
	if (!IsEnabled())
		return;

	if (m_keyDown)
		return;
	
	m_keyDown = true;
	if (!m_mouseDown)
		StartPressing();
}

void 
CTransportButton::ShortcutKeyUp()
{
	if (!m_keyDown)
		return;
	
	m_keyDown = false;
	if (!m_mouseDown)
		DonePressing();
}

// ---------------------------------------------------------------------------
// CBitmapStash Implementation

CBitmapStash::CBitmapStash(
	CTransportButton *button)
	:	m_owner(button)
{
}

CBitmapStash::~CBitmapStash()
{
	// delete all the bitmaps
	for (map<uint32, BBitmap *>::iterator i = m_stash.begin();
		 i != m_stash.end();
		 i++)
	{
		delete (*i).second;
	}
}

BBitmap *
CBitmapStash::GetBitmap(
	uint32 signature)
{
	if (m_stash.find(signature) == m_stash.end()) {
		BBitmap *newBits = m_owner->MakeBitmap(signature);
		ASSERT(newBits);
		m_stash[signature] = newBits;
	}
	
	return m_stash[signature];
}

// ---------------------------------------------------------------------------
// ...

SkipButtonKeypressFilter::SkipButtonKeypressFilter(
	uint32 shortcutKey,
	uint32 shortcutModifier,
	CTransportButton *target)
	:	BMessageFilter(B_ANY_DELIVERY, B_ANY_SOURCE),
		m_shortcutKey(shortcutKey),
		m_shortcutModifier(shortcutModifier),
		m_target(target)
{
}

filter_result 
SkipButtonKeypressFilter::Filter(
	BMessage *message,
	BHandler **handler)
{
	if (m_target->IsEnabled()
		&& (message->what == B_KEY_DOWN || message->what == B_KEY_UP)) {
		uint32 modifiers;
		uint32 rawKeyChar = 0;
		uint8 byte = 0;
		int32 key = 0;
		
		if (message->FindInt32("modifiers", (int32 *)&modifiers) != B_OK
			|| message->FindInt32("raw_char", (int32 *)&rawKeyChar) != B_OK
			|| message->FindInt8("byte", (int8 *)&byte) != B_OK
			|| message->FindInt32("key", &key) != B_OK)
			return B_DISPATCH_MESSAGE;

		modifiers &= B_SHIFT_KEY | B_COMMAND_KEY | B_CONTROL_KEY
			| B_OPTION_KEY | B_MENU_KEY;
			// strip caps lock, etc.

		if (modifiers == m_shortcutModifier && rawKeyChar == m_shortcutKey) {
			if (message->what == B_KEY_DOWN)
				m_target->ShortcutKeyDown();
			else
				m_target->ShortcutKeyUp();
			
			return B_SKIP_MESSAGE;
		}
	}

	// let others deal with this
	return B_DISPATCH_MESSAGE;
}

// END - TransportButton.cpp
