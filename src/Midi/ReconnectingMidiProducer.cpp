/* ===================================================================== *
 * InternalSynth.cpp (MeV/Midi)
 * ===================================================================== */

#include "ReconnectingMidiProducer.h"

// Application Kit
#include <Message.h>
// Interface Kit
#include <Bitmap.h>
#include <GraphicsDefs.h>
// Support Kit
#include <Debug.h>

using namespace Midi;

// ---------------------------------------------------------------------------
// Constructor/Destructor

CReconnectingMidiProducer::CReconnectingMidiProducer(
	const char *name)
		: BMidiLocalProducer(name)
{
}

// ---------------------------------------------------------------------------
// Operations

void
CReconnectingMidiProducer::AddToConnectList(
	const char *name)
{
	BMessage *props = NULL;
	GetProperties (props);
	if (props)
	{
		props->AddString("mev:holdConnection",name);
		SetProperties (props);
	}
}

void
CReconnectingMidiProducer::RemoveFromConnectList(
	const char *name)
{
	BMessage props;
	BString rem;
	GetProperties (&props);
	int index=0;
	while ((props.FindString("mev:holdConnection",index,&rem))==B_OK)
	{
		if (rem.Compare(name)==0)
		{
			props.RemoveData("mev:holdConnection",index);
			
		}
	index++;
	}
	SetProperties(&props);
}

bool
CReconnectingMidiProducer::IsInConnectList(
	const char *name) const
{
	BMessage props;
	BString rem;
	GetProperties (&props);
	int index=0;
	while ((props.FindString("mev:holdConnection",index,&rem))==B_OK)
	{
		if (rem.Compare(name)==0)
		{
			return (true);
		}
	index++;
	}
	return (false);
}

BBitmap *
CReconnectingMidiProducer::GetSmallIcon() const
{
	BMessage props;
	GetProperties(&props);
	return (_createIcon(&props,B_MINI_ICON));
}

BBitmap *
CReconnectingMidiProducer::GetLargeIcon() const 
{
	BMessage props;
	GetProperties(&props);
	return (_createIcon(&props,B_LARGE_ICON));
}

void
CReconnectingMidiProducer::SetSmallIcon(
	BBitmap *icon)
{
}

void
CReconnectingMidiProducer::SetLargeIcon(
	BBitmap *icon)
{
}

BBitmap *
CReconnectingMidiProducer::_createIcon(
	const BMessage *message,
	icon_size which) const
{
	float iconSize;
	uint32 iconType;
	const char* iconName;

	if (which == B_LARGE_ICON) {
		iconSize = B_LARGE_ICON;
		iconType = 'ICON';
		iconName = "be:large_icon";
	} else if (which == B_MINI_ICON) {
		iconSize = B_MINI_ICON;
		iconType = 'MICN';
		iconName = "be:mini_icon";
	} else {
		return NULL;
	}
							
	const void* data;
	ssize_t size;
	BBitmap* bitmap = NULL;

	if (message->FindData(iconName, iconType, &data, &size) == B_OK)
	{
		BRect r(0, 0, iconSize-1, iconSize-1);
		bitmap = new BBitmap(r, B_CMAP8);
		ASSERT((bitmap->BitsLength() == size));
		memcpy(bitmap->Bits(), data, size);
	}
	return bitmap;
}

// END - ReconnectingMidiProducer.cpp
