/* ===================================================================== *
 * DestinationList.h (MeV/Engine)
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
 * A class that manages the destinations.  
 * ---------------------------------------------------------------------
 * History:
 *	6/21/2000		dwalton
 *		Original implementation.
 *	8/01/2000		dwalton
 *      Name change, many improvements.
 * ---------------------------------------------------------------------
 * To Do:
 * 
 * ===================================================================== */
#ifndef __C_VChannelManager_H__
#define __C_VChannelManager_H__
#include "MeV.h"
#include "Destination.h"
#include "Observable.h"
#include "Observer.h"
#include "IFFWriter.h"
#include "IFFReader.h"
#include "MidiManager.h"
#include <String.h>

class CMeVDoc;

class CDestinationList :
	public CObservableSubject,
	public CObserver
{
public:
	CDestinationList(CMeVDoc *inDoc);
	~CDestinationList();
	void _notifyClients();
	void NotifyClients();
	void AddClient(BHandler *nhandler);
	int NewDest();
	void RemoveVC(int id);
	Destination * operator[](int i);
	Destination * get(int i);
	bool IsDefined(int id);
	void First();
	bool IsDone();
	void Next();
	//Destination * GetVC(int id);
	Destination * CurrentDest();
	int CurrentID();
	BBitmap * GetIconFor (int id,BRect r) const;
	void ToggleConnectFor (int id, BMidiConsumer *sink);
	void SetColorFor(int id, rgb_color color, bool postUpdate = true);
	void SetNameFor(int id,BString name);
	void SetChannelFor(int id,int channel);
	void SetPortFor (int id,BMidiLocalProducer *producer);
	void SetMuteFor (int id, bool mute);
	void SetSoloFor (int id, bool solo);
	void SetDisableFor (int id, bool disable);
	void SetDeletedFor (int id, bool deleted);
	void ReadVCTable (CIFFReader &reader);
	void WriteVCTable (CIFFWriter &writer);
	void SetSelectedId(int id);
	int SelectedId();
private:
	int m_selected_id;
	int32 count;
	virtual void OnUpdate(BMessage *msg);
	CMidiManager *m_midimanager;
	BMessenger *m_notifier;
	static const rgb_color m_defaultColorTable[ 16 ] ;
	int32 pos;
	Destination * m_tablerep[Max_Destinations];
	CMeVDoc *m_doc;
};

#endif /* __C_DestinationList_H__ */
