//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "FormLuaScriptHelp.h"
#include <Clipbrd.hpp>
#include <set>
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmLuaScriptHelp *frmLuaScriptHelp;

namespace
{

TfrmLuaScriptHelp::CallbackRunScript callbackRunScript = NULL;

}

//---------------------------------------------------------------------------
__fastcall TfrmLuaScriptHelp::TfrmLuaScriptHelp(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TfrmLuaScriptHelp::lvSymbolsData(TObject *Sender,
      TListItem *Item)
{
	int id = Item->Index;
	const ScriptExec::Symbol &symbol = entries[id];
	Item->Caption = symbol.name;
	Item->SubItems->Add(symbol.brief);
}
//---------------------------------------------------------------------------
void __fastcall TfrmLuaScriptHelp::FormShow(TObject *Sender)
{
	const std::vector<ScriptExec::Symbol>& symbols = ScriptExec::GetSymbols();
	if (symbols.empty())
	{
		// run once empty script to fill global function list
		bool breakRequest = false;
		bool handled = true;
		callbackRunScript(SCRIPT_SRC_SCRIPT_WINDOW, -1, "", breakRequest, handled);
	}

	Filter();
}
//---------------------------------------------------------------------------

void TfrmLuaScriptHelp::Filter(void)
{
	static const std::vector<ScriptExec::Symbol>& symbols = ScriptExec::GetSymbols();
	entries.clear();

	AnsiString needle = UpperCase(edFilter->Text);
	if (needle == "")
	{
		for (std::vector<ScriptExec::Symbol>::const_iterator iter = symbols.begin(); iter != symbols.end(); ++iter)
		{
			entries.push_back(*iter);
		}
	}
	else
	{
		for (std::vector<ScriptExec::Symbol>::const_iterator iter = symbols.begin(); iter != symbols.end(); ++iter)
		{
			const ScriptExec::Symbol &symbol = *iter;;
			if (UpperCase(symbol.name).Pos(needle) > 0 ||
				UpperCase(symbol.brief).Pos(needle) > 0 ||
				UpperCase(symbol.description).Pos(needle)
				)
			{
				entries.push_back(symbol);
			}
		}
	}

	lvSymbols->Items->Count = entries.size();
	lvSymbols->Invalidate();
}

void TfrmLuaScriptHelp::SetCallbackRunScript(CallbackRunScript cb)
{
	assert(cb);
	callbackRunScript = cb;
}

void __fastcall TfrmLuaScriptHelp::edFilterChange(TObject *Sender)
{
	Filter();
	lvSymbols->ClearSelection();
	if (!lvSymbols->Selected)
		memoDescription->Clear();
}
//---------------------------------------------------------------------------

void __fastcall TfrmLuaScriptHelp::lvSymbolsSelectItem(TObject *Sender,
      TListItem *Item, bool Selected)
{
	if (!lvSymbols->Selected)
		memoDescription->Clear();
	if (Selected == false)
		return;
	if (Item == NULL)
	{
		memoDescription->Clear();
		return;
	}
	const ScriptExec::Symbol &symbol = entries[Item->Index];
	memoDescription->Text = symbol.description;
}
//---------------------------------------------------------------------------

void __fastcall TfrmLuaScriptHelp::miCopyFunctionNameClick(TObject *Sender)
{
	TListItem *Item = lvSymbols->Selected;
	if (!Item)
		return;
	int id = Item->Index;
	const ScriptExec::Symbol &symbol = entries[id];
	Clipboard()->AsText = symbol.name;
}
//---------------------------------------------------------------------------

void __fastcall TfrmLuaScriptHelp::miCopyBriefDescriptionClick(TObject *Sender)
{
	TListItem *Item = lvSymbols->Selected;
	if (!Item)
		return;
	int id = Item->Index;
	const ScriptExec::Symbol &symbol = entries[id];
	AnsiString text = symbol.brief;
	if (symbol.description != "")
	{
		text = text + "\r\n" + symbol.description;
	}
	Clipboard()->AsText = text;
}
//---------------------------------------------------------------------------

