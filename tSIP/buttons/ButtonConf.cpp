//---------------------------------------------------------------------------


#pragma hdrstop

#include "ButtonConf.h"
#include "ButtonContainers.h"
#include "ua/AudioModules.h"
#include "ua/VideoModules.h"
#include <json/json.h>
#include <Graphics.hpp>

//---------------------------------------------------------------------------

#pragma package(smart_init)

namespace
{
	const ButtonConf defaultBtn;
}

ButtonConf::ButtonConf(void)
{
	Reset();
}

const char* ButtonConf::GetBlfRemoteIdentityDisplayDescription(enum ButtonConf::BlfRemoteIdentityDisplay val)
{
	switch (val)
	{
	case BLF_REMOTE_IDENTITY_DISPLAY_NAME_OR_NUMBER:
		return "name or number";
	case BLF_REMOTE_IDENTITY_DISPLAY_NAME_AND_NUMBER_MULTI_LINE:
		return "name + number in two lines of Caption #2";
	case BLF_REMOTE_IDENTITY_DISPLAY_NAME_AND_NUMBER_SAME_LINE:
		return "name + number in the same lines";

	default:
		return "???";
	}
}

void ButtonConf::Reset(void)
{
	parentId = BUTTON_CONTAINER_MAIN;
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
	blfRemoteIdentityDisplay = BLF_REMOTE_IDENTITY_DISPLAY_NAME_OR_NUMBER;	
	blfDtmfPrefixDuringCall = "";

	audioTxMod = AudioModules::winwave2;
	audioTxDev = "";
	audioRxMod = AudioModules::winwave2;
	audioRxDev = "";

	videoRxMod = VideoModules::colorbar_generator_animated;
	videoRxDev = "";

	fontLabel2 = font;
}

bool ButtonConf::Contains(const ButtonConf& other) const
{
	if (other.parentId == parentId)
	{
		if (other.left + other.width > left &&
			other.left < left + width &&
			other.top + other.height > top &&
			other.top < top + height)
		{
			return true;
		}
	}
	return false;
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

void ButtonConf::FromJson(const Json::Value &btnJson)
{
	if (btnJson.type() != Json::objectValue)
	{
		return;
	}

	try
	{
		btnJson.getInt("parentId", parentId);
		Button::Type type = (Button::Type)btnJson.get("type", this->type).asInt();
		if (type >= 0 && type < Button::TYPE_LIMITER)
		{
			this->type = type;
		}
		btnJson.getString("caption", caption);
		btnJson.getString("caption2", caption2);
		int captionLines = btnJson.get("captionLines", this->captionLines).asInt();
		if (captionLines >= ButtonConf::CAPTION_LINES_MIN && captionLines <= ButtonConf::CAPTION_LINES_MAX)
		{
			this->captionLines = captionLines;
		}
		{
			// importing setting that was removed/replaced in version 0.2
			const Json::Value& jv = btnJson["noIcon"];
			if (jv.type() == Json::booleanValue)
			{
				if (jv.asBool())
				{
					labelLeft = 4;
				}
				else
				{
					labelLeft = 20;
				}
			}
		}
		btnJson.getString("number", number);
		btnJson.getBool("visible", visible);
		btnJson.getBool("down", down);
		btnJson.getBool("inactive", inactive);
		btnJson.getInt("left", left);
		btnJson.getInt("top", top);
		btnJson.getUInt("width", width);
		btnJson.getUInt("height", height);

		btnJson.getUInt("bevelWidth", bevelWidth);
		btnJson.getBool("customFrame", customFrame);
		btnJson.getBool("labelCenterHorizontally", labelCenterHorizontally);
		btnJson.getBool("labelCenterVertically", labelCenterVertically);
		btnJson.getInt("labelLeft", labelLeft);
		btnJson.getInt("labelTop", labelTop);
		btnJson.getInt("label2Left", label2Left);
		btnJson.getInt("label2Top", label2Top);
		btnJson.getBool("spaceLabelsYEqually", spaceLabelsYEqually);

		btnJson.getBool("label2CenterHorizontally", label2CenterHorizontally);
		btnJson.getBool("imageTransparent", imageTransparent);
		btnJson.getInt("imageLeft", imageLeft);
		btnJson.getBool("imageCenterVertically", imageCenterVertically);
		btnJson.getInt("imageTop", imageTop);

		const Json::Value &colorsJson = btnJson["colors"];
		for (unsigned el=0; el < ButtonConf::EL_LIMITER; el++)
		{
			if (el >= colorsJson.size()) {
				break;
			}
			ButtonConf::Color &color = colors[el];
			const Json::Value &colorJson = colorsJson[el];
			color.idle = colorJson.get("idle", color.idle).asInt();
			color.down = colorJson.get("down", color.down).asInt();
			color.downPressed = colorJson.get("downPressed", color.downPressed).asInt();
			color.inactive = colorJson.get("inactive", color.inactive).asInt();
			color.inactiveDown = colorJson.get("inactiveDown", color.inactiveDown).asInt();
		}

		btnJson.getString("imgIdle", imgIdle);
		btnJson.getString("imgTerminated", imgTerminated);
		btnJson.getString("imgEarly", imgEarly);
		btnJson.getString("imgConfirmed", imgConfirmed);

		{
			ButtonConf::BlfActionDuringCall blfActionDuringCall =
				static_cast<ButtonConf::BlfActionDuringCall>(btnJson.get("blfActionDuringCall", this->blfActionDuringCall).asInt());
			if (blfActionDuringCall >= ButtonConf::BLF_IN_CALL_NONE && blfActionDuringCall < ButtonConf::BLF_IN_CALL_LIMITER)
			{
				this->blfActionDuringCall = blfActionDuringCall;
			}
		}

		{
			ButtonConf::BlfRemoteIdentityDisplay blfRemoteIdentityDisplay =
				static_cast<ButtonConf::BlfRemoteIdentityDisplay>(btnJson.get("blfRemoteIdentityDisplay", this->blfRemoteIdentityDisplay).asInt());
			if (blfRemoteIdentityDisplay >= 0 && blfRemoteIdentityDisplay < ButtonConf::BLF_REMOTE_IDENTITY_DISPLAY_LIMITER)
			{
				this->blfRemoteIdentityDisplay = blfRemoteIdentityDisplay;
			}
		}

		btnJson.getString("blfDtmfPrefixDuringCall", blfDtmfPrefixDuringCall);

		{
			const Json::Value &jblfOverrideIdle = btnJson["blfOverrideIdle"];
			jblfOverrideIdle.getBool("active", blfOverrideIdle.active);
			jblfOverrideIdle.getString("number", blfOverrideIdle.number);
		}

		{
			const Json::Value &jblfOverrideTerminated = btnJson["blfOverrideTerminated"];
			jblfOverrideTerminated.getBool("active", blfOverrideTerminated.active);
			jblfOverrideTerminated.getString("number", blfOverrideTerminated.number);
		}

		{
			const Json::Value &jblfOverrideEarly = btnJson["blfOverrideEarly"];
			jblfOverrideEarly.getBool("active", blfOverrideEarly.active);
			jblfOverrideEarly.getString("number", blfOverrideEarly.number);
		}

		{
			const Json::Value &jblfOverrideConfirmed = btnJson["blfOverrideConfirmed"];
			jblfOverrideConfirmed.getBool("active", blfOverrideConfirmed.active);
			jblfOverrideConfirmed.getString("number", blfOverrideConfirmed.number);
		}

		btnJson.getString("arg1", arg1);

		btnJson.getInt("sipCode", sipCode);
		btnJson.getString("sipReason", sipReason);
		btnJson.getInt("expires", expires);

		btnJson.getString("pagingTxWaveFile", pagingTxWaveFile);
		btnJson.getString("pagingTxCodec", pagingTxCodec);
		btnJson.getUInt("pagingTxPtime", pagingTxPtime);

		btnJson.getString("script", script);

		AudioModules::GetModuleFromJson(btnJson, "audioRxMod", AudioModules::DIR_INPUT, audioRxMod);
		btnJson.getString("audioRxDev", audioRxDev);
		AudioModules::GetModuleFromJson(btnJson, "audioTxMod", AudioModules::DIR_OUTPUT, audioTxMod);
		btnJson.getString("audioTxDev", audioTxDev);

		btnJson.getString("videoRxMod", videoRxMod);
		btnJson.getString("videoRxDev", videoRxDev);

		{
			const Json::Value &jfont = btnJson["font"];
			jfont.getString("name", font.name);
			jfont.getUInt("size", font.size);
			jfont.getBool("bold", font.bold);
			jfont.getBool("italic", font.italic);
			jfont.getBool("underline", font.underline);
			jfont.getBool("strikeout", font.strikeout);
		}

		{
			const Json::Value &jv = btnJson["fontLabel2"];
			ButtonConf::Font &font = fontLabel2;
			jv.getString("name", font.name);
			jv.getUInt("size", font.size);
			jv.getBool("bold", font.bold);
			jv.getBool("italic", font.italic);
			jv.getBool("underline", font.underline);
			jv.getBool("strikeout", font.strikeout);
		}
	}
	catch (const std::runtime_error &e)
	{
	}
}


void ButtonConf::ToJson(Json::Value &jsonBtn, bool saveAllSettings) const
{
	jsonBtn["parentId"] = parentId;
	jsonBtn["type"] = type;
	jsonBtn["caption"] = caption;
	jsonBtn["caption2"] = caption2;
	jsonBtn["captionLines"] = captionLines;
	jsonBtn["number"] = number;
	jsonBtn["visible"] = visible;
	if (saveAllSettings || (down != defaultBtn.down))
	{
		jsonBtn["down"] = down;
	}
	if (saveAllSettings || (inactive != defaultBtn.inactive))
	{
		jsonBtn["inactive"] = inactive;
	}
	jsonBtn["left"] = left;
	jsonBtn["top"] = top;
	jsonBtn["width"] = width;
	jsonBtn["height"] = height;
	if (saveAllSettings || (bevelWidth != defaultBtn.bevelWidth))
	{
		jsonBtn["bevelWidth"] = bevelWidth;
	}
	if (saveAllSettings || (customFrame != defaultBtn.customFrame))
	{
		jsonBtn["customFrame"] = customFrame;
	}
	if (saveAllSettings || (labelCenterHorizontally != defaultBtn.labelCenterHorizontally))
	{
		jsonBtn["labelCenterHorizontally"] = labelCenterHorizontally;
	}
	if (saveAllSettings || (labelCenterVertically != defaultBtn.labelCenterVertically))
	{
		jsonBtn["labelCenterVertically"] = labelCenterVertically;
	}
	if (saveAllSettings || (labelLeft != defaultBtn.labelLeft))
	{
		jsonBtn["labelLeft"] = labelLeft;
	}
	if (saveAllSettings || (labelTop != defaultBtn.labelTop))
	{
		jsonBtn["labelTop"] = labelTop;
	}
	if (saveAllSettings || (label2Left != defaultBtn.label2Left))
	{
		jsonBtn["label2Left"] = label2Left;
	}
	if (saveAllSettings || (label2Top != defaultBtn.label2Top))
	{
		jsonBtn["label2Top"] = label2Top;
	}
	if (saveAllSettings || (label2CenterHorizontally != defaultBtn.label2CenterHorizontally))
	{
		jsonBtn["label2CenterHorizontally"] = label2CenterHorizontally;
	}
	jsonBtn["spaceLabelsYEqually"] = spaceLabelsYEqually;

	Json::Value &jColors = jsonBtn["colors"];
	for (unsigned int el=0; el<sizeof(colors)/sizeof(colors[0]); el++)
	{
		Json::Value &jEl = jColors[el];
		if (saveAllSettings || (colors[el] != defaultBtn.colors[el]))
		{
			const ButtonConf::Color& color = colors[el];
			jEl["idle"] = color.idle;
			jEl["down"] = color.down;
			jEl["downPressed"] = color.downPressed;
			jEl["inactive"] = color.inactive;
			jEl["inactiveDown"] = color.inactiveDown;
		}
		else
		{
			jEl = Json::Value(Json::objectValue);
		}
	}

	if (saveAllSettings || (imageTransparent != defaultBtn.imageTransparent))
	{
		jsonBtn["imageTransparent"] = imageTransparent;
	}
	if (saveAllSettings || (imageLeft != defaultBtn.imageLeft))
	{
		jsonBtn["imageLeft"] = imageLeft;
	}
	if (saveAllSettings || (imageCenterVertically != defaultBtn.imageCenterVertically))
	{
		jsonBtn["imageCenterVertically"] = imageCenterVertically;
	}
	if (saveAllSettings || (imageTop != defaultBtn.imageTop))
	{
		jsonBtn["imageTop"] = imageTop;
	}

	jsonBtn["imgIdle"] = imgIdle;
	jsonBtn["imgTerminated"] = imgTerminated;
	jsonBtn["imgEarly"] = imgEarly;
	jsonBtn["imgConfirmed"] = imgConfirmed;
	if (saveAllSettings || (blfOverrideIdle != defaultBtn.blfOverrideIdle))
	{
		Json::Value &jv = jsonBtn["blfOverrideIdle"];
		jv["active"] = blfOverrideIdle.active;
		jv["number"] = blfOverrideIdle.number;
	}
	if (saveAllSettings || (blfOverrideTerminated != defaultBtn.blfOverrideTerminated))
	{
		Json::Value &jv = jsonBtn["blfOverrideTerminated"];
		jv["active"] = blfOverrideTerminated.active;
		jv["number"] = blfOverrideTerminated.number;
	}
	if (saveAllSettings || (blfOverrideEarly != defaultBtn.blfOverrideEarly))
	{
		Json::Value &jv = jsonBtn["blfOverrideEarly"];
		jv["active"] = blfOverrideEarly.active;
		jv["number"] = blfOverrideEarly.number;
	}
	if (saveAllSettings || (blfOverrideConfirmed != defaultBtn.blfOverrideConfirmed))
	{
		Json::Value &jv = jsonBtn["blfOverrideConfirmed"];
		jv["active"] = blfOverrideConfirmed.active;
		jv["number"] = blfOverrideConfirmed.number;
	}
	if (saveAllSettings || (blfActionDuringCall != defaultBtn.blfActionDuringCall))
	{
		jsonBtn["blfActionDuringCall"] = blfActionDuringCall;
	}
	if (saveAllSettings || (blfRemoteIdentityDisplay != defaultBtn.blfRemoteIdentityDisplay))
	{
		jsonBtn["blfRemoteIdentityDisplay"] = blfRemoteIdentityDisplay;
	}
	if (saveAllSettings || (blfDtmfPrefixDuringCall != defaultBtn.blfDtmfPrefixDuringCall))
	{
		jsonBtn["blfDtmfPrefixDuringCall"] = blfDtmfPrefixDuringCall;
	}
	if (saveAllSettings || (arg1 != defaultBtn.arg1))
	{
		jsonBtn["arg1"] = arg1;
	}
	if (saveAllSettings || (sipCode != defaultBtn.sipCode))
	{
		jsonBtn["sipCode"] = sipCode;
	}
	if (saveAllSettings || (sipReason != defaultBtn.sipReason))
	{
		jsonBtn["sipReason"] = sipReason;
	}
	if (saveAllSettings || (expires != defaultBtn.expires))
	{
		jsonBtn["expires"] = expires;
	}
	if (saveAllSettings || (pagingTxWaveFile != defaultBtn.pagingTxWaveFile))
	{
		jsonBtn["pagingTxWaveFile"] = pagingTxWaveFile;
	}
	if (saveAllSettings || (pagingTxCodec != defaultBtn.pagingTxCodec))
	{
		jsonBtn["pagingTxCodec"] = pagingTxCodec;
	}
	if (saveAllSettings || (pagingTxPtime != defaultBtn.pagingTxPtime))
	{
		jsonBtn["pagingTxPtime"] = pagingTxPtime;
	}
	if (saveAllSettings || (script != defaultBtn.script))
	{
		jsonBtn["script"] = script;
	}

	/*
		Since number of button types increases let's limit information types
		saved only to those related to particular button type.
	*/
	switch (type)
	{
	case Button::SWITCH_AUDIO_SOURCE:
		jsonBtn["audioRxMod"] = audioRxMod;
		jsonBtn["audioRxDev"] = audioRxDev;
		break;
	case Button::SWITCH_AUDIO_PLAYER:
		jsonBtn["audioTxMod"] = audioTxMod;
		jsonBtn["audioTxDev"] = audioTxDev;
		break;
	case Button::SWITCH_VIDEO_SOURCE:
		jsonBtn["videoRxMod"] = videoRxMod;
		jsonBtn["videoRxDev"] = videoRxDev;
		break;
	default:
		break;
	}

	{
		if (saveAllSettings || (font != defaultBtn.font))
		{
			Json::Value &jv = jsonBtn["font"];
			jv["name"] = font.name;
			jv["size"] = font.size;
			jv["bold"] = font.bold;
			jv["italic"] = font.italic;
			jv["underline"] = font.underline;
			jv["strikeout"] = font.strikeout;
		}
	}

	{
		if (saveAllSettings || (fontLabel2 != defaultBtn.fontLabel2))
		{
			Json::Value &jv = jsonBtn["fontLabel2"];
			jv["name"] = fontLabel2.name;
			jv["size"] = fontLabel2.size;
			jv["bold"] = fontLabel2.bold;
			jv["italic"] = fontLabel2.italic;
			jv["underline"] = fontLabel2.underline;
			jv["strikeout"] = fontLabel2.strikeout;
		}
	}
}

int ButtonConf::FromText(const char* jsonText)
{
	Json::Value root;   // will contains the root value after parsing.
	Json::Reader reader;

	bool parsingSuccessful = reader.parse( jsonText, root );
	if ( !parsingSuccessful )
	{
		return -1;
	}
	FromJson(root);
	return 0;
}

