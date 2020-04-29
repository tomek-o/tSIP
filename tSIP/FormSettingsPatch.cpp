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
	DragAcceptFiles(Handle, true);
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

void __fastcall TfrmSettingsPatch::WmDropFiles(TWMDropFiles& Message)
{
	char buff[MAX_PATH];
	HDROP hDrop = (HDROP)Message.Drop;
	int numFiles = DragQueryFile(hDrop, -1, NULL, NULL);
	if (numFiles > 0) {
		DragQueryFile(hDrop, 0, buff, sizeof(buff));
		try
		{
			edJson->Lines->LoadFromFile(buff);
		}
		catch(...)
		{
			AnsiString msg;
			msg.sprintf("Failed to load text from file");
			MessageBox(this->Handle, msg.c_str(), this->Caption.c_str(), MB_ICONEXCLAMATION);
			edJson->Text = "";
		}
	}
	DragFinish(hDrop);
}
