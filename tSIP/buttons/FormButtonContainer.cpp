//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "FormButtonContainer.h"
#include "ProgrammableButton.h"
#include "FormButtonEdit.h"
#include "ProgrammableButtons.h"
#include "Settings.h"
#include "UaMain.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmButtonContainer *frmButtonContainer;

//---------------------------------------------------------------------------
__fastcall TfrmButtonContainer::TfrmButtonContainer(TComponent* Owner,
	ProgrammableButtons &buttons,
	int width, int height, int scalingPercentage,
    int startBtnId, int btnCnt,
	CallbackClick callbackClick)
	: TForm(Owner), buttons(buttons), startBtnId(startBtnId), btnCnt(btnCnt), callbackClick(callbackClick)
{
	assert(startBtnId >= 0);
	assert(btnCnt >= 0);
	assert(callbackClick);
	if (width > 0)
	{
		this->Width = width;
	}
	if (height > 0)
	{
    	this->Height = height;
	}
	useContextMenu = appSettings.frmMain.bSpeedDialPopupMenu;
	for (int i=0; i<btnCnt; i++)
	{
		TProgrammableButton *panel = new TProgrammableButton(flowPanel, imgList, scalingPercentage);
		panel->Tag = i;
		panel->Parent = flowPanel;
		panel->Width = flowPanel->Width-2;
		//panel->AlignWithMargins = true;
		if (i < buttons.btnConf.size())
		{
			ButtonConf &cfg = buttons.btnConf[startBtnId + i];
			panel->SetConfig(cfg);
		}
		panel->OnClick = SpeedDialPanelClick;
        panel->PopupMenu = useContextMenu ? popupPanel : NULL;
		panel->UpdateCallbacks();
		panel->Visible = True;
		vpanels.push_back(panel);
	}
}
//---------------------------------------------------------------------------

void TfrmButtonContainer::SetScaling(int percentage)
{
	for (unsigned int i=0; i<vpanels.size(); i++)
	{
		TProgrammableButton *panel = vpanels[i];
		panel->SetScaling(percentage);
	}
}


void __fastcall TfrmButtonContainer::SpeedDialPanelClick(TObject *Sender)
{
	TProgrammableButton* panel = dynamic_cast<TProgrammableButton*>(Sender);
	if (panel == NULL)
	{
		TComponent *component = dynamic_cast<TComponent*>(Sender);
		assert(component);
		panel = dynamic_cast<TProgrammableButton*>(component->Owner);
	}
	assert(panel);
	int id = startBtnId + panel->Tag;

	ButtonConf &cfg = buttons.btnConf[id];
	if (cfg.type == Button::HOLD || cfg.type == Button::MUTE || cfg.type == Button::MUTE_RING)
	{
    	panel->SetDown(!panel->GetDown());
	}

	callbackClick(id, panel);
}
//---------------------------------------------------------------------------

void __fastcall TfrmButtonContainer::miEditSpeedDialClick(TObject *Sender)
{
	TProgrammableButton* panel = dynamic_cast<TProgrammableButton*>(popupPanel->PopupComponent);
	assert(panel);
	int id = panel->Tag;

	EditContact(id);
}
//---------------------------------------------------------------------------

void TfrmButtonContainer::EditContact(int id)
{
	ButtonConf cfg = buttons.btnConf[startBtnId + id];	// copy
	AnsiString caption;
	caption.sprintf("Edit button #%02d", startBtnId + id);
	frmButtonEdit->Caption = caption;
	frmButtonEdit->ShowModal(&cfg);

	if (frmButtonEdit->isConfirmed())
	{
		if (cfg != buttons.btnConf[startBtnId + id])
		{
			bool restartUa = false;
			if (cfg.UaRestartNeeded(buttons.btnConf[startBtnId + id]))
			{
            	restartUa = true;
			}
			
			buttons.btnConf[startBtnId + id] = cfg;
			TProgrammableButton* panel = vpanels[id];
			panel->SetConfig(cfg);
			if (cfg.type != Button::BLF)
			{
				panel->SetState(DIALOG_INFO_UNKNOWN, true, DIALOG_INFO_DIR_UNKNOWN, "", "");	// make sure BLF icon is cleared
			}
			if (cfg.type != Button::PRESENCE)
			{
				panel->ClearPresenceState();			// clear icon
			}
			buttons.Write();

			if (restartUa)
			{
                buttons.UpdateContacts(appSettings.uaConf.contacts);
				Ua::Instance().Restart();
			}
		}
		else
		{
			// assign anyway - some fields may be unintentionally omited from != operator
			buttons.btnConf[startBtnId + id] = cfg;
			buttons.Write();			
		}
	}
}

void TfrmButtonContainer::UpdateDlgInfoState(int id, int state, bool updateRemoteIdentity, int direction, AnsiString remoteIdentity, AnsiString remoteIdentityDisplay)
{
	id -= startBtnId;
	if (id < 0 || id >= vpanels.size())
		return;	
	vpanels[id]->SetState((enum dialog_info_status)state, updateRemoteIdentity, (enum dialog_info_direction)direction, remoteIdentity, remoteIdentityDisplay);
}

void TfrmButtonContainer::UpdatePresenceState(int id, int state, AnsiString note)
{
	id -= startBtnId;
	if (id < 0 || id >= vpanels.size())
		return;	
	vpanels[id]->SetPresenceState((enum presence_status)state, note);
}

void TfrmButtonContainer::UpdateBtnState(Button::Type type, bool state)
{
	for (unsigned int i=0; i<vpanels.size(); i++)
	{
		ButtonConf &cfg = buttons.btnConf[i + startBtnId];
		if (cfg.type == type)
		{
			vpanels[i]->SetDown(state);
		}
	}
}

void TfrmButtonContainer::UpdateBtnCaption(int id, AnsiString text)
{
	id -= startBtnId;
	if (id < 0 || id >= vpanels.size())
		return;
	vpanels[id]->SetCaption(text);
}

void TfrmButtonContainer::UpdateBtnCaption2(int id, AnsiString text)
{
	id -= startBtnId;
	if (id < 0 || id >= vpanels.size())
		return;
	vpanels[id]->SetCaption2(text);
}

void TfrmButtonContainer::UpdateBtnDown(int id, bool state)
{
	id -= startBtnId;
	if (id < 0 || id >= vpanels.size())
		return;
	vpanels[id]->SetDown(state);
}

void TfrmButtonContainer::UpdateBtnImage(int id, AnsiString file)
{
	id -= startBtnId;
	if (id < 0 || id >= vpanels.size())
		return;
	vpanels[id]->SetImage(file);
}

void TfrmButtonContainer::UpdateMwiState(int newMsg, int oldMsg)
{
	for (unsigned int i=0; i<vpanels.size(); i++)
	{
		ButtonConf &cfg = buttons.btnConf[i + startBtnId];
		if (cfg.type == Button::MWI)
		{
			vpanels[i]->SetMwiState(newMsg, oldMsg);
		}
	}
}

void TfrmButtonContainer::UpdateSettings(void)
{
	if (useContextMenu == appSettings.frmMain.bSpeedDialPopupMenu)
	{
        return;
	}
	useContextMenu = appSettings.frmMain.bSpeedDialPopupMenu;
	for (unsigned int i=0; i<vpanels.size(); i++)
	{
		vpanels[i]->PopupMenu = useContextMenu ? popupPanel : NULL;
	}
}

