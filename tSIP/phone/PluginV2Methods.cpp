//---------------------------------------------------------------------------
#pragma hdrstop

#include "PluginV2Methods.h"
#include "tSIP_plugin_v2.h"
#include "Globals.h"
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
	SIMPLE_Messages::Send(::Utf8ToAnsi(params["target"].asString().c_str()),
		::Utf8ToAnsi(params["text"].asString().c_str()), true);
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
		AnsiString uri = (AnsiString)"sip:" + peer + "@" + appSettings.uaConf.accounts[0].reg_server.c_str();
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
		status = UaCustomRequests::Send(requestUid, uri.c_str(), method.c_str(), headerLines);
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
