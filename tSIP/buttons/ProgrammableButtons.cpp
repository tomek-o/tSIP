/** \note Configuration for buttons is seperated from main configuration
	to avoid rewriting it (~200kB for 180 buttons) every time application is closed.
	It can also be distributed separately (copied between instances or user) easier.
*/

#pragma hdrstop

#include "ProgrammableButtons.h"
#include "SettingsAppVersion.h"
#include "common/TimeCounter.h" 
#include <assert.h>
#include <algorithm>
#include <fstream> 
#include <json/json.h>
#include <Forms.hpp>

//---------------------------------------------------------------------------

#pragma package(smart_init)

namespace
{
	SettingsAppVersion appVersion;
}

void ProgrammableButtons::SetDefaultsForBtnId(int id, ButtonConf& cfg)
{
	enum { HEIGHT = 32 };

	if (id < 0 || id >= BASIC_PANEL_CONSOLE_BTNS + (EXT_CONSOLE_COLUMNS * CONSOLE_BTNS_PER_CONTAINER))
		return;
	if (id < BASIC_PANEL_CONSOLE_BTNS)
	{
		cfg.left = 2;
		cfg.height = HEIGHT;
		cfg.width = 73;
		cfg.top = id * HEIGHT;
	}
	else
	{
		int offset = id - BASIC_PANEL_CONSOLE_BTNS;
		enum { OFFSET_TOP = 4 };
		enum { FIRST_COLUMN_LEFT = 276 };
		enum { COLUMN_WIDTH = 104 };
		enum { COLUMN_SEPARATION = 4 };
		cfg.width = COLUMN_WIDTH;
		int columnId = offset / BASIC_PANEL_CONSOLE_BTNS;
		cfg.left = FIRST_COLUMN_LEFT + columnId * (COLUMN_WIDTH + COLUMN_SEPARATION);
		int offsetTop = (offset % BASIC_PANEL_CONSOLE_BTNS);
		cfg.top = OFFSET_TOP + (offsetTop * HEIGHT);
		if (offsetTop > 10)
		{
			// some empty space before buttons that should not be visible to make aligning easier
			cfg.top += 10;
		}
	}
}

ProgrammableButtons::ProgrammableButtons(void)
{
	btnConf.resize(BASIC_PANEL_CONSOLE_BTNS + (EXT_CONSOLE_COLUMNS * CONSOLE_BTNS_PER_CONTAINER));

	for (unsigned int i=0; i<btnConf.size(); i++)
	{
    	SetDefaultsForBtnId(i, btnConf[i]);
	}

	ButtonConf *cfg;

	cfg = &btnConf[0];
	cfg->caption = "    Redial";
	cfg->noIcon = true;
	cfg->type = Button::REDIAL;

	cfg = &btnConf[1];
	cfg->caption = "    FLASH";
	cfg->noIcon = true;
	cfg->type = Button::DTMF;
	cfg->number = "R";

	cfg = &btnConf[2];
	cfg->caption = "Hold";
	cfg->noIcon = false;
	cfg->type = Button::HOLD;
	cfg->imgIdle = "hold.bmp";

	cfg = &btnConf[3];
	cfg->caption = " Re-register";
	cfg->noIcon = true;
	cfg->type = Button::REREGISTER;

	cfg = &btnConf[4];
	cfg->captionLines = 2;
	cfg->caption = " Right click";
	cfg->caption2 = " to edit btn";
	cfg->labelCenterVertically = false;
	cfg->labelTop = 3;
    cfg->label2Left = 0;
	cfg->label2Top = 17;
	cfg->centerLabel2Horizontally = false;
	cfg->noIcon = true;

	for (unsigned int i=5; i<BASIC_PANEL_CONSOLE_BTNS; i++)
	{
		cfg = &btnConf[i];
		//cfg->top = 5 * HEIGHT;
		cfg->visible = false;
	}
}

int ProgrammableButtons::LoadFromJsonValue(const Json::Value &root)
{
	appVersion.FromJson(root["info"]);

	const Json::Value &btnConfJson = root["btnConf"];
	if (btnConfJson.type() == Json::arrayValue)
	{
		for (int i=0; i<btnConfJson.size(); i++)
		{
			if (i >= btnConf.size())
			{
				break;
			}
			const Json::Value &btnJson = btnConfJson[i];
			if (btnJson.type() != Json::objectValue)
			{
				continue;
			}

			try
			{
				ButtonConf &cfg = btnConf[i];

				Button::Type type = (Button::Type)btnJson.get("type", cfg.type).asInt();
				if (type >= 0 && type < Button::TYPE_LIMITER)
				{
					cfg.type = type;
				}
				cfg.caption = btnJson.get("caption", cfg.caption).asString();
				cfg.caption2 = btnJson.get("caption2", cfg.caption2).asString();
				int captionLines = btnJson.get("captionLines", cfg.captionLines).asInt();
				if (captionLines >= ButtonConf::CAPTION_LINES_MIN && cfg.captionLines <= ButtonConf::CAPTION_LINES_MAX)
				{
					cfg.captionLines = captionLines;
				}
				cfg.number = btnJson.get("number", cfg.number).asString();
				cfg.visible = btnJson.get("visible", cfg.visible).asBool();
				cfg.down = btnJson.get("down", cfg.down).asBool();
				cfg.inactive = btnJson.get("inactive", cfg.inactive).asBool();
				cfg.noIcon = btnJson.get("noIcon", cfg.noIcon).asBool();
				cfg.left = btnJson.get("left", cfg.left).asUInt();
				cfg.top = btnJson.get("top", cfg.top).asUInt();
				cfg.width = btnJson.get("width", cfg.width).asUInt();
				cfg.height = btnJson.get("height", cfg.height).asUInt();

				cfg.bevelWidth = btnJson.get("bevelWidth", cfg.bevelWidth).asUInt();
				cfg.customFrame = btnJson.get("customFrame", cfg.customFrame).asBool();
				cfg.centerTextHorizontally = btnJson.get("centerTextHorizontally", cfg.centerTextHorizontally).asBool();
				cfg.labelCenterVertically = btnJson.get("labelCenterVertically", cfg.labelCenterVertically).asBool();
				cfg.labelTop = btnJson.get("labelTop", cfg.labelTop).asInt();
				cfg.label2Left = btnJson.get("label2Left", cfg.label2Left).asInt();
				cfg.label2Top = btnJson.get("label2Top", cfg.label2Top).asInt();
				btnJson.getBool("spaceLabelsYEqually", cfg.spaceLabelsYEqually);

				cfg.centerLabel2Horizontally = btnJson.get("label2CenterHorizontally", cfg.centerLabel2Horizontally).asBool();
				cfg.imageTransparent = btnJson.get("imageTransparent", cfg.imageTransparent).asBool();
				cfg.imageLeft = btnJson.get("imageLeft", cfg.imageLeft).asInt();
				cfg.imageCenterVertically = btnJson.get("imageCenterVertically", cfg.imageCenterVertically).asBool();
				cfg.imageTop = btnJson.get("imageTop", cfg.imageTop).asInt();

				const Json::Value &colorsJson = btnJson["colors"];
				for (unsigned el=0; el < ButtonConf::EL_LIMITER; el++)
				{
					if (el >= colorsJson.size()) {
						break;
					}
					ButtonConf::Color &color = cfg.colors[el];
					const Json::Value &colorJson = colorsJson[el];
					color.idle = colorJson.get("idle", color.idle).asInt();
					color.down = colorJson.get("down", color.down).asInt();
					color.downPressed = colorJson.get("downPressed", color.downPressed).asInt();
					color.inactive = colorJson.get("inactive", color.inactive).asInt();
					color.inactiveDown = colorJson.get("inactiveDown", color.inactiveDown).asInt();
				}

				cfg.imgIdle = btnJson.get("imgIdle", cfg.imgIdle).asString();
				cfg.imgTerminated = btnJson.get("imgTerminated", cfg.imgTerminated).asString();
				cfg.imgEarly = btnJson.get("imgEarly", cfg.imgEarly).asString();
				cfg.imgConfirmed = btnJson.get("imgConfirmed", cfg.imgConfirmed).asString();

				ButtonConf::BlfActionDuringCall blfActionDuringCall =
					static_cast<ButtonConf::BlfActionDuringCall>(btnJson.get("blfActionDuringCall", cfg.blfActionDuringCall).asInt());
				if (blfActionDuringCall >= ButtonConf::BLF_IN_CALL_NONE && cfg.blfActionDuringCall < ButtonConf::BLF_IN_CALL_LIMITER)
				{
					cfg.blfActionDuringCall = blfActionDuringCall;
				}

				cfg.blfDtmfPrefixDuringCall = btnJson.get("blfDtmfPrefixDuringCall", cfg.blfDtmfPrefixDuringCall).asString();

				{
					const Json::Value &blfOverrideIdle = btnJson["blfOverrideIdle"];
					cfg.blfOverrideIdle.active = blfOverrideIdle.get("active", cfg.blfOverrideIdle.active).asBool();
					cfg.blfOverrideIdle.number = blfOverrideIdle.get("number", cfg.blfOverrideIdle.number).asString();
				}

				{
					const Json::Value &blfOverrideTerminated = btnJson["blfOverrideTerminated"];
					cfg.blfOverrideTerminated.active = blfOverrideTerminated.get("active", cfg.blfOverrideTerminated.active).asBool();
					cfg.blfOverrideTerminated.number = blfOverrideTerminated.get("number", cfg.blfOverrideTerminated.number).asString();
				}

				{
					const Json::Value &blfOverrideEarly = btnJson["blfOverrideEarly"];
					cfg.blfOverrideEarly.active = blfOverrideEarly.get("active", cfg.blfOverrideEarly.active).asBool();
					cfg.blfOverrideEarly.number = blfOverrideEarly.get("number", cfg.blfOverrideEarly.number).asString();
				}

				{
					const Json::Value &blfOverrideConfirmed = btnJson["blfOverrideConfirmed"];
					cfg.blfOverrideConfirmed.active = blfOverrideConfirmed.get("active", cfg.blfOverrideConfirmed.active).asBool();
					cfg.blfOverrideConfirmed.number = blfOverrideConfirmed.get("number", cfg.blfOverrideConfirmed.number).asString();
				}

				cfg.arg1 = btnJson.get("arg1", cfg.arg1).asString();

				btnJson.getInt("sipCode", cfg.sipCode);

				cfg.pagingTxWaveFile = btnJson.get("pagingTxWaveFile", cfg.pagingTxWaveFile).asString();
				cfg.pagingTxCodec = btnJson.get("pagingTxCodec", cfg.pagingTxCodec).asString();
				cfg.pagingTxPtime = btnJson.get("pagingTxPtime", cfg.pagingTxPtime).asUInt();

				cfg.script = btnJson.get("script", cfg.script).asString();

				cfg.audioRxMod = btnJson.get("audioRxMod", cfg.audioRxMod).asString();
				cfg.audioRxDev = btnJson.get("audioRxDev", cfg.audioRxDev).asString();
				cfg.audioTxMod = btnJson.get("audioTxMod", cfg.audioTxMod).asString();
				cfg.audioTxDev = btnJson.get("audioTxDev", cfg.audioTxDev).asString();

				{
					const Json::Value &font = btnJson["font"];
					cfg.font.name = font.get("name", cfg.font.name).asString();
					cfg.font.size = font.get("size", cfg.font.size).asUInt();
					cfg.font.bold = font.get("bold", cfg.font.bold).asBool();
					cfg.font.italic = font.get("italic", cfg.font.italic).asBool();
					cfg.font.underline = font.get("underline", cfg.font.underline).asBool();
				}

				{
					const Json::Value &jv = btnJson["fontLabel2"];
					ButtonConf::Font &font = cfg.fontLabel2;
					font.name = jv.get("name", font.name).asString();
					font.size = jv.get("size", font.size).asUInt();
					font.bold = jv.get("bold", font.bold).asBool();
					font.italic = jv.get("italic", font.italic).asBool();
					font.underline = jv.get("underline", font.underline).asBool();
				}
			}
			catch (const std::runtime_error &e)
			{
			}
		}
	}
	return 0;
}

int ProgrammableButtons::ReadFromString(AnsiString json)
{
	Json::Value root;   // will contains the root value after parsing.
	Json::Reader reader;

	bool parsingSuccessful = reader.parse( json.c_str(), root );
	if ( !parsingSuccessful )
	{
		return 2;
	}
	return LoadFromJsonValue(root);
}


int ProgrammableButtons::ReadFile(AnsiString name)
{
	Json::Value root;   // will contains the root value after parsing.
	Json::Reader reader;

	try
	{
		std::ifstream ifs(name.c_str());
		std::string strConfig((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
		ifs.close();
		bool parsingSuccessful = reader.parse( strConfig, root );
		if ( !parsingSuccessful )
		{
			return 2;
		}
	}
	catch(...)
	{
		return 1;
	}
	return LoadFromJsonValue(root);
}

int ProgrammableButtons::Read(void)
{
	TimeCounter tc("Reading buttons configuration");
	assert(filename != "");

	if (ReadFile(filename) == 0)
	{
		{
			// regular log might not work yet - use OutputDebugString
			char s[100];
			snprintf(s, sizeof(s), "%s  %.3f ms\n", tc.getName(), tc.getTimeMs());
			OutputDebugString(s);
		}
		return 0;
	}
	else
	{
		// earlier versions stored btn config in main file - try to read it
		AnsiString asConfigFile = ChangeFileExt( Application->ExeName, ".json" );
		int rc = ReadFile(asConfigFile);
		(void)rc;
		// and write new, separate file file (either default buttons or buttons from old "main" config,
		// if reading was successful)
		Write();
		return 0;
	}
	//notifyObservers();
}

int ProgrammableButtons::Write(void)
{
	TimeCounter tc("Writing buttons configuration");
	assert(filename != "");
    Json::Value root;
	Json::StyledWriter writer;

	const ButtonConf defaultBtn;

	{
		// update application version in settings
		appVersion.FromAppExe();
		appVersion.ToJson(root["info"]);
	}	

	// write buttons configuration
	Json::Value &jBtnConf = root["btnConf"];
	jBtnConf = Json::Value(Json::arrayValue);
	for (unsigned int i=0; i<btnConf.size(); i++)
	{
		const ButtonConf &cfg = btnConf[i];
		Json::Value &jsonBtn = jBtnConf.append(Json::objectValue);
		jsonBtn["type"] = cfg.type;
		jsonBtn["caption"] = cfg.caption;
		jsonBtn["caption2"] = cfg.caption2;
		jsonBtn["captionLines"] = cfg.captionLines;
		jsonBtn["number"] = cfg.number;
		jsonBtn["visible"] = cfg.visible;
		jsonBtn["down"] = cfg.down;
		jsonBtn["inactive"] = cfg.inactive;
		jsonBtn["noIcon"] = cfg.noIcon;
		jsonBtn["left"] = cfg.left;
		jsonBtn["top"] = cfg.top;
		jsonBtn["width"] = cfg.width;
		jsonBtn["height"] = cfg.height;
		jsonBtn["bevelWidth"] = cfg.bevelWidth;
		jsonBtn["customFrame"] = cfg.customFrame;
		jsonBtn["centerTextHorizontally"] = cfg.centerTextHorizontally;
		jsonBtn["labelCenterVertically"] = cfg.labelCenterVertically;
		jsonBtn["labelTop"] = cfg.labelTop;
		jsonBtn["label2Left"] = cfg.label2Left;
		jsonBtn["label2Top"] = cfg.label2Top;
		jsonBtn["label2CenterHorizontally"] = cfg.centerLabel2Horizontally;
		jsonBtn["spaceLabelsYEqually"] = cfg.spaceLabelsYEqually;

		jsonBtn["imageTransparent"] = cfg.imageTransparent;
		jsonBtn["imageLeft"] = cfg.imageLeft;
		jsonBtn["imageCenterVertically"] = cfg.imageCenterVertically;
		jsonBtn["imageTop"] = cfg.imageTop;

		Json::Value &jColors = jsonBtn["colors"];
		for (unsigned int el=0; el<sizeof(cfg.colors)/sizeof(cfg.colors[0]); el++)
		{
			Json::Value &jEl = jColors[el];
			if (cfg.colors[el] != defaultBtn.colors[el])
			{
				jEl["idle"] = cfg.colors[el].idle;
				jEl["down"] = cfg.colors[el].down;
				jEl["downPressed"] = cfg.colors[el].downPressed;
				jEl["inactive"] = cfg.colors[el].inactive;
				jEl["inactiveDown"] = cfg.colors[el].inactiveDown;
			}
			else
			{
				jEl = Json::Value(Json::objectValue);
			}
		}

		jsonBtn["imgIdle"] = cfg.imgIdle;
		jsonBtn["imgTerminated"] = cfg.imgTerminated;
		jsonBtn["imgEarly"] = cfg.imgEarly;
		jsonBtn["imgConfirmed"] = cfg.imgConfirmed;
		if (cfg.blfOverrideIdle != defaultBtn.blfOverrideIdle)
		{
			jsonBtn["blfOverrideIdle"]["active"] = cfg.blfOverrideIdle.active;
			jsonBtn["blfOverrideIdle"]["number"] = cfg.blfOverrideIdle.number;
		}
		if (cfg.blfOverrideTerminated != defaultBtn.blfOverrideTerminated)
		{
			jsonBtn["blfOverrideTerminated"]["active"] = cfg.blfOverrideTerminated.active;
			jsonBtn["blfOverrideTerminated"]["number"] = cfg.blfOverrideTerminated.number;
		}
		if (cfg.blfOverrideEarly != defaultBtn.blfOverrideEarly)
		{
			jsonBtn["blfOverrideEarly"]["active"] = cfg.blfOverrideEarly.active;
			jsonBtn["blfOverrideEarly"]["number"] = cfg.blfOverrideEarly.number;
		}
		if (cfg.blfOverrideConfirmed != defaultBtn.blfOverrideConfirmed)
		{
			jsonBtn["blfOverrideConfirmed"]["active"] = cfg.blfOverrideConfirmed.active;
			jsonBtn["blfOverrideConfirmed"]["number"] = cfg.blfOverrideConfirmed.number;
		}
		jsonBtn["blfActionDuringCall"] = cfg.blfActionDuringCall;
		jsonBtn["blfDtmfPrefixDuringCall"] = cfg.blfDtmfPrefixDuringCall;
		if (cfg.arg1 != defaultBtn.arg1)
		{
			jsonBtn["arg1"] = cfg.arg1;
		}
		jsonBtn["sipCode"] = cfg.sipCode;
		if (cfg.pagingTxWaveFile != defaultBtn.pagingTxWaveFile)
		{
			jsonBtn["pagingTxWaveFile"] = cfg.pagingTxWaveFile;
		}
		if (cfg.pagingTxCodec != defaultBtn.pagingTxCodec)
		{
			jsonBtn["pagingTxCodec"] = cfg.pagingTxCodec;
		}
		if (cfg.pagingTxPtime != defaultBtn.pagingTxPtime)
		{
			jsonBtn["pagingTxPtime"] = cfg.pagingTxPtime;
		}
		jsonBtn["script"] = cfg.script;

		/*
			Since number of button types increases let's limit information types
			saved only to those related to particular button type.
		*/
		switch (cfg.type)
		{
		case Button::SWITCH_AUDIO_SOURCE:
			jsonBtn["audioRxMod"] = cfg.audioRxMod;
			jsonBtn["audioRxDev"] = cfg.audioRxDev;
			break;
		case Button::SWITCH_AUDIO_PLAYER:
			jsonBtn["audioTxMod"] = cfg.audioTxMod;
			jsonBtn["audioTxDev"] = cfg.audioTxDev;
			break;
		default:
			break;
		}

		{
			const ButtonConf::Font &font = cfg.font;
			if (font != defaultBtn.font)
			{
				Json::Value &jv = jsonBtn["font"];
				jv["name"] = font.name;
				jv["size"] = font.size;
				jv["bold"] = font.bold;
				jv["italic"] = font.italic;
				jv["underline"] = font.underline;
			}
		}

		{
			const ButtonConf::Font &font = cfg.fontLabel2;
			if (font != defaultBtn.fontLabel2)
			{
				Json::Value &jv = jsonBtn["fontLabel2"];
				jv["name"] = font.name;
				jv["size"] = font.size;
				jv["bold"] = font.bold;
				jv["italic"] = font.italic;
				jv["underline"] = font.underline;
			}
		}
	}

	std::string outputConfig = writer.write( root );

	try
	{
		std::ofstream ofs(filename.c_str());
		ofs << outputConfig;
		ofs.close();
	}
	catch(...)
	{
    	return 1;
	}
		
	return 0;
}

void ProgrammableButtons::UpdateContacts(std::vector<UaConf::Contact> &contacts)
{
	contacts.clear();
	for (unsigned int btnId=0; btnId<btnConf.size(); btnId++)
	{
		class ButtonConf &cfg = btnConf[btnId];
		if (cfg.number != "")
		{
			if (cfg.type == Button::BLF)
			{
				bool duplicate = false;
				for (unsigned int i=0; i<contacts.size(); i++)
				{
					if (contacts[i].user == cfg.number && contacts[i].sub_dialog_info)
					{
						contacts[i].btnIds.push_back(btnId);
						duplicate = true;
						break;
					}
				}
				if (duplicate)
				{
					continue;
				}

				UaConf::Contact contact;
				contact.user = cfg.number;
				contact.sub_dialog_info = true;
				contact.btnIds.push_back(btnId);
				contacts.push_back(contact);
			}
			else if (cfg.type == Button::PRESENCE)
			{
				bool duplicate = false;
				for (unsigned int i=0; i<contacts.size(); i++)
				{
					if (contacts[i].user == cfg.number && contacts[i].sub_presence)
					{
						contacts[i].btnIds.push_back(btnId);
						duplicate = true;
						break;
					}
				}
				if (duplicate)
				{
					continue;
				}

				UaConf::Contact contact;
				contact.user = cfg.number;
				contact.sub_presence = true;
				contact.btnIds.push_back(btnId);
				contacts.push_back(contact);
			}
		}
	}
}