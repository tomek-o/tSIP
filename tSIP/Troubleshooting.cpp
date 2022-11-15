//---------------------------------------------------------------------------


#pragma hdrstop

#include "Troubleshooting.h"
#include "Settings.h"
#include "AudioModules.h"
#include "AudioDevicesList.h"
#include "ProgrammableButtons.h"
#include "Globals.h"
#include "NetInterfaces.h"

#include <re.h>
#include <baresip.h>

//---------------------------------------------------------------------------

#pragma package(smart_init)

using namespace Troubleshooting;

namespace {

	std::vector<Item> items;

	struct ItemTypeData
	{
		enum Level level;
		const char* name;
		const char* description;
		bool (*fnCheck)(void);
	};

	bool CheckNoLocalPort(void)
	{
		if (appSettings.uaConf.accounts.size())
		{
			const UaConf::Account &acc = appSettings.uaConf.accounts[0];
			if (acc.reg_expires <= 0)
			{
				if (appSettings.uaConf.local.find(":") == std::string::npos)
				{
					return true;
				}
			}
		}
		return false;
	}

	bool CheckNoRegistration(void)
	{
		if (appSettings.uaConf.accounts.size())
		{
			const UaConf::Account &acc = appSettings.uaConf.accounts[0];
			if (acc.reg_expires <= 0)
			{
				return true;
			}
		}
		return false;
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

		if (appSettings.uaConf.ifname != "")
			return false;

		if (strncmp(appSettings.uaConf.local.c_str(), "0.0.0.0", strlen("0.0.0.0")))
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

		if (appSettings.uaConf.ifname != "")
			return false;

		if (strncmp(appSettings.uaConf.local.c_str(), "0.0.0.0", strlen("0.0.0.0")))
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
		return false;
	}

	bool CheckNetworkInterface(void)
	{
		if (appSettings.uaConf.ifname != "")
		{
			std::vector<NetInterface> interfaces;
			int status = GetNetInterfaces(interfaces);
			if (status == 0)
			{
				for (unsigned int i=0; i<interfaces.size(); i++)
				{
					if (interfaces[i].name == appSettings.uaConf.ifname.c_str())
						return false;
				}
			}
			return true;
		}
		return false;
	}

	bool CheckCommonCodecs(void)
	{
		if (appSettings.uaConf.accounts.size())
		{
			const UaConf::Account &acc = appSettings.uaConf.accounts[0];
			for (unsigned int i=0; i<acc.audio_codecs.size(); i++)
			{
				const std::string &str = acc.audio_codecs[i];
				if (str == "PCMU/8000/1" || str == "PCMA/8000/1")
					return false;
			}
			return true;
		}
		return false;
	}

	bool CheckUncommonFrameLength(void)
	{
		if (appSettings.uaConf.accounts.size())
		{
			const UaConf::Account &acc = appSettings.uaConf.accounts[0];
			if (acc.ptime != UaConf::Account::DEF_PTIME)
				return true;
		}
		return false;
	}

	bool CheckNoCodecsEnabled(void)
	{
		if (appSettings.uaConf.accounts.size())
		{
			const UaConf::Account &acc = appSettings.uaConf.accounts[0];
			if (acc.audio_codecs.empty())
				return true;
		}
		return false;
	}

	bool CheckTooManyCodecs(void)
	{
		if (appSettings.uaConf.accounts.size())
		{
			const UaConf::Account &acc = appSettings.uaConf.accounts[0];
			if (acc.audio_codecs.size() > 10)
				return true;
		}
		return false;
	}

	bool CheckPcmuPcmaPtime(void)
	{
		if (appSettings.uaConf.accounts.size())
		{
			const UaConf::Account &acc = appSettings.uaConf.accounts[0];
			for (unsigned int i=0; i<acc.audio_codecs.size(); i++)
			{
				const std::string& codec = acc.audio_codecs[i];
				if (codec == "PCMU/8000/1" || codec == "PCMA/8000/1")
				{
					if (acc.ptime != 20)
						return true;
				}
			}
		}
		return false;
	}

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

	/** TLS enabled but no media encryption is selected
	*/
	bool CheckAccountMediaEncryptionMissing(void)
	{
		for (unsigned int accId = 0; accId < appSettings.uaConf.accounts.size(); accId++)
		{
			const UaConf::Account &acc = appSettings.uaConf.accounts[accId];
			if (acc.transport == UaConf::Account::TRANSPORT_TLS &&
				acc.mediaenc == "")
			{
            	return true;
			}
		}
		return false;
	}

	/** SRTP enabled but transport/signalization is unencrypted
	*/
	bool CheckAccountTlsForSrtpMissing(void)
	{
		for (unsigned int accId = 0; accId < appSettings.uaConf.accounts.size(); accId++)
		{
			const UaConf::Account &acc = appSettings.uaConf.accounts[accId];
			if (acc.transport != UaConf::Account::TRANSPORT_TLS &&
				(acc.mediaenc == "srtp" || acc.mediaenc == "srtp-mand" || acc.mediaenc == "srtp-mandf"))
			{
				return true;
			}
		}
		return false;
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

	struct ItemTypeData itemTypeData[] =
	{
		{ LevelError, "No specified network interface",
			"Network interface / adapter specified in configuration (Settings / Network tab) was not found.\r\n"
            "Select another (valid) interface or try default interface selection (empty string as network interface GUID)."
			,CheckNetworkInterface },
		{ LevelWarning, "No audio input device", "", CheckAudioInputDevice },
		{ LevelWarning, "No audio output device", "", CheckAudioOutputDevice },
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
		{ LevelWarning, "Invalid frame ptime for L16 codec(s)", "Selected RTP ptime may be too large for selected L16 codec, exceeding network MTU.", CheckL16Ptime },
		{ LevelHint, "Selected ptime is different then commonly used 20ms.", "Ptime other than 20ms with commonly used codecs may cause interoperability problems.", CheckUncommonFrameLength },
		{ LevelHint, "Popular codecs are disabled", "Note: popular/commonly used codecs (G.711a, G.711u) are not enabled.", CheckCommonCodecs },
		{ LevelHint, "No registration", "Note: registration is not enabled (registration expires value is set to zero in account configuration).", CheckNoRegistration },
		{ LevelHint, "No local port assigned",
			"Softphone is configured without registration but local port is not assigned.\n"
			"You most likely won't be able to receive calls with this configuration."
			, CheckNoLocalPort
			},
		{ LevelWarning, "Speex preprocessing enabled", "Audio preprocessing function may be unstable (rare crashes related to this functions were reported), it is for test purposes only.", CheckSpeexPreprocessing },
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
	items.clear();
	for (int i=0; i<ARRAY_SIZE(itemTypeData); i++)
	{
		struct ItemTypeData &it = itemTypeData[i];
		if (it.fnCheck)
		{
			bool result = it.fnCheck();
			if (result)
			{
				AddItem(i, "");
			}
		}
	}
}



