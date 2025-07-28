//---------------------------------------------------------------------------

#ifndef ButtonConfH
#define ButtonConfH
//---------------------------------------------------------------------------

#include "ButtonType.h"
#include <string>
#include <mem.h>

class ButtonConf
{
public:
    int parentId;			///< id of button container where this button belongs
	Button::Type type;

	std::string number;

	bool visible;	// hide whole button
	bool down;
	bool inactive;
	int left, top, width, height;
	unsigned int bevelWidth;
	bool customFrame;

	enum { CAPTION_LINES_MIN = 1 };
	enum { CAPTION_LINES_MAX = 2 };
	int captionLines;
	bool spaceLabelsYEqually;

	std::string caption;
	bool labelCenterHorizontally;
	bool labelCenterVertically;
	int labelLeft;
	int labelTop;

	std::string caption2;	///< optional 2nd caption line
	int label2Left;
	int label2Top;
	bool label2CenterHorizontally;

	enum Element {
		EL_BACKGROUND = 0,
		EL_FONT,
		EL_FRAME,

		EL_LIMITER
	};
	struct Color {
		int idle;
		int down;
		int downPressed;
		int inactive;
		int inactiveDown;
		bool operator==(const Color& right) const {
			return (
				idle == right.idle &&
				down == right.down &&
				downPressed == right.downPressed &&
				inactive == right.inactive &&
				inactiveDown == right.inactiveDown
			);
		}
		bool operator!=(const Color& right) const {
			return !(*this == right);
		}
	} colors[EL_LIMITER];

	bool imageTransparent;
	int imageLeft;
	bool imageCenterVertically;
	int imageTop;					// used if imageCenterVertically == false;

	std::string imgIdle;
	std::string imgTerminated;
	std::string imgEarly;
	std::string imgConfirmed;

	std::string arg1;

	int sipCode;
	std::string sipReason;
	int expires;	

	std::string pagingTxWaveFile;
	std::string pagingTxCodec;
	unsigned int pagingTxPtime;

	std::string script;

	/* Switching audio devices during call */
	std::string audioTxMod;
	std::string audioTxDev;
	std::string audioRxMod;
	std::string audioRxDev;

	/* Switching video source during call */
	std::string videoRxMod;
	std::string videoRxDev;

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
		bool operator==(const BlfOverride& right) const {
			return (
				active == right.active &&
				number == right.number
			);
		}
		bool operator!=(const BlfOverride& right) const {
			return !(*this == right);
		}
	} blfOverrideIdle, blfOverrideTerminated, blfOverrideEarly, blfOverrideConfirmed;

	enum BlfActionDuringCall
	{
		BLF_IN_CALL_NONE = 0,
		BLF_IN_CALL_DTMF,
		BLF_IN_CALL_TRANSFER,
		
		BLF_IN_CALL_LIMITER
	} blfActionDuringCall;

	/** \brief Display style configuration for BLF remote identity
	*/
	enum BlfRemoteIdentityDisplay
	{
		BLF_REMOTE_IDENTITY_DISPLAY_NAME_OR_NUMBER = 0,			///< use display name if present, otherwise number
		BLF_REMOTE_IDENTITY_DISPLAY_NAME_AND_NUMBER_MULTI_LINE,	///< multiline caption: display name in the first line, number in the second
		BLF_REMOTE_IDENTITY_DISPLAY_NAME_AND_NUMBER_SAME_LINE,	///< display name and number separated with spaces

		BLF_REMOTE_IDENTITY_DISPLAY_LIMITER
	} blfRemoteIdentityDisplay;

	static const char* GetBlfRemoteIdentityDisplayDescription(enum BlfRemoteIdentityDisplay val);

	/** Prefix to be added when BLF is pressed during call
		and BLF is configured to send DTMF
	*/
	std::string blfDtmfPrefixDuringCall;

	struct Font
	{
		std::string name;
		int size;
		bool bold, italic, underline, strikeout;
		Font(void):
			name("Tahoma"),
			size(8),
			bold(false),
			italic(false),
			underline(false),
			strikeout(false)
		{}
		bool operator==(const Font& right) const {
			return (
				name == right.name &&
				size == right.size &&
				bold == right.bold &&
				italic == right.italic &&
				underline == right.underline &&
				strikeout == right.strikeout
			);
		}
		bool operator!=(const Font& right) const {
			return !(*this == right);
		}
	} font, fontLabel2;

	ButtonConf(void);

	bool operator==(const ButtonConf& right) const {
		return (
			type == right.type &&
			parentId == right.parentId &&
			number == right.number &&
			caption == right.caption &&
			caption2 == right.caption2 &&
			captionLines == right.captionLines &&
			visible == right.visible &&
			down == right.down &&
			inactive == right.inactive &&
			left == right.left &&
			top == right.top &&
			width == right.width &&
			height == right.height &&
			bevelWidth == right.bevelWidth &&
			customFrame == right.customFrame &&
			labelCenterHorizontally == right.labelCenterHorizontally &&
			labelCenterVertically == right.labelCenterVertically &&
			labelLeft == right.labelLeft &&
			labelTop == right.labelTop &&
			label2Top == right.label2Top &&
			label2CenterHorizontally == right.label2CenterHorizontally &&
			imageTransparent == right.imageTransparent &&
			imageLeft == right.imageLeft &&
			imageCenterVertically == right.imageCenterVertically &&
			imageTop == right.imageTop &&
			memcmp(colors, right.colors, sizeof(colors)) == 0 &&
			imgIdle == right.imgIdle &&
			imgTerminated == right.imgTerminated &&
			imgEarly == right.imgEarly &&
			imgConfirmed == right.imgConfirmed &&
			arg1 == right.arg1 &&
			sipCode == right.sipCode &&
			sipReason == right.sipReason &&
			expires == right.expires &&
            pagingTxWaveFile == right.pagingTxWaveFile &&
			pagingTxCodec == right.pagingTxCodec &&
			pagingTxPtime == right.pagingTxPtime &&
			script == right.script &&
			audioTxMod == right.audioTxMod &&
			audioTxDev == right.audioTxDev &&
			audioRxMod == right.audioRxMod &&
			audioRxDev == right.audioRxDev &&
			font == right.font &&
			fontLabel2 == right.fontLabel2 &&

			blfOverrideIdle == right.blfOverrideIdle &&
			blfOverrideTerminated == right.blfOverrideTerminated &&
			blfOverrideEarly == right.blfOverrideEarly &&
			blfOverrideConfirmed == right.blfOverrideConfirmed &&

			blfActionDuringCall == right.blfActionDuringCall &&
            blfRemoteIdentityDisplay == right.blfRemoteIdentityDisplay &&
            blfDtmfPrefixDuringCall == right.blfDtmfPrefixDuringCall
			);
	}

	bool operator!=(const ButtonConf& right) const {
		return !(*this == right);
	}

    /** Check for button group */
	bool Contains(const ButtonConf& other) const;

	bool UaRestartNeeded(const ButtonConf& right) const;

	void Reset(void);
};

#endif
