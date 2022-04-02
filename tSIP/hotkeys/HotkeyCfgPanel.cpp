//---------------------------------------------------------------------------

#include <vcl.h>

#pragma hdrstop

#include "HotkeyCfgPanel.h"
#include "HotKeyConf.h"
#include "common\KeybKeys.h"
#include "Settings.h"
#include "Paths.h"
#pragma package(smart_init)
//---------------------------------------------------------------------------
// ValidCtrCheck is used to assure that the components created do not have
// any pure virtual functions.
//
static inline void ValidCtrCheck(THotkeyCfgPanel *)
{
	static HotKeyConf cfg;
	new THotkeyCfgPanel(NULL, cfg, 100);
}
//---------------------------------------------------------------------------
__fastcall THotkeyCfgPanel::THotkeyCfgPanel(TComponent* Owner, HotKeyConf &cfg, int programmableButtonsCnt)
	: TPanel(Owner),
	cfg(cfg),
	programmableButtonsCnt(programmableButtonsCnt)
{
	Width = 570;
	Height = 32;

	int left = 16;

	chbGlobal = new TCheckBox(this);
	chbGlobal->Parent = this;
	chbGlobal->Left = left;
	chbGlobal->Top = 8;
	//chbGlobal->Caption = "Global";
	chbGlobal->Visible = true;

	left += 32;
	chbCtrl = new TCheckBox(this);
	chbCtrl->Parent = this;
	chbCtrl->Left = left;
	chbCtrl->Top = 8;
	//chbCtrl->Caption = "Ctrl";
	chbCtrl->Visible = true;

	left += 24;
	chbShift = new TCheckBox(this);
	chbShift->Parent = this;
	chbShift->Left = left;
	chbShift->Top = 8;
	//chbShift->Caption = "Shift";
	chbShift->Visible = true;

	left += 24;
	chbAlt = new TCheckBox(this);
	chbAlt->Parent = this;
	chbAlt->Left = left;
	chbAlt->Top = 8;
	//chbAlt->Caption = "Alt";
	chbAlt->Visible = true;

	left += 24;
	cbKey = new TComboBox(this);
	cbKey->Parent = this;
	cbKey->Left = left;
	cbKey->Top = 5;
	cbKey->Width = 160;
	cbKey->Style = Stdctrls::csDropDownList;
	cbKey->Visible = true;
	cbKey->DropDownCount = 12;

	left += cbKey->Width + 16;
	cbAction = new TComboBox(this);
	cbAction->Parent = this;
	cbAction->Left = left;
	cbAction->Top = 5;
	cbAction->Width = 100;
	cbAction->Style = Stdctrls::csDropDownList;
	cbAction->Visible = true;

	left += cbAction->Width + 25;
	cbId = new TComboBox(this);
	cbId->Parent = this;
	cbId->Left = left;
	cbId->Top = 5;
	cbId->Width = 40;
	cbId->Style = Stdctrls::csDropDownList;
	cbId->Visible = false;

	{
		edScriptFile = new TEdit(this);
		edScriptFile->Parent = this;
		edScriptFile->Left = left - 23;
		edScriptFile->Top = 5;
		edScriptFile->Width = 90;
		edScriptFile->Visible = false;

		btnScriptFileSelect = new TButton(this);
		btnScriptFileSelect->Parent = this;
		btnScriptFileSelect->Left = edScriptFile->Left + edScriptFile->Width + 4;
		btnScriptFileSelect->Top = 5;
		btnScriptFileSelect->Width = 24;
		btnScriptFileSelect->Height = 20;
		btnScriptFileSelect->Visible = false;
		btnScriptFileSelect->Caption = "...";
	}

	left += cbId->Width + 62;
	btnRemove = new TButton(this);
	btnRemove->Parent = this;
	btnRemove->Left = left;
	btnRemove->Top = 5;
	btnRemove->Width = 24;
	btnRemove->Height = 20;
	btnRemove->Caption = "X";
	btnRemove->Visible = true;
}

void __fastcall THotkeyCfgPanel::UpdateCfg(TObject *Sender)
{

	cfg.modifiers = 0;
	if (chbCtrl->Checked)
		cfg.modifiers |= HotKeyConf::CTRL;
	if (chbShift->Checked)
		cfg.modifiers |= HotKeyConf::SHIFT;
	if (chbAlt->Checked)
		cfg.modifiers |= HotKeyConf::ALT;
	if (cbKey->ItemIndex >= 0)
		cfg.keyCode = vkey_list[cbKey->ItemIndex].name;
	else
		cfg.keyCode = "";
	int id = vkey_find(cfg.keyCode.c_str());
	if (id >= 0)
	{
		cfg.vkCode = vkey_list[id].vkey;
	}
	else
	{
		cfg.vkCode = -1;
	}
	cfg.global = chbGlobal->Checked;
	cfg.action.type = static_cast<Action::Type>(cbAction->ItemIndex);
	if (Sender == cbAction)	// do not update action type if not needed - TEdit losing focus for script name 
	{
		UpdateActionTypeView();
	}
	cfg.action.id = cbId->ItemIndex;
	cfg.action.scriptFile = edScriptFile->Text;
}

void THotkeyCfgPanel::UpdateActionTypeView(void)
{
	if (cbAction->ItemIndex == Action::TYPE_BUTTON)
	{
		cbId->Visible = true;
	}
	else
	{
		cbId->Visible = false;
	}

	if (cbAction->ItemIndex == Action::TYPE_SCRIPT_FILE)
	{
		edScriptFile->Visible = true;
		btnScriptFileSelect->Visible = true;
	}
	else
	{
		edScriptFile->Visible = false;
		btnScriptFileSelect->Visible = false;
	}
}

void THotkeyCfgPanel::Start(void)
{
	chbCtrl->OnClick = NULL;
	chbShift->OnClick = NULL;
	chbAlt->OnClick = NULL;
	cbKey->OnChange = NULL;
	cbAction->OnChange = NULL;
	cbId->OnChange = NULL;
	chbGlobal->OnClick = NULL;

	for (unsigned int i=0; i<vkey_list_size(); i++)
	{
    	cbKey->Items->Add(vkey_list[i].description);
	}
	for (int i=0; i<Action::TYPE_LIMITER; i++)
	{
    	cbAction->Items->Add(Action::getTypeDescription(static_cast<Action::Type>(i)));
	}
	for (int i=0; i<programmableButtonsCnt; i++)
	{
    	cbId->Items->Add(i);
	}

	chbCtrl->Checked = cfg.modifiers & HotKeyConf::CTRL;
	chbShift->Checked = cfg.modifiers & HotKeyConf::SHIFT;
	chbAlt->Checked = cfg.modifiers & HotKeyConf::ALT;
	cbKey->ItemIndex = vkey_find(cfg.keyCode.c_str());	// can't do this in constructor
	cbAction->ItemIndex = cfg.action.type;
	UpdateActionTypeView();
	cbId->ItemIndex = cfg.action.id;
	edScriptFile->Text = cfg.action.scriptFile;
	chbGlobal->Checked = cfg.global;

	chbCtrl->OnClick = UpdateCfg;
	chbShift->OnClick = UpdateCfg;
	chbAlt->OnClick = UpdateCfg;
	cbKey->OnChange = UpdateCfg;
	cbAction->OnChange = UpdateCfg;
	cbId->OnChange = UpdateCfg;
	edScriptFile->OnChange = UpdateCfg;
	chbGlobal->OnClick = UpdateCfg;
	btnScriptFileSelect->OnClick = btnBrowseClick;
}

void __fastcall THotkeyCfgPanel::btnBrowseClick(TObject *Sender)
{
	TOpenDialog *openDialog = new TOpenDialog(NULL);

	AnsiString dir = Paths::GetProfileDir() + "\\scripts";
	ForceDirectories(dir);
	openDialog->InitialDir = dir;
	openDialog->Filter = "Lua files (*.lua)|*.lua|All files|*.*";
	AnsiString fileName = dir + "\\" + edScriptFile->Text;
	if (FileExists(fileName))
		openDialog->FileName = fileName;
	else
		openDialog->FileName = "";
	if (openDialog->Execute())
	{
		if (UpperCase(dir) != UpperCase(ExtractFileDir(openDialog->FileName)))
		{
			MessageBox(this->Handle, "Entry was not updated.\nFor portability script files must be placed in \"script\" subdirectory.", this->Caption.c_str(), MB_ICONEXCLAMATION);
		}
		else
		{
			edScriptFile->Text = ExtractFileName(openDialog->FileName);
		}
	}

	delete openDialog;
}
//---------------------------------------------------------------------------
namespace Hotkeycfgpanel
{
	void __fastcall PACKAGE Register()
	{
		TComponentClass classes[1] = {__classid(THotkeyCfgPanel)};
		RegisterComponents("Samples", classes, 0);
	}
}
//---------------------------------------------------------------------------
