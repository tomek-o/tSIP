//---------------------------------------------------------------------------

#ifndef ButtonConfH
#define ButtonConfH
//---------------------------------------------------------------------------

#include "ButtonType.h"
#include <string>

class ButtonConf
{
public:
	Button::Type type;
	std::string caption;
	std::string caption2;	///< optional 2nd caption line
	enum { CAPTION_LINES_MIN = 1 };
	enum { CAPTION_LINES_MAX = 2 };
	int captionLines;
	std::string number;
	bool noIcon;
	int height;
	int marginTop;
	int marginBottom;
	int backgroundColor;

	std::string imgIdle;
	std::string imgTerminated;
	std::string imgEarly;
	std::string imgConfirmed;

	std::string arg1;

	std::string pagingTxWaveFile;
	std::string pagingTxCodec;
	unsigned int pagingTxPtime;	

	std::string script;

	/* Switching audio devices during call */
	std::string audioTxMod;
	std::string audioTxDev;
	std::string audioRxMod;
	std::string audioRxDev;

	/** Overrides default number when BLF button is pressed
		(calls to other number than subscribed)
	*/
	struct BlfOverride
	{
		bool active;
		std::string number;
		BlfOverride(void):
			active(false)
		{}
		void Reset(void)
		{
			active = false;
			number = "";
        }
	} blfOverrideIdle, blfOverrideTerminated, blfOverrideEarly, blfOverrideConfirmed;

	enum BlfActionDuringCall
	{
		BLF_IN_CALL_NONE = 0,
		BLF_IN_CALL_DTMF,
		BLF_IN_CALL_TRANSFER,
		
		BLF_IN_CALL_LIMITER
	} blfActionDuringCall;

	/** Prefix to be added when BLF is pressed during call
		and BLF is configured to send DTMF
	*/
	std::string blfDtmfPrefixDuringCall;

	ButtonConf(void);

	bool operator==(const ButtonConf& right) const {
		return (
			type == right.type &&
			number == right.number &&
			caption == right.caption &&
			caption2 == right.caption2 &&
			captionLines == right.captionLines &&
			noIcon == right.noIcon &&
			height == right.height &&
			marginTop == right.marginTop &&
			marginBottom == right.marginBottom &&
			backgroundColor == right.backgroundColor &&
			imgIdle == right.imgIdle &&
			imgTerminated == right.imgTerminated &&
			imgEarly == right.imgEarly &&
			imgConfirmed == right.imgConfirmed &&
			arg1 == right.arg1 &&
            pagingTxWaveFile == right.pagingTxWaveFile &&
			pagingTxCodec == right.pagingTxCodec &&
			pagingTxPtime == right.pagingTxPtime &&
			audioTxMod == right.audioTxMod &&
			audioTxDev == right.audioTxDev &&
			audioRxMod == right.audioRxMod &&
			audioRxDev == right.audioRxDev
			);
	}

	bool operator!=(const ButtonConf& right) const {
		return !(*this == right);
	}

	void Reset(void);
};

#endif
