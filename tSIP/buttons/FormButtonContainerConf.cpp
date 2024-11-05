//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "FormButtonContainerConf.h"
#include "ButtonContainers.h"
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
	config(NULL)
{
	cbBackgroundColor->Items->Clear();
	for (int i=0; i<Color::clLimiter; i++)
	{
		cbBackgroundColor->Items->Add(Color::IdToText(static_cast<Color::Id>(i)));
	}
}
//---------------------------------------------------------------------------

void __fastcall TfrmButtonContainerConf::ShowModal(int containerId, ButtonContainerConf *cfg)
{
	assert(cfg);
	this->config = cfg;
	tmpConfig = *config;

	edBackgroundBitmap->Text = tmpConfig.backgroundImage;
	chbBackgroundImageTransparent->Checked = tmpConfig.backgroundImageTransparent;

	int colorId = Color::IntTColorToId(tmpConfig.backgroundColor);
	cbBackgroundColor->ItemIndex = colorId;
	btnSelectBackgroundColor->Visible = (colorId == Color::clCustom);

	UpdateColorsPreview();

	lblContainerName->Caption = GetButtonContainerName(static_cast<ButtonContainerId>(containerId));

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
	AnsiString &fname = tmpConfig.backgroundImage;
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
	tmpConfig.backgroundImage = ExtractFileName(edBackgroundBitmap->Text);
	tmpConfig.backgroundImageTransparent = chbBackgroundImageTransparent->Checked;

	*config = tmpConfig;	

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
	if (cbBackgroundColor->ItemIndex != Color::clCustom)
	{
		tmpConfig.backgroundColor = Color::IdToIntTColor(static_cast<Color::Id>(cbBackgroundColor->ItemIndex));
	}	
	UpdateColorsPreview();
}
//---------------------------------------------------------------------------

void TfrmButtonContainerConf::UpdateColorsPreview(void)
{
	shColorBackground->Brush->Color = static_cast<TColor>(tmpConfig.backgroundColor);
}

void __fastcall TfrmButtonContainerConf::btnSelectBackgroundColorClick(
      TObject *Sender)
{
	int *col;
	if (Sender == btnSelectBackgroundColor)
	{
		col = &tmpConfig.backgroundColor;
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

