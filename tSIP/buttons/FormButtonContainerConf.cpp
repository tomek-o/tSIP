//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "FormButtonContainerConf.h"
#include "ButtonContainerConf.h"
#include "common\Colors.h"
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
	cbBackgroundColor->Items->Clear();
	for (int i=0; i<Color::clLimiter; i++)
	{
		cbBackgroundColor->Items->Add(Color::IdToText(static_cast<Color::Id>(i)));
	}
}
//---------------------------------------------------------------------------

void __fastcall TfrmButtonContainerConf::ShowModal(ButtonContainerConf *cfg)
{
	assert(cfg);
	this->cfg = cfg;
	
	edBackgroundBitmap->Text = cfg->backgroundImage;

	int colorId = Color::IntTColorToId(cfg->backgroundColor);
	cbBackgroundColor->ItemIndex = colorId;
	btnSelectBackgroundColor->Visible = (colorId == Color::clCustom);

	UpdateColorsPreview();

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
	if (cbBackgroundColor->ItemIndex != Color::clCustom)
	{
		cfg->backgroundColor = Color::IdToIntTColor(static_cast<Color::Id>(cbBackgroundColor->ItemIndex));
	}

	Close();
}
//---------------------------------------------------------------------------

void __fastcall TfrmButtonContainerConf::FormKeyPress(TObject *Sender,
      char &Key)
{
	if (Key == VK_ESCAPE)
	{
		Key = 0;
		Close();
	}
}
//---------------------------------------------------------------------------

void __fastcall TfrmButtonContainerConf::btnCancelClick(TObject *Sender)
{
	Close();	
}
//---------------------------------------------------------------------------

void __fastcall TfrmButtonContainerConf::cbBackgroundColorChange(
      TObject *Sender)
{
	btnSelectBackgroundColor->Visible = (cbBackgroundColor->ItemIndex == Color::clCustom);
}
//---------------------------------------------------------------------------

void TfrmButtonContainerConf::UpdateColorsPreview(void)
{
	shColorBackground->Brush->Color = static_cast<TColor>(cfg->backgroundColor);
}

void __fastcall TfrmButtonContainerConf::btnSelectBackgroundColorClick(
      TObject *Sender)
{
	int *col;
	if (Sender == btnSelectBackgroundColor)
	{
		col = &cfg->backgroundColor;
	}
	else
	{
		assert(!"Unhandled color type");
		return;
	}
	colorDialog->Color = static_cast<TColor>(*col);
	if (colorDialog->Execute())
	{
		*col = colorDialog->Color;
		UpdateColorsPreview();
	}
}
//---------------------------------------------------------------------------

