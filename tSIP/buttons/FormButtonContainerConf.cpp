//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "FormButtonContainerConf.h"
#include "ButtonContainerConf.h"
#include <assert.h>
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmButtonContainerConf *frmButtonContainerConf = NULL;
//---------------------------------------------------------------------------
__fastcall TfrmButtonContainerConf::TfrmButtonContainerConf(TComponent* Owner)
	: TForm(Owner),
	confirmed(false),
	cfg(NULL)
{
}
//---------------------------------------------------------------------------

void __fastcall TfrmButtonContainerConf::ShowModal(ButtonContainerConf *cfg)
{
	assert(cfg);
	this->cfg = cfg;
	
	edBackgroundBitmap->Text = cfg->backgroundImage;

	TForm::ShowModal();
}

void __fastcall TfrmButtonContainerConf::FormShow(TObject *Sender)
{
	confirmed = false;
}
//---------------------------------------------------------------------------
void __fastcall TfrmButtonContainerConf::btnSelectBackgroundBitmapClick(
      TObject *Sender)
{
	AnsiString &fname = cfg->backgroundImage;
	openDialog->Filter = "Bitmaps (*.bmp)|*.bmp|All files|*.*";
	AnsiString dir = ExtractFileDir(Application->ExeName) + "\\img\\";
	openDialog->InitialDir = dir;
	if (FileExists(dir + fname))
		openDialog->FileName = dir + fname;
	else
		openDialog->FileName = "";
	if (openDialog->Execute())
	{
		edBackgroundBitmap->Text = ExtractFileName(openDialog->FileName);
	}
}
//---------------------------------------------------------------------------

void __fastcall TfrmButtonContainerConf::btnApplyClick(TObject *Sender)
{
	confirmed = true;
	cfg->backgroundImage = ExtractFileName(edBackgroundBitmap->Text);

	Close();
}
//---------------------------------------------------------------------------

void __fastcall TfrmButtonContainerConf::FormKeyPress(TObject *Sender,
      char &Key)
{
	if (Key == VK_ESCAPE)
		Close();	
}
//---------------------------------------------------------------------------

void __fastcall TfrmButtonContainerConf::btnCancelClick(TObject *Sender)
{
	Close();	
}
//---------------------------------------------------------------------------

