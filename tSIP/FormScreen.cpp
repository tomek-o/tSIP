//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "FormScreen.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmScreen *frmScreen;
//---------------------------------------------------------------------------
__fastcall TfrmScreen::TfrmScreen(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------

void __fastcall TfrmScreen::CreateParams(TCreateParams &Params)
{
	TForm::CreateParams(Params);
	Params.ExStyle |= WS_EX_APPWINDOW;
	Params.WndParent = 0;
}


