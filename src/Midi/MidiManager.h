/* ===================================================================== *
 * MidiManager.h (MeV/Midi)
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
 *		Dan Walton (dwalton)
 *
 * ---------------------------------------------------------------------
 * Purpose:
 *  
 *  
 *  
 *  manages midi ports
 *
 * ---------------------------------------------------------------------
 * History:
 *	6/21/2000		dwalton
 *		Original implementation
 *		added to the repository...
 *	8/13/2000		dwalton
 *		modified to support one producer per CDestination.
 * ---------------------------------------------------------------------
 * To Do:
 * I imagine that this could be used for all the midi port management including
 * patch names etc.

 * ===================================================================== 
 some of this code has been based on Be sample code.
 
  Copyright 1999, Be Incorporated.   All Rights Reserved.
  This file may be used under the terms of the Be Sample Code License.
 
 */



#ifndef __C_MidiManager_H__
#define __C_MidiManager_H__

//this class should take care of building new consumers and connecting...etc.

#include <MidiProducer.h>
#include <MidiRoster.h>
#include <MidiConsumer.h>
#include <List.h>
#include <String.h>
#include <Looper.h>
#include <Bitmap.h>
#include <Mime.h>
#include <GraphicsDefs.h>
#include "GeneralMidi.h"
#include "PortNameMap.h"
#include "InternalSynth.h"
#include "Observable.h"

class CMidiManager : public BLooper,public CObservableSubject{
	public:
		static CMidiManager * Instance();
		BMidiProducer * NextProducer(int32 *id);
		BMidiProducer * FindProducer(int32 id);
		BMidiConsumer * NextConsumer(int32 *id);
		BMidiConsumer * FindConsumer (int32 id);
		
		BMidiEndpoint * FindEndpoint (const char* name);
		BMidiConsumer * FindConsumer (const char* name);
		BMidiProducer * FindProducer (const char* name);
		
		BBitmap * ConsumerIcon(int32 id,icon_size which);
		void AddIcons(BMessage* msg, BBitmap* largeIcon, BBitmap* miniIcon) const;

		virtual void MessageReceived(BMessage *msg);
		void AddInternalSynth();
		CInternalSynth * InternalSynth();
		void Die();
		
	protected:
		CMidiManager();
	private:
		//~CMidiManager();
		CInternalSynth * m_internalSynth;
		static CMidiManager *m_instance;
		int32 m_pos;
		BMidiRoster *m_roster;
		void _notifySubscribers();
		void _addProducer(int32 id);
		void _addConsumer(int32 id);
		void _removeProducer(int32 id);
		void _removeConsumer(int32 id);
		void _connect(int32 prod,int32 con);
		void _disconnect(int32 prod,int32 con);
		void _handleMidiEvent(BMessage *msg);
		//port name map:
		//we have the ability to let the user change port name...no problem
		//but there are names they shouldn't see...like /dev/midi/mo/dev
		//or /dev/midi/awe64/1 so this lets us give better names.
		CPortNameMap *m_portNameMap;
		void _copyIcon(const BMessage* smsg,BMessage* dmsg);
		BBitmap* _createIcon(const BMessage* msg, icon_size which);
		//void _addIcons(BMessage* msg, BBitmap* largeIcon, BBitmap* miniIcon) const;
};
#endif