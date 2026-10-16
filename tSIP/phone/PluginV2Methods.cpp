//---------------------------------------------------------------------------
#pragma hdrstop

#include "PluginV2Methods.h"
#include "tSIP_plugin_v2.h"
#include "Globals.h"
#include "ProgrammableButtons.h"
#include "ButtonConf.h"
#include "History.h"
#include "Contacts.h"
#include "Settings.h"
#include "Calls.h"
#include "Call.h"
#include "ControlQueue.h"
#include "SIMPLE_Messages.h"
#include "MessageHistory.h"
#include "AudioDevicesList.h"
#include "AudioModules.h"
#include "Paths.h"
#include "Log.h"
#include "UaMain.h"
#include "UaCustomRequests.h"
#include "common/Utilities.h"
#include <json/json.h>
#include <SysUtils.hpp>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

//---------------------------------------------------------------------------
#pragma package(smart_init)

namespace
{

enum { DEFAULT_LIMIT = 20, MAX_LIMIT = 200 };

unsigned int GetLimit(const Json::Value &params)
{
	int limit = params.get("limit", DEFAULT_LIMIT).asInt();
	if (limit <= 0)
		limit = DEFAULT_LIMIT;
	if (limit > MAX_LIMIT)
		limit = MAX_LIMIT;
	return limit;
}

std::string ToUtf8(const AnsiString &text)
{
	return ::AnsiToUtf8(text).c_str();
}

std::string FormatTimestamp(const History::Entry::Timestamp &ts)
{
	char buf[48];	// enough for max unsigned short values
	sprintf(buf, "%04u-%02u-%02u %02u:%02u:%02u",
		ts.year, ts.month, ts.day, ts.hour, ts.min, ts.sec);
	return std::string(buf);
}

/** \brief Case-insensitive substring search; needle already upper-case */
bool Matches(const AnsiString &text, const AnsiString &needleUpper)
{
	return AnsiUpperCase(text).Pos(needleUpper) > 0;
}

/** params: limit (default 20), filter: "all" (default) | "incoming" | "outgoing" | "missed",
	search: optional text matched against number/URI and names
*/
/** Optional "accountId" param: -1 if not present; false (with error) if invalid */
bool GetOptionalAccountId(const Json::Value &params, Json::Value &result, int &accountId)
{
	accountId = -1;
	if (params["accountId"].isNull())
		return true;
	accountId = params["accountId"].asInt();
	if (!PluginV2Methods::IsAccountUsable(accountId))
	{
		result["error"] = "account does not exist or is disabled";
		return false;
	}
	return true;
}

int GetHistory(const Json::Value &params, Json::Value &result)
{
	unsigned int limit = GetLimit(params);
	std::string filter = params.get("filter", "all").asString();
	if (filter != "all" && filter != "incoming" && filter != "outgoing" && filter != "missed")
		return TSIP_ERR_INVALID_ARG;
	AnsiString needle = AnsiUpperCase(::Utf8ToAnsi(params.get("search", "").asString().c_str()));

	Json::Value jentries(Json::arrayValue);
	const std::deque<History::Entry> &entries = history.GetEntries();
	for (unsigned int i=0; i<entries.size() && jentries.size() < limit; i++)
	{
		const History::Entry &entry = entries[i];	// newest first
		bool missed = entry.incoming && entry.time == 0;
		if ((filter == "incoming" && !entry.incoming) ||
			(filter == "outgoing" && entry.incoming) ||
			(filter == "missed" && !missed))
		{
			continue;
		}
		if (needle != "" &&
			!Matches(entry.uri, needle) && !Matches(entry.paiUri, needle) &&
			!Matches(entry.peerName, needle) && !Matches(entry.paiPeerName, needle) &&
			!Matches(entry.contactName, needle) && !Matches(entry.paiContactName, needle))
		{
			continue;
		}

		Json::Value jv(Json::objectValue);
		jv["timestamp"] = FormatTimestamp(entry.timestamp);
		jv["incoming"] = entry.incoming;
		jv["missed"] = missed;
		jv["duration"] = entry.time;
		if (entry.accountId >= 0)
			jv["accountId"] = entry.accountId;
		jv["uri"] = ToUtf8(entry.uri);
		jv["peerName"] = PluginV2Methods::PeerNameToUtf8(entry.peerName).c_str();
		if (entry.contactName != "")
			jv["contactName"] = ToUtf8(entry.contactName);
		if (entry.paiUri != "")
		{
			jv["paiUri"] = ToUtf8(entry.paiUri);
			jv["paiPeerName"] = PluginV2Methods::PeerNameToUtf8(entry.paiPeerName).c_str();
		}
		if (entry.lastScode != 0)
			jv["lastSipCode"] = entry.lastScode;
		if (entry.reason != "")
			jv["reason"] = ToUtf8(entry.reason);
		jentries.append(jv);
	}
	result["entries"] = jentries;
	return TSIP_OK;
}

/** params: query (required; matched against name, company, numbers/URIs, note), limit (default 20) */
int SearchContacts(const Json::Value &params, Json::Value &result)
{
	if (!params["query"].isString())
		return TSIP_ERR_INVALID_ARG;
	unsigned int limit = GetLimit(params);
	AnsiString needle = AnsiUpperCase(::Utf8ToAnsi(params["query"].asString().c_str()));

	Json::Value jcontacts(Json::arrayValue);
	const std::vector<Contacts::Entry> &entries = contacts.GetEntries();
	for (unsigned int i=0; i<entries.size() && jcontacts.size() < limit; i++)
	{
		const Contacts::Entry &entry = entries[i];
		if (needle != "" &&
			!Matches(entry.description, needle) && !Matches(entry.company, needle) &&
			!Matches(entry.uri1, needle) && !Matches(entry.uri2, needle) && !Matches(entry.uri3, needle) &&
			!Matches(entry.note, needle))
		{
			continue;
		}
		Json::Value jv(Json::objectValue);
		jv["name"] = ToUtf8(entry.description);
		if (entry.company != "")
			jv["company"] = ToUtf8(entry.company);
		Json::Value numbers(Json::arrayValue);
		if (entry.uri1 != "")
			numbers.append(ToUtf8(entry.uri1));
		if (entry.uri2 != "")
			numbers.append(ToUtf8(entry.uri2));
		if (entry.uri3 != "")
			numbers.append(ToUtf8(entry.uri3));
		jv["numbers"] = numbers;
		if (entry.note != "")
			jv["note"] = ToUtf8(entry.note);
		jcontacts.append(jv);
	}
	result["contacts"] = jcontacts;
	return TSIP_OK;
}

/** params: uid (optional, 0/missing: current call), digits: "0-9*#ABCD" (R = flash), max 32 */
int SendDtmf(const Json::Value &params, Json::Value &result)
{
	(void)result;
	if (!params["digits"].isString())
		return TSIP_ERR_INVALID_ARG;
	std::string digits = params["digits"].asString();
	if (digits.empty() || digits.size() > 32)
		return TSIP_ERR_INVALID_ARG;
	for (unsigned int i=0; i<digits.size(); i++)
	{
		char c = toupper(digits[i]);
		if (strchr("0123456789ABCD*#R", c) == NULL)
			return TSIP_ERR_INVALID_ARG;
		digits[i] = c;
	}
	unsigned int callUid = params.get("uid", 0).asUInt();
	if (!PluginV2Methods::ResolveCallUid(callUid))
		return TSIP_ERR_NOT_AVAILABLE;
	Call *call = Calls::FindByUid(callUid);
	if (!call->connected && !call->progress)
		return TSIP_ERR_NOT_AVAILABLE;
	for (unsigned int i=0; i<digits.size(); i++)
	{
		UA->SendDigit(callUid, digits[i]);
	}
	return TSIP_OK;
}

/** params: uid (optional), state (bool, required) */
int Hold(const Json::Value &params, Json::Value &result)
{
	if (!params["state"].isBool())
		return TSIP_ERR_INVALID_ARG;
	unsigned int callUid = params.get("uid", 0).asUInt();
	if (!PluginV2Methods::ResolveCallUid(callUid))
		return TSIP_ERR_NOT_AVAILABLE;
	Call *call = Calls::FindByUid(callUid);
	call->setHold(params["state"].asBool());
	result["hold"] = call->hold;	// e.g. not changed if call has no media yet
	return TSIP_OK;
}

/** params: uid (optional), target (required) - blind transfer (REFER) */
int Transfer(const Json::Value &params, Json::Value &result)
{
	(void)result;
	if (!params["target"].isString() || params["target"].asString().empty())
		return TSIP_ERR_INVALID_ARG;
	unsigned int callUid = params.get("uid", 0).asUInt();
	if (!PluginV2Methods::ResolveCallUid(callUid))
		return TSIP_ERR_NOT_AVAILABLE;
	UA->Transfer(callUid, ::Utf8ToAnsi(params["target"].asString().c_str()));
	return TSIP_OK;
}

PluginV2Methods::CallbackStartRecording callbackStartRecording = NULL;

/** params: uid (optional), state (bool, required) */
int Mute(const Json::Value &params, Json::Value &result)
{
	if (!params["state"].isBool())
		return TSIP_ERR_INVALID_ARG;
	unsigned int callUid = params.get("uid", 0).asUInt();
	if (!PluginV2Methods::ResolveCallUid(callUid))
		return TSIP_ERR_NOT_AVAILABLE;
	Call *call = Calls::FindByUid(callUid);
	call->setMute(params["state"].asBool());
	result["mute"] = call->mute;
	return TSIP_OK;
}

/** params: uid (optional), state (bool, required), level (optional, percent 0..200) */
int Sidetone(const Json::Value &params, Json::Value &result)
{
	if (!params["state"].isBool())
		return TSIP_ERR_INVALID_ARG;
	unsigned int callUid = params.get("uid", 0).asUInt();
	if (!PluginV2Methods::ResolveCallUid(callUid))
		return TSIP_ERR_NOT_AVAILABLE;
	Call *call = Calls::FindByUid(callUid);
	int level = params.get("level", static_cast<int>(call->sidetoneLevel)).asInt();
	if (level < 0 || level > 200)
		return TSIP_ERR_INVALID_ARG;
	result["sidetone"] = call->setSidetone(params["state"].asBool(), level);
	result["level"] = call->sidetoneLevel;
	return TSIP_OK;
}

/** params: uid (optional) - start recording with settings from configuration */
int StartRecording(const Json::Value &params, Json::Value &result)
{
	if (!appSettings.uaConf.recording.enabled)
	{
		result["error"] = "call recording is disabled in tSIP settings";
		return TSIP_ERR_NOT_AVAILABLE;
	}
	if (!callbackStartRecording)
		return TSIP_ERR_NOT_AVAILABLE;
	unsigned int callUid = params.get("uid", 0).asUInt();
	if (!PluginV2Methods::ResolveCallUid(callUid))
		return TSIP_ERR_NOT_AVAILABLE;
	Call *call = Calls::FindByUid(callUid);
	if (!call->recording)
		callbackStartRecording(*call);
	result["recording"] = call->recording;
	if (call->recordFile != "")
		result["file"] = ToUtf8(call->recordFile);
	return TSIP_OK;
}

/** params: target (required), text (required, UTF-8)
	Message is sent through message window (visible to user), result is not awaited.
*/
int SendSimpleMessage(const Json::Value &params, Json::Value &result)
{
	if (!params["target"].isString() || params["target"].asString().empty() ||
		!params["text"].isString() || params["text"].asString().empty())
	{
		return TSIP_ERR_INVALID_ARG;
	}
	if (!appSettings.uaConf.messages.enabled)
	{
		result["error"] = "messaging is disabled in tSIP settings";
		return TSIP_ERR_NOT_AVAILABLE;
	}
	int accountId;
	if (!GetOptionalAccountId(params, result, accountId))
		return TSIP_ERR_INVALID_ARG;
	SIMPLE_Messages::Send(::Utf8ToAnsi(params["target"].asString().c_str()),
		::Utf8ToAnsi(params["text"].asString().c_str()), true, accountId);
	return TSIP_OK;
}

/** params: peer (required; as used in message window, e.g. number or SIP URI), limit (default 20)
	-> {"messages":[{"timestamp","incoming","contentType","body"}]}, oldest first
*/
int GetMessages(const Json::Value &params, Json::Value &result)
{
	if (!params["peer"].isString() || params["peer"].asString().empty())
		return TSIP_ERR_INVALID_ARG;
	unsigned int limit = GetLimit(params);
	AnsiString peer = ::Utf8ToAnsi(params["peer"].asString().c_str());
	std::vector<MessageHistory::Entry> entries = MessageHistory::LoadLast(peer, limit);
	if (entries.empty() && peer.Pos("sip:") != 1 && appSettings.uaConf.accounts.size() > 0)
	{
		// history file may be named after full URI (incoming messages)
		AnsiString uri = (AnsiString)"sip:" + peer + "@" + appSettings.uaConf.accounts[appSettings.GetCurrentAccountId()].reg_server.c_str();
		entries = MessageHistory::LoadLast(uri, limit);
	}
	Json::Value jmessages(Json::arrayValue);
	for (unsigned int i = 0; i < entries.size(); i++)
	{
		const MessageHistory::Entry &entry = entries[i];
		Json::Value jv(Json::objectValue);
		char buf[32] = "";
		struct tm *tm = localtime(&entry.time);
		if (tm)
			strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm);
		jv["timestamp"] = buf;
		jv["incoming"] = entry.incoming;
		jv["contentType"] = entry.contentType.c_str();
		jv["body"] = entry.body.c_str();	// already UTF-8
		jmessages.append(jv);
	}
	result["messages"] = jmessages;
	return TSIP_OK;
}

Json::Value ToJsonArray(const std::vector<AnsiString> &list)
{
	Json::Value arr(Json::arrayValue);
	for (unsigned int i = 0; i < list.size(); i++)
		arr.append(ToUtf8(list[i]));
	return arr;
}

/** \brief Device list for module, NULL if module is not supported for switching */
const std::vector<AnsiString>* GetDeviceList(const std::string &module, bool out)
{
	AudioDevicesList &adl = AudioDevicesList::Instance();
	if (module == AudioModules::portaudio)
		return out ? &adl.portaudioDevsOut : &adl.portaudioDevsIn;
	if (module == AudioModules::winwave || module == AudioModules::winwave2)
		return out ? &adl.winwaveDevsOut : &adl.winwaveDevsIn;
	if (module == AudioModules::wasapi)
		return out ? &adl.wasapiDevsOut : &adl.wasapiDevsIn;
	return NULL;
}

Json::Value AudioCfgToJson(const UaConf::AudioCfg &cfg)
{
	Json::Value jv(Json::objectValue);
	jv["module"] = cfg.mod;
	if (cfg.mod == AudioModules::aufile || cfg.mod == AudioModules::aufileMm || cfg.mod == AudioModules::avformat)
		jv["file"] = ToUtf8(cfg.wavefile.c_str());
	else
		jv["device"] = ToUtf8(cfg.dev.c_str());
	return jv;
}

/** params: none -> {"modules":[{"module","in":[...],"out":[...]}], "configured":{"input":{...},"output":{...}}} */
int ListAudioDevices(const Json::Value &params, Json::Value &result)
{
	(void)params;
	AudioDevicesList &adl = AudioDevicesList::Instance();
	if (!adl.IsFilled())
		adl.Refresh();
	const char* modules[] = { AudioModules::wasapi, AudioModules::winwave2, AudioModules::portaudio };
	Json::Value jmodules(Json::arrayValue);
	for (unsigned int i = 0; i < sizeof(modules) / sizeof(modules[0]); i++)
	{
		Json::Value jm(Json::objectValue);
		jm["module"] = modules[i];
		jm["in"] = ToJsonArray(*GetDeviceList(modules[i], false));
		jm["out"] = ToJsonArray(*GetDeviceList(modules[i], true));
		jmodules.append(jm);
	}
	result["modules"] = jmodules;
	Json::Value configured(Json::objectValue);
	configured["input"] = AudioCfgToJson(appSettings.uaConf.audioCfgSrc);
	configured["output"] = AudioCfgToJson(appSettings.uaConf.audioCfgPlay);
	result["configured"] = configured;
	return TSIP_OK;
}

/** \brief Validate module/device for switching (devices from list of the module) */
int CheckDevice(const std::string &module, const std::string &deviceUtf8, bool out, Json::Value &result)
{
	AudioDevicesList &adl = AudioDevicesList::Instance();
	if (!adl.IsFilled())
		adl.Refresh();
	const std::vector<AnsiString> *list = GetDeviceList(module, out);
	if (list == NULL)
	{
		result["error"] = "unsupported module (use wasapi, winwave2, winwave or portaudio; files: PlayAudioFile)";
		return TSIP_ERR_INVALID_ARG;
	}
	AnsiString device = ::Utf8ToAnsi(deviceUtf8.c_str());
	for (unsigned int i = 0; i < list->size(); i++)
	{
		if ((*list)[i] == device)
			return TSIP_OK;
	}
	result["error"] = "device not found for this module (see ListAudioDevices)";
	return TSIP_ERR_INVALID_ARG;
}

/** params: uid (optional), module + device or configured:true; out: output (player) or input (source) */
namespace
{
	/** values of these keys are never returned to the client */
	bool IsSecretKey(const std::string &key)
	{
		return key == "pwd" || key == "password";
	}

	const char *RedactedValue = "***";

	void RedactSecrets(Json::Value &jv)
	{
		if (jv.type() == Json::objectValue)
		{
			Json::Value::Members names = jv.getMemberNames();
			for (unsigned int i=0; i<names.size(); i++)
			{
				Json::Value &child = jv[names[i]];
				if (IsSecretKey(names[i]) && child.type() == Json::stringValue)
					child = RedactedValue;
				else
					RedactSecrets(child);
			}
		}
		else if (jv.type() == Json::arrayValue)
		{
			for (unsigned int i=0; i<jv.size(); i++)
				RedactSecrets(jv[i]);
		}
	}

	/** objects are merged recursively, other values replace; a redacted secret
		(value returned by GetSettings) keeps the current value */
	void MergeSettings(Json::Value &target, const Json::Value &patch)
	{
		Json::Value::Members names = patch.getMemberNames();
		for (unsigned int i=0; i<names.size(); i++)
		{
			const Json::Value &value = patch[names[i]];
			if (IsSecretKey(names[i]) && value.type() == Json::stringValue && value.asString() == RedactedValue)
				continue;
			if (value.type() == Json::objectValue && target[names[i]].type() == Json::objectValue)
				MergeSettings(target[names[i]], value);
			else
				target[names[i]] = value;
		}
	}
}

/** params: section (optional, top-level key of the settings, e.g. "uaConf") -> {"settings": JSON}
	secrets (account passwords) are returned as "***"
*/
int GetSettings(const Json::Value &params, Json::Value &result)
{
	std::string section = params.get("section", "").asString();
	Json::Value all = appSettings.ToJson();
	Json::Value out = section.empty() ? all : all[section];
	if (!section.empty() && out.isNull())
	{
		result["error"] = "section: unknown settings section";
		return TSIP_ERR_INVALID_ARG;
	}
	RedactSecrets(out);
	result["settings"] = out;
	return TSIP_OK;
}

/** params: settings (JSON object with the part of settings to change) -> saved to tSIP.json and applied in memory
	result: changed (top-level sections that differ), restartRequired (true if any change;
	running components read most settings only at startup)
*/
int SetSettings(const Json::Value &params, Json::Value &result)
{
	const Json::Value &patch = params["settings"];
	if (patch.type() != Json::objectValue)
	{
		result["error"] = "settings: JSON object expected";
		return TSIP_ERR_INVALID_ARG;
	}

	Json::Value before = appSettings.ToJson();
	Json::Value merged = before;
	MergeSettings(merged, patch);

	Json::FastWriter writer;
	if (appSettings.UpdateFromText(AnsiString(writer.write(merged).c_str())) != 0)
	{
		result["error"] = "settings: rejected (invalid value)";
		return TSIP_ERR_INVALID_ARG;
	}
	// in memory first: tSIP writes the in-memory settings on exit
	if (appSettings.Write(Paths::GetConfig()) != 0)
	{
		result["error"] = "settings: cannot write configuration file";
		return TSIP_ERR_GENERIC;
	}

	Json::Value after = appSettings.ToJson();
	Json::Value changed(Json::arrayValue);
	Json::Value::Members names = after.getMemberNames();
	for (unsigned int i=0; i<names.size(); i++)
	{
		if (before[names[i]] != after[names[i]])
			changed.append(names[i]);
	}
	result["changed"] = changed;
	result["restartRequired"] = !changed.empty();
	LOG("MCP: settings changed: %u section(s)\n", changed.size());
	return TSIP_OK;
}

/** params: id (optional, button index 0..count-1) -> {"buttons": [ {"id": n, ...settings...} ], "count": total}
	with id: {"button": {"id": n, ...}}
*/
int GetButtonSettings(const Json::Value &params, Json::Value &result)
{
	result["count"] = static_cast<unsigned int>(buttons.btnConf.size());
	if (params.isMember("id"))
	{
		unsigned int id = params["id"].asUInt();
		if (id >= buttons.btnConf.size())
		{
			result["error"] = "id: no such button";
			return TSIP_ERR_INVALID_ARG;
		}
		Json::Value jb;
		buttons.btnConf[id].ToJson(jb, true);
		jb["id"] = id;
		result["button"] = jb;
		return TSIP_OK;
	}
	Json::Value list(Json::arrayValue);
	for (unsigned int i=0; i<buttons.btnConf.size(); i++)
	{
		Json::Value jb;
		buttons.btnConf[i].ToJson(jb, true);
		jb["id"] = i;
		list.append(jb);
	}
	result["buttons"] = list;
	return TSIP_OK;
}

/** params: id (required), settings (JSON object, merged into the current button settings)
	-> saved to the buttons configuration file and applied in memory;
	result: changed (bool), restartRequired (true if the UA must be restarted to apply, e.g. account or BLF
	button; not done automatically because it would end active calls)
*/
int SetButtonSettings(const Json::Value &params, Json::Value &result)
{
	if (!params.isMember("id") || !params["id"].isIntegral())
	{
		result["error"] = "id: button number required";
		return TSIP_ERR_INVALID_ARG;
	}
	unsigned int id = params["id"].asUInt();
	const Json::Value &patch = params["settings"];
	if (id >= buttons.btnConf.size() || patch.type() != Json::objectValue)
	{
		result["error"] = "id: no such button, or settings: JSON object expected";
		return TSIP_ERR_INVALID_ARG;
	}

	const ButtonConf prev = buttons.btnConf[id];
	Json::Value current;
	prev.ToJson(current, true);
	MergeSettings(current, patch);

	ButtonConf conf;
	conf.FromJson(current);
	if (conf == prev)
	{
		result["changed"] = false;
		result["restartRequired"] = false;
		return TSIP_OK;
	}

	buttons.SetConfig(id, conf);
	if (buttons.Write() != 0)
	{
		result["error"] = "buttons: cannot write configuration file";
		return TSIP_ERR_GENERIC;
	}
	Calls::OnButtonConfigChange();
	result["changed"] = true;
	result["restartRequired"] = conf.UaRestartNeeded(prev);
	LOG("MCP: button %u settings changed\n", id);
	return TSIP_OK;
}

int SetAudio(const Json::Value &params, Json::Value &result, bool out)
{
	unsigned int callUid = params.get("uid", 0).asUInt();
	if (!PluginV2Methods::ResolveCallUid(callUid))
		return TSIP_ERR_NOT_AVAILABLE;
	AnsiString mod, dev;
	if (params.get("configured", false).asBool())
	{
		const UaConf::AudioCfg &cfg = out ? appSettings.uaConf.audioCfgPlay : appSettings.uaConf.audioCfgSrc;
		mod = cfg.mod.c_str();
		if (!out && (cfg.mod == AudioModules::aufile || cfg.mod == AudioModules::aufileMm || cfg.mod == AudioModules::avformat))
		{
			// file source: path relative to profile directory, as on startup
			AnsiString full = Paths::GetProfileDir() + "\\" + cfg.wavefile.c_str();
			dev = FileExists(full) ? full : AnsiString(cfg.wavefile.c_str());
		}
		else
		{
			dev = cfg.dev.c_str();
		}
	}
	else
	{
		if (!params["module"].isString() || !params["device"].isString())
			return TSIP_ERR_INVALID_ARG;
		int status = CheckDevice(params["module"].asString(), params["device"].asString(), out, result);
		if (status != TSIP_OK)
			return status;
		mod = params["module"].asString().c_str();
		dev = ::Utf8ToAnsi(params["device"].asString().c_str());
	}
	if (out)
	{
		UA->SwitchAudioPlayer(callUid, mod, dev);
	}
	else
	{
		Calls::FindByUid(callUid)->keepCallAtAudioFileEnd = false;
		UA->SwitchAudioSource(callUid, mod, dev);
	}
	result["module"] = mod.c_str();
	result["device"] = ToUtf8(dev);
	return TSIP_OK;
}

int SetAudioInput(const Json::Value &params, Json::Value &result)
{
	return SetAudio(params, result, false);
}

int SetAudioOutput(const Json::Value &params, Json::Value &result)
{
	return SetAudio(params, result, true);
}

/** params: uid (optional), file (required) - WAV file name only, from <profile>\audio or <profile>;
	audio input of the call is switched to the file
	-> {"file": full path, "audioErrorCount": value before switching (end of file increments it)}
*/
int PlayAudioFile(const Json::Value &params, Json::Value &result)
{
	if (!params["file"].isString())
		return TSIP_ERR_INVALID_ARG;
	AnsiString name = ::Utf8ToAnsi(params["file"].asString().c_str());
	if (name == "" || name.Pos("\\") || name.Pos("/") || name.Pos(":") || name.Pos("..") ||
		LowerCase(ExtractFileExt(name)) != ".wav")
	{
		result["error"] = "file: WAV file name only (no path), from \"audio\" subdirectory of tSIP profile or profile directory";
		return TSIP_ERR_INVALID_ARG;
	}
	unsigned int callUid = params.get("uid", 0).asUInt();
	if (!PluginV2Methods::ResolveCallUid(callUid))
		return TSIP_ERR_NOT_AVAILABLE;
	AnsiString full = Paths::GetProfileDir() + "\\audio\\" + name;
	if (!FileExists(full))
		full = Paths::GetProfileDir() + "\\" + name;
	if (!FileExists(full))
	{
		result["error"] = "file not found in \"audio\" subdirectory of tSIP profile nor in profile directory";
		return TSIP_ERR_NOT_AVAILABLE;
	}
	Call *call = Calls::FindByUid(callUid);
	result["uid"] = callUid;
	result["audioErrorCount"] = call->audioErrorCount;
	result["file"] = ToUtf8(full);
	call->keepCallAtAudioFileEnd = true;
	UA->SwitchAudioSource(callUid, AudioModules::aufile, full);
	return TSIP_OK;
}

/** params: uid (optional) -> {"audioErrorCount": N (incremented e.g. at end of WAV file input),
	"rxLevel": peak of received audio over last ~100 ms, 0..32767 (not per call)} */
int GetCallAudioState(const Json::Value &params, Json::Value &result)
{
	unsigned int callUid = params.get("uid", 0).asUInt();
	if (!PluginV2Methods::ResolveCallUid(callUid))
		return TSIP_ERR_NOT_AVAILABLE;
	Call *call = Calls::FindByUid(callUid);
	result["uid"] = callUid;
	result["audioErrorCount"] = call->audioErrorCount;
	result["rxLevel"] = Ua::Instance().GetAudioRxSignalLevel();
	return TSIP_OK;
}

/** params: uid (optional) -> {"uid", "incoming", "invite": full text of initial received INVITE
	(raw SIP message as received, empty for outgoing call)} */
int GetCallInvite(const Json::Value &params, Json::Value &result)
{
	unsigned int callUid = params.get("uid", 0).asUInt();
	if (!PluginV2Methods::ResolveCallUid(callUid))
		return TSIP_ERR_NOT_AVAILABLE;
	Call *call = Calls::FindByUid(callUid);
	result["uid"] = callUid;
	result["incoming"] = call->incoming;
	// SIP messages are (normally) UTF-8 already
	result["invite"] = call->initialRxInvite.c_str();
	return TSIP_OK;
}

/** params: method, uri (out-of-dialog request) or uid (request within call dialog, 0/missing:
	current call), headers (optional, {"Name":"value"}) -> {"requestUid"}; reply: GetCustomRequest */
int SendCustomRequest(const Json::Value &params, Json::Value &result)
{
	// methods changing dialog / call state are handled by SIP stack only
	static const char* forbidden[] = { "INVITE", "ACK", "CANCEL", "BYE", "PRACK", "UPDATE" };
	if (!params["method"].isString())
		return TSIP_ERR_INVALID_ARG;
	std::string method = params["method"].asString();
	if (method.empty() || method.size() > 32)
	{
		result["error"] = "invalid method";
		return TSIP_ERR_INVALID_ARG;
	}
	for (unsigned int i=0; i<method.size(); i++)
	{
		if (!isupper(static_cast<unsigned char>(method[i])))
		{
			result["error"] = "method: uppercase letters expected";
			return TSIP_ERR_INVALID_ARG;
		}
	}
	for (unsigned int i=0; i<sizeof(forbidden)/sizeof(forbidden[0]); i++)
	{
		if (method == forbidden[i])
		{
			result["error"] = "method " + method + " cannot be sent as custom request";
			return TSIP_ERR_INVALID_ARG;
		}
	}
	AnsiString headerLines;
	if (!params["headers"].isNull())
	{
		std::string error;
		if (!PluginV2Methods::HeadersToLines(params["headers"], headerLines, error))
		{
			result["error"] = error;
			return TSIP_ERR_INVALID_ARG;
		}
	}
	int requestUid = -1;
	int status;
	if (params["uri"].isString())
	{
		std::string uri = params["uri"].asString();
		if (uri.empty() || uri.find_first_of(std::string("\r\n\0 ", 4)) != std::string::npos)
		{
			result["error"] = "invalid uri";
			return TSIP_ERR_INVALID_ARG;
		}
		int accountId;
		if (!GetOptionalAccountId(params, result, accountId))
			return TSIP_ERR_INVALID_ARG;
		status = UaCustomRequests::Send(requestUid, uri.c_str(), method.c_str(), headerLines, accountId);
	}
	else
	{
		unsigned int callUid = params.get("uid", 0).asUInt();
		if (!PluginV2Methods::ResolveCallUid(callUid))
			return TSIP_ERR_NOT_AVAILABLE;
		status = UaCustomRequests::SendCustomCallRequest(requestUid, callUid, method.c_str(), headerLines);
		result["uid"] = callUid;
	}
	if (status != 0)
	{
		result["error"] = "failed to send request";
		return TSIP_ERR_GENERIC;
	}
	result["requestUid"] = requestUid;
	return TSIP_OK;
}

/** params: requestUid, delete (optional bool: forget request after reading if reply is received)
	-> {"requestUid", "haveReply", "transportError" (transport / timeout error code, 0 = OK),
	"sipStatusCode", "reply" (full text of reply)} */
int GetCustomRequest(const Json::Value &params, Json::Value &result)
{
	if (!params["requestUid"].isIntegral())
		return TSIP_ERR_INVALID_ARG;
	int requestUid = params["requestUid"].asInt();
	UaCustomRequests::Request *req = UaCustomRequests::GetRequest(requestUid);
	if (req == NULL)
		return TSIP_ERR_NOT_AVAILABLE;
	result["requestUid"] = requestUid;
	result["haveReply"] = req->haveReply;
	if (req->haveReply)
	{
		// "error" is used for error description in failed Invoke - use different name
		result["transportError"] = req->error;
		result["sipStatusCode"] = req->sipStatusCode;
		result["reply"] = req->replyText.c_str();
		if (params.get("delete", false).asBool())
			UaCustomRequests::DeleteRequest(requestUid);
	}
	return TSIP_OK;
}

/** params: requestUid - forget request (e.g. after timeout on plugin side) */
int DeleteCustomRequest(const Json::Value &params, Json::Value &result)
{
	if (!params["requestUid"].isIntegral())
		return TSIP_ERR_INVALID_ARG;
	result["requestUid"] = params["requestUid"].asInt();
	UaCustomRequests::DeleteRequest(params["requestUid"].asInt());
	return TSIP_OK;
}

}	// namespace


/** \brief Convert {"Name":"value",...} into header lines for INVITE.
	See declaration.
*/
bool PluginV2Methods::HeadersToLines(const Json::Value &headers, AnsiString &lines, std::string &error)
{
	static const char* forbidden[] = {
		"via", "v", "from", "f", "to", "t", "call-id", "i", "cseq", "contact", "m",
		"route", "record-route", "max-forwards", "content-length", "l", "content-type", "c",
		"content-encoding", "e", "authorization", "proxy-authorization", "expires"
	};
	enum { MAX_HEADERS = 20, MAX_LINE = 1000 };
	if (!headers.isObject())
	{
		error = "headers: object {\"Name\":\"value\"} expected";
		return false;
	}
	if (headers.size() > MAX_HEADERS)
	{
		error = "headers: too many headers";
		return false;
	}
	std::string out;
	Json::Value::Members names = headers.getMemberNames();
	for (unsigned int i=0; i<names.size(); i++)
	{
		const std::string &name = names[i];
		if (name.empty())
		{
			error = "headers: empty header name";
			return false;
		}
		std::string lower;
		for (unsigned int j=0; j<name.size(); j++)
		{
			unsigned char c = name[j];
			// RFC 3261 token
			if (!(isalnum(c) || strchr("-.!%*_+`'~", c)) || c > 127)
			{
				error = "headers: invalid header name \"" + name + "\"";
				return false;
			}
			lower += static_cast<char>(tolower(c));
		}
		for (unsigned int j=0; j<sizeof(forbidden)/sizeof(forbidden[0]); j++)
		{
			if (lower == forbidden[j])
			{
				error = "headers: \"" + name + "\" cannot be set";
				return false;
			}
		}
		const Json::Value &jv = headers[name];
		if (!jv.isString())
		{
			error = "headers: value of \"" + name + "\" must be string";
			return false;
		}
		std::string value = jv.asString();
		if (value.find_first_of(std::string("\r\n\0", 3)) != std::string::npos)
		{
			error = "headers: value of \"" + name + "\" contains CR/LF";
			return false;
		}
		std::string line = name + ": " + value;
		if (line.size() > MAX_LINE)
		{
			error = "headers: \"" + name + "\" too long";
			return false;
		}
		out += line + "\r\n";
	}
	lines = ::Utf8ToAnsi(out.c_str());
	return true;
}

void PluginV2Methods::SetCallbackStartRecording(CallbackStartRecording cb)
{
	callbackStartRecording = cb;
}

namespace {
	PluginV2Methods::CallbackSetCurrentAccount callbackSetCurrentAccount = NULL;
}

void PluginV2Methods::SetCallbackSetCurrentAccount(CallbackSetCurrentAccount cb)
{
	callbackSetCurrentAccount = cb;
}

bool PluginV2Methods::IsAccountUsable(int accountId)
{
	return (accountId >= 0 && accountId < (int)appSettings.uaConf.accounts.size() &&
		appSettings.uaConf.accounts[accountId].enabled);
}

int PluginV2Methods::GetCurrentAccountId(void)
{
	return appSettings.GetCurrentAccountId();
}

void PluginV2Methods::GetAccounts(std::map<int, Json::Value> &accounts)
{
	int current = appSettings.GetCurrentAccountId();
	for (unsigned int i=0; i<appSettings.uaConf.accounts.size(); i++)
	{
		const UaConf::Account &acc = appSettings.uaConf.accounts[i];
		Json::Value ja(Json::objectValue);
		ja["accountId"] = i;
		ja["name"] = ToUtf8(acc.name.c_str());
		ja["user"] = ToUtf8(acc.user.c_str());
		ja["server"] = ToUtf8(acc.reg_server.c_str());
		ja["enabled"] = acc.enabled;
		ja["current"] = ((int)i == current);
		accounts[i] = ja;
	}
}

namespace {

/** params: accountId (required) - account used by default for outgoing calls / messages */
int SetCurrentAccount(const Json::Value &params, Json::Value &result)
{
	if (!params["accountId"].isInt() && !params["accountId"].isUInt())
		return TSIP_ERR_INVALID_ARG;
	int accountId = params["accountId"].asInt();
	if (!PluginV2Methods::IsAccountUsable(accountId))
	{
		result["error"] = "account does not exist or is disabled";
		return TSIP_ERR_INVALID_ARG;
	}
	if (!callbackSetCurrentAccount)
		return TSIP_ERR_NOT_AVAILABLE;
	callbackSetCurrentAccount(accountId);
	result["currentAccountId"] = appSettings.GetCurrentAccountId();
	return TSIP_OK;
}


}	// namespace

int PluginV2Methods::Execute(const AnsiString &method, const Json::Value &params, Json::Value &result)
{
	if (method == "GetHistory")
		return GetHistory(params, result);
	if (method == "SearchContacts")
		return SearchContacts(params, result);
	if (method == "SendDtmf")
		return SendDtmf(params, result);
	if (method == "Hold")
		return Hold(params, result);
	if (method == "Mute")
		return Mute(params, result);
	if (method == "Sidetone")
		return Sidetone(params, result);
	if (method == "StartRecording")
		return StartRecording(params, result);
	if (method == "SendMessage")
		return SendSimpleMessage(params, result);
	if (method == "SetCurrentAccount")
		return SetCurrentAccount(params, result);
	if (method == "GetMessages")
		return GetMessages(params, result);
	if (method == "ListAudioDevices")
		return ListAudioDevices(params, result);
	if (method == "SetAudioInput")
		return SetAudioInput(params, result);
	if (method == "SetAudioOutput")
		return SetAudioOutput(params, result);
	if (method == "PlayAudioFile")
		return PlayAudioFile(params, result);
	if (method == "GetSettings")
		return GetSettings(params, result);
	if (method == "SetSettings")
		return SetSettings(params, result);
	if (method == "GetButtonSettings")
		return GetButtonSettings(params, result);
	if (method == "SetButtonSettings")
		return SetButtonSettings(params, result);
	if (method == "GetCallAudioState")
		return GetCallAudioState(params, result);
	if (method == "GetCallInvite")
		return GetCallInvite(params, result);
	if (method == "SendCustomRequest")
		return SendCustomRequest(params, result);
	if (method == "GetCustomRequest")
		return GetCustomRequest(params, result);
	if (method == "DeleteCustomRequest")
		return DeleteCustomRequest(params, result);
	if (method == "Transfer")
		return Transfer(params, result);
	return TSIP_ERR_UNKNOWN_METHOD;
}

bool PluginV2Methods::ResolveCallUid(unsigned int &callUid)
{
	if (callUid == 0)
	{
		Call *call = Calls::GetCurrentCall();
		if (call == NULL)
			return false;
		callUid = call->uid;
	}
	return Calls::FindByUid(callUid) != NULL;
}

AnsiString PluginV2Methods::PeerNameToUtf8(const AnsiString &peerName)
{
	// reverse of display name decoding done when call is received
	if (appSettings.Display.bDecodeUtfDisplayToAnsi)
		return ::AnsiToUtf8(peerName);
	else
		return peerName;
}
