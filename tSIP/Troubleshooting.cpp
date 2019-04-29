//---------------------------------------------------------------------------


#pragma hdrstop

#include "Troubleshooting.h"
#include "Settings.h"
#include "AudioDevicesList.h"

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

	bool CheckAudioInputDevice(void)
	{
		const UaConf::AudioCfg &cfg = appSettings.uaConf.audioCfgSrc;
		AudioDevicesList &adl = AudioDevicesList::Instance();
		adl.Refresh();
		if (strcmp(cfg.mod, UaConf::modPortaudio) == 0)
		{
            // one default entry is always present (?)
			if (adl.portaudioDevsIn.size() <= 1)
				return true;
		}
		else if (strcmp(cfg.mod, UaConf::modWinwave) == 0)
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
		if (strcmp(cfg.mod, UaConf::modPortaudio) == 0)
		{
            // one default entry is always present (?)
			if (adl.portaudioDevsOut.size() <= 1)
				return true;
		}
		else if (strcmp(cfg.mod, UaConf::modWinwave) == 0)
		{
			if (adl.winwaveDevsOut.empty())
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

	struct ItemTypeData itemTypeData[ItemTypeLimiter] =
	{
		{ LevelWarning, "No audio input device", "", CheckAudioInputDevice },
		{ LevelWarning, "No audio output device", "", CheckAudioOutputDevice },
		{ LevelWarning, "Missing wave file for audio input", "Audio file specified as input device does not exist.", NULL },
		{ LevelWarning, "Incorrect wav file format", "Invalid file format for input device.", NULL },
		{ LevelWarning, "Binding to VirtualBox adapter",
			"Local address seems to be the address of VirtualBox network adapter.\n"
			"It is very like that this configuration would not be usable, e.g. would not allow registering to server.\n"
			"You may/should bind to other network interface by entering local address or network adapter GUID in \"Network\" settings tab."
			, CheckLocalAddressVirtualBox
			},
		{ LevelWarning, "Invalid frame ptime", "Selected RTP ptime may not work well with specified codec(s), exceeding network MTU.", NULL },
		{ LevelHint, "Selected ptime is different then commonly used 20ms.", "Ptime other than 20ms with commonly used codecs may cause interoperability problems.", CheckUncommonFrameLength },
		{ LevelHint, "Popular codecs are disabled", "Note: popular/commonly used codecs (G.711a, G.711u) are not enabled.", CheckCommonCodecs },
        { LevelHint, "No registration", "Note: registration is not enabled (registration expires value is set to zero in account configuration).", CheckNoRegistration },
		{ LevelHint, "No local port assigned",
			"Softphone is configured without registration but local port is not assigned.\n"
			"You most likely won't be able to receive calls with this configuration."
			, CheckNoLocalPort
			},
		{ LevelWarning, "Speex preprocessing enabled", "Audio preprocessing function may be unstable (rare crashes related to this functions were reported), it is for test purposes only.", CheckSpeexPreprocessing },
	};

	enum Level getItemLevel(enum ItemType type)
	{
		if (type >= 0 && type < ItemTypeLimiter)
		{
			return itemTypeData[type].level;
		}
		return LevelLimiter;
	}

	const char* getItemTypeName(enum ItemType type)
	{
		if (type >= 0 && type < ItemTypeLimiter)
		{
			return itemTypeData[type].name;
		}
		return "???";
	}

	const char* getItemTypeDescription(enum ItemType type)
	{
		if (type >= 0 && type < ItemTypeLimiter)
		{
			return itemTypeData[type].description;
		}
		return "???";
	}



}	// namespace

enum Level Item::getLevel(void) const {
	return getItemLevel(type);
}

const char* Item::getName(void) const {
	return getItemTypeName(type);
}

const char* Item::getDescription(void) const {
	return getItemTypeDescription(type);
}


std::vector<Item> getItems(void)
{
	return items;
}

void Troubleshooting::AddItem(ItemType type, AnsiString extraMsg)
{
	Item item;
	item.type = type;
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
	for (int i=0; i<ItemTypeLimiter; i++)
	{
		struct ItemTypeData &it = itemTypeData[i];
		if (it.fnCheck)
		{
			bool result = it.fnCheck();
			if (result)
			{
				AddItem(static_cast<ItemType>(i), "");
			}
		}
	}
}



