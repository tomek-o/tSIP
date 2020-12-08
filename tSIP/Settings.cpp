//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "Settings.h"
#include "common\KeybKeys.h"
#include "common\Utilities.h"
#include "ProgrammableButtons.h"
#include "AudioModules.h"
#include "Branding.h"
#include <algorithm>
#include <fstream>
#include <json/json.h>

//---------------------------------------------------------------------------

#pragma package(smart_init)

Settings appSettings;

inline void strncpyz(char* dst, const char* src, int dstsize) {
	strncpy(dst, src, dstsize);
	dst[dstsize-1] = '\0';
}

Font::Font(void)
{
	name = "Tahoma";
	size = 8;
	style = TFontStyles();
}

Settings::_frmMain::_frmMain(void):
	iPosX(30),
	iPosY(30),
	iWidth(273),
	iHeight(425),
	bWindowMaximized(false),
	bAlwaysOnTop(false),
	bStartMinimizedToTray(false),	
    bSpeedDialVisible(false),
	iSpeedDialSize(1),	// default: 2 columns
	bSpeedDialOnly(false),
	bSpeedDialPopupMenu(true),
	bSpeedDialIgnorePresenceNote(false),
	bSpeedDialIgnoreDialogInfoRemoteIdentity(false),
	bSpeedDialKeepPreviousDialogInfoRemoteIdentityIfMissing(false),
	bSpeedDialIgnoreOrClearDialogInfoRemoteIdentityIfTerminated(true),
	bXBtnMinimize(false),
	bRestoreOnIncomingCall(false),
	bSingleInstance(false),
	bNoBeepOnEnterKey(false),
	bHideSettings(false),
	bHideView(false),
	bHideTools(false),
	bHideHelp(false),
	bKioskMode(false),
	bHideStatusBar(false),
	bHideMainMenu(false),
	bHideDialpad(false),
	bHideSpeedDialToggleButton(false),
	bHideMouseCursor(false),
	bShowWhenAnsweringCall(false),
	bShowWhenMakingCall(false),
	bUseCustomCaption(false),
	customCaption(Branding::appName),
	bUseCustomApplicationTitle(false),
	customApplicationTitle(Branding::appName),
	bShowSettingsIfAccountSettingIsHidden(false),
	bNoTaskbarButtonRestore(false),
	bNoTrayIcon(false),
	layout(0),
	dialComboboxOrder(DialComboboxOrderByNumber)
{
	speedDialWidth.clear();
	for (unsigned int i=0; i<ProgrammableButtons::EXT_CONSOLE_COLUMNS; i++)
	{
		speedDialWidth.push_back(105);
	}
}

Settings::_frmTrayNotifier::_frmTrayNotifier(void):
	iHeight(105),
	iWidth(213),
	iPosX(30),	// overriden later, depending on screen size
	iPosY(30),	// overriden later, depending on screen size
	showOnIncoming(false),
	skipIfMainWindowVisible(false),
	showOnOutgoing(false),
	hideWhenAnsweringCall(false),
	hideWhenAnsweringCallAutomatically(false),
	scalingPct(SCALING_DEF),
	doNotChangePosition(false)
{
	int maxX = GetSystemMetrics(SM_CXSCREEN);
	/** \todo Ugly fixed taskbar margin */
	int maxY = GetSystemMetrics(SM_CYSCREEN) - 32;
	iPosX = maxX - iWidth;
	iPosY = maxY - iHeight;
}

Settings::_frmContactPopup::_frmContactPopup(void):
	showOnIncoming(false),
	showOnOutgoing(false),
	iPosX(100),
	iPosY(100),
	iWidth(400),
	iHeight(140)
{
}

Settings::_Integration::_Integration(void):
	bAddFilterWMCopyData(false),
	asProtocol(Branding::appProto),
	bDoNotUseSipPrefixForDirectIpCalls(false),
	bDoNotPassParametersToPreviousInstance(false),
	bAcceptCommandLineScript(false)
{
}

Settings::Settings(void)
{
	int maxX = GetSystemMetrics(SM_CXSCREEN);
	/** \todo Ugly fixed taskbar margin */
	int maxY = GetSystemMetrics(SM_CYSCREEN) - 32;
	
	frmTrayNotifier.iPosX = maxX - frmTrayNotifier.iWidth;
	frmTrayNotifier.iPosY = maxY - frmTrayNotifier.iHeight;

	Display.bUserOnlyClip = false;
	Display.bDecodeUtfDisplayToAnsi = false;
	Display.bUsePAssertedIdentity = false;

	Ring.defaultRing = "ring.wav";
	for (int i=0; i<sizeof(Ring.bellcore)/sizeof(Ring.bellcore[0]); i++)
	{
		Ring.bellcore[i] = "ring.wav";
	}

	HttpQuery.url = "https://www.google.com/search?q=[number]";
	HttpQuery.openMode = _HttpQuery::openManualOnly;

	SipAccessUrl.accessMode = _SipAccessUrl::accessModeAlwaysPassive;

	History.bNoStoreToFile = false;
	History.bUsePaiForDisplayIfAvailable = true;
	History.bUsePaiForDialIfAvailable = true;
	History.bShowHint = true;
	History.bFormatCallDurationAsHourMinSec = true;
	History.bShowCodecNameInHint = true;

    Messages.ring = "pluck.wav";

	Scripts.timer = 1000;
	Scripts.timer2 = 1000;

	uaConf.accounts.clear();

	struct UaConf::Account new_acc;
	uaConf.accounts.push_back(new_acc);

	ScriptWindow.ClearMruItems();
}

int Settings::UpdateFromText(AnsiString text)
{
	Json::Value root;   // will contains the root value after parsing.
	Json::Reader reader;

	try
	{
		bool parsingSuccessful = reader.parse( text.c_str(), root );
		if ( !parsingSuccessful )
		{
			return 2;
		}
	}
	catch(...)
	{
		return 1;
	}

	return UpdateFromJsonValue(root);
}

int Settings::Read(AnsiString asFileName)
{
	Json::Value root;   // will contains the root value after parsing.
	Json::Reader reader;

	try
	{
		std::ifstream ifs(asFileName.c_str());
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
	
	return UpdateFromJsonValue(root);
}

int Settings::UpdateFromJsonValue(const Json::Value &root)
{
	{
		const Json::Value &jv = root["info"];
		jv.getUInt("FileVersionMS", info.appVersion.FileVersionMS);
		jv.getUInt("FileVersionLS", info.appVersion.FileVersionLS);
	}

	{
		const Json::Value &uaConfJson = root["uaConf"];

		{
			const Json::Value &uaConfAudioCfgSrcJson = uaConfJson["audioCfgSrc"];
			char str[UaConf::AudioCfg::MAX_MOD_LENGTH];
			strncpyz(str, uaConfAudioCfgSrcJson.get("mod", uaConf.audioCfgSrc.mod).asString().c_str(), sizeof(str));
			if (AudioModules::IsInput(str))
			{
				strncpyz(uaConf.audioCfgSrc.mod, str, sizeof(uaConf.audioCfgSrc.mod));
			}
			strncpyz(uaConf.audioCfgSrc.dev, uaConfAudioCfgSrcJson["dev"].asString().c_str(), sizeof(uaConf.audioCfgSrc.dev));
			strncpyz(uaConf.audioCfgSrc.wavefile, uaConfAudioCfgSrcJson["wavefile"].asString().c_str(), sizeof(uaConf.audioCfgSrc.wavefile));
		}

		{
			const Json::Value &uaConfAudioCfgPlayJson = uaConfJson["audioCfgPlay"];
			char str[UaConf::AudioCfg::MAX_MOD_LENGTH];
			strncpyz(str, uaConfAudioCfgPlayJson.get("mod", uaConf.audioCfgPlay.mod).asString().c_str(), sizeof(str));
			if (AudioModules::IsOutput(str))
			{
				strncpyz(uaConf.audioCfgPlay.mod, str, sizeof(uaConf.audioCfgPlay.mod));
			}
			strncpyz(uaConf.audioCfgPlay.dev, uaConfAudioCfgPlayJson["dev"].asString().c_str(), sizeof(uaConf.audioCfgPlay.dev));
		}

		{
			const Json::Value &uaConfAudioCfgAlertJson = uaConfJson["audioCfgAlert"];
			char str[UaConf::AudioCfg::MAX_MOD_LENGTH];
			strncpyz(str, uaConfAudioCfgAlertJson.get("mod", uaConf.audioCfgAlert.mod).asString().c_str(), sizeof(str));
			if (AudioModules::IsOutput(str))
			{
				strncpyz(uaConf.audioCfgAlert.mod, str, sizeof(uaConf.audioCfgAlert.mod));
			}
			strncpyz(uaConf.audioCfgAlert.dev, uaConfAudioCfgAlertJson["dev"].asString().c_str(), sizeof(uaConf.audioCfgAlert.dev));
		}
		
		{
			Settings::_info::_appVersion verCfgRingAdded;
			verCfgRingAdded.FileVersionMS = 1;
			verCfgRingAdded.FileVersionLS = 3866629;
			if (info.appVersion < verCfgRingAdded)
			{
				// new setting added, separated from "alert" - copy alert if found settings from older version
				strncpyz(uaConf.audioCfgRing.mod, uaConf.audioCfgAlert.mod, sizeof(uaConf.audioCfgRing.mod));
				strncpyz(uaConf.audioCfgRing.dev, uaConf.audioCfgAlert.dev, sizeof(uaConf.audioCfgRing.dev));
			}
			else
			{
				const Json::Value &uaConfAudioCfgRingJson = uaConfJson["audioCfgRing"];
				char str[UaConf::AudioCfg::MAX_MOD_LENGTH];
				strncpyz(str, uaConfAudioCfgRingJson.get("mod", uaConf.audioCfgRing.mod).asString().c_str(), sizeof(str));
				if (AudioModules::IsOutput(str))
				{
					strncpyz(uaConf.audioCfgRing.mod, str, sizeof(uaConf.audioCfgRing.mod));
				}
				strncpyz(uaConf.audioCfgRing.dev, uaConfAudioCfgRingJson["dev"].asString().c_str(), sizeof(uaConf.audioCfgRing.dev));
			}
		}

		{
			const Json::Value &uaConfAudioCfgPlayIntercomJson = uaConfJson["audioCfgPlayIntercom"];
			char str[UaConf::AudioCfg::MAX_MOD_LENGTH];
			strncpyz(str, uaConfAudioCfgPlayIntercomJson.get("mod", uaConf.audioCfgPlayIntercom.mod).asString().c_str(), sizeof(str));
			if (AudioModules::IsOutput(str))
			{
				strncpyz(uaConf.audioCfgPlayIntercom.mod, str, sizeof(uaConf.audioCfgPlayIntercom.mod));
			}
			strncpyz(uaConf.audioCfgPlayIntercom.dev, uaConfAudioCfgPlayIntercomJson["dev"].asString().c_str(), sizeof(uaConf.audioCfgPlayIntercom.dev));
		}

		{
			const Json::Value &jv = uaConfJson["audioPortaudio"];
			jv.getDouble("inSuggestedLatency", uaConf.audioPortaudio.inSuggestedLatency);
			jv.getDouble("outSuggestedLatency", uaConf.audioPortaudio.outSuggestedLatency);
		}

		{
			const Json::Value &uaConfAudioSoftVol = uaConfJson["audioSoftVol"];
			uaConf.audioSoftVol.tx = uaConfAudioSoftVol.get("tx", uaConf.audioSoftVol.tx).asUInt();
			uaConf.audioSoftVol.rx = uaConfAudioSoftVol.get("rx", uaConf.audioSoftVol.rx).asUInt();
		}

		{
			const Json::Value &jv = uaConfJson["audioAgcRx"];
			jv.getBool("enabled", uaConf.audioAgcRx.enabled);
			jv.getUInt("target", uaConf.audioAgcRx.target);
			jv.getDouble("maxGain", uaConf.audioAgcRx.maxGain);
			jv.getDouble("attackRate", uaConf.audioAgcRx.attackRate);
			jv.getDouble("releaseRate", uaConf.audioAgcRx.releaseRate);
		}

		if (Branding::recording)
		{
			const Json::Value &uaConfRecordingJson = uaConfJson["recording"];
			uaConfRecordingJson.getBool("enabled", uaConf.recording.enabled);
			UaConf::RecordingCfg::RecDir recDir = static_cast<UaConf::RecordingCfg::RecDir>(uaConfRecordingJson.get("recDir", uaConf.recording.recDir).asInt());
			if (recDir >= 0 && uaConf.recording.recDir < UaConf::RecordingCfg::RecDirLimiter) {
				uaConf.recording.recDir = recDir;
			}
			uaConf.recording.customRecDir = uaConfRecordingJson.get("customRecDir", uaConf.recording.customRecDir).asString();
			unsigned int channels = uaConfRecordingJson.get("channels", uaConf.recording.channels).asUInt();
			if (channels >= UaConf::RecordingCfg::CHANNELS_MIN && channels <= UaConf::RecordingCfg::CHANNELS_MAX) {
				uaConf.recording.channels = channels;
			}
			uaConf.recording.side = uaConfRecordingJson.get("side", uaConf.recording.side).asUInt();
			UaConf::RecordingCfg::RecStart recStart = static_cast<UaConf::RecordingCfg::RecStart>(uaConfRecordingJson.get("recStart", uaConf.recording.recStart).asInt());
			if (recStart >= 0 && recStart < UaConf::RecordingCfg::RecStartLimiter) {
				uaConf.recording.recStart = recStart;
			}
			uaConfRecordingJson.getBool("noNumberB64Encoding", uaConf.recording.noNumberB64Encoding);
		}
		else
		{
			uaConf.recording.enabled = false;
		}

		{
			const Json::Value &uaConfAudioPreprocTxJson = uaConfJson["audioPreprocTx"];
			uaConf.audioPreprocTx.enabled = uaConfAudioPreprocTxJson.get("enabled", uaConf.audioPreprocTx.enabled).asBool();
			uaConf.audioPreprocTx.denoiseEnabled = uaConfAudioPreprocTxJson.get("denoiseEnabled", uaConf.audioPreprocTx.denoiseEnabled).asBool();
			uaConf.audioPreprocTx.agcEnabled = uaConfAudioPreprocTxJson.get("agcEnabled", uaConf.audioPreprocTx.agcEnabled).asBool();
			uaConf.audioPreprocTx.vadEnabled = uaConfAudioPreprocTxJson.get("vadEnabled", uaConf.audioPreprocTx.vadEnabled).asBool();
			uaConf.audioPreprocTx.dereverbEnabled = uaConfAudioPreprocTxJson.get("dereverbEnabled", uaConf.audioPreprocTx.dereverbEnabled).asBool();
			uaConf.audioPreprocTx.agcLevel = uaConfAudioPreprocTxJson.get("agcLevel", uaConf.audioPreprocTx.agcLevel).asInt();
		}

		{
			UaConf::Aec aec = static_cast<UaConf::Aec>(uaConfJson.get("aec", uaConf.aec).asInt());
			if (aec >= 0 && uaConf.aec < UaConf::AEC_LIMIT)
			{
				uaConf.aec = aec;
			}
		}

		{
			const Json::Value &webrtcAec = uaConfJson["webrtcAec"];
			uaConf.webrtcAec.msInSndCardBuf = webrtcAec.get("msInSndCardBuf", uaConf.webrtcAec.msInSndCardBuf).asInt();
			uaConf.webrtcAec.skew = webrtcAec.get("skew", uaConf.webrtcAec.skew).asInt();
		}

		{
			const Json::Value &jv = uaConfJson["opus"];
			UaConf::Opus &opus = uaConf.opus;
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

		uaConf.logMessages = uaConfJson.get("logMessages", uaConf.logMessages).asBool();
		uaConfJson.getBool("logMessagesOnlyFirstLine", uaConf.logMessagesOnlyFirstLine);
        uaConfJson.getBool("logAubuf", uaConf.logAubuf);
		uaConf.local = uaConfJson.get("localAddress", uaConf.local).asString();
		uaConf.ifname = uaConfJson.get("ifName", uaConf.ifname).asString();

		{
			const Json::Value &uaAvtJson = uaConfJson["avt"];
			UaConf::Avt prev = uaConf.avt;
			uaConf.avt.portMin = uaAvtJson.get("portMin", uaConf.avt.portMin).asUInt();
			uaConf.avt.portMax = uaAvtJson.get("portMax", uaConf.avt.portMax).asUInt();
			uaConf.avt.jbufDelayMin = uaAvtJson.get("jbufDelayMin", uaConf.avt.jbufDelayMin).asUInt();
			uaConf.avt.jbufDelayMax = uaAvtJson.get("jbufDelayMax", uaConf.avt.jbufDelayMax).asUInt();
			uaConf.avt.rtpTimeout = uaAvtJson.get("rtpTimeout", uaConf.avt.rtpTimeout).asUInt();
			if (uaConf.avt.Validate())
			{
				uaConf.avt = prev;
			}
		}

		{
			const Json::Value &uaMessagesJson = uaConfJson["messages"];
			uaMessagesJson.getInt("replyCode", uaConf.messages.replyCode);
			uaMessagesJson.getString("replyReason", uaConf.messages.replyReason);
			uaMessagesJson.getBool("doNotReply", uaConf.messages.doNotReply);
		}

		uaConf.autoAnswer = uaConfJson.get("autoAnswer", uaConf.autoAnswer).asBool();
		uaConf.autoAnswerCode = uaConfJson.get("autoAnswerCode", uaConf.autoAnswerCode).asInt();
		unsigned int prevAutoAnswerDelayMin = uaConf.autoAnswerDelayMin;
		unsigned int prevAutoAnswerDelayMax = uaConf.autoAnswerDelayMax;
		uaConf.autoAnswerDelayMin = uaConfJson.get("autoAnswerDelayMin", uaConf.autoAnswerDelayMin). asUInt();
		uaConf.autoAnswerDelayMax = uaConfJson.get("autoAnswerDelayMax", uaConf.autoAnswerDelayMax). asUInt();
		if (uaConf.autoAnswerDelayMin > uaConf.autoAnswerDelayMax)
		{
			uaConf.autoAnswerDelayMin = prevAutoAnswerDelayMin;
			uaConf.autoAnswerDelayMax = prevAutoAnswerDelayMax;
		}
		uaConf.autoAnswerCallInfo = uaConfJson.get("autoAnswerCallInfo", uaConf.autoAnswerCallInfo).asBool();
		uaConf.autoAnswerCallInfoDelayMin = uaConfJson.get("autoAnswerCallInfoDelayMin", uaConf.autoAnswerCallInfoDelayMin).asUInt();

		uaConf.answerOnEventTalk = uaConfJson.get("answerOnEventTalk", uaConf.answerOnEventTalk).asBool();

		uaConf.handleOodRefer = uaConfJson.get("handleOodRefer", uaConf.handleOodRefer).asBool();

		uaConf.customUserAgent = uaConfJson.get("customUserAgent", uaConf.customUserAgent).asBool();
		uaConf.userAgent = uaConfJson.get("userAgent", uaConf.userAgent).asString();
	}

	{
		const Json::Value &accounts = root["Accounts"];
		uaConf.accounts.resize(1);
		for (int i=0; i<std::min(1u, accounts.size()); i++)
		{
			const Json::Value &acc = accounts[i];
			struct UaConf::Account new_acc = uaConf.accounts[i];

			new_acc.reg_server = acc.get("reg_server", new_acc.reg_server).asString();
			new_acc.user = acc.get("user", new_acc.user).asString();
			new_acc.display_name = acc.get("display_name", new_acc.display_name).asString();
			new_acc.auth_user = acc.get("auth_user", new_acc.auth_user).asString();
			new_acc.pwd = acc.get("pwd", new_acc.pwd).asString();
			new_acc.cuser = acc.get("cuser", new_acc.cuser).asString();
			UaConf::Account::TRANSPORT_TYPE transport =
				(UaConf::Account::TRANSPORT_TYPE)acc.get("transport", new_acc.transport).asInt();
			if (transport >= 0 && transport < UaConf::Account::TRANSPORT_LIMITER)
			{
				new_acc.transport = transport;
			}
			// do not register by default
			// (long timeout when unregistering from non-existing server, when application is closing
			// or when account settings are changed to valid)
			int reg_expires = acc.get("reg_expires", new_acc.reg_expires).asInt();
			if (reg_expires >= 0 && reg_expires < 14400)
			{
				new_acc.reg_expires = reg_expires;
			}
			new_acc.dtmf_tx_format = acc.get("dtmf_tx_format", new_acc.dtmf_tx_format).asInt();
			new_acc.answer_any = acc.get("answer_any", new_acc.answer_any).asBool();
			int ptime = acc.get("ptime", new_acc.ptime).asInt();
			if (ptime >= UaConf::Account::MIN_PTIME && new_acc.ptime < UaConf::Account::MAX_PTIME)
			{
				new_acc.ptime = ptime;
			}

			new_acc.hide_reg_server = acc.get("hide_reg_server", new_acc.hide_reg_server).asBool();
			new_acc.hide_user = acc.get("hide_user", new_acc.hide_user).asBool();
			new_acc.hide_auth_user = acc.get("hide_auth_user", new_acc.hide_auth_user).asBool();
			new_acc.hide_pwd = acc.get("hide_pwd", new_acc.hide_pwd).asBool();
			new_acc.hide_cuser = acc.get("hide_cuser", new_acc.hide_cuser).asBool();

			new_acc.stun_server = acc.get("stun_server", new_acc.stun_server).asString();
			new_acc.outbound1 = acc.get("outbound1", new_acc.outbound1).asString();
			new_acc.outbound2 = acc.get("outbound2", new_acc.outbound2).asString();

			const Json::Value &audio_codecs = acc["audio_codecs"];
			if (audio_codecs.type() == Json::arrayValue)
			{
				new_acc.audio_codecs.resize(audio_codecs.size());
				for (int i=0; i<audio_codecs.size(); i++)
				{
					const Json::Value &codec = audio_codecs[i];
					if (codec.type() == Json::stringValue)
					{
						new_acc.audio_codecs[i] = codec.asString();
					}
				}
			}

			uaConf.accounts[0] = new_acc;
		}
	}

	{
		const Json::Value &hotkeyConfJson = root["hotkeyConf"];
		if (hotkeyConfJson.type() == Json::arrayValue)
		{
            hotKeyConf.resize(hotkeyConfJson.size());
			std::list<HotKeyConf>::iterator iter = hotKeyConf.begin();

			for (int i=0; i<hotkeyConfJson.size(); i++)
			{
				const Json::Value &hotkeyJson = hotkeyConfJson[i];
				class HotKeyConf &cfg = *iter++;
				if (hotkeyJson.type() == Json::objectValue)
				{
					cfg.keyCode = hotkeyJson.get("keyCode", cfg.keyCode.c_str()).asString().c_str();
					int id = vkey_find(cfg.keyCode.c_str());
					if (id >= 0)
					{
						cfg.vkCode = vkey_list[id].vkey;
					}
					else
					{
						cfg.vkCode = -1;
					}
					cfg.modifiers = hotkeyJson.get("modifiers", cfg.modifiers).asInt();
					cfg.global = hotkeyJson.get("global", cfg.global).asBool();
					const Json::Value &action = hotkeyJson["action"];
					cfg.action.type = static_cast<Action::Type>(action.get("type", cfg.action.type).asInt());
					cfg.action.id = action.get("id", cfg.action.id).asInt();
				}
			}
		}
	}

	{
		const Json::Value &phoneConfJson = root["phoneConf"];
		if (phoneConfJson.type() == Json::arrayValue)
		{
			phoneConf.resize(phoneConfJson.size());
			std::list<PhoneConf>::iterator iter = phoneConf.begin();
			for (int i=0; i<phoneConfJson.size(); i++)
			{
				const Json::Value &phoneJson = phoneConfJson[i];
				PhoneConf &cfg = *iter++;
				cfg.dllName = phoneJson.get("dllName", cfg.dllName.c_str()).asString().c_str();
			}
		}
	}

	{
		const Json::Value &guiJson = root["gui"];
		int scalingPct = guiJson.get("scalingPct", gui.scalingPct).asInt();
		if (gui.scalingPct >= gui.SCALING_MIN && gui.scalingPct < gui.SCALING_MAX) {
			gui.scalingPct = scalingPct;
		}
	}

	int maxX = GetSystemMetrics(SM_CXSCREEN);
	/** \todo Ugly fixed taskbar margin */
	int maxY = GetSystemMetrics(SM_CYSCREEN) - 32;

	{
		const Json::Value &frmMainJson = root["frmMain"];
		int iWidth = frmMainJson.get("AppWidth", frmMain.iWidth).asInt();
		if (iWidth >= 250 && iWidth <= maxX + 20)
		{
			frmMain.iWidth = iWidth;
		}
		int iHeight = frmMainJson.get("AppHeight", frmMain.iHeight).asInt();
		if (iHeight >= _frmMain::MIN_HEIGHT && iHeight <= maxY + 20)
		{
			frmMain.iHeight = iHeight;
		}
		int iPosX = frmMainJson.get("AppPositionX", frmMain.iPosX).asInt();
		if (iPosX >= 0 && iPosX + frmMain.iWidth <= maxX)
		{
			frmMain.iPosX = iPosX;
		}
		int iPosY = frmMainJson.get("AppPositionY", frmMain.iPosY).asInt();
		if (iPosY >= 0 && frmMain.iPosY + frmMain.iHeight <= maxY)
		{
			frmMain.iPosY = iPosY;
		}
		frmMain.bWindowMaximized = frmMainJson.get("Maximized", frmMain.bWindowMaximized).asBool();
		frmMain.bAlwaysOnTop = frmMainJson.get("AlwaysOnTop", frmMain.bAlwaysOnTop).asBool();
		frmMain.bSpeedDialVisible = frmMainJson.get("SpeedDialVisible", frmMain.bSpeedDialVisible).asBool();
		frmMain.bSpeedDialOnly = frmMainJson.get("SpeedDialOnly", frmMain.bSpeedDialOnly).asBool();
		frmMain.bSpeedDialPopupMenu = frmMainJson.get("SpeedDialPopupMenu", frmMain.bSpeedDialPopupMenu).asBool();
		frmMain.bSpeedDialIgnorePresenceNote = frmMainJson.get("SpeedDialIgnorePresenceNote", frmMain.bSpeedDialIgnorePresenceNote).asBool();
		frmMain.bSpeedDialIgnoreDialogInfoRemoteIdentity = frmMainJson.get("SpeedDialIgnoreDialogInfoRemoteIdentity", frmMain.bSpeedDialIgnoreDialogInfoRemoteIdentity).asBool();
		frmMain.bSpeedDialKeepPreviousDialogInfoRemoteIdentityIfMissing = frmMainJson.get("SpeedDialKeepPreviousDialogInfoRemoteIdentityIfMissing", frmMain.bSpeedDialKeepPreviousDialogInfoRemoteIdentityIfMissing).asBool();
		frmMain.bSpeedDialIgnoreOrClearDialogInfoRemoteIdentityIfTerminated = frmMainJson.get("SpeedDialIgnoreOrClearDialogInfoRemoteIdentityIfTerminated", frmMain.bSpeedDialIgnoreOrClearDialogInfoRemoteIdentityIfTerminated).asBool();
		int iSpeedDialSize = frmMainJson.get("SpeedDialSize", frmMain.iSpeedDialSize).asUInt();
		if (iSpeedDialSize >= 0 && iSpeedDialSize < ProgrammableButtons::EXT_CONSOLE_COLUMNS)
		{
			frmMain.iSpeedDialSize = iSpeedDialSize;
		}

		{
			// speed dial column width(s) - changed from single int to array in 0.1.66.2
			const Json::Value &jvs = frmMainJson["SpeedDialWidth"];
			if (jvs.type() == Json::intValue || jvs.type() == Json::uintValue)
			{
				int iSpeedDialWidth = jvs.asUInt();
				if (iSpeedDialWidth >= _frmMain::MIN_SPEED_DIAL_COL_WIDTH && iSpeedDialWidth <= _frmMain::MAX_SPEED_DIAL_COL_WIDTH)
				{
					for (unsigned int i=0; i<frmMain.speedDialWidth.size(); i++)
					{
						frmMain.speedDialWidth[i] = iSpeedDialWidth;
					}
				}
			}
			else if (jvs.type() == Json::arrayValue)
			{
				for (unsigned int i=0; i<jvs.size(); i++)
				{
					if (i >= frmMain.speedDialWidth.size())
					{
						break;
					}
					int iSpeedDialWidth = jvs[i].asInt();
					if (iSpeedDialWidth >= _frmMain::MIN_SPEED_DIAL_COL_WIDTH && iSpeedDialWidth <= _frmMain::MAX_SPEED_DIAL_COL_WIDTH)
					{
                    	frmMain.speedDialWidth[i] = iSpeedDialWidth;
					}
				}
			}
		}

		frmMain.bStartMinimizedToTray = frmMainJson.get("StartMinimizedToTray", frmMain.bStartMinimizedToTray).asBool();
		frmMain.bXBtnMinimize = frmMainJson.get("XBtnMinimize", frmMain.bXBtnMinimize).asBool();
		frmMain.bRestoreOnIncomingCall = frmMainJson.get("RestoreOnIncomingCall", frmMain.bRestoreOnIncomingCall).asBool();
		frmMain.bSingleInstance = frmMainJson.get("SingleInstance", frmMain.bSingleInstance).asBool();
		frmMain.dialpadBackgroundImage = frmMainJson.get("DialpadBackgroundImage", frmMain.dialpadBackgroundImage.c_str()).asString().c_str();
		frmMain.mainIcon = frmMainJson.get("MainIcon", frmMain.mainIcon.c_str()).asString().c_str();
		frmMain.trayNotificationImage = frmMainJson.get("TrayNotificationImage", frmMain.trayNotificationImage.c_str()).asString().c_str();

		frmMain.bNoBeepOnEnterKey = frmMainJson.get("NoBeepOnEnterKey", frmMain.bNoBeepOnEnterKey).asBool();
		frmMain.bHideSettings = frmMainJson.get("HideSettings", frmMain.bHideSettings).asBool();
		frmMain.bHideView = frmMainJson.get("HideView", frmMain.bHideView).asBool();
		frmMain.bHideTools = frmMainJson.get("HideTools", frmMain.bHideTools).asBool();
		frmMain.bHideHelp = frmMainJson.get("HideHelp", frmMain.bHideHelp).asBool();
		frmMain.bKioskMode = frmMainJson.get("KioskMode", frmMain.bKioskMode).asBool();
		frmMain.bHideStatusBar = frmMainJson.get("HideStatusBar", frmMain.bHideStatusBar).asBool();
		frmMain.bHideMainMenu = frmMainJson.get("HideMainMenu", frmMain.bHideMainMenu).asBool();
		frmMainJson.getBool("HideDialpad", frmMain.bHideDialpad);

		frmMain.bHideSpeedDialToggleButton = frmMainJson.get("HideSpeedDialToggleButton", frmMain.bHideSpeedDialToggleButton).asBool();
		frmMain.bHideMouseCursor = frmMainJson.get("HideMouseCursor", frmMain.bHideMouseCursor).asBool();
		frmMain.bShowWhenAnsweringCall = frmMainJson.get("ShowWhenAnsweringCall", frmMain.bShowWhenAnsweringCall).asBool();
		frmMain.bShowWhenMakingCall = frmMainJson.get("ShowWhenMakingCall", frmMain.bShowWhenMakingCall).asBool();
		frmMain.bUseCustomApplicationTitle = frmMainJson.get("UseCustomApplicationTitle", frmMain.bUseCustomApplicationTitle).asBool();
		frmMain.customApplicationTitle = frmMainJson.get("CustomApplicationTitle", frmMain.customApplicationTitle.c_str()).asString().c_str();
		frmMain.bUseCustomCaption = frmMainJson.get("UseCustomCaption", frmMain.bUseCustomCaption).asBool();
		frmMain.customCaption = frmMainJson.get("CustomCaption", frmMain.customCaption.c_str()).asString().c_str();
		frmMain.bShowSettingsIfAccountSettingIsHidden = frmMainJson.get("ShowSettingsIfAccountSettingIsHidden", frmMain.bShowSettingsIfAccountSettingIsHidden).asBool();
		frmMainJson.getBool("NoTaskbarButtonRestore", frmMain.bNoTaskbarButtonRestore);
		frmMainJson.getBool("NoTrayIcon", frmMain.bNoTrayIcon);
		frmMainJson.getInt("Layout", frmMain.layout);
		{
			int tmp = frmMainJson.get("DialComboboxOrder", frmMain.dialComboboxOrder).asInt();
			if (tmp >= 0 && tmp < _frmMain::DialComboboxOrder_Limiter)
			{
            	frmMain.dialComboboxOrder = static_cast<_frmMain::DialComboboxOrder>(tmp);
			}
		}
		{
			const Json::Value &bitmapsJson = frmMainJson["bitmaps"];
			struct _frmMain::_bitmaps &bitmaps = frmMain.bitmaps;
			bitmapsJson.getAString("bmpBtnResetMicVolume", bitmaps.bmpBtnResetMicVolume);
			bitmapsJson.getAString("bmpBtnResetSpeakerVolume", bitmaps.bmpBtnResetSpeakerVolume);
			bitmapsJson.getAString("bmpBtnBackspace", bitmaps.bmpBtnBackspace);
			bitmapsJson.getAString("bmpConsoleHide", bitmaps.bmpConsoleHide);
			bitmapsJson.getAString("bmpConsoleShow", bitmaps.bmpConsoleShow);
		}
	}

	{
		const Json::Value &frmTrayNotifierJson = root["frmTrayNotifier"];
		int iPosX = frmTrayNotifierJson.get("PosX", frmTrayNotifier.iPosX).asInt();
		if (iPosX >= 0 && iPosX + frmTrayNotifier.iWidth <= maxX)
		{
			frmTrayNotifier.iPosX = iPosX;
		}

		int iPosY = frmTrayNotifierJson.get("PosY", frmTrayNotifier.iPosY).asInt();
		if (iPosY >= 0 && iPosY + frmTrayNotifier.iHeight <= maxY)
		{
			frmTrayNotifier.iPosY = iPosY;
		}
		frmTrayNotifier.showOnIncoming = frmTrayNotifierJson.get("ShowOnIncoming", frmTrayNotifier.showOnIncoming).asBool();
		frmTrayNotifier.skipIfMainWindowVisible = frmTrayNotifierJson.get("SkipIfMainWindowVisible", frmTrayNotifier.skipIfMainWindowVisible).asBool();
		frmTrayNotifier.showOnOutgoing = frmTrayNotifierJson.get("ShowOnOutgoing", frmTrayNotifier.showOnOutgoing).asBool();
		frmTrayNotifier.hideWhenAnsweringCall = frmTrayNotifierJson.get("HideWhenAnsweringCall", frmTrayNotifier.hideWhenAnsweringCall).asBool();
		frmTrayNotifier.hideWhenAnsweringCallAutomatically = frmTrayNotifierJson.get("HideWhenAnsweringCallAutomatically", frmTrayNotifier.hideWhenAnsweringCallAutomatically).asBool();
		frmTrayNotifier.backgroundImage = frmTrayNotifierJson.get("BackgroundImage", frmTrayNotifier.backgroundImage.c_str()).asString().c_str();
		int scalingPct = frmTrayNotifierJson.get("ScalingPct", frmTrayNotifier.scalingPct).asInt();
		if (scalingPct >= _frmTrayNotifier::SCALING_MIN && frmTrayNotifier.scalingPct <= _frmTrayNotifier::SCALING_MAX) {
			frmTrayNotifier.scalingPct = scalingPct;
		}
		frmTrayNotifierJson.getBool("DoNotChangePosition", frmTrayNotifier.doNotChangePosition);
	}

	{
		const Json::Value &frmContactPopupJson = root["frmContactPopup"];
		frmContactPopup.showOnIncoming = frmContactPopupJson.get("ShowOnIncoming", frmContactPopup.showOnIncoming).asBool();
		frmContactPopup.showOnOutgoing = frmContactPopupJson.get("ShowOnOutgoing", frmContactPopup.showOnOutgoing).asBool();
		frmContactPopup.iPosX = frmContactPopupJson.get("PosX", frmContactPopup.iPosX).asInt();
		frmContactPopup.iPosY = frmContactPopupJson.get("PosY", frmContactPopup.iPosY).asInt();
		frmContactPopup.iWidth = frmContactPopupJson.get("Width", frmContactPopup.iWidth).asInt();
		frmContactPopup.iHeight = frmContactPopupJson.get("Height", frmContactPopup.iHeight).asInt();
	}

	{
		const Json::Value &LoggingJson = root["Logging"];
		Logging.bLogToFile = LoggingJson.get("LogToFile", Logging.bLogToFile).asBool();
		Logging.bFlush = LoggingJson.get("Flush", Logging.bFlush).asBool();
		int iMaxFileSize = LoggingJson.get("MaxFileSize", Logging.iMaxFileSize).asInt();
		if (iMaxFileSize >= Settings::_Logging::MIN_MAX_FILE_SIZE && Logging.iMaxFileSize <= Settings::_Logging::MAX_MAX_FILE_SIZE)
		{
			Logging.iMaxFileSize = iMaxFileSize;
		}
		unsigned int iLogRotate = LoggingJson.get("LogRotate", Logging.iLogRotate).asUInt();
		if (iLogRotate <= Settings::_Logging::MAX_LOGROTATE)
		{
			Logging.iLogRotate = iLogRotate;
		}
		Logging.iMaxUiLogLines = LoggingJson.get("MaxUiLogLines", Logging.iMaxUiLogLines).asInt();
		{
			const Json::Value &jv = LoggingJson["ConsoleFont"];
			struct Font &font = Logging.consoleFont;
			font.name = jv.get("name", font.name.c_str()).asAString();
			font.size = jv.get("size", font.size).asInt();
			font.style = TFontStyles();
			bool bold = jv.get("bold", false).asBool();
			if (bold)
			{
				font.style << fsBold;
			}
			bool italic = jv.get("italic", false).asBool();
			if (italic)
			{
				font.style << fsItalic;
			}
			bool underline = jv.get("underline", false).asBool();
			if (underline)
			{
				font.style << fsUnderline;
			}
		}
	}

	{
		const Json::Value &CallsJson = root["Calls"];
		Calls.extraHeaderLines = CallsJson.get("ExtraHeaderLines", Calls.extraHeaderLines.c_str()).asString().c_str();
		Calls.bDisconnectCallOnAudioError = CallsJson.get("DisconnectCallOnAudioError", Calls.bDisconnectCallOnAudioError).asBool();
    }

	{
		const Json::Value &DisplayJson = root["Display"];
		Display.bUserOnlyClip = DisplayJson.get("UserOnlyClip", Display.bUserOnlyClip).asBool();
		Display.bDecodeUtfDisplayToAnsi = DisplayJson.get("DecodeUtfDisplayToAnsi", Display.bDecodeUtfDisplayToAnsi).asBool();
		Display.bUsePAssertedIdentity = DisplayJson.get("UsePAssertedIdentity", Display.bUsePAssertedIdentity).asBool();
    }

	{
		const Json::Value &IntegrationJson = root["Integration"];
		Integration.bAddFilterWMCopyData = IntegrationJson.get("AddFilterWMCopyData", Integration.bAddFilterWMCopyData).asBool();
		Integration.asProtocol = IntegrationJson.get("Protocol", Integration.asProtocol.c_str()).asString().c_str();
		Integration.bDoNotUseSipPrefixForDirectIpCalls = IntegrationJson.get("DoNotUseSipPrefixForDirectIpCalls", Integration.bDoNotUseSipPrefixForDirectIpCalls).asBool();
		IntegrationJson.getBool("DoNotPassParametersToPreviousInstance", Integration.bDoNotPassParametersToPreviousInstance);
		IntegrationJson.getBool("AcceptCommandLineScript", Integration.bAcceptCommandLineScript);
    }

	{
		const Json::Value &RingJson = root["Ring"];
		Ring.defaultRing = RingJson.get("DefaultRing", Ring.defaultRing.c_str()).asString().c_str();
		const Json::Value &bellcore = RingJson["Bellcore"];
		for (int i=0; i<std::min(sizeof(Ring.bellcore)/sizeof(Ring.bellcore[0]), bellcore.size()); i++)
		{
			if (!bellcore[i].asString().empty())
			{
				Ring.bellcore[i] = bellcore[i].asString().c_str();
			}
		}
	}

	{
		const Json::Value &HttpQueryJson = root["HttpQuery"];
		HttpQuery.url = HttpQueryJson.get("Url", HttpQuery.url.c_str()).asString().c_str();
		_HttpQuery::OpenMode openMode = static_cast<_HttpQuery::OpenMode>(HttpQueryJson.get("OpenMode", HttpQuery.openMode).asInt());
		if (openMode >= 0 && openMode < _HttpQuery::openModeLimiter)
		{
			HttpQuery.openMode = openMode;
		}
	}

	{
		const Json::Value &SipAccessUrlJson = root["SipAccessUrl"];
		_SipAccessUrl::AccessMode accessMode = static_cast<_SipAccessUrl::AccessMode>(SipAccessUrlJson.get("AccessMode", SipAccessUrl.accessMode).asInt());
		if (accessMode >= 0 && accessMode < _SipAccessUrl::accessModeLimiter)
		{
			SipAccessUrl.accessMode = accessMode;
		}
	}

	{
		const Json::Value &jv = root["Contacts"];
		jv.getAString("File", Contacts.file);
		jv.getBool("CheckIfFileUpdated", Contacts.checkIfFileUpdated);
		jv.getUInt("CheckIfFileUpdatedPeriod", Contacts.checkIfFileUpdatedPeriod);
		Contacts.filterUsingNote = jv.get("FilterUsingNote", Contacts.filterUsingNote).asBool();
		jv.getBool("OpenFileOnIncoming", Contacts.openFileOnIncoming);
		jv.getBool("OpenFileOnOutgoing", Contacts.openFileOnOutgoing);
		jv.getBool("StoreNoteInSeparateFile", Contacts.storeNoteInSeparateFile);
	}

	{
		const Json::Value &HistoryJson = root["History"];
		History.bNoStoreToFile = HistoryJson.get("NoStoreToFile", History.bNoStoreToFile).asBool();
		History.bUsePaiForDisplayIfAvailable = HistoryJson.get("UsePaiForDisplayIfAvailable", History.bUsePaiForDisplayIfAvailable).asBool();
		History.bUsePaiForDialIfAvailable = HistoryJson.get("UsePaiForDialIfAvailable", History.bUsePaiForDialIfAvailable).asBool();
		History.bShowHint = HistoryJson.get("ShowHint", History.bShowHint).asBool();
		History.bFormatCallDurationAsHourMinSec = HistoryJson.get("FormatCallDurationAsHourMinSec", History.bFormatCallDurationAsHourMinSec).asBool();
		History.bShowCodecNameInHint = HistoryJson.get("ShowCodecNameInHint", History.bShowCodecNameInHint).asBool();
		{
			const Json::Value &jlcw = HistoryJson["ListColumnWidths"];
			if (jlcw.type() == Json::arrayValue)
			{
				for (unsigned int i=0; i<jlcw.size(); i++)
				{
					History.listColumnWidths.push_back(jlcw[i].asInt());
				}
			}
		}
	}

	{
		const Json::Value &MessagesJson = root["Messages"];
		MessagesJson.getAString("Ring", Messages.ring);
	}

	{
		const Json::Value &ScriptsJson = root["Scripts"];
		Scripts.onMakeCall = ScriptsJson.get("OnMakeCall", Scripts.onMakeCall.c_str()).asString().c_str();
		Scripts.onCallState = ScriptsJson.get("OnCallState", Scripts.onCallState.c_str()).asString().c_str();
		Scripts.onStreamingState = ScriptsJson.get("OnStreamingState", Scripts.onStreamingState.c_str()).asString().c_str();
		Scripts.onRegistrationState = ScriptsJson.get("OnRegistrationState", Scripts.onRegistrationState.c_str()).asString().c_str();
		ScriptsJson.getAString("OnRecorderState", Scripts.onRecorderState);
		Scripts.onStartup = ScriptsJson.get("OnStartup", Scripts.onStartup.c_str()).asString().c_str();
		Scripts.onTimer = ScriptsJson.get("OnTimer", Scripts.onTimer.c_str()).asString().c_str();
		int timer = ScriptsJson.get("Timer", Scripts.timer).asInt();
		if (timer > 0)
			Scripts.timer = timer;
		ScriptsJson.getAString("OnTimer2", Scripts.onTimer2);
		int timer2 = ScriptsJson.get("Timer2", Scripts.timer2).asInt();
		if (timer2 > 0)
			Scripts.timer2 = timer2;
		Scripts.onDialogInfo = ScriptsJson.get("OnDialogInfo", Scripts.onDialogInfo.c_str()).asString().c_str();
		Scripts.onDial = ScriptsJson.get("OnDial", Scripts.onDial.c_str()).asString().c_str();
		Scripts.onProgrammableButton = ScriptsJson.get("OnProgrammableButton", Scripts.onProgrammableButton.c_str()).asString().c_str();
		Scripts.onAudioDeviceError = ScriptsJson.get("OnAudioDeviceError", Scripts.onAudioDeviceError.c_str()).asString().c_str();
		ScriptsJson.getAString("OnCustomRequestReply", Scripts.onCustomRequestReply);
		ScriptsJson.getAString("OnContactNoteOpen", Scripts.onContactNoteOpen);
	}

	{
		const Json::Value &jScriptWindow = root["ScriptWindow"];
		{
            ScriptWindow.lastDir = jScriptWindow.get("LastDir", ScriptWindow.lastDir.c_str()).asCString();
			const Json::Value &jMRU = jScriptWindow["MRU"];
			if (jMRU.type() == Json::arrayValue)
			{
				// it's highly unlikely that MRU would need merging old configuration with new one,
				// but old values are still used for consistency
				unsigned int count = std::min<unsigned>(jMRU.size(), _ScriptWindow::MRU_LIMIT);
				ScriptWindow.MRU.resize(count);
				for (unsigned int i=0; i<count; i++)
				{
					if (jMRU[i].type() == Json::stringValue)
					{
						ScriptWindow.MRU[i] = jMRU.get(i, ScriptWindow.MRU[i].c_str()).asString().c_str();
					}
				}
			}
		}
	}

	{
		const Json::Value &jv = root["Translation"];
		jv.getAString("language", Translation.language);
		jv.getBool("logMissingKeys", Translation.logMissingKeys);
	}

	return 0;
}

int Settings::Write(AnsiString asFileName)
{
	Json::Value root;
	Json::StyledWriter writer;

	{
		Json::Value &jv = root["info"];

		// update application version in settings
		GetFileVer(Application->ExeName, info.appVersion.FileVersionMS, info.appVersion.FileVersionLS);

		jv["FileVersionMS"] = info.appVersion.FileVersionMS;
		jv["FileVersionLS"] = info.appVersion.FileVersionLS;
	}

	root["gui"]["scalingPct"] = gui.scalingPct;

	{
		Json::Value& jv = root["frmMain"];
		jv["AppWidth"] = frmMain.iWidth;
		jv["AppHeight"] = frmMain.iHeight;
		jv["AppPositionX"] = frmMain.iPosX;
		jv["AppPositionY"] = frmMain.iPosY;
		jv["Maximized"] = frmMain.bWindowMaximized;
		jv["AlwaysOnTop"] = frmMain.bAlwaysOnTop;
		jv["SpeedDialVisible"] = frmMain.bSpeedDialVisible;
		jv["SpeedDialOnly"] = frmMain.bSpeedDialOnly;
		jv["SpeedDialPopupMenu"] = frmMain.bSpeedDialPopupMenu;
		jv["SpeedDialIgnorePresenceNote"] = frmMain.bSpeedDialIgnorePresenceNote;
		jv["SpeedDialIgnoreDialogInfoRemoteIdentity"] = frmMain.bSpeedDialIgnoreDialogInfoRemoteIdentity;
		jv["SpeedDialKeepPreviousDialogInfoRemoteIdentityIfMissing"] = frmMain.bSpeedDialKeepPreviousDialogInfoRemoteIdentityIfMissing;
		jv["SpeedDialIgnoreOrClearDialogInfoRemoteIdentityIfTerminated"] = frmMain.bSpeedDialIgnoreOrClearDialogInfoRemoteIdentityIfTerminated;
		jv["SpeedDialSize"] = frmMain.iSpeedDialSize;
		Json::Value& jvs = jv["SpeedDialWidth"];
		jvs.resize(frmMain.speedDialWidth.size());
		for (unsigned int i=0; i<frmMain.speedDialWidth.size(); i++)
		{
			jvs[i] = frmMain.speedDialWidth[i];
		}
		jv["StartMinimizedToTray"] = frmMain.bStartMinimizedToTray;
		jv["XBtnMinimize"] = frmMain.bXBtnMinimize;
		jv["RestoreOnIncomingCall"] = frmMain.bRestoreOnIncomingCall;
		jv["SingleInstance"] = frmMain.bSingleInstance;
		jv["DialpadBackgroundImage"] = frmMain.dialpadBackgroundImage.c_str();
		jv["MainIcon"] = frmMain.mainIcon.c_str();
		jv["TrayNotificationImage"] = frmMain.trayNotificationImage.c_str();
		jv["NoBeepOnEnterKey"] = frmMain.bNoBeepOnEnterKey;
		jv["HideSettings"] = frmMain.bHideSettings;
		jv["HideView"] = frmMain.bHideView;
		jv["HideTools"] = frmMain.bHideTools;
		jv["HideHelp"] = frmMain.bHideHelp;
		jv["KioskMode"] = frmMain.bKioskMode;
		jv["HideStatusBar"] = frmMain.bHideStatusBar;
		jv["HideMainMenu"] = frmMain.bHideMainMenu;
		jv["HideDialpad"] = frmMain.bHideDialpad;

		jv["HideSpeedDialToggleButton"] = frmMain.bHideSpeedDialToggleButton;
		jv["HideMouseCursor"] = frmMain.bHideMouseCursor;
		jv["ShowWhenAnsweringCall"] = frmMain.bShowWhenAnsweringCall;
		jv["ShowWhenMakingCall"] = frmMain.bShowWhenMakingCall;
		jv["UseCustomApplicationTitle"] = frmMain.bUseCustomApplicationTitle;
		jv["CustomApplicationTitle"] = frmMain.customApplicationTitle.c_str();
		jv["UseCustomCaption"] = frmMain.bUseCustomCaption;
		jv["CustomCaption"] = frmMain.customCaption.c_str();
		jv["ShowSettingsIfAccountSettingIsHidden"] = frmMain.bShowSettingsIfAccountSettingIsHidden;
		jv["NoTaskbarButtonRestore"] = frmMain.bNoTaskbarButtonRestore;
		jv["NoTrayIcon"] = frmMain.bNoTrayIcon;
		jv["Layout"] = frmMain.layout;
		jv["DialComboboxOrder"] = frmMain.dialComboboxOrder;

		{
			Json::Value &bitmapsJson = jv["bitmaps"];
			const struct _frmMain::_bitmaps &bitmaps = frmMain.bitmaps;
			bitmapsJson["bmpBtnResetMicVolume"] = bitmaps.bmpBtnResetMicVolume;
			bitmapsJson["bmpBtnResetSpeakerVolume"] = bitmaps.bmpBtnResetSpeakerVolume;
			bitmapsJson["bmpBtnBackspace"] = bitmaps.bmpBtnBackspace;
			bitmapsJson["bmpConsoleHide"] = bitmaps.bmpConsoleHide;
			bitmapsJson["bmpConsoleShow"] = bitmaps.bmpConsoleShow;
		}
	}

    {
		Json::Value& jv = root["frmTrayNotifier"];
		jv["PosX"] = frmTrayNotifier.iPosX;
		jv["PosY"] = frmTrayNotifier.iPosY;
		jv["ShowOnIncoming"] = frmTrayNotifier.showOnIncoming;
		jv["SkipIfMainWindowVisible"] = frmTrayNotifier.skipIfMainWindowVisible;
		jv["ShowOnOutgoing"] = frmTrayNotifier.showOnOutgoing;
		jv["HideWhenAnsweringCall"] = frmTrayNotifier.hideWhenAnsweringCall;
		jv["HideWhenAnsweringCallAutomatically"] = frmTrayNotifier.hideWhenAnsweringCallAutomatically;
		jv["BackgroundImage"] = frmTrayNotifier.backgroundImage.c_str();
		jv["ScalingPct"] = frmTrayNotifier.scalingPct;
		jv["DoNotChangePosition"] = frmTrayNotifier.doNotChangePosition;
	}

	root["frmContactPopup"]["ShowOnIncoming"] = frmContactPopup.showOnIncoming;
	root["frmContactPopup"]["ShowOnOutgoing"] = frmContactPopup.showOnOutgoing;
	root["frmContactPopup"]["PosX"] = frmContactPopup.iPosX;
	root["frmContactPopup"]["PosY"] = frmContactPopup.iPosY;
	root["frmContactPopup"]["Width"] = frmContactPopup.iWidth;
	root["frmContactPopup"]["Height"] = frmContactPopup.iHeight;

	{
		Json::Value &jLogging = root["Logging"];
		jLogging["LogToFile"] = Logging.bLogToFile;
		jLogging["Flush"] = Logging.bFlush;
		jLogging["MaxFileSize"] = Logging.iMaxFileSize;
		jLogging["LogRotate"] = Logging.iLogRotate;
		jLogging["MaxUiLogLines"] = Logging.iMaxUiLogLines;
		{
			Json::Value &jFont = jLogging["ConsoleFont"];
			const struct Font &font = Logging.consoleFont;
			jFont["name"] = font.name.c_str();
			jFont["size"] = font.size;
			jFont["bold"] = font.style.Contains(fsBold);
			jFont["italic"] = font.style.Contains(fsItalic);
			jFont["underline"] = font.style.Contains(fsUnderline);
		}
	}

	root["Calls"]["ExtraHeaderLines"] = Calls.extraHeaderLines.c_str();
	root["Calls"]["DisconnectCallOnAudioError"] = Calls.bDisconnectCallOnAudioError;

	root["Display"]["UserOnlyClip"] = Display.bUserOnlyClip;
	root["Display"]["DecodeUtfDisplayToAnsi"] = Display.bDecodeUtfDisplayToAnsi;
	root["Display"]["UsePAssertedIdentity"] = Display.bUsePAssertedIdentity;

	{
		Json::Value &jIntegration = root["Integration"];
		jIntegration["AddFilterWMCopyData"] = Integration.bAddFilterWMCopyData;
		jIntegration["Protocol"] = Integration.asProtocol.c_str();
		jIntegration["DoNotUseSipPrefixForDirectIpCalls"] = Integration.bDoNotUseSipPrefixForDirectIpCalls;
		jIntegration["DoNotPassParametersToPreviousInstance"] = Integration.bDoNotPassParametersToPreviousInstance;
		jIntegration["AcceptCommandLineScript"] = Integration.bAcceptCommandLineScript;
	}

	root["Ring"]["DefaultRing"] = Ring.defaultRing.c_str();
	for (int i=0; i<sizeof(Ring.bellcore)/sizeof(Ring.bellcore[0]); i++)
	{
		root["Ring"]["Bellcore"][i] = Ring.bellcore[i].c_str();
	}

	root["HttpQuery"]["Url"] = HttpQuery.url.c_str();
	root["HttpQuery"]["OpenMode"] = HttpQuery.openMode;

	root["SipAccessUrl"]["AccessMode"] = SipAccessUrl.accessMode;

	{
		Json::Value &jv = root["Contacts"];
		jv["File"] = Contacts.file;
		jv["CheckIfFileUpdated"] = Contacts.checkIfFileUpdated;
		jv["CheckIfFileUpdatedPeriod"] = Contacts.checkIfFileUpdatedPeriod;
		jv["FilterUsingNote"] = Contacts.filterUsingNote;
		jv["OpenFileOnIncoming"] = Contacts.openFileOnIncoming;
		jv["OpenFileOnOutgoing"] = Contacts.openFileOnOutgoing;
		jv["StoreNoteInSeparateFile"] = Contacts.storeNoteInSeparateFile;
	}

	{
		Json::Value &jv = root["History"];
		jv["NoStoreToFile"] = History.bNoStoreToFile;
		jv["UsePaiForDisplayIfAvailable"] = History.bUsePaiForDisplayIfAvailable;
		jv["UsePaiForDialIfAvailable"] = History.bUsePaiForDialIfAvailable;
		jv["ShowHint"] = History.bShowHint;
		jv["FormatCallDurationAsHourMinSec"] = History.bFormatCallDurationAsHourMinSec;
		jv["ShowCodecNameInHint"] = History.bShowCodecNameInHint;

		Json::Value &jlcw = jv["ListColumnWidths"];
		jlcw.resize(0);
		for (unsigned int i=0; i<History.listColumnWidths.size(); i++)
		{
			jlcw.append(History.listColumnWidths[i]);
		}
	}

	{
		Json::Value &MessagesJson = root["Messages"];
		MessagesJson["Ring"] = Messages.ring;
	}

	{
		Json::Value &jv = root["Scripts"];
		jv["OnMakeCall"] = Scripts.onMakeCall.c_str();
		jv["OnCallState"] = Scripts.onCallState.c_str();
		jv["OnStreamingState"] = Scripts.onStreamingState.c_str();
		jv["OnRegistrationState"] = Scripts.onRegistrationState.c_str();
		jv["OnRecorderState"] = Scripts.onRecorderState;
		jv["OnStartup"] = Scripts.onStartup.c_str();
		jv["OnTimer"] = Scripts.onTimer.c_str();
		jv["Timer"] = Scripts.timer;
		jv["OnTimer2"] = Scripts.onTimer2;
		jv["Timer2"] = Scripts.timer2;
		jv["OnDialogInfo"] = Scripts.onDialogInfo.c_str();
		jv["OnDial"] = Scripts.onDial.c_str();
		jv["OnProgrammableButton"] = Scripts.onProgrammableButton.c_str();
		jv["OnAudioDeviceError"] = Scripts.onAudioDeviceError.c_str();
		jv["OnCustomRequestReply"] = Scripts.onCustomRequestReply;
		jv["OnContactNoteOpen"] = Scripts.onContactNoteOpen;
	}

	root["uaConf"]["audioCfgSrc"]["mod"] = uaConf.audioCfgSrc.mod;
	root["uaConf"]["audioCfgSrc"]["dev"] = uaConf.audioCfgSrc.dev;
	root["uaConf"]["audioCfgSrc"]["wavefile"] = uaConf.audioCfgSrc.wavefile;
	root["uaConf"]["audioCfgPlay"]["mod"] = uaConf.audioCfgPlay.mod;
	root["uaConf"]["audioCfgPlay"]["dev"] = uaConf.audioCfgPlay.dev;
	root["uaConf"]["audioCfgAlert"]["mod"] = uaConf.audioCfgAlert.mod;
	root["uaConf"]["audioCfgAlert"]["dev"] = uaConf.audioCfgAlert.dev;
	root["uaConf"]["audioCfgRing"]["mod"] = uaConf.audioCfgRing.mod;
	root["uaConf"]["audioCfgRing"]["dev"] = uaConf.audioCfgRing.dev;
	root["uaConf"]["audioCfgPlayIntercom"]["mod"] = uaConf.audioCfgPlayIntercom.mod;
	root["uaConf"]["audioCfgPlayIntercom"]["dev"] = uaConf.audioCfgPlayIntercom.dev;

	root["uaConf"]["audioPortaudio"]["inSuggestedLatency"] = uaConf.audioPortaudio.inSuggestedLatency;
	root["uaConf"]["audioPortaudio"]["outSuggestedLatency"] = uaConf.audioPortaudio.outSuggestedLatency;

	root["uaConf"]["audioSoftVol"]["tx"] = uaConf.audioSoftVol.tx;
	root["uaConf"]["audioSoftVol"]["rx"] = uaConf.audioSoftVol.rx;

	{
		Json::Value &jv = root["uaConf"]["audioAgcRx"];
		jv["enabled"] = uaConf.audioAgcRx.enabled;
		jv["target"] = uaConf.audioAgcRx.target;
		jv["maxGain"] = uaConf.audioAgcRx.maxGain;
		jv["attackRate"] = uaConf.audioAgcRx.attackRate;
		jv["releaseRate"] = uaConf.audioAgcRx.releaseRate;
	}

	if (Branding::recording)
	{
		Json::Value &jv = root["uaConf"]["recording"];
		jv["enabled"] = uaConf.recording.enabled;
		jv["recDir"] = uaConf.recording.recDir;
		jv["customRecDir"] = uaConf.recording.customRecDir;
		jv["channels"] = uaConf.recording.channels;
		jv["side"] = uaConf.recording.side;
		jv["recStart"] = uaConf.recording.recStart;
		jv["noNumberB64Encoding"] = uaConf.recording.noNumberB64Encoding;
	}

	{
		Json::Value &cfgJson = root["uaConf"]["audioPreprocTx"];
		const UaConf::AudioPreproc &tx = uaConf.audioPreprocTx;
		cfgJson["enabled"] = tx.enabled;
		cfgJson["denoiseEnabled"] = tx.denoiseEnabled;
		cfgJson["agcEnabled"] = tx.agcEnabled;
		cfgJson["vadEnabled"] = tx.vadEnabled;
		cfgJson["dereverbEnabled"] = tx.dereverbEnabled;
		cfgJson["agcLevel"] = tx.agcLevel;
	}

	root["uaConf"]["aec"] = uaConf.aec;
	root["uaConf"]["logMessages"] = uaConf.logMessages;
	root["uaConf"]["logMessagesOnlyFirstLine"] = uaConf.logMessagesOnlyFirstLine;
	root["uaConf"]["logAubuf"] = uaConf.logAubuf;	
	
	root["uaConf"]["localAddress"] = uaConf.local;
	root["uaConf"]["ifName"] = uaConf.ifname;
	root["uaConf"]["avt"]["portMin"] = uaConf.avt.portMin;
	root["uaConf"]["avt"]["portMax"] = uaConf.avt.portMax;
	root["uaConf"]["avt"]["jbufDelayMin"] = uaConf.avt.jbufDelayMin;
	root["uaConf"]["avt"]["jbufDelayMax"] = uaConf.avt.jbufDelayMax;
	root["uaConf"]["avt"]["rtpTimeout"] = uaConf.avt.rtpTimeout;

	{
		Json::Value &jv = root["uaConf"]["messages"];
		jv["replyCode"] = uaConf.messages.replyCode;
		jv["replyReason"] = uaConf.messages.replyReason;
		jv["doNotReply"] = uaConf.messages.doNotReply;
	}

	root["uaConf"]["autoAnswer"] = uaConf.autoAnswer;
	root["uaConf"]["autoAnswerCode"] = uaConf.autoAnswerCode;
	root["uaConf"]["autoAnswerDelayMin"] = uaConf.autoAnswerDelayMin;
	root["uaConf"]["autoAnswerDelayMax"] = uaConf.autoAnswerDelayMax;
	root["uaConf"]["autoAnswerCallInfo"] = uaConf.autoAnswerCallInfo;
	root["uaConf"]["autoAnswerCallInfoDelayMin"] = uaConf.autoAnswerCallInfoDelayMin;

	root["uaConf"]["answerOnEventTalk"] = uaConf.answerOnEventTalk;

	root["uaConf"]["handleOodRefer"] = uaConf.handleOodRefer;

	root["uaConf"]["customUserAgent"] = uaConf.customUserAgent;
	root["uaConf"]["userAgent"] = uaConf.userAgent;

	root["uaConf"]["webrtcAec"]["msInSndCardBuf"] = uaConf.webrtcAec.msInSndCardBuf;
	root["uaConf"]["webrtcAec"]["skew"] = uaConf.webrtcAec.skew;

	{
		Json::Value &jv = root["uaConf"]["opus"];
		const UaConf::Opus &opus = uaConf.opus;
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

	// write accounts
	for (unsigned int i=0; i<uaConf.accounts.size(); i++)
	{
		UaConf::Account &acc = uaConf.accounts[i];
		Json::Value &cfgAcc = root["Accounts"][i];
		if (!acc.hide_reg_server)
			cfgAcc["reg_server"] = acc.reg_server;
		if (!acc.hide_user)
			cfgAcc["user"] = acc.user;
		if (!acc.hide_display_name)
			cfgAcc["display_name"] = acc.display_name;
		if (!acc.hide_auth_user)
			cfgAcc["auth_user"] = acc.auth_user;
		if (!acc.hide_pwd)
			cfgAcc["pwd"] = acc.pwd;
		if (!acc.hide_cuser)
			cfgAcc["cuser"] = acc.cuser;
		cfgAcc["transport"] = acc.transport;
		cfgAcc["reg_expires"] = acc.reg_expires;
		cfgAcc["dtmf_tx_format"] = acc.dtmf_tx_format;
		cfgAcc["answer_any"] = acc.answer_any;
		cfgAcc["ptime"] = acc.ptime;

		cfgAcc["hide_reg_server"] = acc.hide_reg_server;
		cfgAcc["hide_user"] = acc.hide_user;
		cfgAcc["hide_display_name"] = acc.hide_display_name;
		cfgAcc["hide_auth_user"] = acc.hide_auth_user;
		cfgAcc["hide_pwd"] = acc.hide_pwd;
		cfgAcc["hide_cuser"] = acc.hide_cuser;

		cfgAcc["stun_server"] = acc.stun_server;
		cfgAcc["outbound1"] = acc.outbound1;
		cfgAcc["outbound2"] = acc.outbound2;
		for (unsigned int j=0; j<acc.audio_codecs.size(); j++)
		{
			cfgAcc["audio_codecs"][j] = acc.audio_codecs[j];
		}
	}

	{
		int i = 0;
		std::list<HotKeyConf>::iterator iter;
		for (iter = hotKeyConf.begin(); iter != hotKeyConf.end(); ++iter)
		{
			struct HotKeyConf &cfg = *iter;
			Json::Value &cfgJson = root["hotkeyConf"][i++];
			cfgJson["keyCode"] = cfg.keyCode.c_str();
			cfgJson["modifiers"] = cfg.modifiers;
			cfgJson["global"] = cfg.global;
			cfgJson["action"]["type"] = cfg.action.type;
			cfgJson["action"]["id"] = cfg.action.id;
		}
	}

	{
		int i = 0;
		std::list<PhoneConf>::iterator iter;
		Json::Value &jPhoneConf = root["phoneConf"];
		jPhoneConf.resize(0);
		for (iter = phoneConf.begin(); iter != phoneConf.end(); ++iter)
		{
			PhoneConf &cfg = *iter;
			if (cfg.dllName != "")
			{
				Json::Value &cfgJson = jPhoneConf[i++];
				cfgJson["dllName"] = cfg.dllName.c_str();
			}
		}
	}

	{
		Json::Value &jScriptWindow = root["ScriptWindow"];
		{
			jScriptWindow["LastDir"] = ScriptWindow.lastDir.c_str();
			Json::Value &jMRU = jScriptWindow["MRU"];
			jMRU.resize(0);
			for (unsigned int i=0; i < ScriptWindow.MRU.size(); i++)
			{
				jMRU.append(Json::Value(ScriptWindow.MRU[i].c_str()));
			}
		}
	}

	{
		Json::Value &jv = root["Translation"];
		jv["language"] = Translation.language;
		jv["logMissingKeys"] = Translation.logMissingKeys;
	}


	std::string outputConfig = writer.write( root );

	try
	{
		std::ofstream ofs(asFileName.c_str());
		ofs << outputConfig;
		ofs.close();
	}
	catch(...)
	{
    	return 1;
	}

	return 0;
}

void Settings::_ScriptWindow::AddMru(AnsiString item)
{
	// check if item is already in MRU Lua list
	std::deque<AnsiString>::iterator iter;
	for (iter=MRU.begin(); iter != MRU.end(); )
	{
		if (*iter == item)
		{
			iter = MRU.erase(iter);
		}
		else
		{
			++iter;
		}
	}
	// push item on top
	MRU.push_back(item);
	while (MRU.size() > MRU_LIMIT)
	{
		MRU.pop_front();
    }
}

void Settings::_ScriptWindow::ClearMruItems(void)
{
	MRU.clear();
}
