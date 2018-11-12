//---------------------------------------------------------------------------


#pragma hdrstop

#include "Settings.h"
#include "KeybKeys.h"
#include "ProgrammableButtons.h"
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

void Settings::SetDefault(void)
{
    info.appVersion.SetDefault();

	frmMain.iWidth = 350;
	frmMain.iHeight = 300;
	frmMain.iPosX = 30;
	frmMain.iPosY = 30;
	frmMain.bWindowMaximized = false;
	frmMain.bAlwaysOnTop = false;
	frmMain.bSpeedDialOnly = false;
	frmMain.bSpeedDialPopupMenu = true;
	frmMain.bSpeedDialIgnorePresenceNote = false;
	frmMain.bSpeedDialIgnoreDialogInfoRemoteIdentity = false;
	frmMain.bXBtnMinimize = false;
	frmMain.bRestoreOnIncomingCall = false;
	frmMain.bSingleInstance = false;
	frmMain.bNoBeepOnEnterKey = false;
	frmMain.bHideSettings = false;
	frmMain.bHideView = false;
	frmMain.bHideHelp = false;
	frmMain.bKioskMode = false;
	frmMain.bHideMouseCursor = false;
	frmMain.bShowWhenAnsweringCall = false;

    frmTrayNotifier.hideWhenAnsweringCall = false;

	frmContactPopup.showOnIncoming = false;
	frmContactPopup.showOnOutgoing = false;
	frmContactPopup.iPosX = 100;
	frmContactPopup.iPosY = 100;
	frmContactPopup.iWidth = 400;
	frmContactPopup.iHeight = 140;

	gui.scalingPct = 100;

	frmTrayNotifier.scalingPct = _frmTrayNotifier::SCALING_DEF;

	Logging.bLogToFile = false;
	Logging.bFlush = false;
	Logging.iMaxFileSize = Settings::_Logging::DEF_MAX_FILE_SIZE;
	Logging.iLogRotate = Settings::_Logging::DEF_LOGROTATE;
	Logging.iMaxUiLogLines = 5000;

    Calls.extraHeaderLines = "";

	Display.bUserOnlyClip = false;
	Display.bDecodeUtfDisplayToAnsi = false;

	Integration.asProtocol = Branding::appProto;

	HttpQuery.url = "https://www.google.com/search?q=[number]";
	HttpQuery.openMode = _HttpQuery::openManualOnly;
	Contacts.filterUsingNote = false;

	SipAccessUrl.accessMode = _SipAccessUrl::accessModeAlwaysPassive;

	History.bNoStoreToFile = false;

	Ring.defaultRing = "ring.wav";
	for (int i=0; i<sizeof(Ring.bellcore)/sizeof(Ring.bellcore[0]); i++)
	{
		Ring.bellcore[i] = "ring.wav";
	}

	Scripts.timer = 1000;

	uaConf.accounts.clear();

	struct UaConf::Account new_acc;
	uaConf.accounts.push_back(new_acc);
}

int Settings::Read(AnsiString asFileName)
{
	Json::Value root;   // will contains the root value after parsing.
	Json::Reader reader;

    SetDefault();

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

	{
		const Json::Value &jv = root["info"];
		info.appVersion.FileVersionMS = jv.get("FileVersionMS", info.appVersion.FileVersionMS).asUInt();
		info.appVersion.FileVersionLS = jv.get("FileVersionLS", info.appVersion.FileVersionLS).asUInt();
	}

	const Json::Value &uaConfJson = root["uaConf"];
	const Json::Value &uaConfAudioCfgSrcJson = uaConfJson["audioCfgSrc"];
	strncpyz(uaConf.audioCfgSrc.mod, uaConfAudioCfgSrcJson.get("mod", UaConf::modWinwave).asString().c_str(), sizeof(uaConf.audioCfgSrc.mod));
	strncpyz(uaConf.audioCfgSrc.dev, uaConfAudioCfgSrcJson["dev"].asString().c_str(), sizeof(uaConf.audioCfgSrc.dev));
	strncpyz(uaConf.audioCfgSrc.wavefile, uaConfAudioCfgSrcJson["wavefile"].asString().c_str(), sizeof(uaConf.audioCfgSrc.wavefile));
	if (strcmp(uaConf.audioCfgSrc.mod, UaConf::modPortaudio) &&
		strcmp(uaConf.audioCfgSrc.mod, UaConf::modWinwave) &&
		strcmp(uaConf.audioCfgSrc.mod, UaConf::modAufile) &&
		strcmp(uaConf.audioCfgSrc.mod, UaConf::modNullaudio)
		) {
		strncpyz(uaConf.audioCfgSrc.mod, UaConf::modWinwave, sizeof(uaConf.audioCfgSrc.mod));
	}

	const Json::Value &uaConfAudioCfgPlayJson = uaConfJson["audioCfgPlay"];
	strncpyz(uaConf.audioCfgPlay.mod, uaConfAudioCfgPlayJson.get("mod", UaConf::modWinwave).asString().c_str(), sizeof(uaConf.audioCfgPlay.mod));
	strncpyz(uaConf.audioCfgPlay.dev, uaConfAudioCfgPlayJson["dev"].asString().c_str(), sizeof(uaConf.audioCfgPlay.dev));
	if (strcmp(uaConf.audioCfgPlay.mod, UaConf::modPortaudio) &&
		strcmp(uaConf.audioCfgPlay.mod, UaConf::modWinwave) &&
		strcmp(uaConf.audioCfgPlay.mod, UaConf::modNullaudio)
		) {
		strncpyz(uaConf.audioCfgPlay.mod, UaConf::modWinwave, sizeof(uaConf.audioCfgPlay.mod));
	}
	const Json::Value &uaConfAudioCfgAlertJson = uaConfJson["audioCfgAlert"];
	strncpyz(uaConf.audioCfgAlert.mod, uaConfAudioCfgAlertJson.get("mod", UaConf::modWinwave).asString().c_str(), sizeof(uaConf.audioCfgAlert.mod));
	strncpyz(uaConf.audioCfgAlert.dev, uaConfAudioCfgAlertJson["dev"].asString().c_str(), sizeof(uaConf.audioCfgAlert.dev));
	if (strcmp(uaConf.audioCfgAlert.mod, UaConf::modPortaudio) &&
		strcmp(uaConf.audioCfgAlert.mod, UaConf::modWinwave) &&
		strcmp(uaConf.audioCfgAlert.mod, UaConf::modNullaudio)
		) {
		strncpyz(uaConf.audioCfgAlert.mod, UaConf::modWinwave, sizeof(uaConf.audioCfgAlert.mod));
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
			strncpyz(uaConf.audioCfgRing.mod, uaConfAudioCfgRingJson.get("mod", UaConf::modWinwave).asString().c_str(), sizeof(uaConf.audioCfgRing.mod));
			strncpyz(uaConf.audioCfgRing.dev, uaConfAudioCfgRingJson["dev"].asString().c_str(), sizeof(uaConf.audioCfgRing.dev));
			if (strcmp(uaConf.audioCfgRing.mod, UaConf::modPortaudio) &&
				strcmp(uaConf.audioCfgRing.mod, UaConf::modWinwave) &&
				strcmp(uaConf.audioCfgRing.mod, UaConf::modNullaudio)
				) {
				strncpyz(uaConf.audioCfgRing.mod, UaConf::modWinwave, sizeof(uaConf.audioCfgRing.mod));
			}
		}
	}
	const Json::Value &uaConfAudioCfgPlayIntercomJson = uaConfJson["audioCfgPlayIntercom"];
	strncpyz(uaConf.audioCfgPlayIntercom.mod, uaConfAudioCfgPlayIntercomJson.get("mod", UaConf::modWinwave).asString().c_str(), sizeof(uaConf.audioCfgPlayIntercom.mod));
	strncpyz(uaConf.audioCfgPlayIntercom.dev, uaConfAudioCfgPlayIntercomJson["dev"].asString().c_str(), sizeof(uaConf.audioCfgPlayIntercom.dev));
	if (strcmp(uaConf.audioCfgPlayIntercom.mod, UaConf::modPortaudio) &&
		strcmp(uaConf.audioCfgPlayIntercom.mod, UaConf::modWinwave) &&
		strcmp(uaConf.audioCfgPlayIntercom.mod, UaConf::modNullaudio)
		) {
		strncpyz(uaConf.audioCfgPlayIntercom.mod, UaConf::modWinwave, sizeof(uaConf.audioCfgPlayIntercom.mod));
	}

	const Json::Value &uaConfAudioSoftVol = uaConfJson["audioSoftVol"];
	uaConf.audioSoftVol.tx = uaConfAudioSoftVol.get("tx", uaConf.audioSoftVol.tx).asUInt();
	uaConf.audioSoftVol.rx = uaConfAudioSoftVol.get("rx", uaConf.audioSoftVol.rx).asUInt();

	if (Branding::recording)
	{
		const Json::Value &uaConfRecordingJson = uaConfJson["recording"];
		uaConf.recording.enabled = uaConfRecordingJson.get("enabled", false).asBool();
		uaConf.recording.recDir = static_cast<UaConf::RecordingCfg::RecDir>(uaConfRecordingJson.get("recDir", UaConf::RecordingCfg::RecDirRelative).asInt());
		if (uaConf.recording.recDir < 0 || uaConf.recording.recDir >= UaConf::RecordingCfg::RecDirLimiter) {
			uaConf.recording.recDir = UaConf::RecordingCfg::RecDirRelative;
		}
		uaConf.recording.customRecDir = uaConfRecordingJson.get("customRecDir", "").asString();
		uaConf.recording.channels = uaConfRecordingJson.get("channels", uaConf.recording.channels).asUInt();
		if (uaConf.recording.channels < UaConf::RecordingCfg::CHANNELS_MIN || uaConf.recording.channels > UaConf::RecordingCfg::CHANNELS_MAX) {
			uaConf.recording.channels = UaConf::RecordingCfg::CHANNELS_MIN;
		}
		enum UaConf::RecStart recStart = uaConfRecordingJson.get("recStart", uaConf.recording.recStart).asInt();
		if (recStart >= 0 && recStart < UaConf::RecordingCfg::RecStartLimiter) {
			uaConf.recording.recStart = static_cast<UaConf::RecordingCfg::RecStart>(recStart);
        }
	}
	else
	{
		uaConf.recording.enabled = false;
	}

	const Json::Value &uaConfAudioPreprocTxJson = uaConfJson["audioPreprocTx"];
	uaConf.audioPreprocTx.enabled = uaConfAudioPreprocTxJson.get("enabled", uaConf.audioPreprocTx.enabled).asBool();
	uaConf.audioPreprocTx.denoiseEnabled = uaConfAudioPreprocTxJson.get("denoiseEnabled", uaConf.audioPreprocTx.denoiseEnabled).asBool();
	uaConf.audioPreprocTx.agcEnabled = uaConfAudioPreprocTxJson.get("agcEnabled", uaConf.audioPreprocTx.agcEnabled).asBool();
	uaConf.audioPreprocTx.vadEnabled = uaConfAudioPreprocTxJson.get("vadEnabled", uaConf.audioPreprocTx.vadEnabled).asBool();
	uaConf.audioPreprocTx.dereverbEnabled = uaConfAudioPreprocTxJson.get("dereverbEnabled", uaConf.audioPreprocTx.dereverbEnabled).asBool();
	uaConf.audioPreprocTx.agcLevel = uaConfAudioPreprocTxJson.get("agcLevel", uaConf.audioPreprocTx.agcLevel).asInt();

	uaConf.aec = (enum UaConf::Aec)uaConfJson.get("aec", UaConf::AEC_WEBRTC).asInt();
	if (uaConf.aec < 0 || uaConf.aec >= UaConf::AEC_LIMIT)
	{
    	uaConf.aec = UaConf::AEC_WEBRTC;
	}

	const Json::Value &webrtcAec = uaConfJson["webrtcAec"];
	uaConf.webrtcAec.msInSndCardBuf = webrtcAec.get("msInSndCardBuf", 120).asInt();
	uaConf.webrtcAec.skew = webrtcAec.get("skew", 0).asInt();

	uaConf.logMessages = uaConfJson.get("logMessages", false).asBool();
	uaConf.local = uaConfJson.get("localAddress", "").asString();
	uaConf.ifname = uaConfJson.get("ifName", "").asString();

	const Json::Value &uaAvtJson = uaConfJson["avt"];
	uaConf.avt.portMin = uaAvtJson.get("portMin", uaConf.avt.portMin).asUInt();
	uaConf.avt.portMax = uaAvtJson.get("portMax", uaConf.avt.portMax).asUInt();
	uaConf.avt.jbufDelayMin = uaAvtJson.get("jbufDelayMin", uaConf.avt.jbufDelayMin).asUInt();
	uaConf.avt.jbufDelayMax = uaAvtJson.get("jbufDelayMax", uaConf.avt.jbufDelayMax).asUInt();
	uaConf.avt.rtpTimeout = uaAvtJson.get("rtpTimeout", uaConf.avt.rtpTimeout).asUInt();
	uaConf.avt.Validate();	

	uaConf.autoAnswer = uaConfJson.get("autoAnswer", false).asBool();
	uaConf.autoAnswerCode = uaConfJson.get("autoAnswerCode", 200).asInt();
	uaConf.autoAnswerDelayMin = uaConfJson.get("autoAnswerDelayMin", 0). asUInt();
	uaConf.autoAnswerDelayMax = uaConfJson.get("autoAnswerDelayMax", 0). asUInt();
	if (uaConf.autoAnswerDelayMin > uaConf.autoAnswerDelayMax)
	{
		uaConf.autoAnswerDelayMin = 0;
		uaConf.autoAnswerDelayMax = 0;
	}
	uaConf.autoAnswerCallInfo = uaConfJson.get("autoAnswerCallInfo", false).asBool();
	uaConf.autoAnswerCallInfoDelayMin = uaConfJson.get("autoAnswerCallInfoDelayMin", 0).asUInt();

	uaConf.answerOnEventTalk = uaConfJson.get("answerOnEventTalk", false).asBool();

	uaConf.handleOodRefer = uaConfJson.get("handleOodRefer", uaConf.handleOodRefer).asBool();

	uaConf.customUserAgent = uaConfJson.get("customUserAgent", false).asBool();
	uaConf.userAgent = uaConfJson.get("userAgent", "").asString();

	const Json::Value &accounts = root["Accounts"];
	//uaConf.accounts.clear();
	for (int i=0; i<std::min(1u, accounts.size()); i++)
	{
		const Json::Value &acc = accounts[i];
		struct UaConf::Account new_acc;

		new_acc.reg_server = acc.get("reg_server", "").asString();
		new_acc.user = acc.get("user", "").asString();
		new_acc.auth_user = acc.get("auth_user", "").asString();
		new_acc.pwd = acc.get("pwd", "").asString();
		new_acc.cuser = acc.get("cuser", "").asString();
		new_acc.transport =
			(UaConf::Account::TRANSPORT_TYPE)acc.get("transport", UaConf::Account::TRANSPORT_UDP).asInt();
		if (new_acc.transport < 0 || new_acc.transport >= UaConf::Account::TRANSPORT_LIMITER)
		{
			new_acc.transport = UaConf::Account::TRANSPORT_UDP;
		}
		// do not register by default
		// (long timeout when unregistering from non-existing server, when application is closing
		// or when account settings are changed to valid)
		new_acc.reg_expires = acc.get("reg_expires", 0).asInt();
		if (new_acc.reg_expires < 0 || new_acc.reg_expires > 7200)
		{
			new_acc.reg_expires = 60;
		}
		new_acc.answer_any = acc.get("answer_any", false).asBool();
		new_acc.ptime = acc.get("ptime", new_acc.ptime).asInt();
		if (new_acc.ptime < UaConf::Account::MIN_PTIME || new_acc.ptime > UaConf::Account::MAX_PTIME)
		{
			new_acc.ptime = UaConf::Account::DEF_PTIME;
        }

		new_acc.stun_server = acc.get("stun_server", "").asString();
		new_acc.outbound1 = acc.get("outbound1", "").asString();
		new_acc.outbound2 = acc.get("outbound2", "").asString();

		const Json::Value &audio_codecs = acc["audio_codecs"];
		if (audio_codecs.type() == Json::arrayValue)
		{
            new_acc.audio_codecs.clear();
			for (int i=0; i<audio_codecs.size(); i++)
			{
				const Json::Value &codec = audio_codecs[i];
				new_acc.audio_codecs.push_back(codec.asString());
			}
		}
		else
		{
			// default
			new_acc.audio_codecs.push_back("PCMU/8000/1");
			new_acc.audio_codecs.push_back("PCMA/8000/1");
		}

		uaConf.accounts[0] = new_acc;
	}

	const Json::Value &hotkeyConfJson = root["hotkeyConf"];
	for (int i=0; i<hotkeyConfJson.size(); i++)
	{
		const Json::Value &hotkeyJson = hotkeyConfJson[i];
		class HotKeyConf cfg;
		cfg.keyCode = hotkeyJson.get("keyCode", "").asString().c_str();
		int id = vkey_find(cfg.keyCode.c_str());
		if (id >= 0)
		{
        	cfg.vkCode = vkey_list[id].vkey;
		}
		else
		{
        	cfg.vkCode = -1;
		}
		cfg.modifiers = hotkeyJson.get("modifiers", 0).asInt();
		cfg.global = hotkeyJson.get("global", false).asBool();
		const Json::Value &action = hotkeyJson["action"];
		cfg.action.type = static_cast<Action::Type>(action.get("type", 0).asInt());
		cfg.action.id = action.get("id", 0).asInt();

		hotKeyConf.push_back(cfg); 
	}

	const Json::Value &phoneConfJson = root["phoneConf"];
	for (int i=0; i<phoneConfJson.size(); i++)
	{
		const Json::Value &phoneJson = phoneConfJson[i];
		PhoneConf cfg;
		cfg.dllName = phoneJson.get("dllName", "").asString().c_str();
		if (cfg.dllName != "")
		{
        	phoneConf.push_back(cfg);
		}
	}

	const Json::Value &guiJson = root["gui"];
	gui.scalingPct = guiJson.get("scalingPct", 100).asInt();
	if (gui.scalingPct < gui.SCALING_MIN || gui.scalingPct > gui.SCALING_MAX) {
    	gui.scalingPct = 100;    
	}

	int maxX = GetSystemMetrics(SM_CXSCREEN);
	/** \todo Ugly fixed taskbar margin */
	int maxY = GetSystemMetrics(SM_CYSCREEN) - 32;

	const Json::Value &frmMainJson = root["frmMain"];
	frmMain.iWidth = frmMainJson.get("AppWidth", 350).asInt();
	frmMain.iHeight = frmMainJson.get("AppHeight", 300).asInt();
	if (frmMain.iWidth < 250 || frmMain.iWidth > maxX + 20)
	{
		frmMain.iWidth = 250;
	}
	if (frmMain.iHeight < 200 || frmMain.iHeight > maxY + 20)
	{
		frmMain.iHeight = 200;
	}
	frmMain.iPosX = frmMainJson.get("AppPositionX", 30).asInt();
	frmMain.iPosY = frmMainJson.get("AppPositionY", 30).asInt();
	if (frmMain.iPosX < 0)
		frmMain.iPosX = 0;
	if (frmMain.iPosY < 0)
		frmMain.iPosY = 0;
	if (frmMain.iPosX + frmMain.iWidth > maxX)
		frmMain.iPosX = maxX - frmMain.iWidth;
	if (frmMain.iPosY + frmMain.iHeight > maxY)
		frmMain.iPosY = maxY - frmMain.iHeight;
	frmMain.bWindowMaximized = frmMainJson.get("Maximized", false).asBool();
	frmMain.bAlwaysOnTop = frmMainJson.get("AlwaysOnTop", false).asBool();
	frmMain.bSpeedDialVisible = frmMainJson.get("SpeedDialVisible", false).asBool();
	frmMain.bSpeedDialOnly = frmMainJson.get("SpeedDialOnly", frmMain.bSpeedDialOnly).asBool();
	frmMain.bSpeedDialPopupMenu = frmMainJson.get("SpeedDialPopupMenu", frmMain.bSpeedDialPopupMenu).asBool();
	frmMain.bSpeedDialIgnorePresenceNote = frmMainJson.get("SpeedDialIgnorePresenceNote", frmMain.bSpeedDialIgnorePresenceNote).asBool();
	frmMain.bSpeedDialIgnoreDialogInfoRemoteIdentity = frmMainJson.get("SpeedDialIgnoreDialogInfoRemoteIdentity", frmMain.bSpeedDialIgnoreDialogInfoRemoteIdentity).asBool();	
	frmMain.iSpeedDialSize = frmMainJson.get("SpeedDialSize", 1).asUInt();
	if (frmMain.iSpeedDialSize > ProgrammableButtons::EXT_CONSOLE_COLUMNS)
		frmMain.iSpeedDialSize = 0;
	frmMain.iSpeedDialWidth = frmMainJson.get("SpeedDialWidth", 105).asUInt();
	if (frmMain.iSpeedDialWidth < 50 || frmMain.iSpeedDialWidth > 300)
        frmMain.iSpeedDialWidth = 105;
	frmMain.bStartMinimizedToTray = frmMainJson.get("StartMinimizedToTray", false).asBool();
	frmMain.bXBtnMinimize = frmMainJson.get("XBtnMinimize", false).asBool();
	frmMain.bRestoreOnIncomingCall = frmMainJson.get("RestoreOnIncomingCall", false).asBool();
	frmMain.bSingleInstance = frmMainJson.get("SingleInstance", frmMain.bSingleInstance).asBool();
	frmMain.dialpadBackgroundImage = frmMainJson.get("DialpadBackgroundImage", frmMain.dialpadBackgroundImage.c_str()).asString().c_str();
	frmMain.mainIcon = frmMainJson.get("MainIcon", frmMain.mainIcon.c_str()).asString().c_str();
	frmMain.trayNotificationImage = frmMainJson.get("TrayNotificationImage", frmMain.trayNotificationImage.c_str()).asString().c_str();

	frmMain.bNoBeepOnEnterKey = frmMainJson.get("NoBeepOnEnterKey", frmMain.bNoBeepOnEnterKey).asBool();
	frmMain.bHideSettings = frmMainJson.get("HideSettings", frmMain.bHideSettings).asBool();
	frmMain.bHideView = frmMainJson.get("HideView", frmMain.bHideView).asBool();
	frmMain.bHideHelp = frmMainJson.get("HideHelp", frmMain.bHideHelp).asBool();
	frmMain.bKioskMode = frmMainJson.get("KioskMode", frmMain.bKioskMode).asBool();
	frmMain.bHideMouseCursor = frmMainJson.get("HideMouseCursor", frmMain.bHideMouseCursor).asBool();
	frmMain.bShowWhenAnsweringCall = frmMainJson.get("ShowWhenAnsweringCall", frmMain.bShowWhenAnsweringCall).asBool();


	const Json::Value &frmTrayNotifierJson = root["frmTrayNotifier"];
	frmTrayNotifier.iHeight = 105;
	frmTrayNotifier.iWidth = 213;
	frmTrayNotifier.iPosX = frmTrayNotifierJson.get("PosX", maxX - frmTrayNotifier.iWidth).asInt();
	frmTrayNotifier.iPosY = frmTrayNotifierJson.get("PosY", maxY - frmTrayNotifier.iHeight).asInt();
	if (frmTrayNotifier.iPosX < 0)
		frmTrayNotifier.iPosX = 0;
	if (frmTrayNotifier.iPosY < 0)
		frmTrayNotifier.iPosY = 0;
	if (frmTrayNotifier.iPosX + frmTrayNotifier.iWidth > maxX)
		frmTrayNotifier.iPosX = maxX - frmTrayNotifier.iWidth;
	if (frmTrayNotifier.iPosY + frmTrayNotifier.iHeight > maxY)
		frmTrayNotifier.iPosY = maxY - frmTrayNotifier.iHeight;
	frmTrayNotifier.showOnIncoming = frmTrayNotifierJson.get("ShowOnIncoming", false).asBool();
	frmTrayNotifier.skipIfMainWindowVisible = frmTrayNotifierJson.get("SkipIfMainWindowVisible", false).asBool();
	frmTrayNotifier.showOnOutgoing = frmTrayNotifierJson.get("ShowOnOutgoing", false).asBool();
	frmTrayNotifier.hideWhenAnsweringCall = frmTrayNotifierJson.get("HideWhenAnsweringCall", frmTrayNotifier.hideWhenAnsweringCall).asBool();
	frmTrayNotifier.backgroundImage = frmTrayNotifierJson.get("BackgroundImage", frmTrayNotifier.backgroundImage.c_str()).asString().c_str();
	frmTrayNotifier.scalingPct = frmTrayNotifierJson.get("ScalingPct", frmTrayNotifier.scalingPct).asInt();
	if (frmTrayNotifier.scalingPct < _frmTrayNotifier::SCALING_MIN || frmTrayNotifier.scalingPct > _frmTrayNotifier::SCALING_MAX) {
		frmTrayNotifier.scalingPct = _frmTrayNotifier::SCALING_DEF;
	}

	const Json::Value &frmContactPopupJson = root["frmContactPopup"];
	frmContactPopup.showOnIncoming = frmContactPopupJson.get("ShowOnIncoming", false).asBool();
	frmContactPopup.showOnOutgoing = frmContactPopupJson.get("ShowOnOutgoing", false).asBool();
	frmContactPopup.iPosX = frmContactPopupJson.get("PosX", 100).asInt();
	frmContactPopup.iPosY = frmContactPopupJson.get("PosY", 100).asInt();
	frmContactPopup.iWidth = frmContactPopupJson.get("Width", 400).asInt();
	frmContactPopup.iHeight = frmContactPopupJson.get("Height", 140).asInt();

	const Json::Value &LoggingJson = root["Logging"];
	Logging.bLogToFile = LoggingJson.get("LogToFile", false).asBool();
	Logging.bFlush = LoggingJson.get("Flush", Logging.bFlush).asBool();
	Logging.iMaxFileSize = LoggingJson.get("MaxFileSize", Logging.iMaxFileSize).asInt();
	if (Logging.iMaxFileSize < Settings::_Logging::MIN_MAX_FILE_SIZE || Logging.iMaxFileSize > Settings::_Logging::MIN_MAX_FILE_SIZE)
	{
		Logging.iMaxFileSize = Settings::_Logging::DEF_MAX_FILE_SIZE;
	}
	Logging.iLogRotate = LoggingJson.get("LogRotate", Logging.iLogRotate).asUInt();
	if (Logging.iLogRotate > Settings::_Logging::MAX_LOGROTATE)
	{
        Logging.iLogRotate = Settings::_Logging::DEF_LOGROTATE;
    }
	Logging.iMaxUiLogLines = LoggingJson.get("MaxUiLogLines", 5000).asInt();

	const Json::Value &CallsJson = root["Calls"];
	Calls.extraHeaderLines = CallsJson.get("ExtraHeaderLines", Calls.extraHeaderLines.c_str()).asString().c_str();

	const Json::Value &DisplayJson = root["Display"];
	Display.bUserOnlyClip = DisplayJson.get("UserOnlyClip", Display.bUserOnlyClip).asBool();
	Display.bDecodeUtfDisplayToAnsi = DisplayJson.get("DecodeUtfDisplayToAnsi", Display.bDecodeUtfDisplayToAnsi).asBool();

	const Json::Value &IntegrationJson = root["Integration"];
	Integration.bAddFilterWMCopyData = IntegrationJson.get("AddFilterWMCopyData", false).asBool();
	Integration.asProtocol = IntegrationJson.get("Protocol", Branding::appProto.c_str()).asString().c_str();

	const Json::Value &RingJson = root["Ring"];
	Ring.defaultRing = RingJson.get("DefaultRing", "ring.wav").asString().c_str();
	const Json::Value &bellcore = RingJson["Bellcore"];
	for (int i=0; i<std::min(sizeof(Ring.bellcore)/sizeof(Ring.bellcore[0]), bellcore.size()); i++)
	{
		Ring.bellcore[i] = bellcore[i].asString().c_str();
	}

	const Json::Value &HttpQueryJson = root["HttpQuery"];
	HttpQuery.url = HttpQueryJson.get("Url", HttpQuery.url.c_str()).asString().c_str();
	HttpQuery.openMode = static_cast<_HttpQuery::OpenMode>(HttpQueryJson.get("OpenMode", HttpQuery.openMode).asInt());
	if (HttpQuery.openMode < 0 || HttpQuery.openMode >= _HttpQuery::openModeLimiter)
	{
        HttpQuery.openMode = _HttpQuery::openManualOnly;
	}

	const Json::Value &SipAccessUrlJson = root["SipAccessUrl"];
	SipAccessUrl.accessMode = static_cast<_SipAccessUrl::AccessMode>(SipAccessUrlJson.get("AccessMode", SipAccessUrl.accessMode).asInt());
	if (SipAccessUrl.accessMode < 0 || SipAccessUrl.accessMode >= _SipAccessUrl::accessModeLimiter)
	{
        SipAccessUrl.accessMode = _SipAccessUrl::accessModeAlwaysPassive;
    }

	const Json::Value &ContactsJson = root["Contacts"];
	Contacts.filterUsingNote = ContactsJson.get("FilterUsingNote", Contacts.filterUsingNote).asBool();

	const Json::Value &HistoryJson = root["History"];
	History.bNoStoreToFile = HistoryJson.get("NoStoreToFile", History.bNoStoreToFile).asBool();

	const Json::Value &ScriptsJson = root["Scripts"];
	Scripts.onMakeCall = ScriptsJson.get("OnMakeCall", Scripts.onMakeCall.c_str()).asString().c_str();
	Scripts.onCallState = ScriptsJson.get("OnCallState", Scripts.onCallState.c_str()).asString().c_str();
	Scripts.onStreamingState = ScriptsJson.get("OnStreamingState", Scripts.onStreamingState.c_str()).asString().c_str();
	Scripts.onRegistrationState = ScriptsJson.get("OnRegistrationState", Scripts.onRegistrationState.c_str()).asString().c_str();
	Scripts.onStartup = ScriptsJson.get("OnStartup", Scripts.onStartup.c_str()).asString().c_str();
	Scripts.onTimer = ScriptsJson.get("OnTimer", Scripts.onTimer.c_str()).asString().c_str();
	Scripts.timer = ScriptsJson.get("Timer", Scripts.timer).asInt();
	if (Scripts.timer <= 0)
		Scripts.timer = 1000;
	Scripts.onDialogInfo = ScriptsJson.get("OnDialogInfo", Scripts.onDialogInfo.c_str()).asString().c_str();
	Scripts.onDial = ScriptsJson.get("OnDial", Scripts.onDial.c_str()).asString().c_str();

	return 0;
}

int Settings::Write(AnsiString asFileName)
{
	Json::Value root;
	Json::StyledWriter writer;

	{
		Json::Value &jv = root["info"];
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
		jv["SpeedDialSize"] = frmMain.iSpeedDialSize;
		jv["SpeedDialWidth"] = frmMain.iSpeedDialWidth;
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
		jv["HideHelp"] = frmMain.bHideHelp;
		jv["KioskMode"] = frmMain.bKioskMode;
		jv["HideMouseCursor"] = frmMain.bHideMouseCursor;
		jv["ShowWhenAnsweringCall"] = frmMain.bShowWhenAnsweringCall;
	}

    {
		Json::Value& jv = root["frmTrayNotifier"];
		jv["PosX"] = frmTrayNotifier.iPosX;
		jv["PosY"] = frmTrayNotifier.iPosY;
		jv["ShowOnIncoming"] = frmTrayNotifier.showOnIncoming;
		jv["SkipIfMainWindowVisible"] = frmTrayNotifier.skipIfMainWindowVisible;
		jv["ShowOnOutgoing"] = frmTrayNotifier.showOnOutgoing;
		jv["HideWhenAnsweringCall"] = frmTrayNotifier.hideWhenAnsweringCall;
		jv["BackgroundImage"] = frmTrayNotifier.backgroundImage.c_str();
		jv["ScalingPct"] = frmTrayNotifier.scalingPct;
	}

	root["frmContactPopup"]["ShowOnIncoming"] = frmContactPopup.showOnIncoming;
	root["frmContactPopup"]["ShowOnOutgoing"] = frmContactPopup.showOnOutgoing;
	root["frmContactPopup"]["PosX"] = frmContactPopup.iPosX;
	root["frmContactPopup"]["PosY"] = frmContactPopup.iPosY;
	root["frmContactPopup"]["Width"] = frmContactPopup.iWidth;
	root["frmContactPopup"]["Height"] = frmContactPopup.iHeight;

	root["Logging"]["LogToFile"] = Logging.bLogToFile;
	root["Logging"]["Flush"] = Logging.bFlush;
	root["Logging"]["MaxFileSize"] = Logging.iMaxFileSize;
	root["Logging"]["LogRotate"] = Logging.iLogRotate;
	root["Logging"]["MaxUiLogLines"] = Logging.iMaxUiLogLines;

	root["Calls"]["ExtraHeaderLines"] = Calls.extraHeaderLines.c_str();

	root["Display"]["UserOnlyClip"] = Display.bUserOnlyClip;
	root["Display"]["DecodeUtfDisplayToAnsi"] = Display.bDecodeUtfDisplayToAnsi;

	root["Integration"]["AddFilterWMCopyData"] = Integration.bAddFilterWMCopyData;
	root["Integration"]["Protocol"] = Integration.asProtocol.c_str();

	root["Ring"]["DefaultRing"] = Ring.defaultRing.c_str();
	for (int i=0; i<sizeof(Ring.bellcore)/sizeof(Ring.bellcore[0]); i++)
	{
		root["Ring"]["Bellcore"][i] = Ring.bellcore[i].c_str();
	}

	root["HttpQuery"]["Url"] = HttpQuery.url.c_str();
	root["HttpQuery"]["OpenMode"] = HttpQuery.openMode;

	root["SipAccessUrl"]["AccessMode"] = SipAccessUrl.accessMode;

	root["Contacts"]["FilterUsingNote"] = Contacts.filterUsingNote;

	root["History"]["NoStoreToFile"] = History.bNoStoreToFile;

	root["Scripts"]["OnMakeCall"] = Scripts.onMakeCall.c_str();
	root["Scripts"]["OnCallState"] = Scripts.onCallState.c_str();
	root["Scripts"]["OnStreamingState"] = Scripts.onStreamingState.c_str();
	root["Scripts"]["OnRegistrationState"] = Scripts.onRegistrationState.c_str();
	root["Scripts"]["OnStartup"] = Scripts.onStartup.c_str();
	root["Scripts"]["OnTimer"] = Scripts.onTimer.c_str();
	root["Scripts"]["Timer"] = Scripts.timer;
	root["Scripts"]["OnDialogInfo"] = Scripts.onDialogInfo.c_str();
	root["Scripts"]["OnDial"] = Scripts.onDial.c_str();

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

	root["uaConf"]["audioSoftVol"]["tx"] = uaConf.audioSoftVol.tx;
	root["uaConf"]["audioSoftVol"]["rx"] = uaConf.audioSoftVol.rx;

	if (Branding::recording)
	{
		root["uaConf"]["recording"]["enabled"] = uaConf.recording.enabled;
		root["uaConf"]["recording"]["recDir"] = uaConf.recording.recDir;
		root["uaConf"]["recording"]["customRecDir"] = uaConf.recording.customRecDir;
		root["uaConf"]["recording"]["channels"] = uaConf.recording.channels;
		root["uaConf"]["recording"]["recStart"] = uaConf.recording.recStart;
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
	
	root["uaConf"]["localAddress"] = uaConf.local;
	root["uaConf"]["ifName"] = uaConf.ifname;
	root["uaConf"]["avt"]["portMin"] = uaConf.avt.portMin;
	root["uaConf"]["avt"]["portMax"] = uaConf.avt.portMax;
	root["uaConf"]["avt"]["jbufDelayMin"] = uaConf.avt.jbufDelayMin;
	root["uaConf"]["avt"]["jbufDelayMax"] = uaConf.avt.jbufDelayMax;
	root["uaConf"]["avt"]["rtpTimeout"] = uaConf.avt.rtpTimeout;

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

	// write accounts
	for (unsigned int i=0; i<uaConf.accounts.size(); i++)
	{
		UaConf::Account &acc = uaConf.accounts[i];
		Json::Value &cfgAcc = root["Accounts"][i];
		cfgAcc["reg_server"] = acc.reg_server;
		cfgAcc["user"] = acc.user;
		cfgAcc["auth_user"] = acc.auth_user;
		cfgAcc["pwd"] = acc.pwd;
		cfgAcc["cuser"] = acc.cuser;
		cfgAcc["transport"] = acc.transport;
		cfgAcc["reg_expires"] = acc.reg_expires;
		cfgAcc["answer_any"] = acc.answer_any;
        cfgAcc["ptime"] = acc.ptime;
		cfgAcc["stun_server"] = acc.stun_server;
		cfgAcc["outbound1"] = acc.outbound1;
		cfgAcc["outbound2"] = acc.outbound2;
		for (unsigned int j=0; j<acc.audio_codecs.size(); j++)
		{
			cfgAcc["audio_codecs"][j] = acc.audio_codecs[j];
		}
	}
#if 0	// obsolete, replaced with "btnConf"
	// write contacts
	for (unsigned int i=0; i<uaConf.contacts.size(); i++)
	{
		UaConf::Contact &contact = uaConf.contacts[i];
		root["Contacts"][i]["description"] = contact.description;
		root["Contacts"][i]["user"] = contact.user;
		root["Contacts"][i]["sub_dialog_info"] = contact.sub_dialog_info;
		root["Contacts"][i]["sub_presence"] = contact.sub_presence;
	}
#endif

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
		for (iter = phoneConf.begin(); iter != phoneConf.end(); ++iter)
		{
			PhoneConf &cfg = *iter;
			Json::Value &cfgJson = root["phoneConf"][i++];
			cfgJson["dllName"] = cfg.dllName.c_str();
		}
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


