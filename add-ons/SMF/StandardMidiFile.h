/* ===================================================================== *
 * DocApp.h (MeV/Application Framework)
 * ---------------------------------------------------------------------
 * License:
 *  The contents of this file are subject to the Mozilla Public
 *  License Version 1.1 (the "License"); you may not use this file
 *  except in compliance with the License. You may obtain a copy of
 *  the License at http://www.mozilla.org/MPL/
 *
 *  Software distributed under the License is distributed on an "AS
 *  IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 *  implied. See the License for the specific language governing
 *  rights and limitations under the License.
 *
 *  The Original Code is MeV (Musical Environment) code.
 *
 *  The Initial Developer of the Original Code is Sylvan Technical 
 *  Arts. Portions created by Sylvan are Copyright (C) 1997 Sylvan 
 *  Technical Arts. All Rights Reserved.
 *
 *  Contributor(s): 
 *		Curt Malouin (malouin)
 *
 * ---------------------------------------------------------------------
 * Purpose:
 * 	Plugin which imports and exports standard MIDI files
 * ---------------------------------------------------------------------
 * History:
 *	1997?		Talin
 *		Plugin interface and file handling
 *	07/11/2000	malouin
 *		Standard MIDI File parsing and writing
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */
#ifndef __C_StandardMidiFile_H__
#define __C_StandardMidiFile_H__
 
#include "MeVPlugin.h"

#include <queue>

class CIFFWriter;

class CStandardMidiFile :
	public MeVPlugIn
{
public:
	CStandardMidiFile();
	~CStandardMidiFile();

	virtual char *AboutText();

	virtual void OnImport( BMessage *inMsg, entry_ref *ref, int32 inDetectedType );
	virtual void OnExport( BMessage *inMsg, int32 inDocID, entry_ref *ref );
	virtual int32 DetectFileType(	const entry_ref	*ref,
						BNode			*node,
						struct stat		*st,
						const char		*filetype );

	void ShowError( const char *msg, ... );

private:
	struct smf_time_base
	{
		enum { METERED = 0, TIME_CODE = 0x8000 };
		enum { SMPTE_30 = -30, SMPTE_30_DROP = -29, SMPTE_25 = -25, SMPTE_24 = -24 };
		
		int format;
		union
		{
			struct
			{
				int16 ticksPerQuarterNote;
			} metered;
			
			struct
			{
				int16 smpteFormat;
				int16 ticksPerFrame;
			} timeCode;
		} base;
	};

	// reads events from file track and converts to MeV events
	class SMFTrackReader
	{
	public:
		SMFTrackReader(CStandardMidiFile& smfPlugIn, const uint8* data, int32 length,
		               int* destinationIDs, const smf_time_base& tBase);
		
		// returns the next event in the track.  Event
		// vChannel is the MIDI channel number (0-15).
		bool	GetNextEvent(Event& outEvent);

	private:
				uint8	GetByte();
				uint8	PeekByte() const;
				void	GetBytes(uint8* buffer, int32 numBytes);
				void	SkipBytes(int32 numBytes);
				uint32	GetVariableLengthNumber();
				
				int64	GetTime();
				uint8	GetStatusByte();
		
				bool	ReadNoteOn(			uint8 statusByte, Event& outEvent);
				bool	ReadNoteOff(		uint8 statusByte, Event& outEvent);
				bool	ReadPolyPressure(	uint8 statusByte, Event& outEvent);
				bool	ReadChannelPressure(uint8 statusByte, Event& outEvent);
				bool	ReadControlChange(	uint8 statusByte, Event& outEvent);
				bool	ReadProgramChange(	uint8 statusByte, Event& outEvent);
				bool	ReadPitchBend(		uint8 statusByte, Event& outEvent);
				bool	ReadSystemExclusive(uint8 statusByte, Event& outEvent);
				bool	ReadMetaEvent(		uint8 statusByte, Event& outEvent);
		
		inline	uint32	Position() const;	// offset within track data
		
		CStandardMidiFile&	m_plugin;
		int*				m_destinationID;
		const uint8*		m_pData;
		const uint8*		m_pEnd;
		int32				m_trackLength;
		smf_time_base		m_timeBase;
		int64				m_fileTime_ticks;
		uint8				m_runningStatus;
	};
	
	// writes MeV events to file track
	class SMFTrackWriter
	{
	public:
		SMFTrackWriter(CIFFWriter& writer, MeVDocHandle doc);
		
		~SMFTrackWriter();
		
				void	WriteEvent(Event& event);
				void	WriteTempo(double tempo_bpm, uint32 deltaTime = 0);
				void	WriteTrackName(const char* text, uint32 deltaTime = 0);

				void	WriteEndOfTrack(uint32 time);

	private:
		class CmpEventTime
		{
		public:
			bool operator()(const Event& ev1, const Event& ev2)
				{ return ev1.Start() > ev2.Start(); }
		};

				void	WriteNoteOn(			const Event& event, uint32 deltaTime);
				void	WriteNoteOff(			const Event& event, uint32 deltaTime);
				void	WritePolyPressure(		const Event& event, uint32 deltaTime);
				void	WriteChannelPressure(	const Event& event, uint32 deltaTime);
				void	WriteControlChange(		const Event& event, uint32 deltaTime);
				void	WriteProgramChange(		const Event& event, uint32 deltaTime);
				void	WritePitchBend(			const Event& event, uint32 deltaTime);
				void	WriteSystemExclusive(	const Event& event, uint32 deltaTime);
				void	WriteTextMetaEvent(		const Event& event, uint32 deltaTime);
				void	WriteTempoMetaEvent(	const Event& event, uint32 deltaTime);
				void	WriteTimeSigMetaEvent(	const Event& event, uint32 deltaTime);
				
				void	WritePendingNoteOffs(	int32 atOrBeforeTime = 0x7FFFFFFF);
				
				void	WriteVariableLengthNumber(uint32 value);
				
				/**	Converts event start time to delta time from last event. */
				uint32	CalculateDeltaTime(int32 startTime);
				
				/**	Returns the MIDI channel of an event. */
				uint8	GetMidiChannel(const Event& event);

		priority_queue<Event,vector<Event>,CmpEventTime>	m_pendingNoteOffs;
		CIFFWriter&											m_writer;
		MeVDocHandle										m_doc;
		int32												m_lastEventTicks;
	};
	
	// functions for import	
	void CreateDestinations(MeVDocHandle doc, const char* filename);
	bool ReadTrack( MeVDocHandle doc, uint8 *trackBuffer, long length,
	                const smf_time_base& timeBase );
	void SetInitialTempo(MeVDocHandle doc);

	// functions for export
	status_t CountTracks(MeVDocHandle doc, uint16& out_numTracks, TClockType& out_clockType);
	status_t WriteHeaderChunk(CIFFWriter& writer, uint16 numTracks, TClockType clockType);
	status_t WriteTrack(CIFFWriter& writer, MeVDocHandle doc, MeVTrackHandle track);
	
	int					m_destinationID[16];

};

inline uint32 CStandardMidiFile::SMFTrackReader::Position() const
{
	return m_pData - (m_pEnd - m_trackLength);
}

#endif /* __C_StandardMidiFile_H__ */
 
