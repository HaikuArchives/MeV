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
class CVChannelModifier:
	public BWindow {

private:
	VChannelEntry *m_vc;   //pointer to the currently selected vc.
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

public:
	CVChannelModifier(BRect frame,VChannelEntry *vc,CVCTableManager *tm);  //new vchannel;
	//CVChannelModifier(BRect frame,uint8 editChannel,CVCTableManager *tm); //edit channel;
	virtual void MessageReceived(BMessage *msg);
	void Update();
	virtual bool QuitRequested();
	virtual void Quit();
	void Die();
};
