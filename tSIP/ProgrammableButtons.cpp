/** \note Configuration for buttons is seperated from main configuration
	to avoid rewriting it (~200kB for 180 buttons) every time application is closed.
	It can also be distributed separately (copied between instances or user) easier.
*/

#pragma hdrstop

#include "ProgrammableButtons.h"
#include <assert.h>
#include <algorithm>
#include <fstream> 
#include <json/json.h>
#include <Forms.hpp>

//---------------------------------------------------------------------------

#pragma package(smart_init)

void ProgrammableButtons::SetDefault(void)
{
	btnConf.resize((1 /*basic column*/ + EXT_CONSOLE_COLUMNS) * CONSOLE_BTNS_PER_COLUMN);

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
	cfg->marginBottom = 200;
}

int ProgrammableButtons::ReadFile(AnsiString name)
{
	Json::Value root;   // will contains the root value after parsing.
	Json::Reader reader;

    SetDefault();

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

	const Json::Value &btnConfJson = root["btnConf"];
	for (int i=0; i<btnConfJson.size(); i++)
	{
		const Json::Value &btnJson = btnConfJson[i];
		class ButtonConf cfg;
		cfg.type = (Button::Type)btnJson.get("type", 0).asInt();
		if (cfg.type >= Button::TYPE_LIMITER)
		{
			cfg.type = Button::DISABLED;
		}
		cfg.caption = btnJson.get("caption", "").asString();
		cfg.caption2 = btnJson.get("caption2", "").asString();
		cfg.captionLines = btnJson.get("captionLines", 1).asInt();
		if (cfg.captionLines < ButtonConf::CAPTION_LINES_MIN || cfg.captionLines > ButtonConf::CAPTION_LINES_MAX)
		{
            cfg.captionLines = 1;
        }
		cfg.number = btnJson.get("number", "").asString();
		cfg.noIcon = btnJson.get("noIcon", false).asBool();
		cfg.height = btnJson.get("height", 32).asUInt();
		cfg.marginTop = btnJson.get("marginTop", 0).asUInt();
		if (cfg.marginTop > 200)
			cfg.marginTop = 0;
		cfg.marginBottom = btnJson.get("marginBottom", 0).asUInt();
		if (cfg.marginBottom > 200)
			cfg.marginBottom = 200;
		cfg.backgroundColor = btnJson.get("backgroundColor", clBtnFace).asInt();
		if (i >= btnConf.size())
			break;
		cfg.imgIdle = btnJson.get("imgIdle", "idle.bmp").asString();
		cfg.imgTerminated = btnJson.get("imgTerminated", "terminated.bmp").asString();
		cfg.imgEarly = btnJson.get("imgEarly", "early.bmp").asString();
		cfg.imgConfirmed = btnJson.get("imgConfirmed", "confirmed.bmp").asString();

		cfg.blfActionDuringCall =
			static_cast<ButtonConf::BlfActionDuringCall>(btnJson.get("blfActionDuringCall", ButtonConf::BLF_IN_CALL_TRANSFER).asInt());
		if (cfg.blfActionDuringCall < ButtonConf::BLF_IN_CALL_NONE || cfg.blfActionDuringCall >= ButtonConf::BLF_IN_CALL_TRANSFER)
		{
			cfg.blfActionDuringCall = ButtonConf::BLF_IN_CALL_TRANSFER;
		}

		cfg.blfDtmfPrefixDuringCall = btnJson.get("blfDtmfPrefixDuringCall", "").asString();

		const Json::Value &blfOverrideIdle = btnJson["blfOverrideIdle"];
		cfg.blfOverrideIdle.active = blfOverrideIdle.get("active", false).asBool();
		cfg.blfOverrideIdle.number = blfOverrideIdle.get("number", "").asString();

		const Json::Value &blfOverrideTerminated = btnJson["blfOverrideTerminated"];
		cfg.blfOverrideTerminated.active = blfOverrideTerminated.get("active", false).asBool();
		cfg.blfOverrideTerminated.number = blfOverrideTerminated.get("number", "").asString();

		const Json::Value &blfOverrideEarly = btnJson["blfOverrideEarly"];
		cfg.blfOverrideEarly.active = blfOverrideEarly.get("active", false).asBool();
		cfg.blfOverrideEarly.number = blfOverrideEarly.get("number", "").asString();

		const Json::Value &blfOverrideConfirmed = btnJson["blfOverrideConfirmed"];
		cfg.blfOverrideConfirmed.active = blfOverrideConfirmed.get("active", false).asBool();
		cfg.blfOverrideConfirmed.number = blfOverrideConfirmed.get("number", "").asString();

		cfg.arg1 = btnJson.get("arg1", "").asString();

		cfg.pagingTxWaveFile = btnJson.get("pagingTxWaveFile", "").asString();
		cfg.pagingTxCodec = btnJson.get("pagingTxCodec", cfg.pagingTxCodec.c_str()).asString();
		cfg.pagingTxPtime = btnJson.get("pagingTxPtime", cfg.pagingTxPtime).asUInt();

		cfg.script = btnJson.get("script", "").asString();

		cfg.audioRxMod = btnJson.get("audioRxMod", cfg.audioRxMod.c_str()).asString();
		cfg.audioRxDev = btnJson.get("audioRxDev", cfg.audioRxDev.c_str()).asString();
		cfg.audioTxMod = btnJson.get("audioTxMod", cfg.audioTxMod.c_str()).asString();
		cfg.audioTxDev = btnJson.get("audioTxDev", cfg.audioTxDev.c_str()).asString();

		btnConf[i] = cfg;
	}
	return 0;
}

int ProgrammableButtons::Read(void)
{
	assert(filename != "");

	if (ReadFile(filename) == 0)
	{
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
	assert(filename != "");
    Json::Value root;
	Json::StyledWriter writer;
	
	// write buttons configuration
	for (unsigned int i=0; i<btnConf.size(); i++)
	{
		class ButtonConf &cfg = btnConf[i];
		Json::Value &jsonBtn = root["btnConf"][i];
		jsonBtn["type"] = cfg.type;
		jsonBtn["caption"] = cfg.caption;
		jsonBtn["caption2"] = cfg.caption2;
		jsonBtn["captionLines"] = cfg.captionLines;
		jsonBtn["number"] = cfg.number;
		jsonBtn["noIcon"] = cfg.noIcon;
		jsonBtn["height"] = cfg.height;
		jsonBtn["marginTop"] = cfg.marginTop;
		jsonBtn["marginBottom"] = cfg.marginBottom;
		jsonBtn["backgroundColor"] = cfg.backgroundColor;		
		jsonBtn["imgIdle"] = cfg.imgIdle;
		jsonBtn["imgTerminated"] = cfg.imgTerminated;
		jsonBtn["imgEarly"] = cfg.imgEarly;
		jsonBtn["imgConfirmed"] = cfg.imgConfirmed;
		jsonBtn["blfOverrideIdle"]["active"] = cfg.blfOverrideIdle.active;
		jsonBtn["blfOverrideIdle"]["number"] = cfg.blfOverrideIdle.number;
		jsonBtn["blfOverrideTerminated"]["active"] = cfg.blfOverrideTerminated.active;
		jsonBtn["blfOverrideTerminated"]["number"] = cfg.blfOverrideTerminated.number;
		jsonBtn["blfOverrideEarly"]["active"] = cfg.blfOverrideEarly.active;
		jsonBtn["blfOverrideEarly"]["number"] = cfg.blfOverrideEarly.number;
		jsonBtn["blfOverrideConfirmed"]["active"] = cfg.blfOverrideConfirmed.active;
		jsonBtn["blfOverrideConfirmed"]["number"] = cfg.blfOverrideConfirmed.number;
		jsonBtn["blfActionDuringCall"] = cfg.blfActionDuringCall;
		jsonBtn["blfDtmfPrefixDuringCall"] = cfg.blfDtmfPrefixDuringCall;
		jsonBtn["arg1"] = cfg.arg1;
		jsonBtn["pagingTxWaveFile"] = cfg.pagingTxWaveFile;
		jsonBtn["pagingTxCodec"] = cfg.pagingTxCodec;
		jsonBtn["pagingTxPtime"] = cfg.pagingTxPtime;
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