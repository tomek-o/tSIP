//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "ButtonsDataModule.h"
#include "ProgrammableButton.h"
#include "ProgrammableButtons.h"
#include "Globals.h"
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
	AnsiString typeText = Button::TypeName(buttons.btnConf[id].type);
	text.sprintf("Button #%d: %s", id, typeText.c_str());;
	miInfo->Caption = text;	
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
	buttons.Move(id);
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

