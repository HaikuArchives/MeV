/* ===================================================================== *
 * LoopButton.cpp (MeV/UI)
 * ===================================================================== */

#include "LoopButton.h"

// ---------------------------------------------------------------------------
// Constructor/Destructor

CLoopButton::CLoopButton(
	BRect frame,
	const char *name,
	BBitmap *normalBitmap,
	BBitmap *disabledBitmap,
	BBitmap *pressedBitmap,
	BBitmap *normalLoopingBitmap,
	BBitmap *pressedLoopingBitmap,
	BMessage *message,
	uint32 key,
	uint32 modifiers,
	uint32 resizingMode)
	:	CTransportButton(frame, name,
						 normalBitmap, disabledBitmap, pressedBitmap,
						 message, 0, key, modifiers, 
						 resizingMode),
		m_normalLoopingBitmap(normalLoopingBitmap),
		m_pressedLoopingBitmap(pressedLoopingBitmap),
		m_looping(false)
{
}

// ---------------------------------------------------------------------------
// Operations

void 
CLoopButton::SetLooping(
	bool looping)
{
	m_looping = looping;
	Invalidate();
}

// ---------------------------------------------------------------------------
// CTransportButton Implementation

BBitmap *
CLoopButton::MakeBitmap(
	uint32 mask)
{
	switch (mask)
	{
		case LOOPING_MASK:
			return m_normalLoopingBitmap;
		case LOOPING_MASK | PRESSED_MASK:
			return m_pressedLoopingBitmap;
		default:
			return CTransportButton::MakeBitmap(mask);
	}	

	return 0;
}

uint32 
CLoopButton::ModeMask() const
{
	if (!IsEnabled())
		return DISABLED_MASK;
	
	uint32 result = 0;

	if (Value())
		result = PRESSED_MASK;

	if (IsLooping())
		result |= LOOPING_MASK;

	return result;
}

void 
CLoopButton::DonePressing()
{
	if (IsLooping())
		SetLooping(false);
	else
		SetLooping(true);

	CTransportButton::DonePressing();
}

// END - LoopButton.cpp
