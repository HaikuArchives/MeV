#ifndef __C_VChannelModifier_H__
#define __C_VChannelModifier_H__
#include <Window.h>
#include <View.h>
#include <MenuItem.h>
#include <Button.h>
#include <TextControl.h>
#include <ColorControl.h> 
#include <Message.h>
#include <PopUpMenu.h>
#include "MidiManager.h"
#include "Observer.h"
#include "DestinationList.h"
#include <MidiProducer.h>
class CDestinationModifier :
	public BWindow,public CObserver {
private:
	Destination *m_vc;   //pointer to the currently selected dest.
	int32 m_id;
	BHandler *m_parent;
	//int m_selected_id;
	BView *m_background;
	BPopUpMenu *m_midiPorts;
	BPopUpMenu *m_channels;
	BTextControl *m_name;
	BCheckBox *m_mute;
	BButton *m_done;
	BButton *m_cancel;
	BColorControl *m_colors;
	CMidiManager *m_midiManager;	
	void _buildUI();
	CDestinationList *m_tm;
	virtual void OnUpdate(BMessage *msg);
virtual void AttachedToWindow();
virtual void MenusBeginning();
void Update();
public:
	CDestinationModifier(BRect frame,int32 id,CDestinationList *tm,BHandler *parent);  //new vchannel;
	virtual void MessageReceived(BMessage *msg);
	virtual bool QuitRequested();
};
#endif
