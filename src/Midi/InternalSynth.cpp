/* ===================================================================== *
 * InternalSynth.cpp (MeV/Midi)
 * ===================================================================== */

#include "InternalSynth.h"
#include <MidiSynth.h>
#include <Midi.h>
#include <stdio.h>
#include <iostream.h>

CInternalSynth::CInternalSynth(char *name) : BMidiLocalConsumer(name)
{
	m_midiSynth=new BMidiSynth;
   	m_midiSynth->EnableInput(true, true);
}
void CInternalSynth::NoteOff(uchar channel,
                          uchar note,
                          uchar velocity,
                          bigtime_t time)
{
    m_midiSynth->NoteOff(channel+1,note,velocity,time/1000);
	
}

void CInternalSynth::NoteOn(uchar channel,
                         uchar note,
                         uchar velocity,
                         bigtime_t time)
{
        m_midiSynth->NoteOn(channel+1,note,velocity,time/1000);
}
void CInternalSynth::KeyPressure(uchar channel,
                              uchar note,
                              uchar pressure,
                              bigtime_t time )
{
        m_midiSynth->KeyPressure(channel+1,note,pressure,time/1000);
}
void CInternalSynth::ControlChange(uchar channel,
                                uchar controlNumber,
                                uchar controlValue,
                                bigtime_t time )
{
         m_midiSynth->ControlChange(channel+1,controlNumber,controlValue,time/1000);
}
void CInternalSynth::ProgramChange(uchar channel,
                                uchar programNumber,
                                bigtime_t time)
{
		m_midiSynth->ProgramChange(channel+1,programNumber,time/1000);
}
void CInternalSynth::ChannelPressure(uchar channel,
                                  uchar pressure,
                                  bigtime_t time)
{
    	m_midiSynth->ChannelPressure(channel+1,pressure,time/1000);
}
void CInternalSynth::PitchBend(uchar channel,
                            uchar lsb,
                            uchar msb,
                            bigtime_t time)
{
    	m_midiSynth->PitchBend(channel+1,lsb,msb,time/1000);
}
void CInternalSynth::SystemExclusive(void* data,
                                  size_t dataLength,
                                  bigtime_t time)
{
}
void CInternalSynth::SystemCommon(uchar statusByte,
                               uchar data1,
                               uchar data2,
                               bigtime_t time)
{
}
void CInternalSynth::SystemRealTime(uchar statusByte, bigtime_t time )
{
 }
void CInternalSynth::TempoChange(int32 bpm, bigtime_t time = B_NOW)
{
}
/* void	CInternalSynth::AllNotesOff(bool justChannel = true, bigtime_t time )
		{
		cout << "all notes off" << endl;
		}*/

CInternalSynth::~CInternalSynth()
{
    delete m_midiSynth;
}


