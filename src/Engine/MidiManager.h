/* ===================================================================== *
 * MidiManager.h (MeV/engine)
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
 *  Builds a list of MidiProducers one for each consumer in the system.
 *  And otherwise manages midi ports.  We are following the singleton
 *  design pattern.
 *
 * ---------------------------------------------------------------------
 * History:
 *	6/21/2000		dwalton
 *		Original implementation, not yet usuable
 *		added to the repository...
 * ---------------------------------------------------------------------
 * To Do:
 * I imagine that this could be used for all the midi port management including
 * patch names etc.
 * ===================================================================== */

#ifndef __C_MidiManager_H__
#define __C_MidiManager_H__

//this class should take care of building new consumers and connecting...etc.

#include <MidiLocalProducer.h>
#include <MidiRoster.h>
#include <MidiLocalConsumer.h>
#include <MidiConsumer.h>
#include <List.h>
#include <String.h>
#include <Handler.h>
class CMidiManager : public BHandler{
	public:
		static CMidiManager* Instance();
		BMidiLocalProducer * GetProducer(BString *name);
		BMidiLocalProducer * GetProducer (int32 id);
		
		//GetConsumer methods need to be added.
		//
		//a little cursor kinda deal
		//void FirstProducer();
		//void NextProducer();
		//bool IsLastProducer();
		//BString *name CurrentProducerName();
		//int32 CurrentProducerID();
		 
		virtual void MessageReceived(BMessage *msg);
		
	
	protected:
		CMidiManager();
	private:
		static CMidiManager *m_instance;
		BList m_midiProducers;
		BMidiRoster *m_roster;
		void _addProducer(int32 id);
		void _addConsumer(int32 id);
		void _removeProducer(int32 id);
		void _removeConsumer(int32 id);
		void _handleMidiEvent(BMessage *msg);
	};
#endif