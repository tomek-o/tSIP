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
    parentId = DEFAULT_PARENT_ID;	// "main" button container
	type = Button::DISABLED;
	visible = true;
	caption = "";
	caption2 = "";
	captionLines = CAPTION_LINES_MIN;
	number = "";
	down = false;
	inactive = false;
	left = 275;
	top = 0;
	width = 100;
	height = 32;
	bevelWidth = 1;
	customFrame = false;
	labelCenterHorizontally = false;
	labelCenterVertically = true;
	labelLeft = 20;
	labelTop = 0;
	spaceLabelsYEqually = true;

	label2Left = 20;
	label2Top = 20;
	label2CenterHorizontally = false;

	imageTransparent = true;
	imageLeft = 2;
	imageCenterVertically = true;
	imageTop = 0;
	for (unsigned int i=0; i<sizeof(colors)/sizeof(colors[0]); i++)
	{
		Color &col = colors[i];
		TColor defColor;
		if (i == EL_BACKGROUND)
			defColor = clBtnFace;
		else if (i == EL_FONT)
			defColor = clWindowText;
		else if (i == EL_FRAME)
			defColor = clWindowText;
		else
		{
			assert(!"Unhandled defColor!");
			return;
		}
		col.idle = defColor;
		col.down = defColor;
		col.downPressed = defColor;
		col.inactive = defColor;
		col.inactiveDown = defColor;
	}
	imgIdle = "idle.bmp";
	imgTerminated = "terminated.bmp";
	imgEarly = "early.bmp";
	imgConfirmed = "confirmed.bmp";

	arg1 = "";

	sipCode = 200;
	sipReason = "OK";
	expires = 600;

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

	fontLabel2 = font;
}

bool ButtonConf::UaRestartNeeded(const ButtonConf& right) const
{
	if (type == Button::BLF || right.type == Button::BLF ||
		type == Button::PRESENCE || right.type == Button::PRESENCE
		)
	{
        /** \todo Make UA restart condition for button config more precise (no need to restart if e.g. button height changed) */
		return true;
	}
	return false;
}

