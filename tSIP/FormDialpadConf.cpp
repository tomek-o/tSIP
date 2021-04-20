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
}

