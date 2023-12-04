//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "FormScriptPluginVariables.h"
#include "ScriptExec.h"
#include <map>
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmScriptPluginVariables *frmScriptPluginVariables = NULL;
//---------------------------------------------------------------------------
__fastcall TfrmScriptPluginVariables::TfrmScriptPluginVariables(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------

void TfrmScriptPluginVariables::Read(void)
{
	lvVariables->Items->Count = 0;
	variables.clear();

	std::map<AnsiString, AnsiString> vm = ScriptExec::GetVariables();
	for (std::map<AnsiString, AnsiString>::iterator iter = vm.begin(); iter != vm.end(); ++iter)
	{
		struct Var v;
		v.name = iter->first;
		v.value = iter->second;
		variables.push_back(v);
	}

	lvVariables->Items->Count = variables.size();
	lvVariables->Invalidate();
}

void __fastcall TfrmScriptPluginVariables::lvVariablesData(TObject *Sender,
      TListItem *Item)
{
	int id = Item->Index;
	const struct Var &v = variables[id];
	Item->Caption = v.name;
	Item->SubItems->Add(v.value);	
}
//---------------------------------------------------------------------------

void __fastcall TfrmScriptPluginVariables::FormShow(TObject *Sender)
{
	Read();	
}
//---------------------------------------------------------------------------

void __fastcall TfrmScriptPluginVariables::btnReadClick(TObject *Sender)
{
	Read();	
}
//---------------------------------------------------------------------------

void __fastcall TfrmScriptPluginVariables::tmrAutoRefreshTimer(TObject *Sender)
{
	if (!Visible)
		return;
	if (!chbAutoRefresh->Checked)
		return;
	tmrAutoRefresh->Enabled = false;
	Read();
	tmrAutoRefresh->Enabled = true;
}
//---------------------------------------------------------------------------

void __fastcall TfrmScriptPluginVariables::chbAutoRefreshClick(TObject *Sender)
{
	tmrAutoRefresh->Enabled = true;	
}
//---------------------------------------------------------------------------

void __fastcall TfrmScriptPluginVariables::chbAutoRefreshMouseDown(
      TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
	tmrAutoRefresh->Enabled = true;	
}
//---------------------------------------------------------------------------

void __fastcall TfrmScriptPluginVariables::FormKeyPress(TObject *Sender,
      char &Key)
{
	if (Key == VK_ESCAPE)
		Close();	
}
//---------------------------------------------------------------------------

