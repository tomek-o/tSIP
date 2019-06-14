//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "FormTroubleshooting.h"
#include "Troubleshooting.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmTroubleshooting *frmTroubleshooting;
//---------------------------------------------------------------------------

using namespace Troubleshooting;

__fastcall TfrmTroubleshooting::TfrmTroubleshooting(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------

void __fastcall TfrmTroubleshooting::FormShow(TObject *Sender)
{
	const std::vector<Troubleshooting::Item> &items = Troubleshooting::getItems();
	lvItems->Items->Count = items.size();
	lvItems->Invalidate();
	if (lvItems->Selected)
	{
		SelectItem(lvItems->Selected);
	}
	else
	{
    	memoDetails->Text = "";
	}
}
//---------------------------------------------------------------------------

void __fastcall TfrmTroubleshooting::lvItemsData(TObject *Sender,
      TListItem *Item)
{
	const std::vector<Troubleshooting::Item> &items = Troubleshooting::getItems();
	if (Item == NULL)
		return;
	int id = Item->Index;
	const Troubleshooting::Item &ti = items[id];

	Troubleshooting::Level level = ti.getLevel();
	switch(level)
	{
	case LevelError:
		Item->ImageIndex = 0;
		break;
	case LevelWarning:
		Item->ImageIndex = 1;
		break;
	case LevelHint:
		Item->ImageIndex = 2;
		break;
	default:
		Item->ImageIndex = -1;
		break;
	}

	Item->SubItems->Add(ti.getName());
}
//---------------------------------------------------------------------------

void __fastcall TfrmTroubleshooting::lvItemsSelectItem(TObject *Sender,
      TListItem *Item, bool Selected)
{
	if (Item && Selected)
	{
		SelectItem(Item);
	}
}
//---------------------------------------------------------------------------

void TfrmTroubleshooting::SelectItem(TListItem *Item)
{
	int id = Item->Index;
	const std::vector<Troubleshooting::Item> &items = Troubleshooting::getItems();
	const Troubleshooting::Item &ti = items[id];
	memoDetails->Text = ti.getDescription();
	if (ti.extraMsg != "")
	{
		memoDetails->Text = memoDetails->Text + "\n" + ti.extraMsg;
	}
}

