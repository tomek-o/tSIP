//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "FormPhones.h"
#include "PhoneConf.h"
#include "PhoneInterface.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmPhones *frmPhones;
//---------------------------------------------------------------------------
__fastcall TfrmPhones::TfrmPhones(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TfrmPhones::lvDllsData(TObject *Sender, TListItem *Item)
{
	int id = Item->Index;
#if 0
	std::list<PhoneConf>::iterator iter = cfg->begin();
	std::advance(iter, id);

	Item->Caption = iter->dllName;
#else
	DllInfo &info = PhoneInterface::dlls[id];
	PhoneConf *pconf = FindCfg(info.name);
	if (pconf)
	{
    	Item->Caption = "yes";
	}
	else
	{
    	Item->Caption = "no";
	}
	Item->SubItems->Add(info.name);
	Item->SubItems->Add(info.file_version);
	Item->SubItems->Add(info.description);
#endif
}
//---------------------------------------------------------------------------

void TfrmPhones::SetCfg(std::list<PhoneConf> *phoneCfg)
{
	cfg = phoneCfg;
	
	lvDlls->Items->Count = PhoneInterface::dlls.size();
	lvDlls->Invalidate();
}

PhoneConf* TfrmPhones::FindCfg(AnsiString dllName)
{
	std::list<PhoneConf>::iterator iter;
	for (iter = cfg->begin(); iter != cfg->end(); ++iter)
	{
		if (iter->dllName == dllName)
		{
			return &*iter;
		}
	}
	return NULL;
}

void TfrmPhones::ToggleActivation(void)
{
	if (!lvDlls->Selected)
		return;
	int id = lvDlls->Selected->Index;
	DllInfo &info = PhoneInterface::dlls[id];
	AnsiString dllName = info.name;
	std::list<PhoneConf>::iterator iter;
	for (iter = cfg->begin(); iter != cfg->end(); ++iter)
	{
		if (iter->dllName == dllName)
		{
			cfg->erase(iter);
			lvDlls->Invalidate();
			return;
		}
	}
	PhoneConf newConf;
	newConf.dllName = dllName;
	cfg->push_back(newConf);
	lvDlls->Invalidate();
}

void __fastcall TfrmPhones::btnToggleActivationClick(TObject *Sender)
{
	ToggleActivation();
}
//---------------------------------------------------------------------------

void __fastcall TfrmPhones::btnRefreshListClick(TObject *Sender)
{
	PhoneInterface::ReEnumerateDlls();
	lvDlls->Items->Count = PhoneInterface::dlls.size();
	lvDlls->Invalidate();
}
//---------------------------------------------------------------------------

void __fastcall TfrmPhones::lvDllsDblClick(TObject *Sender)
{
	ToggleActivation();	
}
//---------------------------------------------------------------------------

