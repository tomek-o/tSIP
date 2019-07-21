//---------------------------------------------------------------------------


#pragma hdrstop

#include "ButtonConf.h"
#include <Graphics.hpp>

//---------------------------------------------------------------------------

#pragma package(smart_init)

ButtonConf::ButtonConf(void)
{
	Reset();
}

void ButtonConf::Reset(void)
{
	type = Button::DISABLED;
	caption = "";
	caption2 = "";
	captionLines = CAPTION_LINES_MIN;
	number = "";
	noIcon = false;
	height = 32;
	marginTop = 0;
	marginBottom = 0;
	backgroundColor = clBtnFace;
	imgIdle = "idle.bmp";
	imgTerminated = "terminated.bmp";
	imgEarly = "early.bmp";
	imgConfirmed = "confirmed.bmp";

	arg1 = "";

	pagingTxWaveFile = "";
	pagingTxCodec = "PCMA";
	pagingTxPtime = 20;

	script = "";

	blfOverrideIdle.Reset();
	blfOverrideTerminated.Reset();
	blfOverrideEarly.Reset();
	blfOverrideConfirmed.Reset();

	blfActionDuringCall = BLF_IN_CALL_TRANSFER;
	blfDtmfPrefixDuringCall = "";

	audioTxMod = "winwave";
	audioTxDev = "";
	audioRxMod = "winwave";
	audioRxDev = "";
}
