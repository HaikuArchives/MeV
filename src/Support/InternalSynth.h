/* ===================================================================== *
 * LinearEditor.h (MeV/User Interface)
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
 *		Dan Walton
 *
 * ---------------------------------------------------------------------
 * Purpose:
 *  Midi2 wrapper for the midisynth.
 * ---------------------------------------------------------------------
 * History:
 *	5/19/2000 dwalton
 *		Original implementation
 * ---------------------------------------------------------------------
 * To Do:
 *
 * ===================================================================== */

#ifndef __C_InternalSynth_H__
#define __C_InternalSynth_H__
#include <MidiConsumer.h>
#include <MidiSynth.h>
class CInternalSynth : public BMidiLocalConsumer
{
public:
    CInternalSynth(char *name);
    virtual	void	NoteOff(uchar channel,
                         uchar note,
                         uchar velocity,
                         bigtime_t time);

    virtual	void	NoteOn(uchar channel,
                        uchar note,
                        uchar velocity,
                        bigtime_t time  );

    virtual	void	KeyPressure(uchar channel,
                             uchar note,
                             uchar pressure,
                             bigtime_t time  );

    virtual	void	ControlChange(uchar channel,
                               uchar controlNumber,
                               uchar controlValue,
                               bigtime_t time  );

    virtual	void	ProgramChange(uchar channel,
                               uchar programNumber,
                               bigtime_t time  );

    virtual	void	ChannelPressure(uchar channel,
                                 uchar pressure,
                                 bigtime_t time );

    virtual	void	PitchBend(uchar channel,
                           uchar lsb,
                           uchar msb,
                           bigtime_t time  );

    virtual	void	SystemExclusive(void* data,
                                 size_t dataLength,
                                 bigtime_t time  );

    virtual	void	SystemCommon(uchar statusByte,
                              uchar data1,
                              uchar data2,
                              bigtime_t time  );

    virtual	void	SystemRealTime(uchar statusByte, bigtime_t time  );

    virtual	void	TempoChange(int32 bpm, bigtime_t time );

    //virtual void	AllNotesOff(bool justChannel = true, int32 time );

    ~CInternalSynth();
private:
    //i think think we are going to need an icon
    /*void GetIcons(BBitmap* largeIcon, BBitmap* miniIcon);
    void AddIcons(BMessage* msg, BBitmap* largeIcon, BBitmap* miniIcon) const;*/
    BMidiSynth *m_midiSynth;
};
#endif /*__C_InternalSynth_H__ */

