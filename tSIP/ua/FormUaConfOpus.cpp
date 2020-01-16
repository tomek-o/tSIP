//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "FormUaConfOpus.h"
#include "assert.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmUaConfOpus *frmUaConfOpus;
//---------------------------------------------------------------------------
__fastcall TfrmUaConfOpus::TfrmUaConfOpus(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------

void TfrmUaConfOpus::SetCfg(UaConf::Opus* opus)
{
	this->opus = opus;
	assert(opus);

	chbStereo->Checked = opus->stereo;
	chbSpropStereo->Checked = opus->spropStereo;
	chbSetBitrate->Checked = opus->setBitrate;
	edBitrate->Text = opus->bitrate;
	chbSetSamplerate->Checked = opus->setSamplerate;
	cbSamplerate->Text = opus->samplerate;
	chbSetCbr->Checked = opus->setCbr;
	chbCbr->Checked = opus->cbr;
	chbSetInbandFec->Checked = opus->setInbandFec;
	chbInbandFec->Checked = opus->inbandFec;
	chbSetDtx->Checked = opus->setDtx;
	chbDtx->Checked = opus->dtx;
	chbMirror->Checked = opus->mirror;
	cbComplexity->Text = opus->complexity;
	chbSetApplication->Checked = opus->setApplication;
	if (opus->application < cbApplication->Items->Count)
	{
		cbApplication->ItemIndex = opus->application;
	}
	chbSetPacketLoss->Checked = opus->setPacketLoss;
	edPacketLoss->Text = opus->packetLoss;
}

void TfrmUaConfOpus::Apply(void)
{
	assert(opus);

	opus->stereo = chbStereo->Checked;
	opus->spropStereo = chbSpropStereo->Checked;
	opus->setBitrate = chbSetBitrate->Checked;
	opus->bitrate = StrToIntDef(edBitrate->Text, opus->bitrate);
	opus->setSamplerate = chbSetSamplerate->Checked;
	opus->samplerate = StrToIntDef(cbSamplerate->Text, opus->samplerate);
	opus->setCbr = chbSetCbr->Checked;
	opus->cbr = chbCbr->Checked;
	opus->setInbandFec = chbSetInbandFec->Checked;
	opus->inbandFec = chbInbandFec->Checked;
	opus->setDtx = chbSetDtx->Checked;
	opus->dtx = chbDtx->Checked;
	opus->mirror = chbMirror->Checked;
	opus->complexity = StrToIntDef(cbComplexity->Text, opus->complexity);
	opus->setApplication = chbSetApplication->Checked;
	opus->application = cbApplication->ItemIndex;
	opus->setPacketLoss = chbSetPacketLoss->Checked;
	opus->packetLoss = StrToIntDef(edPacketLoss->Text, opus->packetLoss);
}