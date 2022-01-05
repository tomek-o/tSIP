/** \note Configuration for buttons is seperated from main configuration
	to avoid rewriting it (~200kB for 180 buttons) every time application is closed.
	It can also be distributed separately (copied between instances or user) easier.
*/

#pragma hdrstop

#include "ProgrammableButtons.h"
#include "ProgrammableButton.h"
#include "ButtonsDataModule.h"
#include "ButtonContainers.h"
#include "FormButtonContainer.h"
#include "FormButtonEdit.h"
#include "SettingsAppVersion.h"
#include "Settings.h"	// just for transition from column-based version < 0.2
#include "Sizes.h"
#include "common/TimeCounter.h" 
#include <assert.h>
#include <algorithm>
#include <fstream> 
#include <json/json.h>
#include <Forms.hpp>

//---------------------------------------------------------------------------

#pragma package(smart_init)

#define ARRAY_SIZE(a)    (sizeof(a)/sizeof(a[0]))

namespace
{
	SettingsAppVersion appVersion;
	bool IsShiftPressed(void)
	{
		int res = GetAsyncKeyState(VK_SHIFT);
		if (res & 0x8000)
			return true;
		return false;
	}	
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

		cfg.width = Sizes::COLUMN_WIDTH;
		int columnId = offset / BASIC_PANEL_CONSOLE_BTNS;
		cfg.left = Sizes::FIRST_COLUMN_LEFT + columnId * (Sizes::COLUMN_WIDTH + Sizes::COLUMN_SEPARATION);
		int offsetTop = (offset % BASIC_PANEL_CONSOLE_BTNS);
		cfg.top = Sizes::BUTTON_OFFSET_TOP + (offsetTop * HEIGHT);
		if (offsetTop > 10)
		{
			// some empty space before buttons that should not be visible to make aligning easier
			cfg.top += 10;
		}
	}
}

ProgrammableButtons::ProgrammableButtons(void):
	dmButtons(NULL),
	saveAllSettings(true),
	updated(false),
	panelIsMoving(false),
	panelIsResizing(false),
	editedPanelId(-1),
	scalingPercentage(100)	
{
	btnConf.resize(BASIC_PANEL_CONSOLE_BTNS + (EXT_CONSOLE_COLUMNS * CONSOLE_BTNS_PER_CONTAINER));

	for (unsigned int i=0; i<btnConf.size(); i++)
	{
    	SetDefaultsForBtnId(i, btnConf[i]);
	}
}

int ProgrammableButtons::LoadFromJsonValue(const Json::Value &root)
{
	int status = -1;

	appVersion.FromJson(root["info"]);

	const Json::Value &btnConfJson = root["btnConf"];
	if (btnConfJson.type() == Json::arrayValue)
	{
        status = 0;
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

				btnJson.getInt("parentId", cfg.parentId);
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
				{
					// importing setting that was removed/replaced in version 0.2
					const Json::Value& jv = btnJson["noIcon"];
					if (jv.type() == Json::booleanValue)
					{
						if (jv.asBool())
						{
							cfg.labelLeft = 4;
						}
						else
						{
							cfg.labelLeft = 20;
						}
					}
				}
				cfg.number = btnJson.get("number", cfg.number).asString();
				cfg.visible = btnJson.get("visible", cfg.visible).asBool();
				cfg.down = btnJson.get("down", cfg.down).asBool();
				cfg.inactive = btnJson.get("inactive", cfg.inactive).asBool();
				cfg.left = btnJson.get("left", cfg.left).asUInt();
				cfg.top = btnJson.get("top", cfg.top).asUInt();
				cfg.width = btnJson.get("width", cfg.width).asUInt();
				cfg.height = btnJson.get("height", cfg.height).asUInt();

				cfg.bevelWidth = btnJson.get("bevelWidth", cfg.bevelWidth).asUInt();
				cfg.customFrame = btnJson.get("customFrame", cfg.customFrame).asBool();
				cfg.labelCenterHorizontally = btnJson.get("labelCenterHorizontally", cfg.labelCenterHorizontally).asBool();
				cfg.labelCenterVertically = btnJson.get("labelCenterVertically", cfg.labelCenterVertically).asBool();
				btnJson.getInt("labelLeft", cfg.labelLeft);
				cfg.labelTop = btnJson.get("labelTop", cfg.labelTop).asInt();
				cfg.label2Left = btnJson.get("label2Left", cfg.label2Left).asInt();
				cfg.label2Top = btnJson.get("label2Top", cfg.label2Top).asInt();
				btnJson.getBool("spaceLabelsYEqually", cfg.spaceLabelsYEqually);

				btnJson.getBool("label2CenterHorizontally", cfg.label2CenterHorizontally);
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
				btnJson.getString("sipReason", cfg.sipReason);
				btnJson.getInt("expires", cfg.expires);

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

	{
		// dealing with transition to version 0.2.00.00 - new console configuration
		// trying to keep previous column width and number
		SettingsAppVersion ver0p2;
		ver0p2.FileVersionMS = 2;
		ver0p2.FileVersionLS = 0;

		// first version with programmable buttons on whole dialpad area
		SettingsAppVersion ver0p2p1;
		ver0p2p1.FileVersionMS = 2;
		ver0p2p1.FileVersionLS = 65535;

		if (appVersion < ver0p2)
		{
			if (appSettings.frmMain.pre0p2speedDialWidth.size() > 0)
			{
				int left = Sizes::FIRST_COLUMN_LEFT;
				int btnId = BASIC_PANEL_CONSOLE_BTNS;
				for (int widthId = 0; widthId < appSettings.frmMain.pre0p2speedDialWidth.size(); widthId++)
				{
					int width = appSettings.frmMain.pre0p2speedDialWidth[widthId];
					for (int i=0; i<BASIC_PANEL_CONSOLE_BTNS; i++)
					{
						btnConf[btnId].left = left;
						btnConf[btnId].width = width;
						btnId++;
					}
					left += width + Sizes::COLUMN_SEPARATION;
				}
				if (btnId < btnConf.size() - 1)
				{
					int offset = btnConf[btnId].left - left;
					for (btnId; btnId < btnConf.size(); btnId++)
					{
						btnConf[btnId].left -= offset;
					}
				}
				updated = true;
			}
		}

		if (appVersion < ver0p2p1)
		{
			SetInitialDialpad();
			for (unsigned int i=0; i<BASIC_PANEL_CONSOLE_BTNS; i++)
			{
				ButtonConf &cfg = btnConf[i];
				cfg.left += 184;
				cfg.parentId = 0;
			}
		}
	}

	return status;
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

	// assume this is newest configuration version if version is not present in text
	if (root["info"].type() == Json::nullValue)
	{
		SettingsAppVersion appVersion;
		appVersion.FromAppExe();
		appVersion.ToJson(root["info"]);
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
		if (updated)
		{
			Write();
			updated = false;
		}

		return 0;
	}
	else
	{
		// earlier versions stored btn config in main file - try to read it
		AnsiString asConfigFile = ChangeFileExt( Application->ExeName, ".json" );
		int rc = ReadFile(asConfigFile);
		if (rc != 0)
		{
			SetInitialSettings();
		}
		// and write new, separate file (either default buttons or buttons from old "main" config,
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
		jsonBtn["parentId"] = cfg.parentId;
		jsonBtn["type"] = cfg.type;
		jsonBtn["caption"] = cfg.caption;
		jsonBtn["caption2"] = cfg.caption2;
		jsonBtn["captionLines"] = cfg.captionLines;
		jsonBtn["number"] = cfg.number;
		jsonBtn["visible"] = cfg.visible;
		if (saveAllSettings || (cfg.down != defaultBtn.down))
		{
			jsonBtn["down"] = cfg.down;
		}
		if (saveAllSettings || (cfg.inactive != defaultBtn.inactive))
		{
			jsonBtn["inactive"] = cfg.inactive;
		}
		jsonBtn["left"] = cfg.left;
		jsonBtn["top"] = cfg.top;
		jsonBtn["width"] = cfg.width;
		jsonBtn["height"] = cfg.height;
		if (saveAllSettings || (cfg.bevelWidth != defaultBtn.bevelWidth))
		{
			jsonBtn["bevelWidth"] = cfg.bevelWidth;
		}
		if (saveAllSettings || (cfg.customFrame != defaultBtn.customFrame))
		{
			jsonBtn["customFrame"] = cfg.customFrame;
		}
		if (saveAllSettings || (cfg.labelCenterHorizontally != defaultBtn.labelCenterHorizontally))
		{
			jsonBtn["labelCenterHorizontally"] = cfg.labelCenterHorizontally;
		}
		if (saveAllSettings || (cfg.labelCenterVertically != defaultBtn.labelCenterVertically))
		{
			jsonBtn["labelCenterVertically"] = cfg.labelCenterVertically;
		}
		if (saveAllSettings || (cfg.labelLeft != defaultBtn.labelLeft))
		{
			jsonBtn["labelLeft"] = cfg.labelLeft;
		}
		if (saveAllSettings || (cfg.labelTop != defaultBtn.labelTop))
		{
			jsonBtn["labelTop"] = cfg.labelTop;
		}
		if (saveAllSettings || (cfg.label2Left != defaultBtn.label2Left))
		{
			jsonBtn["label2Left"] = cfg.label2Left;
		}
		if (saveAllSettings || (cfg.label2Top != defaultBtn.label2Top))
		{
			jsonBtn["label2Top"] = cfg.label2Top;
		}
		if (saveAllSettings || (cfg.label2CenterHorizontally != defaultBtn.label2CenterHorizontally))
		{
			jsonBtn["label2CenterHorizontally"] = cfg.label2CenterHorizontally;
		}
		jsonBtn["spaceLabelsYEqually"] = cfg.spaceLabelsYEqually;

		Json::Value &jColors = jsonBtn["colors"];
		for (unsigned int el=0; el<sizeof(cfg.colors)/sizeof(cfg.colors[0]); el++)
		{
			Json::Value &jEl = jColors[el];
			if (saveAllSettings || (cfg.colors[el] != defaultBtn.colors[el]))
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

		if (saveAllSettings || (cfg.imageTransparent != defaultBtn.imageTransparent))
		{
			jsonBtn["imageTransparent"] = cfg.imageTransparent;
		}
		if (saveAllSettings || (cfg.imageLeft != defaultBtn.imageLeft))
		{
			jsonBtn["imageLeft"] = cfg.imageLeft;
		}
		if (saveAllSettings || (cfg.imageCenterVertically != defaultBtn.imageCenterVertically))
		{
			jsonBtn["imageCenterVertically"] = cfg.imageCenterVertically;
		}
		if (saveAllSettings || (cfg.imageTop != defaultBtn.imageTop))
		{
			jsonBtn["imageTop"] = cfg.imageTop;
		}

		jsonBtn["imgIdle"] = cfg.imgIdle;
		jsonBtn["imgTerminated"] = cfg.imgTerminated;
		jsonBtn["imgEarly"] = cfg.imgEarly;
		jsonBtn["imgConfirmed"] = cfg.imgConfirmed;
		if (saveAllSettings || (cfg.blfOverrideIdle != defaultBtn.blfOverrideIdle))
		{
			jsonBtn["blfOverrideIdle"]["active"] = cfg.blfOverrideIdle.active;
			jsonBtn["blfOverrideIdle"]["number"] = cfg.blfOverrideIdle.number;
		}
		if (saveAllSettings || (cfg.blfOverrideTerminated != defaultBtn.blfOverrideTerminated))
		{
			jsonBtn["blfOverrideTerminated"]["active"] = cfg.blfOverrideTerminated.active;
			jsonBtn["blfOverrideTerminated"]["number"] = cfg.blfOverrideTerminated.number;
		}
		if (saveAllSettings || (cfg.blfOverrideEarly != defaultBtn.blfOverrideEarly))
		{
			jsonBtn["blfOverrideEarly"]["active"] = cfg.blfOverrideEarly.active;
			jsonBtn["blfOverrideEarly"]["number"] = cfg.blfOverrideEarly.number;
		}
		if (saveAllSettings || (cfg.blfOverrideConfirmed != defaultBtn.blfOverrideConfirmed))
		{
			jsonBtn["blfOverrideConfirmed"]["active"] = cfg.blfOverrideConfirmed.active;
			jsonBtn["blfOverrideConfirmed"]["number"] = cfg.blfOverrideConfirmed.number;
		}
		if (saveAllSettings || (cfg.blfActionDuringCall != defaultBtn.blfActionDuringCall))
		{
			jsonBtn["blfActionDuringCall"] = cfg.blfActionDuringCall;
		}
		if (saveAllSettings || (cfg.blfDtmfPrefixDuringCall != defaultBtn.blfDtmfPrefixDuringCall))
		{
			jsonBtn["blfDtmfPrefixDuringCall"] = cfg.blfDtmfPrefixDuringCall;
		}
		if (saveAllSettings || (cfg.arg1 != defaultBtn.arg1))
		{
			jsonBtn["arg1"] = cfg.arg1;
		}
		if (saveAllSettings || (cfg.sipCode != defaultBtn.sipCode))
		{
			jsonBtn["sipCode"] = cfg.sipCode;
		}
		if (saveAllSettings || (cfg.sipReason != defaultBtn.sipReason))
		{
			jsonBtn["sipReason"] = cfg.sipReason;
		}
		if (saveAllSettings || (cfg.expires != defaultBtn.expires))
		{
        	jsonBtn["expires"] = cfg.expires;
		}
		if (saveAllSettings || (cfg.pagingTxWaveFile != defaultBtn.pagingTxWaveFile))
		{
			jsonBtn["pagingTxWaveFile"] = cfg.pagingTxWaveFile;
		}
		if (saveAllSettings || (cfg.pagingTxCodec != defaultBtn.pagingTxCodec))
		{
			jsonBtn["pagingTxCodec"] = cfg.pagingTxCodec;
		}
		if (saveAllSettings || (cfg.pagingTxPtime != defaultBtn.pagingTxPtime))
		{
			jsonBtn["pagingTxPtime"] = cfg.pagingTxPtime;
		}
		if (saveAllSettings || (cfg.script != defaultBtn.script))
		{
			jsonBtn["script"] = cfg.script;
		}

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
			if (saveAllSettings || (font != defaultBtn.font))
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
			if (saveAllSettings || (font != defaultBtn.fontLabel2))
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
				contact.sub_dialog_info_expires = cfg.expires;
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
				contact.sub_presence_expires = cfg.expires;				
				contact.btnIds.push_back(btnId);
				contacts.push_back(contact);
			}
		}
	}
}

void ProgrammableButtons::SetSaveAllSettings(bool state)
{
	if (saveAllSettings != state)
	{
		saveAllSettings = state;
		if (saveAllSettings && filename != "")
		{
        	Write();
		}
	}
}

void ProgrammableButtons::SetInitialSettings(void)
{
	ButtonConf *cfg;
	enum { LEFT = 184 };
	enum { WIDTH = 73 };
	enum { HEIGHT = 32 };

	for (unsigned int i=0; i<BASIC_PANEL_CONSOLE_BTNS; i++)
	{
		cfg = &btnConf[i];
		cfg->parentId = 0;
		cfg->left = LEFT;
		cfg->width = WIDTH;
		cfg->height = HEIGHT;
	}

	int top = 3;

	cfg = &btnConf[0];
	cfg->caption = "    Redial";
	cfg->labelLeft = 0;
	cfg->type = Button::REDIAL;
	cfg->top = top;

	cfg = &btnConf[1];
	cfg->caption = "    FLASH";
	cfg->labelLeft = 0;
	cfg->type = Button::DTMF;
	cfg->number = "R";
	top += HEIGHT;
	cfg->top = top;

	cfg = &btnConf[2];
	cfg->caption = "Hold";
	cfg->type = Button::HOLD;
	cfg->imgIdle = "hold.bmp";
	top += HEIGHT;
	cfg->top = top;

	cfg = &btnConf[3];
	cfg->caption = " Re-register";
	cfg->labelLeft = 4;
	cfg->type = Button::REREGISTER;
	top += HEIGHT;
	cfg->top = top;

	cfg = &btnConf[4];
	cfg->captionLines = 2;
	cfg->caption = " Right click";
	cfg->caption2 = " to edit btn";
	cfg->labelCenterVertically = false;
	cfg->labelLeft = 4;
	cfg->labelTop = 3;
	cfg->label2Left = 4;
	cfg->label2Top = 17;
	cfg->label2CenterHorizontally = false;
	top += HEIGHT;
	cfg->top = top;

	for (unsigned int i=5; i<BASIC_PANEL_CONSOLE_BTNS; i++)
	{
		cfg = &btnConf[i];
		//cfg->top = 5 * HEIGHT;
		cfg->visible = false;
	}

	SetInitialDialpad();
}

void ProgrammableButtons::SetInitialDialpad(void)
{
	enum { START_BTN = 200 };
	enum { BTN_CNT = 12 };
	enum { WIDTH = 40 };
	enum { HEIGHT = 40 };
	assert(START_BTN + BTN_CNT < GetTotalCnt());
	for (unsigned int i=START_BTN; i<START_BTN + BTN_CNT; i++)
	{
		ButtonConf &cfg = btnConf[i];
		cfg.parentId = 0;
		cfg.visible = true;
		cfg.width = WIDTH;
		cfg.height = HEIGHT;
		cfg.type = Button::DTMF;
		cfg.font.size = 22;
		cfg.labelCenterHorizontally = true;
	}

	unsigned int id = START_BTN;
	ButtonConf *cfg;

	enum { LEFT1 = 4, LEFT2 = 51, LEFT3 = 99 };
	enum { TOP1 = 3, TOP2 = 46, TOP3 = 90, TOP4 = 134 };

	cfg = &btnConf[id++];
	cfg->caption = cfg->number = "1";
	cfg->left = LEFT1;
	cfg->top = TOP1;

	cfg = &btnConf[id++];
	cfg->caption = cfg->number = "2";
	cfg->left = LEFT2;
	cfg->top = TOP1;

	cfg = &btnConf[id++];
	cfg->caption = cfg->number = "3";
	cfg->left = LEFT3;
	cfg->top = TOP1;

	cfg = &btnConf[id++];
	cfg->caption = cfg->number = "4";
	cfg->left = LEFT1;
	cfg->top = TOP2;

	cfg = &btnConf[id++];
	cfg->caption = cfg->number = "5";
	cfg->left = LEFT2;
	cfg->top = TOP2;

	cfg = &btnConf[id++];
	cfg->caption = cfg->number = "6";
	cfg->left = LEFT3;
	cfg->top = TOP2;

	cfg = &btnConf[id++];
	cfg->caption = cfg->number = "7";
	cfg->left = LEFT1;
	cfg->top = TOP3;

	cfg = &btnConf[id++];
	cfg->caption = cfg->number = "8";
	cfg->left = LEFT2;
	cfg->top = TOP3;

	cfg = &btnConf[id++];
	cfg->caption = cfg->number = "9";
	cfg->left = LEFT3;
	cfg->top = TOP3;

	cfg = &btnConf[id++];
	cfg->caption = cfg->number = "*";
	cfg->left = LEFT1;
	cfg->top = TOP4;

	cfg = &btnConf[id++];
	cfg->caption = cfg->number = "0";
	cfg->left = LEFT2;
	cfg->top = TOP4;

	cfg = &btnConf[id++];
	cfg->caption = cfg->number = "#";
	cfg->left = LEFT3;
	cfg->top = TOP4;


}

TfrmButtonContainer* ProgrammableButtons::GetBtnContainer(int btnId)
{
    assert(btnId >= 0 && btnId < btnConf.size());
	const ButtonConf &cfg = btnConf[btnId];
	if (cfg.parentId <= ARRAY_SIZE(frmButtonContainers))
	{
		return frmButtonContainers[cfg.parentId];
	}
	else
	{
		assert(ARRAY_SIZE(frmButtonContainers) < ButtonConf::DEFAULT_PARENT_ID);
		return frmButtonContainers[ButtonConf::DEFAULT_PARENT_ID];
	}
}


void ProgrammableButtons::Create(TComponent* Owner,
		int scalingPercentage,
		CallbackClick callbackClick,
		CallbackMouseUpDown callbackMouseUpDown,
		CallbackSetKeepForeground callbackSetKeepForeground,
		CallbackRestartUa callbackRestartUa
		)
{
	assert(dmButtons == NULL);
	dmButtons = new TdmButtons(NULL);
	this->callbackClick = callbackClick;
	this->callbackSetKeepForeground = callbackSetKeepForeground;
	this->callbackMouseUpDown = callbackMouseUpDown;
	this->callbackRestartUa = callbackRestartUa;
	for (unsigned int i=0; i<ARRAY_SIZE(frmButtonContainers); i++)
	{
		TfrmButtonContainer *frm = frmButtonContainers[i];
		frm->OnKeyPress = FormKeyPress;
		frm->imgBackground->OnClick = containerBackgroundClick;
		frm->panelMain->OnClick = containerBackgroundClick;
	}
	for (unsigned int i=0; i<btnConf.size(); i++)
	{
		TProgrammableButton *panel = new TProgrammableButton(Owner, dmButtons->imgList, scalingPercentage);
		panel->Tag = i;
		//panel->AlignWithMargins = true;
		if (i < btnConf.size())
		{
			panel->Parent = GetBtnContainer(i)->GetButtonParent();
			const ButtonConf &cfg = btnConf[i];			
			panel->SetConfig(cfg);
		}
		panel->OnClick = SpeedDialPanelClick;
		panel->OnDblClick = SpeedDialPanelClick;
		panel->SetMouseUpDownCallback(OnPanelMouseUpDown);
		panel->UpdateCallbacks();
		btns.push_back(panel);
	}

	dmButtons->tmrMoving->OnTimer = this->tmrMovingTimer;
}

void ProgrammableButtons::Destroy(void)
{
	for (unsigned int i=0; i<btns.size(); i++)
	{
		delete btns[i];
	}
	btns.clear();
	if (dmButtons)
	{
		delete dmButtons;
		dmButtons = NULL;
	}
}

void __fastcall ProgrammableButtons::containerBackgroundClick(TObject *Sender)
{
	if (panelIsMoving || panelIsResizing)
	{
    	SpeedDialPanelClick(Sender);
	}
}

void __fastcall ProgrammableButtons::SpeedDialPanelClick(TObject *Sender)
{
	if (panelIsMoving)
	{
		panelIsMoving = false;

		TfrmButtonContainer *container = GetBtnContainer(editedPanelId);

		container->imgBackground->Cursor = crDefault;

		TPoint P = container->ScreenToClient(Mouse->CursorPos);
		if (P.x < 0)
			P.x = 0;
		if (P.y < 0)
			P.y = 0;
		ButtonConf cfg = btnConf[editedPanelId];	// copy
		cfg.left = P.x * 100/scalingPercentage;
		cfg.top = P.y * 100/scalingPercentage;

		if (!IsShiftPressed() && appSettings.frmSpeedDial.useGrid)
		{
			int grid = appSettings.frmSpeedDial.gridSize;
			int modX = cfg.left % grid;
			if (modX >= grid/2)
				cfg.left += grid - modX;
			else
				cfg.left -= modX;
			int modY = cfg.top % grid;
			if (modY >= grid/2)
				cfg.top += grid - modY;
			else
				cfg.top -= modY;
		}

		ApplyButtonCfg(editedPanelId, cfg);
	}
	else if (panelIsResizing)
	{
		panelIsResizing = false;
		TfrmButtonContainer *container = GetBtnContainer(editedPanelId);
		container->imgBackground->Cursor = crDefault;

		TPoint P = container->ScreenToClient(Mouse->CursorPos);
		if (P.x < 0)
			P.x = 0;
		if (P.y < 0)
			P.y = 0;
		ButtonConf cfg = btnConf[editedPanelId];	// copy
		cfg.width = P.x * 100/scalingPercentage - cfg.left;
		cfg.height = P.y * 100/scalingPercentage - cfg.top;

		if (cfg.width < 10 || cfg.height < 10)
			return;

		if (!IsShiftPressed() && appSettings.frmSpeedDial.useGrid)
		{
			int grid = appSettings.frmSpeedDial.gridSize;
			int modX = cfg.width % grid;
			if (modX >= grid/2)
				cfg.width += grid - modX;
			else
				cfg.width -= modX;
			int modY = cfg.height % grid;
			if (modY >= grid/2)
				cfg.height += grid - modY;
			else
				cfg.height -= modY;
		}

		ApplyButtonCfg(editedPanelId, cfg);
	}
	else
	{
		TProgrammableButton* panel = dynamic_cast<TProgrammableButton*>(Sender);
		if (panel == NULL)
		{
			TComponent *component = dynamic_cast<TComponent*>(Sender);
			assert(component);
			panel = dynamic_cast<TProgrammableButton*>(component->Owner);
		}
		assert(panel);
		if (panel->GetInactive() == true)
		{
			return;
        }
		int id = panel->Tag;

		ButtonConf &cfg = btnConf[id];
		if (cfg.type == Button::HOLD || cfg.type == Button::MUTE || cfg.type == Button::MUTE_RING)
		{
			panel->SetDown(!panel->GetDown());
		}

		callbackClick(id, panel);
	}
}

void ProgrammableButtons::OnPanelMouseUpDown(TProgrammableButton *btn)
{
	if (panelIsMoving || panelIsResizing)
	{
		return;
	}
	int id = btn->Tag;
	callbackMouseUpDown(id, btn);	
}

void ProgrammableButtons::UseContextMenu(bool state)
{
	for (unsigned int i=0; i<btns.size(); i++)
	{
		btns[i]->PopupMenu = state ? dmButtons->popupPanel : NULL;
	}
}

void ProgrammableButtons::ApplyButtonCfg(int id, const ButtonConf &cfg)
{
	if (cfg != btnConf[id])
	{
		bool restartUa = false;
		if (cfg.UaRestartNeeded(btnConf[id]))
		{
			restartUa = true;
		}

		btnConf[id] = cfg;
		TProgrammableButton* panel = btns[id];
		panel->Parent = GetBtnContainer(id)->GetButtonParent();
		panel->SetConfig(cfg);
		if (cfg.type != Button::BLF)
		{
			panel->SetState(DIALOG_INFO_UNKNOWN, true, DIALOG_INFO_DIR_UNKNOWN, "", "");	// make sure BLF icon is cleared
		}
		if (cfg.type != Button::PRESENCE)
		{
			panel->ClearPresenceState();			// clear icon
		}
		Write();

		if (restartUa && callbackRestartUa)
		{
        	callbackRestartUa();
		}
	}
	else
	{
		// assign anyway - some fields may be unintentionally omited from != operator
		btnConf[id] = cfg;
		Write();
	}
}

void ProgrammableButtons::UpdateDlgInfoState(int id, int state, bool updateRemoteIdentity, int direction, AnsiString remoteIdentity, AnsiString remoteIdentityDisplay)
{
	if (id < 0 || id >= btns.size())
		return;
	btns[id]->SetState((enum dialog_info_status)state, updateRemoteIdentity, (enum dialog_info_direction)direction, remoteIdentity, remoteIdentityDisplay);
}

void ProgrammableButtons::UpdatePresenceState(int id, int state, AnsiString note)
{
	if (id < 0 || id >= btns.size())
		return;
	btns[id]->SetPresenceState((enum presence_status)state, note);
}

void ProgrammableButtons::UpdateBtnState(Button::Type type, bool state)
{
	for (unsigned int i=0; i<btns.size(); i++)
	{
		const ButtonConf &cfg = btnConf[i];
		if (cfg.type == type)
		{
			btns[i]->SetDown(state);
		}
	}
}

void ProgrammableButtons::UpdateMwiState(int newMsg, int oldMsg)
{
	for (unsigned int i=0; i<btns.size(); i++)
	{
		const ButtonConf &cfg = btnConf[i];
		if (cfg.type == Button::MWI)
		{
			btns[i]->SetMwiState(newMsg, oldMsg);
		}
	}
}

void ProgrammableButtons::UpdateAutoAnswer(bool enabled, int sipCode, AnsiString sipReason)
{
	for (unsigned int i=0; i<btns.size(); i++)
	{
		const ButtonConf &cfg = btnConf[i];
		if (cfg.type == Button::AUTO_ANSWER_DND)
		{
			if (cfg.sipCode == sipCode && cfg.sipReason == sipReason.c_str() && enabled)
			{
				btns[i]->SetDown(true);
			}
			else
			{
				btns[i]->SetDown(false);
			}
		}
	}
}

void ProgrammableButtons::Edit(int id)
{
	ButtonConf cfg = btnConf[id];	// copy
	AnsiString caption;
	caption.sprintf("Edit button #%02d", id);
	frmButtonEdit->Caption = caption;
	
	callbackSetKeepForeground(false);
	frmButtonEdit->ShowModal(&cfg, id);
	callbackSetKeepForeground(true);	

	if (frmButtonEdit->isConfirmed())
	{
		ApplyButtonCfg(id, cfg);
		TfrmButtonContainer *container = GetBtnContainer(id);
		container->Repaint();
	}
}

void ProgrammableButtons::Move(int id)
{
	editedPanelId = id;
	panelIsMoving = true;
	TfrmButtonContainer *container = GetBtnContainer(editedPanelId);
	container->imgBackground->Cursor = crCross;
	dmButtons->tmrMoving->Enabled = true;
}

void ProgrammableButtons::Resize(int id)
{
	editedPanelId = id;
	panelIsResizing = true;
	TfrmButtonContainer *container = GetBtnContainer(editedPanelId);
	container->imgBackground->Cursor = crCross;
	dmButtons->tmrMoving->Enabled = true;	
}

void ProgrammableButtons::UpdateAll(void)
{
	for (int id = 0; id < btnConf.size(); id++)
	{
		if (id >= btns.size())
			break;
		TProgrammableButton* panel = btns[id];
		panel->Parent = GetBtnContainer(id)->GetButtonParent();		
		panel->SetConfig(btnConf[id]);
	}
	for (unsigned int i=0; i<ARRAY_SIZE(frmButtonContainers); i++)
	{
		frmButtonContainers[i]->Repaint();
	}
}

void ProgrammableButtons::SetConfig(int btnId, const ButtonConf &conf)
{
	TProgrammableButton* btn = GetBtn(btnId);
	if (btn)
	{
		btn->Parent = GetBtnContainer(btnId)->GetButtonParent();		
		btn->SetConfig(conf);
	}
}

void __fastcall ProgrammableButtons::tmrMovingTimer(TObject *Sender)
{
	if (!panelIsMoving && !panelIsResizing)
	{
		dmButtons->tmrMoving->Enabled = false;
		TfrmButtonContainer *container = GetBtnContainer(editedPanelId);
		container->movingFrame->Visible = false;
		return;
	}

	TfrmButtonContainer *container = GetBtnContainer(editedPanelId);

	{
		SHORT state = GetAsyncKeyState( VK_ESCAPE );
		// Test high bit - if set, key was down when GetAsyncKeyState was called.
		if( ( 1 << 15 ) & state )
		{
			EndEditing(container);
			return;
		}
	}

	const ButtonConf &cfg = btnConf[editedPanelId];	// copy

	AnsiString text;
	AnsiString caption = "[unnamed]";
	if (cfg.caption != "")
		caption = cfg.caption.c_str();
	text.sprintf("#%02d: %s", editedPanelId, caption.Trim().c_str());
	container->movingFrame->Caption = text;

	container->movingFrame->Visible = true;
	TPoint P = container->ScreenToClient(Mouse->CursorPos);
	if (P.x < 0)
		P.x = 0;
	if (P.y < 0)
		P.y = 0;

	if (panelIsMoving)
	{
		int left = P.x * 100/scalingPercentage;
		int top = P.y * 100/scalingPercentage;

		if (!IsShiftPressed() && appSettings.frmSpeedDial.useGrid)
		{
			int grid = appSettings.frmSpeedDial.gridSize;
			int modX = left % grid;
			if (modX >= grid/2)
				left += grid - modX;
			else
				left -= modX;
			int modY = top % grid;
			if (modY >= grid/2)
				top += grid - modY;
			else
				top -= modY;
		}
		container->movingFrame->Top = top;
		container->movingFrame->Left = left;
		container->movingFrame->Width = cfg.width;
		container->movingFrame->Height = cfg.height;
		container->movingFrame->BringToFront();
	}
	else if (panelIsResizing)
	{
		int width = P.x * 100/scalingPercentage - cfg.left;
		int height = P.y * 100/scalingPercentage - cfg.top;

		if (width < 10 || height < 10)
			return;

		if (!IsShiftPressed() && appSettings.frmSpeedDial.useGrid)
		{
			int grid = appSettings.frmSpeedDial.gridSize;
			int modX = width % grid;
			if (modX >= grid/2)
				width += grid - modX;
			else
				width -= modX;
			int modY = height % grid;
			if (modY >= grid/2)
				height += grid - modY;
			else
				height -= modY;
		}
		container->movingFrame->Top = cfg.top;
		container->movingFrame->Left = cfg.left;
		container->movingFrame->Width = width;
		container->movingFrame->Height = height;
		container->movingFrame->BringToFront();
	}
}

void __fastcall ProgrammableButtons::FormKeyPress(TObject *Sender, char &Key)
{
	if (Key == VK_ESCAPE)
	{
		TfrmButtonContainer *container = dynamic_cast<TfrmButtonContainer*>(Sender);
		EndEditing(container);
	}
}

void ProgrammableButtons::EndEditing(TfrmButtonContainer *container)
{
	if (panelIsMoving || panelIsResizing)
	{
		panelIsMoving = false;
		panelIsResizing = false;

		assert(container);
		container->imgBackground->Cursor = crDefault;
	}
}


