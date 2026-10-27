//---------------------------------------------------------------------------


#pragma hdrstop

#include "Troubleshooting.h"
#include "Settings.h"
#include "AudioModules.h"
#include "AudioDevicesList.h"
#include "ProgrammableButtons.h"
#include "Globals.h"
#include "NetInterfaces.h"
#include "Paths.h"
#include "Log.h"

#include <re.h>
#include <baresip.h>

//---------------------------------------------------------------------------

#pragma package(smart_init)

using namespace Troubleshooting;

namespace {
	unsigned int lineButtonCount = 0;	///< number of line buttons

	/** \brief Utility: count/update the number line buttons */
	void CountLineButtons(void)
	{
		lineButtonCount = 0;
		const std::vector<ButtonConf> &conf = buttons.btnConf;
		for (unsigned int i=0; i<conf.size(); i++)
		{
			const ButtonConf &c = conf[i];
			if (c.type == Button::LINE)
			{
				lineButtonCount++;
			}
		}
	}

	std::vector<Item> items;

	/** Extra message for item added by check function (e.g. list of affected accounts) */
	AnsiString checkExtraMsg;

	/** Add account to list of affected accounts in checkExtraMsg */
	void AddAffectedAccount(unsigned int accountId)
	{
		if (checkExtraMsg == "")
			checkExtraMsg = "Account(s): ";
		else
			checkExtraMsg += ", ";
		checkExtraMsg += appSettings.GetAccountDescription(accountId);
	}

	/** Run check for each enabled account, true if any account matches */
	bool CheckAccounts(bool (*fn)(const UaConf::Account &acc))
	{
		bool result = false;
		for (unsigned int i=0; i<appSettings.uaConf.accounts.size(); i++)
		{
			const UaConf::Account &acc = appSettings.uaConf.accounts[i];
			if (!acc.enabled)
				continue;
			if (fn(acc))
			{
				result = true;
				AddAffectedAccount(i);
			}
		}
		return result;
	}

	struct ItemTypeData
	{
		enum Level level;
		const char* name;
		const char* description;
		bool (*fnCheck)(void);
	};

	bool AccNoRegistration(const UaConf::Account &acc)
	{
		return (acc.reg_expires <= 0);
	}

	bool CheckNoLocalPort(void)
	{
		if (appSettings.uaConf.netLocal.find(":") != std::string::npos)
			return false;
		return CheckAccounts(AccNoRegistration);
	}

	bool CheckNoRegistration(void)
	{
		return CheckAccounts(AccNoRegistration);
	}

	bool CheckSpeexPreprocessing(void)
	{
		return appSettings.uaConf.audioPreprocTx.enabled;
	}

	/** Virtual (duh) VirtualBox adapter has often highest priority,
		but binding to it would not allow even registering to server
	*/
	bool CheckLocalAddressVirtualBox(void)
	{
		const struct sa *addr = net_laddr_af(AF_INET);
		assert(addr);

		if (appSettings.uaConf.netIfName != "")
			return false;

		if (strncmp(appSettings.uaConf.netLocal.c_str(), "0.0.0.0", strlen("0.0.0.0")))
			return false;

		char buf[64];
		int st = sa_ntop(addr, buf, sizeof(buf));
		if (st == 0)
		{
			if (strcmp(buf, "192.168.56.1") == 0)
			{
            	return true;
			}
		}
		return false;
	}

	/** 169.254.x.x = Automatic Private IP address (Windows)
	*/
	bool CheckLocalAutomaticPrivateIpAddress(void)
	{
		const struct sa *addr = net_laddr_af(AF_INET);
		assert(addr);

		if (appSettings.uaConf.netIfName != "")
			return false;

		if (strncmp(appSettings.uaConf.netLocal.c_str(), "0.0.0.0", strlen("0.0.0.0")))
			return false;

		char buf[64];
		int st = sa_ntop(addr, buf, sizeof(buf));
		if (st == 0)
		{
			if (strncmp(buf, "169.154.", strlen("169.154.")) == 0)
			{
				return true;
			}
		}
		return false;
	}

	bool CheckAudioInputDevice(void)
	{
		const UaConf::AudioCfg &cfg = appSettings.uaConf.audioCfgSrc;
		AudioDevicesList &adl = AudioDevicesList::Instance();
		adl.Refresh();
		if (cfg.mod == AudioModules::portaudio)
		{
			// one default entry is always present (?)
			if (adl.portaudioDevsIn.size() <= 1)
				return true;
		}
		else if (cfg.mod == AudioModules::winwave || cfg.mod == AudioModules::winwave2)
		{
			if (adl.winwaveDevsIn.empty())
				return true;
		}
		else if (cfg.mod == AudioModules::wasapi)
		{
			// two "default" entries (input + loopback) are always present if WASAPI is available
			if (adl.wasapiDevsIn.size() <= 2)
				return true;
		}
		return false;
	}

	bool CheckAudioInputFile(void)
	{
		const UaConf::AudioCfg &cfg = appSettings.uaConf.audioCfgSrc;
		if (cfg.mod == AudioModules::aufile || cfg.mod == AudioModules::aufileMm)
		{
			AnsiString dir = Paths::GetProfileDir();
			AnsiString fileName = dir + "\\" + cfg.dev.c_str();
			if (FileExists(fileName) == false)
			{
				return true;
			}
		}
		return false;
	}

	bool CheckAudioOutputDevice(void)
	{
		const UaConf::AudioCfg &cfg = appSettings.uaConf.audioCfgPlay;
		AudioDevicesList &adl = AudioDevicesList::Instance();
		adl.Refresh();
		if (cfg.mod == AudioModules::portaudio)
		{
            // one default entry is always present (?)
			if (adl.portaudioDevsOut.size() <= 1)
				return true;
		}
		else if (cfg.mod == AudioModules::winwave || cfg.mod == AudioModules::winwave2)
		{
			if (adl.winwaveDevsOut.empty())
				return true;
		}
		else if (cfg.mod == AudioModules::wasapi)
		{
			// one "default" entry is always present if WASAPI is available
			if (adl.wasapiDevsOut.size() <= 1)
				return true;
		}
		return false;
	}

	/** \brief True if device selected in cfg is not on the list of devices of its module
		(modules silently use the default device in such case) */
	bool IsAudioDeviceMissing(const UaConf::AudioCfg &cfg, bool out)
	{
		if (cfg.dev.empty())
			return false;

		AudioDevicesList &adl = AudioDevicesList::Instance();
		const std::vector<AnsiString> *devs = NULL;
		if (cfg.mod == AudioModules::portaudio)
			devs = out ? &adl.portaudioDevsOut : &adl.portaudioDevsIn;
		else if (cfg.mod == AudioModules::winwave || cfg.mod == AudioModules::winwave2)
			devs = out ? &adl.winwaveDevsOut : &adl.winwaveDevsIn;
		else if (cfg.mod == AudioModules::wasapi)
			devs = out ? &adl.wasapiDevsOut : &adl.wasapiDevsIn;

		// no list (other module or enumeration failed): covered by CheckAudio...Device
		if (!devs || devs->empty())
			return false;

		const AnsiString name = cfg.dev.c_str();
		for (unsigned int i=0; i<devs->size(); i++)
		{
			if ((*devs)[i] == name)
				return false;
		}
		return true;
	}

	void AddMissingAudioDevice(const char *use, const UaConf::AudioCfg &cfg)
	{
		if (checkExtraMsg != "")
			checkExtraMsg += "; ";
		checkExtraMsg.cat_sprintf("%s: %s", use, cfg.dev.c_str());
	}

	bool CheckSelectedAudioDevices(void)
	{
		const UaConf &conf = appSettings.uaConf;
		bool result = false;
		AudioDevicesList::Instance().Refresh();
		if (IsAudioDeviceMissing(conf.audioCfgSrc, false))
		{
			AddMissingAudioDevice("input", conf.audioCfgSrc);
			result = true;
		}
		if (IsAudioDeviceMissing(conf.audioCfgPlay, true))
		{
			AddMissingAudioDevice("output", conf.audioCfgPlay);
			result = true;
		}
		if (IsAudioDeviceMissing(conf.audioCfgRing, true))
		{
			AddMissingAudioDevice("ring", conf.audioCfgRing);
			result = true;
		}
		if (IsAudioDeviceMissing(conf.audioCfgAlert, true))
		{
			AddMissingAudioDevice("alert", conf.audioCfgAlert);
			result = true;
		}
		if (IsAudioDeviceMissing(conf.audioCfgPlayIntercom, true))
		{
			AddMissingAudioDevice("intercom", conf.audioCfgPlayIntercom);
			result = true;
		}
		return result;
	}

	bool CheckSubscribeServerOpen(void)
	{
		const UaConf::Sip &sip = appSettings.uaConf.sip;
		if (!sip.presenceServer && !sip.dialogServer)
			return false;
		AnsiString list = sip.subscribeAllowFrom.c_str();
		for (int i=1; i<=list.Length(); i++)
		{
			if (list[i] == ';' || list[i] == ',' || list[i] == '\t')
				list[i] = ' ';
		}
		list = " " + list.LowerCase() + " ";
		return list.Pos(" any ") > 0 || list.Pos(" * ") > 0 || list.Pos(" 0.0.0.0/0 ") > 0 || list.Pos(" ::/0 ") > 0;
	}

	bool CheckNetworkInterface(void)
	{
		if (appSettings.uaConf.netIfName != "")
		{
			std::vector<NetInterface> interfaces;
			int status = GetNetInterfaces(interfaces);
			if (status == 0)
			{
				for (unsigned int i=0; i<interfaces.size(); i++)
				{
					if (interfaces[i].name == appSettings.uaConf.netIfName.c_str())
						return false;
				}
			}
			return true;
		}
		return false;
	}

	bool AccNoCommonCodecs(const UaConf::Account &acc)
	{
		for (unsigned int i=0; i<acc.audio_codecs.size(); i++)
		{
			const std::string &str = acc.audio_codecs[i];
			if (str == "PCMU/8000/1" || str == "PCMA/8000/1")
				return false;
		}
		return true;
	}

	bool CheckCommonCodecs(void)
	{
		return CheckAccounts(AccNoCommonCodecs);
	}

	bool AccUncommonFrameLength(const UaConf::Account &acc)
	{
		return (acc.ptime != UaConf::Account::DEF_PTIME);
	}

	bool CheckUncommonFrameLength(void)
	{
		return CheckAccounts(AccUncommonFrameLength);
	}

	bool AccNoCodecsEnabled(const UaConf::Account &acc)
	{
		return acc.audio_codecs.empty();
	}

	bool CheckNoCodecsEnabled(void)
	{
		return CheckAccounts(AccNoCodecsEnabled);
	}

	bool AccTooManyCodecs(const UaConf::Account &acc)
	{
		return (acc.audio_codecs.size() > 10);
	}

	bool CheckTooManyCodecs(void)
	{
		return CheckAccounts(AccTooManyCodecs);
	}

	bool AccPcmuPcmaPtime(const UaConf::Account &acc)
	{
		for (unsigned int i=0; i<acc.audio_codecs.size(); i++)
		{
			const std::string& codec = acc.audio_codecs[i];
			if (codec == "PCMU/8000/1" || codec == "PCMA/8000/1")
			{
				if (acc.ptime != 20)
					return true;
			}
		}
		return false;
	}

	bool CheckPcmuPcmaPtime(void)
	{
		return CheckAccounts(AccPcmuPcmaPtime);
	}

#if 0
	bool CheckL16Ptime(void)
	{
		for (unsigned int accId = 0; accId < appSettings.uaConf.accounts.size(); accId++)
		{
			const UaConf::Account &acc = appSettings.uaConf.accounts[accId];
			for (unsigned int i=0; i<acc.audio_codecs.size(); i++)
			{
				const std::string& codec = acc.audio_codecs[i];
				/*
					L16/48000/1: 1920 B @ ptime = 20
					L16/44100/1: 1764 B @ ptime = 20
				*/
				if (codec == "L16/48000/1" || codec == "L16/44100/1")
				{
					if (acc.ptime > 10)
						return true;
				}
				else if (codec == "L16/48000/2" || codec == "L16/44100/2")
				{
					if (acc.ptime > 5)
						return true;
				}
			}
		}
		return false;
	}
#endif

	/** TLS enabled but no media encryption is selected
	*/
	bool AccMediaEncryptionMissing(const UaConf::Account &acc)
	{
		return (acc.transport == UaConf::Account::TRANSPORT_TLS && acc.mediaenc == "");
	}

	bool CheckAccountMediaEncryptionMissing(void)
	{
		return CheckAccounts(AccMediaEncryptionMissing);
	}

	/** SRTP enabled but transport/signalization is unencrypted
	*/
	bool AccTlsForSrtpMissing(const UaConf::Account &acc)
	{
		return (acc.transport != UaConf::Account::TRANSPORT_TLS &&
			(acc.mediaenc == "srtp" || acc.mediaenc == "srtp-mand" || acc.mediaenc == "srtp-mandf"));
	}

	bool CheckAccountTlsForSrtpMissing(void)
	{
		return CheckAccounts(AccTlsForSrtpMissing);
	}

	bool CheckPresenceBitmaps(void)
	{
		ButtonConf defaultBtnConf;
		const std::vector<ButtonConf> &conf = buttons.btnConf;
		for (unsigned int i=0; i<conf.size(); i++)
		{
			const ButtonConf &c = conf[i];
			if (c.type == Button::PRESENCE)
			{
				if (c.imgTerminated == defaultBtnConf.imgTerminated ||
					c.imgEarly == defaultBtnConf.imgEarly)
				{
					return true;
				}
			}
		}
		return false;
	}

	bool CheckSingleLine(void)
	{
		return (lineButtonCount == 1);
	}

	bool CheckAttendedTransferWithSingleLine(void)
	{
		if (lineButtonCount > 1)
			return false;

		const std::vector<ButtonConf> &conf = buttons.btnConf;
		for (unsigned int i=0; i<conf.size(); i++)
		{
			const ButtonConf &c = conf[i];
			if (c.type == Button::ATTENDED_TRANSFER)
			{
				return true;
			}
		}
		return false;
	}

	bool CheckConferenceWithSingleLine(void)
	{
		if (lineButtonCount > 1)
			return false;

		const std::vector<ButtonConf> &conf = buttons.btnConf;
		for (unsigned int i=0; i<conf.size(); i++)
		{
			const ButtonConf &c = conf[i];
			if (c.type == Button::CONFERENCE_START)
			{
				return true;
			}
		}
		return false;
	}

	bool CheckButtonNewCallDuringCallWithSingleLine(void)
	{
		if (lineButtonCount > 1)
			return false;
		const std::vector<ButtonConf> &conf = buttons.btnConf;
		for (unsigned int i=0; i<conf.size(); i++)
		{
			const ButtonConf &c = conf[i];
			if (c.type == Button::SPEED_DIAL || c.type == Button::PRESENCE || c.type == Button::BLF)
			{
				if (c.blfActionDuringCall == ButtonConf::BLF_IN_CALL_MAKE_ANOTHER_CALL ||
                    c.blfActionDuringCall == ButtonConf::BLF_IN_CALL_TRANSFER_OR_MAKE_ANOTHER_CALL)
				{
					LOG("More than one LINE button is required to be able to make another call if call is already active on button #%u\n", i);
					return true;
				}
			}
		}
		return false;
	}

	bool CheckNoEnabledAccount(void)
	{
		for (unsigned int i=0; i<appSettings.uaConf.accounts.size(); i++)
		{
			if (appSettings.uaConf.accounts[i].enabled)
				return false;
		}
		return true;
	}

	/** Incoming requests are matched to account by user part of request URI (contact user) -
		accounts with same contact user can not be distinguished
	*/
	bool CheckDuplicatedContactUser(void)
	{
		const std::vector<UaConf::Account> &accounts = appSettings.uaConf.accounts;
		// contact user as used at runtime: cuser, or user name if cuser is empty
		std::vector<bool> affected(accounts.size(), false);
		bool result = false;
		for (unsigned int i=0; i<accounts.size(); i++)
		{
			if (!accounts[i].enabled)
				continue;
			std::string ci = accounts[i].cuser != "" ? accounts[i].cuser : accounts[i].user;
			for (unsigned int j=i+1; j<accounts.size(); j++)
			{
				if (!accounts[j].enabled)
					continue;
				std::string cj = accounts[j].cuser != "" ? accounts[j].cuser : accounts[j].user;
				if (AnsiString(ci.c_str()).LowerCase() == AnsiString(cj.c_str()).LowerCase())
				{
					affected[i] = true;
					affected[j] = true;
					result = true;
				}
			}
		}
		// each affected account listed once
		for (unsigned int i=0; i<accounts.size(); i++)
		{
			if (affected[i])
				AddAffectedAccount(i);
		}
		return result;
	}

	bool CheckMultipleAnswerAny(void)
	{
		int count = 0;
		for (unsigned int i=0; i<appSettings.uaConf.accounts.size(); i++)
		{
			const UaConf::Account &acc = appSettings.uaConf.accounts[i];
			if (acc.enabled && acc.answer_any)
			{
				AddAffectedAccount(i);
				count++;
			}
		}
		return (count > 1);
	}

	/** Buttons assigned to account that does not exist or is disabled */
	bool CheckButtonAccount(void)
	{
		const std::vector<UaConf::Account> &accounts = appSettings.uaConf.accounts;
		const std::vector<ButtonConf> &conf = buttons.btnConf;
		bool result = false;
		for (unsigned int i=0; i<conf.size(); i++)
		{
			const ButtonConf &c = conf[i];
			if (c.type != Button::ACCOUNT && c.type != Button::BLF && c.type != Button::PRESENCE &&
				c.type != Button::MWI && c.type != Button::AUTO_ANSWER_DND)
				continue;
			if (c.accountId < 0)
				continue;
			if (c.accountId >= (int)accounts.size() || !accounts[c.accountId].enabled)
			{
				if (checkExtraMsg == "")
					checkExtraMsg = "Button(s): ";
				else
					checkExtraMsg += ", ";
				checkExtraMsg.cat_sprintf("#%u (account #%d)", i, c.accountId + 1);
				result = true;
			}
		}
		return result;
	}

	struct ItemTypeData itemTypeData[] =
	{
		{ LevelError, "No specified network interface",
			"Network interface / adapter specified in configuration (Settings / Network tab) was not found.\r\n"
            "Select another (valid) interface or try default interface selection (empty string as network interface GUID)."
			,CheckNetworkInterface },
		{ LevelWarning, "No audio input device", "", CheckAudioInputDevice },
		{ LevelWarning, "No audio input source file found", "", CheckAudioInputFile },		
		{ LevelWarning, "No audio output device", "", CheckAudioOutputDevice },
		{ LevelWarning, "Selected audio device not found",
			"Audio device selected in Settings (input, output, ring, alert or intercom) is not present, e.g. unplugged or renamed. "
			"The default device of the audio module would be used instead. Select the device again in Settings.",
			CheckSelectedAudioDevices },
		{ LevelWarning, "Presence / dialog server accepts subscriptions from any address",
			"Presence or dialog (BLF) server is enabled and \"Allowed subscriber addresses\" (Settings / SIP) contains \"any\": "
			"anyone who can reach the application can read your presence and call state (including remote identity of the calls). "
			"Prefer \"lan\" or a list of addresses / networks.",
			CheckSubscribeServerOpen },
		{ LevelWarning, "Missing wave file for audio input", "Audio file specified as input device does not exist.", NULL },
		{ LevelWarning, "Incorrect wav file format", "Invalid file format for input device.", NULL },
		{ LevelWarning, "Binding to VirtualBox network adapter",
			"Local address seems to be the address of VirtualBox network adapter.\n"
			"It is very likely that this configuration would not be usable, e.g. would not allow registering to server.\n"
			"You may/should bind to other network interface by entering local address or network adapter GUID in \"Network\" settings tab."
			, CheckLocalAddressVirtualBox
			},
		{ LevelWarning, "Binding to network adapter with automatic private IP address",
			"Local address seems to be Windows' automatic private IP address.\n"
			"It is very likely that this configuration would not be usable, e.g. would not allow registering to server.\n"
			"You may/should bind to other network interface by entering local address or network adapter GUID in \"Network\" settings tab."
			"If this is correct interface - make sure ethernet cable is connected / DHCP server is working."
			, CheckLocalAutomaticPrivateIpAddress
			},
		{ LevelWarning, "Invalid frame ptime", "Selected RTP ptime may not work well with specified codec(s), exceeding network MTU.", NULL },
		//{ LevelWarning, "Invalid frame ptime for L16 codec(s)", "Selected RTP ptime may be too large for selected L16 codec, exceeding network MTU.", CheckL16Ptime },
		{ LevelHint, "Selected ptime is different then commonly used 20ms.", "Ptime other than 20ms with commonly used codecs may cause interoperability problems.", CheckUncommonFrameLength },
		{ LevelHint, "Popular codecs are disabled", "Note: popular/commonly used codecs (G.711a, G.711u) are not enabled.", CheckCommonCodecs },
		{ LevelHint, "No registration",
			"Note: registration is not enabled (registration expires value is set to zero in account configuration).\n"
			"This is normal for peer-to-peer communication, but for typical PABX-based systems would not allow to receive any incoming calls."
			, CheckNoRegistration },
		{ LevelHint, "No local port assigned",
			"Softphone is configured without registration but local port is not assigned.\n"
			"You most likely won't be able to receive calls with this configuration."
			, CheckNoLocalPort
			},
		{ LevelHint, "Speex preprocessing enabled", "Audio preprocessing function was unstable (rare crashes related to this functions were reported), but after few fixes I'm using it since December 2025. If you experiencing any kind of crash during the calls, please report it and maybe disable Settings / \"Audio Processing\" / \"Enable audio preprocessing using speex for microphone path (transmitted audio)\".", CheckSpeexPreprocessing },
		{ LevelWarning, "No codecs enabled", "List of enabled codecs is empty.", CheckNoCodecsEnabled },
		{ LevelWarning, "List of enabled codecs might be too long",
			"Having too many codecs activated might cause problems with UDP message fragmentation.",
			CheckTooManyCodecs
			},
		{ LevelWarning, "Packetization time (ptime) other than 20 ms for PCMU/PCMA",
			"For PCMU/PCMA (G.711u/a) codecs 20 ms packetization time is recommended for compatibility reasons.",
			CheckPcmuPcmaPtime
			},
		{ LevelHint, "TLS is selected for account but no media encryption",
			"TLS is selected for account transport (signaling) but no media encryption - it is possible that this is an oversight.",
			CheckAccountMediaEncryptionMissing
			},
		{ LevelHint, "SRTP without TLS is used",
			"SRTP is selected as account media encryption, but selected transport is not secure (non-TLS) - SRTP keys would be sent unencrypted.",
			CheckAccountTlsForSrtpMissing
			},
		{ LevelHint, "Presence buttons bitmaps",
			"Presence button(s) probably should use deticated bitmaps (check.bmp, cross.bmp) instead of default BLF bitmaps.",
			CheckPresenceBitmaps
			},
		{ LevelWarning, "Defining single LINE button is not useful", "Defining single LINE button would give same behavior as not defining any LINE button at all.", CheckSingleLine },
		{ LevelWarning, "Attended transfer requires min. 2 lines", "Define at least two line buttons to use attended transfer.", CheckAttendedTransferWithSingleLine },
		{ LevelWarning, "Conference function requires min. 2 lines", "Define at least two line buttons to use conference function.", CheckConferenceWithSingleLine },
		{ LevelWarning, "Making another call during call requires min. 2 lines", "Found SPEED_DIAL, BLF or PRESENCE button (see log for ID) with action during the call set to making another call, but this requires at least TWO LINE BUTTONS defined.", CheckButtonNewCallDuringCallWithSingleLine },
		{ LevelError, "No enabled account", "All SIP accounts are disabled - calls can not be made or received. Enable at least one account in Settings / SIP account.", CheckNoEnabledAccount },
		{ LevelWarning, "Accounts with the same contact user",
			"Incoming calls and messages are assigned to account by user part of request URI (contact user, or user if contact user is empty). "
			"With the same contact user on multiple enabled accounts incoming calls may be assigned to wrong account (e.g. wrong account shown, calls back / DND using wrong account). "
			"Set different \"Contact\" user in extra settings of one of the accounts.",
			CheckDuplicatedContactUser },
		{ LevelHint, "Multiple accounts set to answer any call",
			"\"Answer any call/message\" (local account) is enabled for more than one account - only first of them would be used for requests not matching any account.",
			CheckMultipleAnswerAny },
		{ LevelWarning, "Button assigned to not available account",
			"ACCOUNT, BLF, PRESENCE, MWI or DND button is assigned to account that does not exist (e.g. removed) or is disabled. "
			"BLF / PRESENCE / MWI would use first available account instead.",
			CheckButtonAccount },
	};

	enum Level getItemLevel(int typeId)
	{
		if (typeId >= 0 && typeId < ARRAY_SIZE(itemTypeData))
		{
			return itemTypeData[typeId].level;
		}
		return LevelLimiter;
	}

	const char* getItemTypeName(int typeId)
	{
		if (typeId >= 0 && typeId < ARRAY_SIZE(itemTypeData))
		{
			return itemTypeData[typeId].name;
		}
		return "???";
	}

	const char* getItemTypeDescription(int typeId)
	{
		if (typeId >= 0 && typeId < ARRAY_SIZE(itemTypeData))
		{
			return itemTypeData[typeId].description;
		}
		return "???";
	}



}	// namespace

enum Level Item::getLevel(void) const {
	return getItemLevel(typeId);
}

const char* Item::getName(void) const {
	return getItemTypeName(typeId);
}

const char* Item::getDescription(void) const {
	return getItemTypeDescription(typeId);
}


std::vector<Item> getItems(void)
{
	return items;
}

void Troubleshooting::AddItem(int typeId, AnsiString extraMsg)
{
	Item item;
	item.typeId = typeId;
	item.extraMsg = extraMsg;
	items.push_back(item);
}

const std::vector<Item>& Troubleshooting::getItems(void)
{
	return items;
}

void Troubleshooting::Update(void)
{
    CountLineButtons();

	items.clear();
	for (int i=0; i<ARRAY_SIZE(itemTypeData); i++)
	{
		struct ItemTypeData &it = itemTypeData[i];
		if (it.fnCheck)
		{
			checkExtraMsg = "";
			bool result = it.fnCheck();
			if (result)
			{
				AddItem(i, checkExtraMsg);
			}
		}
	}
}



