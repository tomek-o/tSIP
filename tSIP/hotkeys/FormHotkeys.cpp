//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "hotkeys/FormHotkeys.h"
#include "hotkeys/HotKeyConf.h"
#include "hotkeys/HotkeyCfgPanel.h"
#include "ProgrammableButtons.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
//TfrmHotkeys *frmHotkeys;
//---------------------------------------------------------------------------
__fastcall TfrmHotkeys::TfrmHotkeys(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------

void TfrmHotkeys::SetCfg(std::list<HotKeyConf> *hotkeyCfg)
{
	cfg = hotkeyCfg;
	std::list<THotkeyCfgPanel*>::iterator iter;
	for (iter = hkCfgPanels.begin(); iter != hkCfgPanels.end(); ++iter)
	{
		delete *iter;
	}
	hkCfgPanels.clear();

	for (std::list<HotKeyConf>::iterator iter2 = cfg->begin(); iter2 != cfg->end(); ++iter2)
	{
		THotkeyCfgPanel *cfgPanel = new THotkeyCfgPanel(panel, *iter2, ProgrammableButtons::GetTotalCnt());
		cfgPanel->btnRemove->OnClick = btnRemoveClick;
		cfgPanel->Parent = panel;
		//cfgPanel->Start();
		cfgPanel->Visible = true;

		hkCfgPanels.push_back(cfgPanel);
	}
	
	for (iter = hkCfgPanels.begin(); iter != hkCfgPanels.end(); ++iter)
	{
		(*iter)->Start();
	}
}

void __fastcall TfrmHotkeys::btnAddHotkeyClick(TObject *Sender)
{
	HotKeyConf cfgNew;
	cfg->push_back(cfgNew);

	THotkeyCfgPanel *cfgPanel = new THotkeyCfgPanel(panel, *(--cfg->end()), ProgrammableButtons::GetTotalCnt());
	cfgPanel->btnRemove->OnClick = btnRemoveClick;
	cfgPanel->Parent = panel;
	cfgPanel->Visible = true;

	hkCfgPanels.push_back(cfgPanel);
	cfgPanel->Start();
}
//---------------------------------------------------------------------------

void __fastcall TfrmHotkeys::btnRemoveClick(TObject *Sender)
{
	TButton *btn = dynamic_cast<TButton*>(Sender);
	THotkeyCfgPanel *cfgPanel = dynamic_cast<THotkeyCfgPanel*>(btn->Parent);
	std::list<THotkeyCfgPanel*>::iterator iter;
	int id = 0;
	for (iter = hkCfgPanels.begin(); iter != hkCfgPanels.end(); ++iter)
	{
		if (*iter == cfgPanel)
		{
			hkCfgPanels.erase(iter);
			std::list<HotKeyConf>::iterator iter2;
			int id2 = 0;
			for (iter2 = cfg->begin(); iter2 != cfg->end(); ++iter2)
			{
				if (id2 == id)
				{
					cfg->erase(iter2);
					break;
				}
				id2++;
			}
			removedPanel = cfgPanel;
			timerRemove->Enabled = true;
			break;
		}
		id++;
	}
}
//---------------------------------------------------------------------------

void __fastcall TfrmHotkeys::timerRemoveTimer(TObject *Sender)
{
	timerRemove->Enabled = false;
	delete removedPanel;
}
//---------------------------------------------------------------------------

