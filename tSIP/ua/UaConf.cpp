//---------------------------------------------------------------------------
#pragma hdrstop

#include "UaConf.h"
#include "SettingsAppVersion.h"
#include "Branding.h"
#include "common/MathUtils.h"
#include <json/json.h>

//---------------------------------------------------------------------------

#pragma package(smart_init)


namespace
{
	inline void strncpyz(char* dst, const char* src, int dstsize) {
		strncpy(dst, src, dstsize);
		dst[dstsize-1] = '\0';
	}
}

bool UaConf::AudioAgc::operator==(const UaConf::AudioAgc& right) const {
	return (
		enabled == right.enabled &&
		target == right.target &&
		MathUtils::AlmostEqual(maxGain, right.maxGain) &&
		MathUtils::AlmostEqual(attackRate, right.attackRate) &&
		MathUtils::AlmostEqual(releaseRate, right.releaseRate)
	);
}

bool UaConf::AudioGate::operator==(const UaConf::AudioGate& right) const {
	return (
		enabled == right.enabled &&
		closeThreshold == right.closeThreshold &&
		holdMs == right.holdMs &&
		attackMs == right.attackMs &&
		releaseMs == right.releaseMs
	);
}

bool UaConf::AudioPortaudio::operator==(const UaConf::AudioPortaudio& right) const {
	return (
		MathUtils::AlmostEqual(inSuggestedLatency, right.inSuggestedLatency) &&
		MathUtils::AlmostEqual(outSuggestedLatency, right.outSuggestedLatency)
	);
}

void UaConf::fromJson(const Json::Value& uaConfJson, const struct SettingsAppVersion& settingsAppVersion)
{
	{
		const Json::Value &uaConfAudioCfgSrcJson = uaConfJson["audioCfgSrc"];
		std::string str = audioCfgSrc.mod;
		uaConfAudioCfgSrcJson.getString("mod", str);
		if (AudioModules::IsInput(str))
		{
			audioCfgSrc.mod = str;
		}
		uaConfAudioCfgSrcJson.getString("dev", audioCfgSrc.dev);
		uaConfAudioCfgSrcJson.getString("wavefile", audioCfgSrc.wavefile);
	}

	{
		const Json::Value &uaConfAudioCfgPlayJson = uaConfJson["audioCfgPlay"];
		std::string str = audioCfgPlay.mod;
		uaConfAudioCfgPlayJson.getString("mod", str);
		if (AudioModules::IsOutput(str))
		{
			audioCfgPlay.mod = str;
		}
		uaConfAudioCfgPlayJson.getString("dev", audioCfgPlay.dev);
	}

	{
		const Json::Value &uaConfAudioCfgAlertJson = uaConfJson["audioCfgAlert"];
		std::string str = audioCfgAlert.mod;
		uaConfAudioCfgAlertJson.getString("mod", str);
		if (AudioModules::IsOutput(str))
		{
			audioCfgAlert.mod = str;
		}
		uaConfAudioCfgAlertJson.getString("dev", audioCfgAlert.dev);
	}

	{
		SettingsAppVersion verCfgRingAdded;
		verCfgRingAdded.FileVersionMS = 1;
		verCfgRingAdded.FileVersionLS = 3866629;
		if (settingsAppVersion < verCfgRingAdded)
		{
			// new setting added, separated from "alert" - copy alert if found settings from older version
			audioCfgRing.mod = audioCfgAlert.mod;
			audioCfgRing.dev = audioCfgAlert.dev;
		}
		else
		{
			const Json::Value &uaConfAudioCfgRingJson = uaConfJson["audioCfgRing"];
			std::string str = audioCfgRing.mod;
			uaConfAudioCfgRingJson.getString("mod", str);
			if (AudioModules::IsOutput(str))
			{
				audioCfgRing.mod = str;
			}
			uaConfAudioCfgRingJson.getString("dev", audioCfgRing.dev);
		}
	}

	{
		const Json::Value &uaConfAudioCfgPlayIntercomJson = uaConfJson["audioCfgPlayIntercom"];
		std::string str = audioCfgPlayIntercom.mod;
		uaConfAudioCfgPlayIntercomJson.getString("mod", str);
		if (AudioModules::IsOutput(str))
		{
			audioCfgPlayIntercom.mod = str;
		}
		uaConfAudioCfgPlayIntercomJson.getString("dev", audioCfgPlayIntercom.dev);
	}

	{
		const Json::Value &jv = uaConfJson["audioPortaudio"];
		jv.getDouble("inSuggestedLatency", audioPortaudio.inSuggestedLatency);
		jv.getDouble("outSuggestedLatency", audioPortaudio.outSuggestedLatency);
	}

	{
		const Json::Value &uaConfAudioSoftVol = uaConfJson["audioSoftVol"];
		audioSoftVol.tx = uaConfAudioSoftVol.get("tx", audioSoftVol.tx).asUInt();
		audioSoftVol.rx = uaConfAudioSoftVol.get("rx", audioSoftVol.rx).asUInt();
	}

	{
		const Json::Value &jv = uaConfJson["audioAgcRx"];
		jv.getBool("enabled", audioAgcRx.enabled);
		jv.getUInt("target", audioAgcRx.target);
		jv.getDouble("maxGain", audioAgcRx.maxGain);
		jv.getDouble("attackRate", audioAgcRx.attackRate);
		jv.getDouble("releaseRate", audioAgcRx.releaseRate);
	}

	{
		const Json::Value &jv = uaConfJson["audioGateTx"];
		jv.getBool("enabled", audioGateTx.enabled);
		jv.getUInt("closeThreshold", audioGateTx.closeThreshold);
		jv.getUInt("holdMs", audioGateTx.holdMs);
		jv.getUInt("attackMs", audioGateTx.attackMs);
		jv.getUInt("releaseMs", audioGateTx.releaseMs);
	}

	uaConfJson.getBool("loopRingWithoutSilence", loopRingWithoutSilence);
	uaConfJson.getBool("startAudioSourceAtCallStart", startAudioSourceAtCallStart);

	if (Branding::recording)
	{
		const Json::Value &uaConfRecordingJson = uaConfJson["recording"];
		uaConfRecordingJson.getBool("enabled", recording.enabled);
		UaConf::RecordingCfg::RecDir recDir = static_cast<UaConf::RecordingCfg::RecDir>(uaConfRecordingJson.get("recDir", recording.recDir).asInt());
		if (recDir >= 0 && recDir < UaConf::RecordingCfg::RecDirLimiter) {
			recording.recDir = recDir;
		}
		recording.customRecDir = uaConfRecordingJson.get("customRecDir", recording.customRecDir).asString();
		unsigned int channels = uaConfRecordingJson.get("channels", recording.channels).asUInt();
		if (channels >= UaConf::RecordingCfg::CHANNELS_MIN && channels <= UaConf::RecordingCfg::CHANNELS_MAX) {
			recording.channels = channels;
		}
		recording.side = uaConfRecordingJson.get("side", recording.side).asUInt();
		uaConfRecordingJson.getUInt("fileFormat", recording.fileFormat);
		uaConfRecordingJson.getUInt("bitrate", recording.bitrate);
		UaConf::RecordingCfg::RecStart recStart = static_cast<UaConf::RecordingCfg::RecStart>(uaConfRecordingJson.get("recStart", recording.recStart).asInt());
		if (recStart >= 0 && recStart < UaConf::RecordingCfg::RecStartLimiter) {
			recording.recStart = recStart;
		}
		uaConfRecordingJson.getBool("noNumberB64Encoding", recording.noNumberB64Encoding);
	}
	else
	{
		recording.enabled = false;
	}

	{
		const Json::Value &uaConfAudioPreprocTxJson = uaConfJson["audioPreprocTx"];
		audioPreprocTx.enabled = uaConfAudioPreprocTxJson.get("enabled", audioPreprocTx.enabled).asBool();
		audioPreprocTx.denoiseEnabled = uaConfAudioPreprocTxJson.get("denoiseEnabled", audioPreprocTx.denoiseEnabled).asBool();
		audioPreprocTx.agcEnabled = uaConfAudioPreprocTxJson.get("agcEnabled", audioPreprocTx.agcEnabled).asBool();
		audioPreprocTx.vadEnabled = uaConfAudioPreprocTxJson.get("vadEnabled", audioPreprocTx.vadEnabled).asBool();
		audioPreprocTx.dereverbEnabled = uaConfAudioPreprocTxJson.get("dereverbEnabled", audioPreprocTx.dereverbEnabled).asBool();
		audioPreprocTx.agcLevel = uaConfAudioPreprocTxJson.get("agcLevel", audioPreprocTx.agcLevel).asInt();
	}

	{
		UaConf::Aec aec = static_cast<UaConf::Aec>(uaConfJson.get("aec", aec).asInt());
		if (aec >= 0 && aec < UaConf::AEC_LIMIT)
		{
			this->aec = aec;
		}
	}

	{
		const Json::Value &jv = uaConfJson["webrtcAec"];
		jv.getInt("msInSndCardBuf", webrtcAec.msInSndCardBuf);
		jv.getInt("skew", webrtcAec.skew);
	}

	{
		const Json::Value &jv = uaConfJson["opus"];
		jv.getBool("stereo", opus.stereo);
		jv.getBool("spropStereo", opus.spropStereo);
		jv.getBool("setBitrate", opus.setBitrate);
		jv.getUInt("bitrate", opus.bitrate);
		jv.getBool("setSamplerate", opus.setSamplerate);
		jv.getUInt("samplerate", opus.samplerate);
		jv.getBool("setCbr", opus.setCbr);
		jv.getBool("cbr", opus.cbr);
		jv.getBool("setInbandFec", opus.setInbandFec);
		jv.getBool("inbandFec", opus.inbandFec);
		jv.getBool("setDtx", opus.setDtx);
		jv.getBool("dtx", opus.dtx);
		jv.getBool("mirror", opus.mirror);
		jv.getUInt("complexity", opus.complexity);
		jv.getBool("setApplication", opus.setApplication);
		jv.getInt("application", opus.application);
		jv.getBool("setPacketLoss", opus.setPacketLoss);
		jv.getUInt("packetLoss", opus.packetLoss);
	}

	{
		const Json::Value &jv = uaConfJson["zrtp"];
		jv.getBool("startParallel", zrtp.startParallel);
	}

	{
		const Json::Value &jv = uaConfJson["tls"];
		jv.getString("certificate", tls.certificate);
		jv.getString("caFile", tls.caFile);
		jv.getBool("useWindowsRootCaStore", tls.useWindowsRootCaStore);
		jv.getBool("verifyServer", tls.verifyServer);
	}

	logMessages = uaConfJson.get("logMessages", logMessages).asBool();
	uaConfJson.getBool("logMessagesOnlyFirstLine", logMessagesOnlyFirstLine);
	uaConfJson.getBool("logAubuf", logAubuf);
	local = uaConfJson.get("localAddress", local).asString();
	ifname = uaConfJson.get("ifName", ifname).asString();

	{
		const Json::Value &uaAvtJson = uaConfJson["avt"];
		UaConf::Avt prev = avt;
		avt.portMin = uaAvtJson.get("portMin", avt.portMin).asUInt();
		avt.portMax = uaAvtJson.get("portMax", avt.portMax).asUInt();
		avt.jbufDelayMin = uaAvtJson.get("jbufDelayMin", avt.jbufDelayMin).asUInt();
		avt.jbufDelayMax = uaAvtJson.get("jbufDelayMax", avt.jbufDelayMax).asUInt();
		avt.rtpTimeout = uaAvtJson.get("rtpTimeout", avt.rtpTimeout).asUInt();
		if (avt.Validate())
		{
			avt = prev;
		}
	}

	{
		const Json::Value &uaMessagesJson = uaConfJson["messages"];
		uaMessagesJson.getInt("replyCode", messages.replyCode);
		uaMessagesJson.getString("replyReason", messages.replyReason);
		uaMessagesJson.getBool("doNotReply", messages.doNotReply);
	}

	autoAnswer = uaConfJson.get("autoAnswer", autoAnswer).asBool();
	autoAnswerCode = uaConfJson.get("autoAnswerCode", autoAnswerCode).asInt();
	uaConfJson.getString("autoAnswerReason", autoAnswerReason);
	unsigned int prevAutoAnswerDelayMin = autoAnswerDelayMin;
	unsigned int prevAutoAnswerDelayMax = autoAnswerDelayMax;
	autoAnswerDelayMin = uaConfJson.get("autoAnswerDelayMin", autoAnswerDelayMin). asUInt();
	autoAnswerDelayMax = uaConfJson.get("autoAnswerDelayMax", autoAnswerDelayMax). asUInt();
	if (autoAnswerDelayMin > autoAnswerDelayMax)
	{
		autoAnswerDelayMin = prevAutoAnswerDelayMin;
		autoAnswerDelayMax = prevAutoAnswerDelayMax;
	}
	autoAnswerCallInfo = uaConfJson.get("autoAnswerCallInfo", autoAnswerCallInfo).asBool();
	autoAnswerCallInfoDelayMin = uaConfJson.get("autoAnswerCallInfoDelayMin", autoAnswerCallInfoDelayMin).asUInt();

	answerOnEventTalk = uaConfJson.get("answerOnEventTalk", answerOnEventTalk).asBool();

	handleOodRefer = uaConfJson.get("handleOodRefer", handleOodRefer).asBool();

	customUserAgent = uaConfJson.get("customUserAgent", customUserAgent).asBool();
	userAgent = uaConfJson.get("userAgent", userAgent).asString();

	{
		const Json::Value &jv = uaConfJson["video"];
		{
			const Json::Value &jSource = jv["videoSource"];
			if (jSource.type() == Json::objectValue)
			{
				jSource.getString("mod", video.videoSource.mod);
				jSource.getString("dev", video.videoSource.dev);
			}
		}
		jv.getUInt("width", video.width);
		jv.getUInt("height", video.height);
		jv.getUInt("bitrate", video.bitrate);
		jv.getUInt("fps", video.fps);
		{
			const Json::Value &jsv = jv["selfview"];
			Video::Selfview &selfview = video.selfview;
			jsv.getBool("enabled", selfview.enabled);
			jsv.getBool("pip", selfview.pip);
		}
		{
			const Json::Value &j = jv["dshow"];
			Video::Dshow &dshow = video.dshow;
			j.getBool("skipReadingBackMediaFormat", dshow.skipReadingBackMediaFormat);
		}
	}
}

void UaConf::toJson(Json::Value& uaConfJson) const
{
	uaConfJson["audioCfgSrc"]["mod"] = audioCfgSrc.mod;
	uaConfJson["audioCfgSrc"]["dev"] = audioCfgSrc.dev;
	uaConfJson["audioCfgSrc"]["wavefile"] = audioCfgSrc.wavefile;
	uaConfJson["audioCfgPlay"]["mod"] = audioCfgPlay.mod;
	uaConfJson["audioCfgPlay"]["dev"] = audioCfgPlay.dev;
	uaConfJson["audioCfgAlert"]["mod"] = audioCfgAlert.mod;
	uaConfJson["audioCfgAlert"]["dev"] = audioCfgAlert.dev;
	uaConfJson["audioCfgRing"]["mod"] = audioCfgRing.mod;
	uaConfJson["audioCfgRing"]["dev"] = audioCfgRing.dev;
	uaConfJson["audioCfgPlayIntercom"]["mod"] = audioCfgPlayIntercom.mod;
	uaConfJson["audioCfgPlayIntercom"]["dev"] = audioCfgPlayIntercom.dev;

	uaConfJson["audioPortaudio"]["inSuggestedLatency"] = audioPortaudio.inSuggestedLatency;
	uaConfJson["audioPortaudio"]["outSuggestedLatency"] = audioPortaudio.outSuggestedLatency;

	uaConfJson["audioSoftVol"]["tx"] = audioSoftVol.tx;
	uaConfJson["audioSoftVol"]["rx"] = audioSoftVol.rx;

	{
		Json::Value &jv = uaConfJson["audioAgcRx"];
		jv["enabled"] = audioAgcRx.enabled;
		jv["target"] = audioAgcRx.target;
		jv["maxGain"] = audioAgcRx.maxGain;
		jv["attackRate"] = audioAgcRx.attackRate;
		jv["releaseRate"] = audioAgcRx.releaseRate;
	}

	{
		Json::Value &jv = uaConfJson["audioGateTx"];
		jv["enabled"] = audioGateTx.enabled;
		jv["closeThreshold"] = audioGateTx.closeThreshold;
		jv["holdMs"] = audioGateTx.holdMs;
		jv["attackMs"] = audioGateTx.attackMs;
		jv["releaseMs"] = audioGateTx.releaseMs;
	}


	uaConfJson["loopRingWithoutSilence"] = loopRingWithoutSilence;
	uaConfJson["startAudioSourceAtCallStart"] = startAudioSourceAtCallStart;

	if (Branding::recording)
	{
		Json::Value &jv = uaConfJson["recording"];
		jv["enabled"] = recording.enabled;
		jv["recDir"] = recording.recDir;
		jv["customRecDir"] = recording.customRecDir;
		jv["channels"] = recording.channels;
		jv["side"] = recording.side;
		jv["fileFormat"] = recording.fileFormat;
		jv["bitrate"] = recording.bitrate;
		jv["recStart"] = recording.recStart;
		jv["noNumberB64Encoding"] = recording.noNumberB64Encoding;
	}

	{
		Json::Value &cfgJson = uaConfJson["audioPreprocTx"];
		cfgJson["enabled"] = audioPreprocTx.enabled;
		cfgJson["denoiseEnabled"] = audioPreprocTx.denoiseEnabled;
		cfgJson["agcEnabled"] = audioPreprocTx.agcEnabled;
		cfgJson["vadEnabled"] = audioPreprocTx.vadEnabled;
		cfgJson["dereverbEnabled"] = audioPreprocTx.dereverbEnabled;
		cfgJson["agcLevel"] = audioPreprocTx.agcLevel;
	}

	uaConfJson["aec"] = aec;
	uaConfJson["logMessages"] = logMessages;
	uaConfJson["logMessagesOnlyFirstLine"] = logMessagesOnlyFirstLine;
	uaConfJson["logAubuf"] = logAubuf;

	uaConfJson["localAddress"] = local;
	uaConfJson["ifName"] = ifname;
	uaConfJson["avt"]["portMin"] = avt.portMin;
	uaConfJson["avt"]["portMax"] = avt.portMax;
	uaConfJson["avt"]["jbufDelayMin"] = avt.jbufDelayMin;
	uaConfJson["avt"]["jbufDelayMax"] = avt.jbufDelayMax;
	uaConfJson["avt"]["rtpTimeout"] = avt.rtpTimeout;

	{
		Json::Value &jv = uaConfJson["messages"];
		jv["replyCode"] = messages.replyCode;
		jv["replyReason"] = messages.replyReason;
		jv["doNotReply"] = messages.doNotReply;
	}

	uaConfJson["autoAnswer"] = autoAnswer;
	uaConfJson["autoAnswerCode"] = autoAnswerCode;
	uaConfJson["autoAnswerReason"] = autoAnswerReason;
	uaConfJson["autoAnswerDelayMin"] = autoAnswerDelayMin;
	uaConfJson["autoAnswerDelayMax"] = autoAnswerDelayMax;
	uaConfJson["autoAnswerCallInfo"] = autoAnswerCallInfo;
	uaConfJson["autoAnswerCallInfoDelayMin"] = autoAnswerCallInfoDelayMin;

	uaConfJson["answerOnEventTalk"] = answerOnEventTalk;

	uaConfJson["handleOodRefer"] = handleOodRefer;

	uaConfJson["customUserAgent"] = customUserAgent;
	uaConfJson["userAgent"] = userAgent;

	uaConfJson["webrtcAec"]["msInSndCardBuf"] = webrtcAec.msInSndCardBuf;
	uaConfJson["webrtcAec"]["skew"] = webrtcAec.skew;

	{
		Json::Value &jv = uaConfJson["opus"];
		jv["stereo"] = opus.stereo;
		jv["spropStereo"] =  opus.spropStereo;
		jv["setBitrate"] = opus.setBitrate;
		jv["bitrate"] = opus.bitrate;
		jv["setSamplerate"] = opus.setSamplerate;
		jv["samplerate"] = opus.samplerate;
		jv["setCbr"] = opus.setCbr;
		jv["cbr"] = opus.cbr;
		jv["setInbandFec"] = opus.setInbandFec;
		jv["inbandFec"] = opus.inbandFec;
		jv["setDtx"] = opus.setDtx;
		jv["dtx"] = opus.dtx;
		jv["mirror"] = opus.mirror;
		jv["complexity"] = opus.complexity;
		jv["setApplication"] = opus.setApplication;
		jv["application"] = opus.application;
		jv["setPacketLoss"] = opus.setPacketLoss;
		jv["packetLoss"] = opus.packetLoss;
	}

	{
		Json::Value &jv = uaConfJson["zrtp"];
		jv["startParallel"] = zrtp.startParallel;
	}

	{
		Json::Value &jv = uaConfJson["tls"];
		jv["certificate"] = tls.certificate;
		jv["caFile"] = tls.caFile;
		jv["useWindowsRootCaStore"] = tls.useWindowsRootCaStore;
		jv["verifyServer"] = tls.verifyServer;
	}

	{
		Json::Value &jv = uaConfJson["video"];
		{
			Json::Value &jSource = jv["videoSource"];
			const Video::Device &device = video.videoSource;
			jSource["mod"] = device.mod;
			jSource["dev"] = device.dev;
		}
		jv["width"] = video.width;
		jv["height"] = video.height;
		jv["bitrate"] = video.bitrate;
		jv["fps"] = video.fps;
		{
			Json::Value &jsv = jv["selfview"];
			const Video::Selfview &selfview = video.selfview;
			jsv["enabled"] = selfview.enabled;
			jsv["pip"] = selfview.pip;
		}
		{
			Json::Value &j = jv["dshow"];
			const Video::Dshow &dshow = video.dshow;
			j["skipReadingBackMediaFormat"] = dshow.skipReadingBackMediaFormat;
		}
	}

}

