//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "ButtonsDataModule.h"
#include "ProgrammableButton.h"
#include "ProgrammableButtons.h"
#include "Globals.h"
#include <Clipbrd.hpp>
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
//---------------------------------------------------------------------------
__fastcall TdmButtons::TdmButtons(TComponent* Owner)
	: TDataModule(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TdmButtons::popupPanelPopup(TObject *Sender)
{
	TProgrammableButton* panel = dynamic_cast<TProgrammableButton*>(popupPanel->PopupComponent);
	assert(panel);
	int id = panel->Tag;

	AnsiString text;
	const ButtonConf &cfg = buttons.btnConf[id];
	AnsiString typeText = Button::TypeName(cfg.type);
	text.sprintf("Button #%d: %s", id, typeText.c_str());;
	miInfo->Caption = text;

	// "button group" = buttons with higher id placed on the button with lower id,
	// working as kind of background/frame
	unsigned int groupBtnCount = 0;
	for (int i=id+1; i<buttons.btnConf.size(); i++)
	{
		if (cfg.Contains(buttons.btnConf[i]))
			groupBtnCount++;
	}
	if (groupBtnCount == 0)
	{
		miMoveSpeedDialGroup->Visible = false;
	}
	else
	{
		AnsiString text;
		text.sprintf("Move group, %u button(s) inside", groupBtnCount);
		miMoveSpeedDialGroup->Caption = text;
		miMoveSpeedDialGroup->Visible = true;
	}
}

//---------------------------------------------------------------------------

void __fastcall TdmButtons::miEditSpeedDialClick(TObject *Sender)
{
	TProgrammableButton* panel = dynamic_cast<TProgrammableButton*>(popupPanel->PopupComponent);
	assert(panel);
	int id = panel->Tag;
	buttons.Edit(id);
}

void __fastcall TdmButtons::miMoveSpeedDialClick(TObject *Sender)
{
	TProgrammableButton* panel = dynamic_cast<TProgrammableButton*>(popupPanel->PopupComponent);
	assert(panel);
	int id = panel->Tag;
	buttons.Move(id, false);
}
//---------------------------------------------------------------------------

void __fastcall TdmButtons::miResizeSpeedDialClick(TObject *Sender)
{
	TProgrammableButton* panel = dynamic_cast<TProgrammableButton*>(popupPanel->PopupComponent);
	assert(panel);
	int id = panel->Tag;
	buttons.Resize(id);
}
//---------------------------------------------------------------------------

void __fastcall TdmButtons::miCopyCaption1Click(TObject *Sender)
{
	TProgrammableButton* panel = dynamic_cast<TProgrammableButton*>(popupPanel->PopupComponent);
	assert(panel);
	Clipboard()->AsText = panel->GetCaption();
}
//---------------------------------------------------------------------------

void __fastcall TdmButtons::miCopyCaption2Click(TObject *Sender)
{
	TProgrammableButton* panel = dynamic_cast<TProgrammableButton*>(popupPanel->PopupComponent);
	assert(panel);
	Clipboard()->AsText = panel->GetCaption2();
}
//---------------------------------------------------------------------------

void __fastcall TdmButtons::miCopyButtonPropertiesClick(TObject *Sender)
{
	TProgrammableButton* panel = dynamic_cast<TProgrammableButton*>(popupPanel->PopupComponent);
	assert(panel);
	int id = panel->Tag;
	buttons.CopyConfig(id);
}
//---------------------------------------------------------------------------

void __fastcall TdmButtons::miMoveSpeedDialGroupClick(TObject *Sender)
{
	TProgrammableButton* panel = dynamic_cast<TProgrammableButton*>(popupPanel->PopupComponent);
	assert(panel);
	int id = panel->Tag;
	buttons.Move(id, true);	
}
//---------------------------------------------------------------------------

