/* ===================================================================== *
 * VCTableManager.h (MeV/engine)
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
 * A class that wraps the vchannel array.  
 * ---------------------------------------------------------------------
 * History:
 *	6/21/2000		dwalton
 *		Original implementation.
 *		
 * ---------------------------------------------------------------------
 * To Do:
 * 
 * ===================================================================== */
#ifndef __C_VChannelManager_H__
#define __C_VChannelManager_H__
#include "MeV.h"
#include "VChannel.h"
#include "Observer.h"
#include <String.h>
//enum ID {
//	VCTM_NOTIFY='ntfy'
//	};
class CMeVDoc;
class CVCTableManager :
	public CObservableSubject {
public:
	CVCTableManager(CMeVDoc *inDoc);
	void _notifyClients();
	void NotifyClients();
	void AddClient(BHandler *nhandler);
	int NewVC();
	void RemoveVC(int id);
	VChannelEntry * operator[](int i);
	VChannelEntry * get(int i);
	bool IsDefined(int id);
	void First();
	bool IsDone();
	void Next();
	//VChannelEntry * GetVC(int id);
	VChannelEntry * CurrentVC();
	int CurrentID();
	
	void SetColorFor(int id, rgb_color color);
	void SetNameFor(int id,BString name);
	void SetChannelFor(int id,int channel);
	void SetPortFor (int id,BMidiLocalProducer *producer);
	void SetMuteFor (int id, bool mute);
	int count;
private:
	
	BMessenger *m_notifier;
	static const rgb_color m_defaultColorTable[ 16 ] ;
	int pos;
	VChannelEntry * m_tablerep[Max_VChannels];
	CMeVDoc *m_doc;
};
#endif /* __C_VChannelManager_H__ */