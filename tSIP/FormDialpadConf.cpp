//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "FormDialpadConf.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmDialpadConf *frmDialpadConf;
//---------------------------------------------------------------------------
__fastcall TfrmDialpadConf::TfrmDialpadConf(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------

void TfrmDialpadConf::SetCfg(DialpadCfg *cfg)
{
	this->cfg = cfg;
	
}
