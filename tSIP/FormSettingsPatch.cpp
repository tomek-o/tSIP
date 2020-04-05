//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "FormSettingsPatch.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmSettingsPatch *frmSettingsPatch;
//---------------------------------------------------------------------------
__fastcall TfrmSettingsPatch::TfrmSettingsPatch(TComponent* Owner)
	: TForm(Owner),
	onUpdateSettings(NULL)
{
}
//---------------------------------------------------------------------------
void __fastcall TfrmSettingsPatch::btnApplyClick(TObject *Sender)
{
	if (onUpdateSettings == NULL)
		return;
	AnsiString text = edJson->Text.Trim();
	if (text == "")
		return;
	int status = onUpdateSettings(text);
	if (status != 0)
	{
		AnsiString msg;
		msg.sprintf("Failed to update settings (invalid JSON?)");
		MessageBox(this->Handle, msg.c_str(), this->Caption.c_str(), MB_ICONEXCLAMATION);
	}
}
//---------------------------------------------------------------------------
void __fastcall TfrmSettingsPatch::FormKeyDown(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
	if (Key == VK_ESCAPE)
		Close();	
}
//---------------------------------------------------------------------------
