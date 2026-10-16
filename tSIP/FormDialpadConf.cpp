//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "FormDialpadConf.h"
#include "DialpadConf.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmDialpadConf *frmDialpadConf;


namespace
{
	void S2Int(AnsiString text, int& val)
	{
		val = StrToIntDef(text, val);
	}
	void S2Uint(AnsiString text, unsigned int& val)
	{
		int tmp = StrToIntDef(text, val);
		if (tmp >= 0)
			val = tmp;
	}
}


//---------------------------------------------------------------------------
__fastcall TfrmDialpadConf::TfrmDialpadConf(TComponent* Owner)
	: TForm(Owner),
	cfg(NULL)
{
	CreateMeterColorControls();
}

// Rows below the existing controls: label, color swatch, change button
void TfrmDialpadConf::CreateMeterColorControls(void)
{
	int top = 0;
	for (int i=0; i<ControlCount; i++)
	{
		int bottom = Controls[i]->Top + Controls[i]->Height;
		if (bottom > top)
			top = bottom;
	}
	top += 16;

	static const char *captions[DialpadConf::MC_CNT] = {
		"Meter low level", "Meter mid level", "Meter high level",
		"Peak marker", "Slider track", "Slider thumb" };

	TLabel *title = new TLabel(this);
	title->Parent = this;
	title->Caption = "Volume sliders colors";
	title->Left = 8;
	title->Top = top;
	top += 24;

	dlgColor = new TColorDialog(this);
	for (int i=0; i<DialpadConf::MC_CNT; i++)
	{
		lblMeterColors[i] = new TLabel(this);
		lblMeterColors[i]->Parent = this;
		lblMeterColors[i]->Caption = captions[i];
		lblMeterColors[i]->Left = 8;
		lblMeterColors[i]->Top = top + 3;

		pnlMeterColors[i] = new TPanel(this);
		pnlMeterColors[i]->Parent = this;
		pnlMeterColors[i]->Left = 180;
		pnlMeterColors[i]->Top = top;
		pnlMeterColors[i]->Width = 40;
		pnlMeterColors[i]->Height = 20;
		pnlMeterColors[i]->BevelOuter = bvLowered;
		pnlMeterColors[i]->Caption = "";

		btnMeterColors[i] = new TButton(this);
		btnMeterColors[i]->Parent = this;
		btnMeterColors[i]->Caption = "Change...";
		btnMeterColors[i]->Left = 230;
		btnMeterColors[i]->Top = top;
		btnMeterColors[i]->Width = 80;
		btnMeterColors[i]->Height = 22;
		btnMeterColors[i]->OnClick = btnMeterColorClick;

		top += 28;
	}

	btnMeterColorsDefault = new TButton(this);
	btnMeterColorsDefault->Parent = this;
	btnMeterColorsDefault->Caption = "Default colors";
	btnMeterColorsDefault->Left = 180;
	btnMeterColorsDefault->Top = top;
	btnMeterColorsDefault->Width = 130;
	btnMeterColorsDefault->Height = 22;
	btnMeterColorsDefault->OnClick = btnMeterColorsDefaultClick;
}

void __fastcall TfrmDialpadConf::btnMeterColorClick(TObject *Sender)
{
	for (int i=0; i<DialpadConf::MC_CNT; i++)
	{
		if (Sender == btnMeterColors[i])
		{
			dlgColor->Color = pnlMeterColors[i]->Color;
			if (dlgColor->Execute())
				pnlMeterColors[i]->Color = dlgColor->Color;
			return;
		}
	}
}

void __fastcall TfrmDialpadConf::btnMeterColorsDefaultClick(TObject *Sender)
{
	DialpadConf defaults;
	for (int i=0; i<DialpadConf::MC_CNT; i++)
		pnlMeterColors[i]->Color = (TColor)defaults.meterColors[i];
}
//---------------------------------------------------------------------------

void TfrmDialpadConf::SetCfg(DialpadConf *cfg)
{
	this->cfg = cfg;
	{
		const DialpadConf::ElementConf &el = cfg->elements[DialpadConf::EL_ED_TRANSFER];
		chbTransferEditVisible->Checked = el.visible;
		edTransferEditLeft->Text = el.left;
		edTransferEditTop->Text = el.top;
		edTransferEditWidth->Text = el.width;
		edTransferEditHeight->Text = el.height;
	}
	{
		const DialpadConf::ElementConf &el = cfg->elements[DialpadConf::EL_TRBAR_MIC_VOLUME];
		chbMicVolumeTrbarVisible->Checked = el.visible;
		edMicVolumeTrbarLeft->Text = el.left;
		edMicVolumeTrbarTop->Text = el.top;
		edMicVolumeTrbarWidth->Text = el.width;
		edMicVolumeTrbarHeight->Text = el.height;
	}
	{
		const DialpadConf::ElementConf &el = cfg->elements[DialpadConf::EL_BTN_RESET_MIC_VOLUME];
		chbMicVolumeResetBtnVisible->Checked = el.visible;
		edMicVolumeResetBtnLeft->Text = el.left;
		edMicVolumeResetBtnTop->Text = el.top;
		edMicVolumeResetBtnWidth->Text = el.width;
		edMicVolumeResetBtnHeight->Text = el.height;
	}
	{
		const DialpadConf::ElementConf &el = cfg->elements[DialpadConf::EL_TRBAR_SPEAKER_VOLUME];
		chbSpeakerVolumeTrbarVisible->Checked = el.visible;
		edSpeakerVolumeTrbarLeft->Text = el.left;
		edSpeakerVolumeTrbarTop->Text = el.top;
		edSpeakerVolumeTrbarWidth->Text = el.width;
		edSpeakerVolumeTrbarHeight->Text = el.height;
	}
	{
		const DialpadConf::ElementConf &el = cfg->elements[DialpadConf::EL_BTN_RESET_SPEAKER_VOLUME];
		chbSpeakerVolumeResetBtnVisible->Checked = el.visible;
		edSpeakerVolumeResetBtnLeft->Text = el.left;
		edSpeakerVolumeResetBtnTop->Text = el.top;
		edSpeakerVolumeResetBtnWidth->Text = el.width;
		edSpeakerVolumeResetBtnHeight->Text = el.height;
	}
	for (int i=0; i<DialpadConf::MC_CNT; i++)
		pnlMeterColors[i]->Color = (TColor)cfg->meterColors[i];
}

void TfrmDialpadConf::Apply(void)
{
	assert(cfg);
	{
		DialpadConf::ElementConf &el = cfg->elements[DialpadConf::EL_ED_TRANSFER];
		el.visible = chbTransferEditVisible->Checked;
		S2Int(edTransferEditLeft->Text, el.left);
		S2Int(edTransferEditTop->Text, el.top);
		S2Uint(edTransferEditWidth->Text, el.width);
		S2Uint(edTransferEditHeight->Text, el.height);
	}
	{
		DialpadConf::ElementConf &el = cfg->elements[DialpadConf::EL_TRBAR_MIC_VOLUME];
		el.visible = chbMicVolumeTrbarVisible->Checked;
		S2Int(edMicVolumeTrbarLeft->Text, el.left);
		S2Int(edMicVolumeTrbarTop->Text, el.top);
		S2Uint(edMicVolumeTrbarWidth->Text, el.width);
		S2Uint(edMicVolumeTrbarHeight->Text, el.height);
	}
	{
		DialpadConf::ElementConf &el = cfg->elements[DialpadConf::EL_BTN_RESET_MIC_VOLUME];
		el.visible = chbMicVolumeResetBtnVisible->Checked;
		S2Int(edMicVolumeResetBtnLeft->Text, el.left);
		S2Int(edMicVolumeResetBtnTop->Text, el.top);
		S2Uint(edMicVolumeResetBtnWidth->Text, el.width);
		S2Uint(edMicVolumeResetBtnHeight->Text, el.height);
	}
	{
		DialpadConf::ElementConf &el = cfg->elements[DialpadConf::EL_TRBAR_SPEAKER_VOLUME];
		el.visible = chbSpeakerVolumeTrbarVisible->Checked;
		S2Int(edSpeakerVolumeTrbarLeft->Text, el.left);
		S2Int(edSpeakerVolumeTrbarTop->Text, el.top);
		S2Uint(edSpeakerVolumeTrbarWidth->Text, el.width);
		S2Uint(edSpeakerVolumeTrbarHeight->Text, el.height);
	}
	{
		DialpadConf::ElementConf &el = cfg->elements[DialpadConf::EL_BTN_RESET_SPEAKER_VOLUME];
		el.visible = chbSpeakerVolumeResetBtnVisible->Checked;
		S2Int(edSpeakerVolumeResetBtnLeft->Text, el.left);
		S2Int(edSpeakerVolumeResetBtnTop->Text, el.top);
		S2Uint(edSpeakerVolumeResetBtnWidth->Text, el.width);
		S2Uint(edSpeakerVolumeResetBtnHeight->Text, el.height);
	}
	for (int i=0; i<DialpadConf::MC_CNT; i++)
		cfg->meterColors[i] = (unsigned int)pnlMeterColors[i]->Color;
}

