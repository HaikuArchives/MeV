// StandardMIDIFile -- importer and exporter for SMF format

#include "StandardMidiFile.h"

#include "BeFileWriter.h"
#include "BeFileReader.h"
#include "IFFWriter.h"
#include "Error.h"

// Application Kit
#include <Message.h>
// Interface Kit
#include <Alert.h>
// Storage Kit
#include <Directory.h>
#include <Entry.h>
#include <NodeInfo.h>
#include <Path.h>
// Support Kit
#include <Debug.h>
// C & Standard Template Library
#include <cctype>
#include <cmath>
#include <cstdio>
#include <algorithm>
#include <memory>
#include <vector>

enum {
	Import_ID		= 3,
	Export_ID		= 4,
};

// ---------------------------------------------------------------------------
// Exception classes

class PrematureEndOfTrack : public IError
{
public:
	virtual	const char*		Description() const
								{ return "Track data is corrupt."; }
};

// ---------------------------------------------------------------------------
// Function objects

class NoteEq
{
public:
	NoteEq(const CEvent& event)
		:	vChannel(event.GetVChannel()),
			pitch(event.GetAttribute(EvAttr_Pitch))
		{
		}

	bool operator()(const CEvent& event)
		{
			return    event.GetAttribute(EvAttr_Pitch) == pitch
			       && event.GetVChannel()               == vChannel;
		}

private:
	int8 vChannel;
	int8 pitch;
};

class HandleHungNote
{
public:
	HandleHungNote(MeVTrackHandle trk) : track(trk) { }

	void operator()(CEvent& event)
		{
			PRINT(("\tWarning! Hung note at %ld on vChannel %d, pitch %d\n",
			       event.Start(), event.GetVChannel(), event.note.pitch));

			event.SetDuration(0);
			event.SetAttribute(EvAttr_ReleaseVelocity, 64);
			
			track->Merge(&event, 1);
		}

private:
	MeVTrackHandle track;
};

// ---------------------------------------------------------------------------
// Main function

MeVPlugIn *CreatePlugin()
{
	return new CStandardMidiFile();
}

// ---------------------------------------------------------------------------
// Plugin constructor...

CStandardMidiFile::CStandardMidiFile()
{
	AddMenuItem( "Standard MIDI File...", Import_List, new BMessage( Import_ID ) );
	AddMenuItem( "Standard MIDI File...", Export_List, new BMessage( Export_ID ) );
}

// ---------------------------------------------------------------------------
// Destructor

CStandardMidiFile::~CStandardMidiFile()
{
}

// ---------------------------------------------------------------------------
// Function to display error code

void CStandardMidiFile::ShowError( const char *msg, ... )
{
	BAlert		*alert;
	va_list		argptr;
	char			msgBuf[ 256 ];
	
	va_start( argptr, msg );
	vsprintf( msgBuf, msg, argptr );
	va_end( argptr );
	
	alert = new BAlert(  NULL, msgBuf,
					"Continue",
					NULL,
					NULL,
					B_WIDTH_AS_USUAL, B_INFO_ALERT); 
	alert->SetShortcut( 1, B_ESCAPE );
	alert->Go();
}

// ---------------------------------------------------------------------------
// Text for "about plug-ins" dialog

char *CStandardMidiFile::AboutText()
{
	return "This plug-in imports and exports Standard MIDI Files.";
}

// ---------------------------------------------------------------------------
// File import function

void CStandardMidiFile::OnImport( BMessage *inMsg, entry_ref *ref, int32 inDetectedType )
{
	BFile				file;
	status_t			error;
	char				name[ B_FILE_NAME_LENGTH ];
	MeVDocHandle		doc = NULL;
	
	error = file.SetTo( ref, B_READ_ONLY );
	if (error != B_NO_ERROR)
	{
		ShowError(LookupErrorText(error));
		return;
	}
	
	BEntry		entry( ref );
	error = entry.GetName( name );
	if (error < B_OK)
	{
		ShowError( LookupErrorText( error ) );
		return;
	}

	try {
		int32			chunkID;
		int32			chunkLength;
		uint16			format;
		uint16			numTracks;
		uint16			division;
		smf_time_base	timeBase;
	
			// Establish a reader for this file. Note that the reader automatically takes care
			// of endian-flipping on little-endian machines.
		CBeFileReader	reader(file);
		
		reader >> chunkID >> chunkLength;

		if (chunkID != 'MThd' || chunkLength < 6)
		{
			ShowError( "'%s' is not a Standard MIDI File.", name );
			return;
		}
		
		PRINT(("Importing standard MIDI file \"%s\"\n", name));

			// Read in the header information
		reader >> format >> numTracks >> division;

		PRINT(("format %hu, %hu tracks, ", format, numTracks));
	
		switch (timeBase.format = (division & 0x8000))
		{
			case smf_time_base::METERED:
				timeBase.base.metered.ticksPerQuarterNote = division;
				PRINT(("%hd ticks per quarter note\n", timeBase.base.metered.ticksPerQuarterNote));
				break;
			case smf_time_base::TIME_CODE:
				timeBase.base.timeCode.smpteFormat   = division / 256;
				timeBase.base.timeCode.ticksPerFrame = division & 0x00FF;
				PRINT(("SMPTE format '%hd', %hd ticks per frame\n",
				       timeBase.base.timeCode.smpteFormat,
				       timeBase.base.timeCode.ticksPerFrame));
				break;
		}
		
			// Skip over any extra part of the header...
		reader.Skip( chunkLength - (sizeof format + sizeof numTracks + sizeof division) );
		
			// At this point, it's time to create a new document.
		BString docName(name);
		docName << " (Converted)";
		doc = NewDocument(docName.String(), false);
		CreateDestinations(doc, name);
		
			// Read in each of the tracks.
		for (int trackNum = 0; reader.BytesAvailable() >= 8; trackNum++)
		{
			reader >> chunkID >> chunkLength;
			if (chunkID != 'MTrk')
			{
				reader.Skip(chunkLength);
				continue;
			}

			auto_ptr<uint8> trackBuffer(new uint8[chunkLength]);
			reader.MustRead(trackBuffer.get(), chunkLength);
			if (ReadTrack(doc, trackBuffer.get(), chunkLength, timeBase) == false)
				break;
		}

	} catch (IError &e) {
		ShowError( e.Description() );
	}

	if (doc)
	{
		SetInitialTempo(doc);
		doc->ShowWindow();
	}
}

// ---------------------------------------------------------------------------
// Function to create destinations for imported events

void CStandardMidiFile::CreateDestinations(MeVDocHandle doc, const char* filename)
{
	for (int ch = 0; ch < 16; ++ch)
	{
		BString name(filename);
		name << " " << ch + 1;
		int internalSynth = doc->GetInternalSynthConsumerID();
		
		m_destinationID[ch] = doc->NewDestination(name.String(), internalSynth, ch + 1);
	}
}

bool
CStandardMidiFile::ReadTrack(
	MeVDocHandle doc,
	uint8 *ptr,
	long length,
	const smf_time_base& timeBase)
{
	CEvent event;
	MeVTrackHandle track = NULL;
	TClockType clockType = (timeBase.format == smf_time_base::METERED)
						   ? ClockType_Metered
						   : ClockType_Real;
	vector<CEvent> notesInProgress;

	try
	{
		SMFTrackReader fileTrack(*this, ptr, length, m_destinationID,
								 timeBase);
		track = doc->NewEventTrack(clockType);
		PRINT(("\tNew %s track: ID=%ld\n",
		       (clockType == ClockType_Metered) ? "metered" : "real-time",
		       track->GetID()));
	
		while (fileTrack.GetNextEvent(event))
		{
			if (event.Command() == EvtType_Text && event.text.textType == 0x03)
				track->SetName(reinterpret_cast<char *>(event.ExtendedData()));

			if (event.Command() == EvtType_Note)
			{
				notesInProgress.push_back(event);
			}		
			else if (event.Command() == EvtType_NoteOff)
			{
				// find matching noteon
				vector<CEvent>::iterator noteOn = find_if(notesInProgress.begin(),
														  notesInProgress.end(),
														  NoteEq(event));
				if (noteOn != notesInProgress.end())
				{
					noteOn->SetDuration(event.Start() - noteOn->Start());
					noteOn->SetAttribute(EvAttr_ReleaseVelocity,
										 event.GetAttribute(EvAttr_ReleaseVelocity));
					track->Merge(&(*noteOn), 1);
					notesInProgress.erase(noteOn);
				}
				else
				{
					PRINT(("\tUnmatched note-off at %ld: vChannel = %d, pitch = %ld\n",
					       event.Start(), event.GetVChannel(),
					       event.GetAttribute(EvAttr_Pitch)));
				}
			}
			else
			{
				track->Merge(&event, 1);
			}
		}
		
		// shut off any hung notes
		for_each(notesInProgress.begin(), notesInProgress.end(), HandleHungNote(track));

		// create an instance of the track
		MeVTrackHandle master = doc->ActiveMasterTrack();
		CEvent trackEv;
		trackEv.SetCommand(EvtType_Sequence);
		trackEv.SetStart(0);
		trackEv.SetDuration(track->Duration());
		trackEv.sequence.sequence = track->GetID();
		trackEv.sequence.transposition = 0;
		trackEv.sequence.transposition = 0;
		trackEv.sequence.flags = 0;
		trackEv.sequence.vPos = track->GetID() - 2;
		master->Merge(&trackEv, 1);
		doc->ReleaseTrack(master);

		doc->ReleaseTrack(track);
		return true;
	}
	catch (IError& e)
	{
		ShowError(e.Description());
		if (track)
			doc->ReleaseTrack(track);
			return false;
	}
}

// ---------------------------------------------------------------------------
// Function to find and set initial tempo for document

void CStandardMidiFile::SetInitialTempo(MeVDocHandle doc)
{
	// default tempo is 120 BPM.  For type 1 files, tempo map should be in first track
	double tempo = 120.0;
	
	MeVTrackHandle track = doc->FirstTrack();
	if (track)
	{
		MeVEventHandle pEvent;
		for (pEvent = track->FirstEvent();
		     pEvent->Valid() && pEvent->EventPtr()->Start() == 0;
		     pEvent->Seek(1))
		{
			const CEvent* event = pEvent->EventPtr();
			if (event->Command() == EvtType_Tempo)
			{
				tempo = double(event->GetAttribute(EvAttr_TempoValue)) / 1000.0;
				break;
			}
		}

		track->ReleaseEventRef(pEvent);
		doc->ReleaseTrack(track);
	}
	
	doc->SetInitialTempo(tempo);
}

// ---------------------------------------------------------------------------
// File export function

void CStandardMidiFile::OnExport( BMessage *inMsg, int32 inDocID, entry_ref *ref )
{
	BEntry				entry(ref);
	BFile				file;
	status_t			error;
	
	MeVDocHandle doc = FindDocument(inDocID);
	if (!doc)
		return;
		
	try
	{
		TClockType clockType;
		uint16 numTracks;
		if (CountTracks(doc, numTracks, clockType) < B_OK || numTracks == 0)
			return;

		if ((error = entry.InitCheck()) < B_OK)
		{
			ShowError(LookupErrorText(error));
			return;
		}
		
		if ((error = file.SetTo(&entry, B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE)) < B_OK)
		{
			ShowError(LookupErrorText(error));
			return;
		}
		
		CBeFileWriter	fileWriter(file);
		CIFFWriter		writer(fileWriter);
		
		// write header
		if (WriteHeaderChunk(writer, numTracks + 1, clockType) < B_OK)
			return;
		
		// write tempo track
		{
			SMFTrackWriter tempoTrack(writer, doc);
			
			// sequence name
			char name[B_FILE_NAME_LENGTH];
			doc->GetName(name, B_FILE_NAME_LENGTH);
			tempoTrack.WriteTrackName(name);
			
			// tempo
			tempoTrack.WriteTempo(doc->GetInitialTempo());
			tempoTrack.WriteEndOfTrack(0);
		}

		// write tracks
		MeVTrackHandle track = doc->FirstTrack();
		for (bool trackExists = track; trackExists; trackExists = track->NextTrack())
		{
			// skip non-MIDI tracks
			if (track->GetClockType() != clockType)
				continue;
			
			if (WriteTrack(writer, doc, track) < B_OK)
			{
				return;
			}
		}
		
		doc->ReleaseTrack(track);
		
		BNodeInfo nodeInfo(&file);
		if (nodeInfo.InitCheck() == B_OK)
			nodeInfo.SetType("audio/x-midi");
	}
	catch (IError& e)
	{
	}
	catch (...)
	{
	}
}

// ---------------------------------------------------------------------------
// Write a track
status_t CStandardMidiFile::WriteTrack(CIFFWriter& writer, MeVDocHandle doc, MeVTrackHandle track)
{
	SMFTrackWriter trackWriter(writer, doc);
	
	char name[B_FILE_NAME_LENGTH];
	track->GetName(name, B_FILE_NAME_LENGTH);
	trackWriter.WriteTrackName(name);
 
	MeVEventHandle pEvent;
	CEvent event;
	for (pEvent = track->FirstEvent(); pEvent->GetEvent(&event); pEvent->Seek(1))
		trackWriter.WriteEvent(event);

	track->ReleaseEventRef(pEvent);

	return true;
}

// ---------------------------------------------------------------------------
// Return number of tracks in document

status_t CStandardMidiFile::CountTracks(MeVDocHandle doc, uint16& numTracks, TClockType& clockType)
{
	numTracks = 0;

	// find clock type (first metered or real-time track)
	MeVTrackHandle track = doc->FirstTrack();
	for (bool trackExists = track; trackExists; trackExists = track->NextTrack())
	{
		clockType = track->GetClockType();
		if (clockType == ClockType_Metered || clockType == ClockType_Real)
		{
			++numTracks;
			break;
		}
	}
	
	// count real-time or metered tracks
	if (track)
	{
		while (track->NextTrack())
		{
			if (track->GetClockType() == clockType)
			{
				++numTracks;
			}
			else if (track->GetClockType() == ClockType_Real || track->GetClockType() == ClockType_Metered)
			{
				ShowError("Cannot mix real time and metered tracks in a single MIDI file.");
				doc->ReleaseTrack(track);
				return B_ERROR;
			}
		}
	}
	
	return B_OK;
}

status_t CStandardMidiFile::WriteHeaderChunk(CIFFWriter& writer, uint16 numTracks, TClockType clockType)
{
	// write header chunk
	uint16 division;
	switch (clockType)
	{
		case ClockType_Metered:
			division = 480;
			break;
		case ClockType_Real:
			division = 0x8000 | (int8(-25) * 256) | 40;
			break;
		default:
			ShowError("Unknown clock type");
			return B_ERROR;
	}
	
	writer.Push('MThd');
	writer << uint16(1) << numTracks << division;
	writer.Pop();

	return B_OK;
}

// ---------------------------------------------------------------------------
// File type filter

int32 CStandardMidiFile::DetectFileType(
	const entry_ref	*ref,
	BNode			*node,
	struct stat		*st,
	const char		*filetype )
{
	char				name[ B_FILE_NAME_LENGTH ];

		// First, check MIME type
	if (strstr( filetype, "audio/x-midi" ) != NULL) return 0;
	
		// If MIME type not set correctly, then check the filename for
		// known common MIDI filename extensions
	BEntry		entry( ref );
	if (entry.GetName( name ) == B_NO_ERROR)
	{
		char			*dot = strrchr( name, '.' );
		if (dot == NULL) return 0;
		
			// Downcase the name, since we don't have stricmp()
		for (char *p = dot; *p; p++) *p = tolower( *p );
		
		if (	strcmp( dot, ".mid" ) == 0
			|| strcmp( dot, ".smf" ) == 0) return 0;
	}
	
	return -1;
}

// ---------------------------------------------------------------------------
// Utility class for reading standard MIDI file tracks

CStandardMidiFile::SMFTrackReader::SMFTrackReader(CStandardMidiFile& smfPlugin, const uint8* data, int32 len,
                                                  int* destinationIDs, const smf_time_base& tBase)
	:	m_plugin(smfPlugin),
		m_destinationID(destinationIDs),
		m_pData(data),
		m_pEnd(data + len),
		m_trackLength(len),
		m_timeBase(tBase),
		m_fileTime_ticks(0),
		m_runningStatus(0)
{
}

bool CStandardMidiFile::SMFTrackReader::GetNextEvent(CEvent& outEvent)
{
	CEvent	event;	// start with fresh event so all fields are initialized
	uint8	status;
	bool	gotEvent = false;

	while (!gotEvent)
	{
		if (m_pData >= m_pEnd)
			return false;
		
		int64 mevTime = GetTime();
		if (mevTime > 2147483647)
		{
			PRINT(("Event time is %Ld (max %ld)\n", mevTime, 2147483647L));
			m_plugin.ShowError( "Track exceeds maximum length." );
			return false;
		}
	
		event.SetStart(mevTime);
		PRINT(("\t\tEvent time=%ld\n", event.Start()));

		status = GetStatusByte();
		switch (status & 0xf0)
		{
			case 0x80:
				gotEvent = ReadNoteOff(status, event);
				break;
	
			case 0x90:
				gotEvent = ReadNoteOn(status, event);
				break;
	
			case 0xA0:
				gotEvent = ReadPolyPressure(status, event);
				break;
	
			case 0xB0:
				gotEvent = ReadControlChange(status, event);
				break;
	
			case 0xC0:
				gotEvent = ReadProgramChange(status, event);
				break;
	
			case 0xD0:
				gotEvent = ReadChannelPressure(status, event);
				break;
	
			case 0xE0:
				gotEvent = ReadPitchBend(status, event);
				break;
	
			case 0xF0:			// sysex and meta
				switch (status)
				{
					case 0xf0:
						gotEvent = ReadSystemExclusive(status, event);
						break;

					case 0xff:
						gotEvent = ReadMetaEvent(status, event);
						break;
						
					default:
						m_plugin.ShowError("Error reading file: %x byte encountered at offset %d in track.",
						                   status, Position());
						return false;
				}
				break;

			default:						// unrecognized event
				m_plugin.ShowError("Error reading file: unrecognized status byte '%x' encountered at offset %d in track.",
				                   status, Position());
				return false;
		}
	}
	
	outEvent = event;
	return true;
}

int64 CStandardMidiFile::SMFTrackReader::GetTime()
{
	int64 realTime_usec;
	int64 mevTime;

	uint32 deltaTime = GetVariableLengthNumber();
	m_fileTime_ticks += deltaTime;
	
	if (m_timeBase.format == smf_time_base::METERED)
	{
		mevTime = (m_fileTime_ticks * Ticks_Per_QtrNote) / m_timeBase.base.metered.ticksPerQuarterNote;
	}
	else
	{
		switch (m_timeBase.base.timeCode.smpteFormat)
		{
			case smf_time_base::SMPTE_24:
				realTime_usec = (m_fileTime_ticks * 1000000) / (24 * int64(m_timeBase.base.timeCode.ticksPerFrame));
				break;
			case smf_time_base::SMPTE_25:
				realTime_usec = (m_fileTime_ticks * 1000000) / (25 * int64(m_timeBase.base.timeCode.ticksPerFrame));
				break;
			case smf_time_base::SMPTE_30_DROP:
				{
					// 30 frames per second, but skip two frames at the beginning
					// of each minute that does not end in 0 (0, 10, 20, ...).
					// We treat it like 30 non drop, but add time to adjust for dropped frames.
					int64 frame = m_fileTime_ticks / int64(m_timeBase.base.timeCode.ticksPerFrame);
					int64 framesDropped = (frame / 17982) * 18 + ((frame % 17982 - 2) / 1798) * 2;

					realTime_usec  = (m_fileTime_ticks * 1000000) / (30 * int64(m_timeBase.base.timeCode.ticksPerFrame));
					realTime_usec += (framesDropped    * 1000000) /  30;
				}
				break;
			case smf_time_base::SMPTE_30:
			default:
				realTime_usec = (m_fileTime_ticks * 1000000) / (30 * int64(m_timeBase.base.timeCode.ticksPerFrame));
				break;
		}
		
		// round to nearest millisecond
		mevTime = (realTime_usec + 500) / 1000;
	}

	return mevTime;
}

uint8 CStandardMidiFile::SMFTrackReader::GetStatusByte()
{
	// handle running status
	if (PeekByte() & 0x80)
	{
		uint8 status = GetByte();
		if (status < 0xF8) // not realtime
			m_runningStatus = status;
		return status;
	}
	else
	{
		return m_runningStatus;
	}
}

bool CStandardMidiFile::SMFTrackReader::ReadNoteOn(uint8 status, CEvent& event)
{
	uint8 pitch = GetByte();
	uint8 vel   = GetByte();

	PRINT(("\t\t\tNote "));
	if (vel == 0)
	{
		event.SetCommand(EvtType_NoteOff);
		vel = 64;
		PRINT(("Off"));
	}
	else
	{
		event.SetCommand(EvtType_Note);
		PRINT(("On"));
	}
	event.SetVChannel(m_destinationID[status & 0x0F]);
	event.SetAttribute(EvAttr_Pitch,          pitch);
	event.SetAttribute(EvAttr_AttackVelocity, vel);

	PRINT(("       vChannel=%-3d, pitch=%-3ld, vel=%-3ld\n",
	       event.GetVChannel(),
	       event.GetAttribute(EvAttr_Pitch),
	       event.GetAttribute(EvAttr_AttackVelocity)));

	if (event.GetAttribute(EvAttr_AttackVelocity) == 0)
	{
		event.SetCommand(EvtType_NoteOff);
		event.SetAttribute(EvAttr_ReleaseVelocity, 64);
		PRINT(("\t\t\t--->Converted to Note Off\n"));
	}
	
	return true;
}

bool CStandardMidiFile::SMFTrackReader::ReadNoteOff(uint8 status, CEvent& event)
{
	event.SetCommand(EvtType_NoteOff);
	event.SetVChannel(m_destinationID[status & 0x0F]);
	event.SetAttribute(EvAttr_Pitch,           GetByte());
	event.SetAttribute(EvAttr_ReleaseVelocity, GetByte());

	PRINT(("\t\t\tNote Off      vChannel=%-3d, pitch=%-3ld, vel=%-3ld\n",
	       event.GetVChannel(),
	       event.GetAttribute(EvAttr_Pitch),
	       event.GetAttribute(EvAttr_ReleaseVelocity)));
	return true;
}

bool CStandardMidiFile::SMFTrackReader::ReadPolyPressure(uint8 status, CEvent& event)
{
	event.SetCommand(EvtType_PolyATouch);
	event.SetVChannel(m_destinationID[status & 0x0F]);
	event.SetAttribute(EvAttr_Pitch,      GetByte());
	event.SetAttribute(EvAttr_AfterTouch, GetByte());
	
	PRINT(("\t\t\tPoly Pressure vChannel=%-3d, pitch=%-3ld, value=%-3ld\n",
	       event.GetVChannel(),
	       event.GetAttribute(EvAttr_Pitch),
	       event.GetAttribute(EvAttr_AfterTouch)));

	return true;
}

bool CStandardMidiFile::SMFTrackReader::ReadChannelPressure(uint8 status, CEvent& event)
{
	event.SetCommand(EvtType_ChannelATouch);
	event.SetVChannel(m_destinationID[status & 0x0F]);
	event.SetAttribute(EvAttr_AfterTouch, GetByte());
	PRINT(("\t\t\tChannel Pressure vChannel=%-3d, value=%-3ld\n",
	       event.GetVChannel(),
	       event.GetAttribute(EvAttr_AfterTouch)));

	return true;
}

bool CStandardMidiFile::SMFTrackReader::ReadControlChange(uint8 status, CEvent& event)
{
	event.SetCommand(EvtType_Controller);
	event.SetVChannel(m_destinationID[status & 0x0F]);
	event.SetAttribute(EvAttr_ControllerNumber, GetByte());
	event.SetAttribute(EvAttr_ControllerValue8, GetByte());
	PRINT(("\t\t\tControl Change vChannel=%-3d, controller=%-3ld, value=%-3ld\n",
	       event.GetVChannel(),
	       event.GetAttribute(EvAttr_ControllerNumber),
	       event.GetAttribute(EvAttr_ControllerValue8)));

	return true;
}

bool CStandardMidiFile::SMFTrackReader::ReadProgramChange(uint8 status, CEvent& event)
{
	// To do: merge with bank select?
	event.SetCommand(EvtType_ProgramChange);
	event.SetVChannel(m_destinationID[status & 0x0F]);
	event.SetAttribute(EvAttr_Program, GetByte());
	PRINT(("\t\t\tProgram Change vChannel=%-3d, program=%-3ld\n",
	       event.GetVChannel(),
	       event.GetAttribute(EvAttr_Program)));

	return true;
}

bool CStandardMidiFile::SMFTrackReader::ReadPitchBend(uint8 status, CEvent& event)
{
	event.SetCommand(EvtType_PitchBend);
	event.SetVChannel(m_destinationID[status & 0x0F]);

	int16 bendAmount = GetByte() - 8192;
	bendAmount += GetByte() * 128;
	event.SetAttribute(EvAttr_BendValue,   bendAmount);
	event.SetAttribute(EvAttr_InitialBend, bendAmount);
	PRINT(("\t\t\tPitch Bend vChannel=%-3d, amount=%-3ld\n",
	       event.GetVChannel(),
	       event.GetAttribute(EvAttr_BendValue)));

	return true;
}

bool CStandardMidiFile::SMFTrackReader::ReadSystemExclusive(uint8 status, CEvent& event)
{
	vector<uint8> buf;
	uint8 ch;

	buf.reserve(256); // avoid reallocations for most messages
	
	while ((ch = GetByte()) != 0xF7)
		buf.push_back(ch);

	event.SetCommand(EvtType_SysEx);
	event.SetVChannel(m_destinationID[status & 0x0F]); // is this correct ???
	if (event.SetExtendedDataSize(buf.size()))
		memcpy(event.ExtendedData(), &buf[0], buf.size());
	PRINT(("\t\t\tSystem Exclusive length=%ld\n", buf.size()));

	return true;
}

bool CStandardMidiFile::SMFTrackReader::ReadMetaEvent(uint8 status, CEvent& event)
{
	uint32	usecPerQtr;

	uint8  type   = GetByte();
	uint32 length = GetVariableLengthNumber();
	
	switch (type)
	{
		case 0x01:			// text
		case 0x02:			// Copyright notice
		case 0x03:			// Sequence/track name
		case 0x04:			// Instrument name
		case 0x05:			// Lyric
		case 0x06:			// Marker
		case 0x07:			// Cue point
			event.SetCommand(EvtType_Text);
			event.text.textType = type;
			if (event.SetExtendedDataSize(length + 1))
			{
				GetBytes(reinterpret_cast<uint8*>(event.ExtendedData()), length);
				reinterpret_cast<char*>(event.ExtendedData())[length] = '\0';
			}
			else
			{
				SkipBytes(length);
			}
			PRINT(("\t\t\tMeta Data text type=%d length=%lu\n",
			       event.text.textType, length));
			break;
		
		case 0x2f:			// End of track
			event.SetCommand(EvtType_End);
			SkipBytes(length); // should be 0
			PRINT(("\t\t\tEnd Of Track\n"));
			break;
			
		case 0x51:			// Set tempo
			usecPerQtr  = GetByte() * 65536;
			usecPerQtr += GetByte() *   256;
			usecPerQtr += GetByte();
			event.SetCommand(EvtType_Tempo);
			event.SetAttribute(EvAttr_TempoValue, uint32(1000.0 * 60.0 * 1000000.0 / double(usecPerQtr) + 0.5));
			PRINT(("\t\t\tSet Tempo=%ld (%f BPM, %ld usecs/qtr)\n",
			       event.GetAttribute(EvAttr_TempoValue),
			       double(event.GetAttribute(EvAttr_TempoValue)) / 1000.0,
			       usecPerQtr));
			break;
		
		case 0x58:			// Time signaturee
			event.SetCommand(EvtType_TimeSig);
			event.SetAttribute(EvAttr_TSigBeatCount, GetByte());
			event.SetAttribute(EvAttr_TSigBeatSize,  1 << GetByte());
			event.SetDuration(0);
			SkipBytes(2);
			PRINT(("\t\t\tTime Signature %ld/%ld\n",
			       event.GetAttribute(EvAttr_TSigBeatCount),
			       event.GetAttribute(EvAttr_TSigBeatSize)));
			break;
		
		case 0x00:			// sequence ID 						(handled separately)
		case 0x20:			// MIDI channel prefix				(not handled)
		case 0x54:			// SMPTE Offset						(not handled)
		case 0x59:			// Key signaturee					(not handled)
		case 0x7F:			// Sequencer-specific meta event	(not used)
		default:
			PRINT(("\t\t\tMeta code %d, length %ld\n", type, length));
			SkipBytes(length);
			return false;
	}

	return true;
}

uint8 CStandardMidiFile::SMFTrackReader::GetByte()
{
	if (m_pData >= m_pEnd)
		throw PrematureEndOfTrack();
	
	return *m_pData++;
}

uint8 CStandardMidiFile::SMFTrackReader::PeekByte() const
{
	if (m_pData >= m_pEnd)
		throw PrematureEndOfTrack();
	
	return *m_pData;
}

void CStandardMidiFile::SMFTrackReader::GetBytes(uint8* buffer, int32 numBytes)
{
	if (m_pData + numBytes >= m_pEnd)
		throw PrematureEndOfTrack();

	memcpy(buffer, m_pData, numBytes);
	m_pData += numBytes;
}

void CStandardMidiFile::SMFTrackReader::SkipBytes(int32 numBytes)
{
	if (m_pData + numBytes >= m_pEnd && numBytes > 0)
		throw PrematureEndOfTrack();
	
	m_pData += numBytes;
}

uint32 CStandardMidiFile::SMFTrackReader::GetVariableLengthNumber()
{
	uint32 v = 0;
	uint8 b;

	do
	{
		b = GetByte();
		v = (v << 7) | (b & 0x7f);
	} while (b & 0x80);
	
	return v;
}

// ---------------------------------------------------------------------------
// Utility class for reading standard MIDI file tracks
CStandardMidiFile::SMFTrackWriter::SMFTrackWriter(CIFFWriter& writer, MeVDocHandle doc)
	:	m_writer(writer), m_doc(doc), m_lastEventTicks(0)
{
	m_writer.AllowOddLengthChunks();
	m_writer.Push('MTrk');
}

CStandardMidiFile::SMFTrackWriter::~SMFTrackWriter()
{
	m_writer.Pop();
}

void CStandardMidiFile::SMFTrackWriter::WriteTempo(double tempo_bpm, uint32 deltaTime)
{
	uint32 usecPerQtr = uint32((60.0 * 1000000.0) / tempo_bpm);
	WriteVariableLengthNumber(deltaTime);
	m_writer << uint8(0xFF) << uint8(0x51) << uint8(0x03)
	         << uint8((usecPerQtr >> 16) & 0xFF)
	         << uint8((usecPerQtr >>  8) & 0xFF)
	         << uint8( usecPerQtr        & 0xFF);
}

void CStandardMidiFile::SMFTrackWriter::WriteTrackName(const char* name, uint32 deltaTime)
{
	uint32 length = strlen(name);

	WriteVariableLengthNumber(deltaTime);
	m_writer << uint8(0xFF) << uint8(0x03);
	WriteVariableLengthNumber(length);
	m_writer.MustWrite(const_cast<char*>(name), length);
}

void CStandardMidiFile::SMFTrackWriter::WriteEvent(CEvent& event)
{
	// check for any pending note-offs whose times have arrived
	WritePendingNoteOffs(event.Start());

	uint32 deltaTime = CalculateDeltaTime(event.Start());
	
	switch (event.Command())
	{
		case EvtType_Note:
			WriteNoteOn(event, deltaTime);
			break;

		case EvtType_NoteOff:
			WriteNoteOff(event, deltaTime);
			break;

		case EvtType_ChannelATouch:
			WriteChannelPressure(event, deltaTime);
			break;

		case EvtType_PolyATouch:
			WritePolyPressure(event, deltaTime);
			break;

		case EvtType_Controller:
			WriteControlChange(event, deltaTime);
			break;

		case EvtType_ProgramChange:
			WriteProgramChange(event, deltaTime);
			break;

		case EvtType_PitchBend:
			WritePitchBend(event, deltaTime);
			break;

		case EvtType_SysEx:
			WriteSystemExclusive(event, deltaTime);
			break;

		case EvtType_Text:
			WriteTextMetaEvent(event, deltaTime);
			break;

		case EvtType_Tempo:
			WriteTempoMetaEvent(event, deltaTime);
			break;

		case EvtType_TimeSig:
			WriteTimeSigMetaEvent(event, deltaTime);
			break;
		
		case EvtType_End:
			WriteEndOfTrack(event.Start());
			break;
	}
}

uint32 CStandardMidiFile::SMFTrackWriter::CalculateDeltaTime(int32 startTime)
{
	ASSERT(startTime >= m_lastEventTicks);
	
	int32 delta = startTime - m_lastEventTicks;
	m_lastEventTicks = startTime;
	return uint32(delta);
}

uint8 CStandardMidiFile::SMFTrackWriter::GetMidiChannel(const CEvent& event)
{
	int channel = m_doc->GetChannelForDestination(event.GetVChannel());

	ASSERT(1 <= channel && channel <= 16);
	return uint8(channel);
}

void CStandardMidiFile::SMFTrackWriter::WriteNoteOn(const CEvent& event, uint32 deltaTime)
{
	WriteVariableLengthNumber(deltaTime);
	m_writer << uint8(0x90 + GetMidiChannel(event))
	         << uint8(event.GetAttribute(EvAttr_Pitch))
	         << uint8(event.GetAttribute(EvAttr_AttackVelocity));
	{
		// add NoteOff event to list
		CEvent noteOff;
		noteOff.SetCommand(EvtType_NoteOff);
		noteOff.SetStart(event.Start() + event.Duration());
		noteOff.SetVChannel(event.GetVChannel());
		noteOff.SetAttribute(EvAttr_Pitch, event.GetAttribute(EvAttr_Pitch));
		noteOff.SetAttribute(EvAttr_ReleaseVelocity,
		                     event.GetAttribute(EvAttr_ReleaseVelocity));
		m_pendingNoteOffs.push(noteOff);
	}
}

void CStandardMidiFile::SMFTrackWriter::WriteNoteOff(const CEvent& event, uint32 deltaTime)
{
	WriteVariableLengthNumber(deltaTime);
	m_writer << uint8(0x80 + GetMidiChannel(event))
	         << uint8(event.GetAttribute(EvAttr_Pitch))
	         << uint8(event.GetAttribute(EvAttr_ReleaseVelocity));
}

void CStandardMidiFile::SMFTrackWriter::WritePolyPressure(const CEvent& event, uint32 deltaTime)
{
	WriteVariableLengthNumber(deltaTime);
	m_writer << uint8(0xA0 + GetMidiChannel(event))
	         << uint8(event.GetAttribute(EvAttr_Pitch))
	         << uint8(event.GetAttribute(EvAttr_AfterTouch));
}

void CStandardMidiFile::SMFTrackWriter::WriteChannelPressure(const CEvent& event, uint32 deltaTime)
{
	WriteVariableLengthNumber(deltaTime);
	m_writer << uint8(0xD0 + GetMidiChannel(event))
	         << uint8(event.GetAttribute(EvAttr_AfterTouch));
}

void CStandardMidiFile::SMFTrackWriter::WriteControlChange(const CEvent& event, uint32 deltaTime)
{
	uint8 controllerNumber = event.GetAttribute(EvAttr_ControllerNumber);
	WriteVariableLengthNumber(deltaTime);
	m_writer << uint8(0xB0 + GetMidiChannel(event)) << controllerNumber;
	if (controllerNumber <= 31)
	{
		uint16 val = uint16(event.GetAttribute(EvAttr_ControllerValue16));
		m_writer << uint8(val >> 8);
		if (val & 0x00FF)
		{
			WriteVariableLengthNumber(deltaTime);
			m_writer << uint8(0xB0 + GetMidiChannel(event))
			         << uint8(controllerNumber + 32)
			         << uint8(val & 0x00FF);
		}
	}
	else
	{
		m_writer << uint8(event.GetAttribute(EvAttr_ControllerValue8));
	}
}

void CStandardMidiFile::SMFTrackWriter::WriteProgramChange(const CEvent& event, uint32 deltaTime)
{
	WriteVariableLengthNumber(deltaTime);
	m_writer << uint8(0xC0 + GetMidiChannel(event))
	         << uint8(event.GetAttribute(EvAttr_Program));
}

void CStandardMidiFile::SMFTrackWriter::WritePitchBend(const CEvent& event, uint32 deltaTime)
{
	WriteVariableLengthNumber(deltaTime);
	m_writer << uint8(0xE0 + GetMidiChannel(event))
	         << uint8((event.GetAttribute(EvAttr_BendValue) >> 8) & 0x7F)
	         << uint8(event.GetAttribute(EvAttr_BendValue) & 0x7F);
}

void CStandardMidiFile::SMFTrackWriter::WriteSystemExclusive(const CEvent& event, uint32 deltaTime)
{
	WriteVariableLengthNumber(deltaTime);
	m_writer << uint8(0xF0);
	m_writer.MustWrite(reinterpret_cast<uint32*>(event.ExtendedData()),
	                   event.ExtendedDataSize());
	m_writer << uint8(0xF7);
}

void CStandardMidiFile::SMFTrackWriter::WriteTextMetaEvent(const CEvent& event, uint32 deltaTime)
{
	int32 count = event.ExtendedDataSize() - 1;
	uint32* p = reinterpret_cast<uint32*>(event.ExtendedData());
	WriteVariableLengthNumber(deltaTime);
	m_writer << uint8(0xFF)
	         << uint8(event.text.textType);
	WriteVariableLengthNumber(count);
	m_writer.MustWrite(p, count);
}

void CStandardMidiFile::SMFTrackWriter::WriteTempoMetaEvent(const CEvent& event, uint32 deltaTime)
{
	WriteVariableLengthNumber(deltaTime);
	m_writer << uint8(0xFF) << uint8(0x51) << uint8(0x03);
	uint32 usecPerQtr = uint32(60000000000.0 / double(event.GetAttribute(EvAttr_TempoValue)) + 0.5);
	m_writer << uint8((usecPerQtr >> 16) & 0xFF)
	         << uint8((usecPerQtr >>  8) & 0xFF)
	         << uint8( usecPerQtr        & 0xFF);
}

void CStandardMidiFile::SMFTrackWriter::WriteTimeSigMetaEvent(const CEvent& event, uint32 deltaTime)
{
	WriteVariableLengthNumber(deltaTime);
	int32 log2Size = int32(log(event.GetAttribute(EvAttr_TSigBeatSize)) / log(2));
	m_writer << uint8(0xFF) << uint8(0x58) << uint8(0x04)
	         << uint8(event.GetAttribute(EvAttr_TSigBeatCount))
	         << uint8(log2Size)
	         << uint8(24)
	         << uint8(8);
}

void CStandardMidiFile::SMFTrackWriter::WriteEndOfTrack(uint32 time)
{
	// write any remaining note-offs (shifted forward to current time)
	WritePendingNoteOffs(time);

	WriteVariableLengthNumber(CalculateDeltaTime(time));
	m_writer << uint8(0xFF) << uint8(0x2F) << uint8(0x00);
}

void CStandardMidiFile::SMFTrackWriter::WritePendingNoteOffs(int32 time)
{
	while (!m_pendingNoteOffs.empty() &&  m_pendingNoteOffs.top().Start() <= time)
	{
		const CEvent& noteOff = m_pendingNoteOffs.top();
		WriteNoteOff(noteOff, CalculateDeltaTime(noteOff.Start()));
		
		m_pendingNoteOffs.pop();
	}
}

void CStandardMidiFile::SMFTrackWriter::WriteVariableLengthNumber(uint32 value )
{
	ASSERT(value <= 0x0FFFFFFF); // largest number allowed by SMF spec

	uint32	buffer = value & 0x7f;
	while ((value >>= 7) > 0)
	{
		buffer <<= 8;
		buffer |= 0x80;
		buffer += (value & 0x7f);
	}

	while (true)
	{
		m_writer << uint8(buffer & 0xFF);
		if (buffer & 0x80)
			buffer >>= 8;
		else
			break;
	}
}
