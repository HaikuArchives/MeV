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
#include "VCTableManager.h"
#include <MidiProducer.h>
class CVChannelModifier :
	public BWindow {

private:
	VChannelEntry *m_vc;   //pointer to the currently selected vc.
	int32 m_id;
	BHandler *m_parent;
	//int m_selected_id;
	BView *m_background;
	BPopUpMenu *m_midiPorts;
	BPopUpMenu *m_channels;
	BTextControl *m_name;
	BButton *m_done;
	BButton *m_cancel;
	BColorControl *m_colors;
	CMidiManager *m_midiManager;	
	void _buildUI();
	CVCTableManager *m_tm;
virtual void AttachedToWindow();
virtual void MenusBeginning();
public:
	CVChannelModifier(BRect frame,int32 id,CVCTableManager *tm,BHandler *parent);  //new vchannel;
	
	virtual void MessageReceived(BMessage *msg);
	void Update();
	virtual bool QuitRequested();

};
#endif
