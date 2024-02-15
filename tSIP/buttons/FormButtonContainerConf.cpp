//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "FormButtonContainerConf.h"
#include "ButtonContainerConf.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmButtonContainerConf *frmButtonContainerConf;
//---------------------------------------------------------------------------
__fastcall TfrmButtonContainerConf::TfrmButtonContainerConf(TComponent* Owner)
	: TForm(Owner),
	confirmed(false)
{
}
//---------------------------------------------------------------------------

void __fastcall TfrmButtonContainerConf::ShowModal(ButtonContainerConf *cfg)
{

}

void __fastcall TfrmButtonContainerConf::FormShow(TObject *Sender)
{
	confirmed = false;
}
//---------------------------------------------------------------------------
