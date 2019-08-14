//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop                                                                                        

#include "UaMain.h"
#include "UaConf.h"
#include "ControlQueue.h"
#include "Command.h"
#include "CallbackQueue.h"
#include "Callback.h"
#include "Settings.h"
#include "Paths.h"
#include <re.h>
#include "baresip.h"
#include "Log.h"
#include "common\Utilities.h"
#include "Branding.h"
#include <assert.h>

#pragma package(smart_init)

inline void strncpyz(char* dst, const char* src, int dstsize) {
	strncpy(dst, src, dstsize);
	dst[dstsize-1] = '\0';
}

namespace {
	std::vector<AnsiString> audioCodecs;
	volatile bool audioCodecsAvailable = false;
}

//---------------------------------------------------------------------------
class Worker : public TThread
{
private:
protected:
	void __fastcall Execute();
public:
	__fastcall Worker(bool CreateSuspended);
};


#define DEBUG_MODULE "UaMain"
#define DEBUG_LEVEL 6
#include <re_dbg.h>

static struct {
	uint32_t n_uas;       /**< Number of User Agents           */
	bool terminating;     /**< Application is terminating flag */
	struct ua *ua;
	struct call *callp;
	struct paging_tx *paging_txp;	
} app;

struct ua* ua_cur(void) {
	return app.ua;
}

static bool appRestart = false;
static bool appQuit = false;

static void ua_exit_handler(int err)
{
	(void)err;
	re_cancel();
}

static void ua_event_handler(struct ua *ua, enum ua_event ev,
	struct call *call, const char *prm, void *arg)
{
	Callback::ua_state_e state;
	Callback::reg_state_e reg_state;
	/** \todo acc_id */

	const char* peer_name = "";
	int scode = 0;
	if (call)
	{
		peer_name = call_peername(call);
		if (peer_name == NULL)
			peer_name = "";
		scode = call_scode(call);
    }

	switch (ev)
	{
	case UA_EVENT_CALL_INCOMING:
		{
			if (app.paging_txp)
			{
				LOG("Denying incoming call (paging TX active)\n");
				ua_hangup(ua_cur(), call, 486, NULL);
				break;
			}
			const char* initial_rx_invite = call_initial_rx_invite(call);
			if (initial_rx_invite == NULL)
				initial_rx_invite = "";
			UA_CB->SetCallData(initial_rx_invite);

			state = Callback::CALL_STATE_INCOMING;
			app.callp = call;

			const char* alert_info = call_alert_info(call);
			if (alert_info == NULL)
				alert_info = "";

			const char* access_url = call_access_url(call);
			if (access_url == NULL)
				access_url = "";

			const char* pai_peer_uri = call_pai_peeruri(call);
			if (pai_peer_uri == NULL)
				pai_peer_uri = "";

			const char* pai_peer_name = call_pai_peername(call);
			if (pai_peer_name == NULL)
				pai_peer_name = "";

			UA_CB->ChangeCallState(state, prm, peer_name, scode, call_answer_after(call), alert_info, access_url, call_access_url_mode(call), pai_peer_uri, pai_peer_name, "");
			break;
		}
	case UA_EVENT_CALL_RINGING:
		state = Callback::CALL_STATE_RINGING;
		UA_CB->ChangeCallState(state, prm, peer_name, scode,  -1, "", "", -1, "", "", "");
		break;
	case UA_EVENT_CALL_TRYING:
		state = Callback::CALL_STATE_TRYING;
		UA_CB->ChangeCallState(state, prm, peer_name, scode,  -1, "", "", -1, "", "", "");
		break;
	case UA_EVENT_CALL_OUTGOING:
		state = Callback::CALL_STATE_OUTGOING;
		UA_CB->ChangeCallState(state, prm, peer_name, scode,  -1, "", "", -1, "", "", "");
		break;
	case UA_EVENT_CALL_PROGRESS:
		state = Callback::CALL_STATE_PROGRESS;
		UA_CB->ChangeCallState(state, prm, peer_name, scode,  -1, "", "", -1, "", "", "");
		break;
	case UA_EVENT_CALL_ESTABLISHED:
		{
			state = Callback::CALL_STATE_ESTABLISHED;
			const char* pai_peer_uri = call_pai_peeruri(call);
			if (pai_peer_uri == NULL)
				pai_peer_uri = "";

			const char* pai_peer_name = call_pai_peername(call);
			if (pai_peer_name == NULL)
				pai_peer_name = "";

			const char* codec_name = "";
			const struct audio* au = call_audio(call);
			if (au) {
				codec_name = audio_get_rx_aucodec_name(au);
			}

			UA_CB->ChangeCallState(state, prm, peer_name, scode,  -1, "", "", -1, pai_peer_uri, pai_peer_name, codec_name);
		}
		break;
	case UA_EVENT_CALL_CLOSED:
		if (call == app.callp)
		{
			state = Callback::CALL_STATE_CLOSED;
			UA_CB->ChangeCallState(state, prm, peer_name, scode,  -1, "", "", -1, "", "", "");
			app.callp = NULL;
		}
		else
		{
			LOG("Ignoring UA_EVENT_CALL_CLOSED (call transferred?)\n");
		}
		break;
	case UA_EVENT_CALL_DTMF_START:
		UA_CB->ChangeCallDtmfState(prm, true);
		break;
	case UA_EVENT_CALL_DTMF_END:
		UA_CB->ChangeCallDtmfState(prm, false);
		break;
	case UA_EVENT_CALL_TRANSFER:
		app.callp = call;
		state = Callback::CALL_STATE_TRANSFER;
		UA_CB->ChangeCallState(state, prm, peer_name, scode, -1, "", "", -1, "", "", "");
		break;
	case UA_EVENT_CALL_TRANSFER_OOD:
		state = Callback::CALL_STATE_TRANSFER_OOD;
		UA_CB->ChangeCallState(state, prm, peer_name, scode, -1, "", "", -1, "", "", "");
		break;
	case UA_EVENT_CALL_REINVITE_RECEIVED:
		{
			const char* pai_peer_uri = call_pai_peeruri(call);
			if (pai_peer_uri == NULL)
				pai_peer_uri = "";

			const char* pai_peer_name = call_pai_peername(call);
			if (pai_peer_name == NULL)
				pai_peer_name = "";

			UA_CB->OnReinviteReceived(prm, peer_name, pai_peer_uri, pai_peer_name);
		}
		break;
	case UA_EVENT_REGISTERING:
		reg_state = Callback::REG_STATE_REGISTERING;
		UA_CB->ChangeRegState(0, reg_state, prm);
		break;
	case UA_EVENT_REGISTER_OK:
		reg_state = Callback::REG_STATE_REGISTER_OK;
		UA_CB->ChangeRegState(0, reg_state, prm);
		break;
	case UA_EVENT_REGISTER_FAIL:
		reg_state = Callback::REG_STATE_REGISTER_FAIL;
		UA_CB->ChangeRegState(0, reg_state, prm);
		break;
	case UA_EVENT_UNREGISTERING:
		reg_state = Callback::REG_STATE_UNREGISTERING;
		UA_CB->ChangeRegState(0, reg_state, prm);
		break;
	case UA_EVENT_UNREGISTER_OK:
		reg_state = Callback::REG_STATE_UNREGISTER_OK;
		UA_CB->ChangeRegState(0, reg_state, prm);
		break;
	case UA_EVENT_UNREGISTER_FAIL:
		reg_state = Callback::REG_STATE_UNREGISTER_FAIL;
		UA_CB->ChangeRegState(0, reg_state, prm);
		break;
	case UA_EVENT_AUDIO_ERROR:
		UA_CB->NotifyAudioError();
		break;
	default:
		assert(!"Unhandled UA event");
	}

}

static int ua_add(const struct pl *addr, const char *pwd, const char *cuser)
{
	char buf[1024];
	int err;

	pl_strcpy(addr, buf, sizeof(buf));
	/** \note Do not pass stack variable as a ua argument, its value
		would be overwritten on object destruction! */
	err = ua_alloc(&app.ua, buf, pwd, cuser);
	if (err)
		return err;

	return err;
}

extern "C" void on_log(int level, const char *p)
{
	LOG("%s", p);
}

extern "C" int dialog_info_handler(int id, enum dialog_info_status status, enum dialog_info_direction direction, const char *remote_identity, const char *remote_identity_display)
{
	UA_CB->ChangeDlgInfoState(id, status, direction, remote_identity, remote_identity_display);
	return 0;
}

extern "C" int presence_handler(int id, enum presence_status status, const char *note)
{
	UA_CB->ChangePresenceState(id, status, note);
	return 0;
}


extern "C" int unsolicited_mwi_handler(struct sipevent_sock *sock, const struct sip_msg *msg) {
	if (msg->mb) {
		struct pl body;
		pl_set_mbuf(&body, msg->mb);
		// RFC 3842
		// Voice-Message: new/old (urgent new/old)
		//
		//  Messages-Waiting: yes
		//  Message-Account: sip:[service]@[local_ip]
		//  Voice-Message: 2/8 (0/2)
		re_printf("Received MWI:\n%r\n", &body);
		char bodyz[1024];
		if (body.l < sizeof(bodyz))
		{
			memcpy(bodyz, body.p, body.l);
			bodyz[body.l] = '\0';
			const char *vm = strstr(bodyz, "Voice-Message");
			if (vm)
			{
				const char* colon = strchr(vm, ':');
				if (colon)
				{
					colon++;
					int newmsg, oldmsg;
					if (sscanf(colon, "%d/%d", &newmsg, &oldmsg) == 2)
					{
						LOG("Messages: %d new, %d old\n", newmsg, oldmsg);
						UA_CB->ChangeMwiState(newmsg, oldmsg);
					}
				}
			}
		}
		else
		{
			LOG("Body size too large!\n");
		}
	}
	sip_reply(sipevent_sock_sip(sock), msg, 200, "OK");
	return 0;
}

/** Handle NOTIFY with "Event: talk" (some CRM integration through asterisk AMI)
*/
extern "C" int unsolicited_event_talk_handler(struct sipevent_sock *sock, const struct sip_msg *msg) {
	UA_CB->NotifyEventTalk();
	sip_reply(sipevent_sock_sip(sock), msg, 200, "OK");
	return 0;
}


extern "C" int paging_tx_handler(enum paging_tx_event ev)
{
	switch (ev)
	{
	case PAGING_TX_STOPPED:
		if (app.paging_txp)
		{
			struct paging_tx *txp = app.paging_txp;
			app.paging_txp = NULL;
			mem_deref(txp);
		}
		UA_CB->ChangePagingTxState(Callback::PAGING_TX_ENDED);
		break;
	default:
		assert(!"Unhandled paging TX message");
	}
	return 0;
}

static int print_handler_log(const char *p, size_t size, void *arg)
{
	(void)arg;
	if (size < sizeof(int))
	{
		// CG access overrun if p points to single character variable (write_padded)
		char tmp[4] = {0, 0, 0, 0};
		memcpy(tmp, p, size);
		LOG("%.*s", size, tmp);		
	}
	else
	{
		LOG("%.*s", size, p);
	}
	return 0;
}

static int app_init(void)
{
	int err;
	static struct re_printf pf_log = {print_handler_log, NULL};	

	memset(&app, 0, sizeof(app));

	print_handler_set(on_log);

	/* Initialise System library */
	err = libre_init();
	if (err)
		return err;

	dbg_init(DBG_INFO, DBG_NONE);

	/* Initialise dynamic modules */
	mod_init();

	struct config * cfg = conf_config();

	strncpyz(cfg->audio.src_mod, appSettings.uaConf.audioCfgSrc.mod, sizeof(cfg->audio.src_mod));
	if (strcmp(appSettings.uaConf.audioCfgSrc.mod, UaConf::modAufile))
	{
		strncpyz(cfg->audio.src_dev, appSettings.uaConf.audioCfgSrc.dev, sizeof(cfg->audio.src_dev));
	}
	else
	{
		AnsiString fileFull;
		fileFull.sprintf("%s\\%s", Paths::GetProfileDir().c_str(), appSettings.uaConf.audioCfgSrc.wavefile);
		strncpyz(cfg->audio.src_dev, fileFull.c_str(), sizeof(cfg->audio.src_dev));
	}
	strncpyz(cfg->audio.play_mod, appSettings.uaConf.audioCfgPlay.mod, sizeof(cfg->audio.play_mod));
	strncpyz(cfg->audio.play_dev, appSettings.uaConf.audioCfgPlay.dev, sizeof(cfg->audio.play_dev));
	strncpyz(cfg->audio.alert_mod, appSettings.uaConf.audioCfgAlert.mod, sizeof(cfg->audio.alert_mod));
	strncpyz(cfg->audio.alert_dev, appSettings.uaConf.audioCfgAlert.dev, sizeof(cfg->audio.alert_dev));
	strncpyz(cfg->audio.ring_mod, appSettings.uaConf.audioCfgRing.mod, sizeof(cfg->audio.ring_mod));
	strncpyz(cfg->audio.ring_dev, appSettings.uaConf.audioCfgRing.dev, sizeof(cfg->audio.ring_dev));

	cfg->aec = (config::e_aec)appSettings.uaConf.aec;

	cfg->webrtc.msInSndCardBuf = appSettings.uaConf.webrtcAec.msInSndCardBuf;
	cfg->webrtc.skew = appSettings.uaConf.webrtcAec.skew;

	strncpyz(cfg->sip.local, appSettings.uaConf.local.c_str(), sizeof(cfg->sip.local));
	strncpyz(cfg->net.ifname, appSettings.uaConf.ifname.c_str(), sizeof(cfg->net.ifname));

	net_debug(&pf_log, NULL);

	cfg->avt.rtp_ports.min = appSettings.uaConf.avt.portMin;
	cfg->avt.rtp_ports.max = appSettings.uaConf.avt.portMax;
	cfg->avt.jbuf_del.min = appSettings.uaConf.avt.jbufDelayMin;
	cfg->avt.jbuf_del.max = appSettings.uaConf.avt.jbufDelayMax;
    cfg->avt.rtp_timeout = appSettings.uaConf.avt.rtpTimeout;

	cfg->recording.enabled = appSettings.uaConf.recording.enabled;

	cfg->audio_preproc_tx.enabled = appSettings.uaConf.audioPreprocTx.enabled;
	cfg->audio_preproc_tx.denoise_enabled = appSettings.uaConf.audioPreprocTx.denoiseEnabled;
	cfg->audio_preproc_tx.agc_enabled = appSettings.uaConf.audioPreprocTx.agcEnabled;
	cfg->audio_preproc_tx.vad_enabled = appSettings.uaConf.audioPreprocTx.vadEnabled;
	cfg->audio_preproc_tx.dereverb_enabled = appSettings.uaConf.audioPreprocTx.dereverbEnabled;
	cfg->audio_preproc_tx.agc_level = appSettings.uaConf.audioPreprocTx.agcLevel;

	configure();

	/* Initialise User Agents */
	AnsiString uaName;
	if (appSettings.uaConf.customUserAgent == false) {
		uaName.sprintf("%s %s", Branding::appName.c_str(), GetFileVer(Application->ExeName).c_str());
	} else {
		uaName = appSettings.uaConf.userAgent.c_str();
    }

	err = ua_init(uaName.c_str(), appSettings.uaConf.logMessages,
		true, true, true, false);
	if (err)
		return err;

	uag_event_register(ua_event_handler, NULL);

	sipevent_reset_unsolicited_handlers();

	if (sipevent_register_unsolicited_handler("message-summary", unsolicited_mwi_handler)) {
		LOG("Failed to register unsolicited MWI handler!\n");
	}

	if (sipevent_register_unsolicited_handler("talk", unsolicited_event_talk_handler)) {
		LOG("Failed to register unsolicited MWI handler!\n");
	}

	if (paging_tx_register_handler(paging_tx_handler)) {
    	LOG("Failed to register paging TX handler!\n");
	}

	return 0;
}


static int app_start(void)
{
    int n;
	app.n_uas = 0;
	for (int i=0; i<appSettings.uaConf.accounts.size(); i++)
	{
		AnsiString addr;
		UaConf::Account &acc = appSettings.uaConf.accounts[i];
		addr.sprintf("<sip:%s@%s;transport=%s>;regint=%d", //;sipnat=outbound",
			acc.user.c_str(), acc.reg_server.c_str(),
			acc.getTransportStr(),
			acc.reg_expires
			);
		if (acc.auth_user != "")
		{
			addr.cat_printf(";auth_user=%s", acc.auth_user.c_str());
		}

		if (acc.stun_server != "")
		{
			addr.cat_printf(";medianat=stun;stunserver=stun:%s",
				acc.stun_server.c_str());
		}

		if (acc.outbound1 != "")
		{
			addr.cat_printf(";outbound1=sip:%s", acc.outbound1.c_str());
		}
		/** \todo outbound2 not working? only single Route line in outgoing REGISTER
		*/
		if (acc.outbound2 != "")
		{
			addr.cat_printf(";outbound2=sip:%s", acc.outbound2.c_str());
		}
		if (acc.answer_any != 0)
		{
			// "local account"
			addr.cat_printf(";answer_any=1");
		}
		addr.cat_printf(";dtmf_tx_format=%d", acc.dtmf_tx_format);
		addr.cat_printf(";ptime=%d", acc.ptime);

		{
			addr.cat_printf(";audio_codecs=");
			for (unsigned int i=0; i<acc.audio_codecs.size(); i++)
			{
				addr.cat_printf("%s", acc.audio_codecs[i].c_str());
				if (i < acc.audio_codecs.size() - 1)
				{
					addr.cat_printf(",");
				}
			}
		}

		pl pl_addr;
		pl_set_str(&pl_addr, addr.c_str());
		std::string cuser = acc.cuser;
		if (cuser.empty())
		{
			// interoperability: using user name as default contact user
			// instead of default, semi-random contact name based on memory address
			// (although this was valid this was problem for some operator)
			cuser = acc.user;
		}
		if (ua_add(&pl_addr, acc.pwd.c_str(), cuser.c_str()) == 0)
		{
			app.n_uas++;
		}
	}
	if (!app.n_uas)
	{
		DEBUG_WARNING("No SIP accounts found - check your config\n");
		return ENOENT;
	}

	if (appSettings.uaConf.accounts.size() != 1)
	{
		DEBUG_WARNING("Multiple accounts not handled!");
	}
	if (appSettings.uaConf.accounts.size() > 0)
	{
		UaConf::Account &acc = appSettings.uaConf.accounts[0];
		for (int i=0; i<appSettings.uaConf.contacts.size(); i++) {
			UaConf::Contact contact = appSettings.uaConf.contacts[i];
			if (contact.user == "")
			{
				continue;
			}
			AnsiString addr;
			if (contact.user.find("sip:") != std::string::npos)
			{
				addr.sprintf("<%s>", contact.user.c_str());
			}
			else
			{
				addr.sprintf("<sip:%s@%s;transport=%s>",
					contact.user.c_str(),
					acc.reg_server.c_str(),
					acc.getTransportStr()
					);
			}
			if (contact.sub_dialog_info)
			{
				addr.cat_printf(";dlginfo=p2p");
			}
			if (contact.sub_presence)
			{
				addr.cat_printf(";presence=p2p");
			}
			pl pl_addr;
			pl_set_str(&pl_addr, addr.c_str());
			contact_add(NULL, &pl_addr, i, dialog_info_handler, presence_handler);
		}
	}

	// contact list must be initialized here
	struct pl modname;
	pl_set_str(&modname, "dialog-info");
	load_module2(NULL, &modname);

	// contact list must be initialized here
	pl_set_str(&modname, "presence");
	load_module2(NULL, &modname);


	AnsiString dllPath;
	dllPath.sprintf("%s\\modules", Paths::GetProfileDir().c_str());
	struct pl plDllPath;
	pl_set_str(&plDllPath, dllPath.c_str());

	LOG("Loading module dlls...\n");
	WIN32_FIND_DATA file;
	AnsiString asSrchPath = dllPath + "\\*.dll";
	HANDLE hFind = FindFirstFile(asSrchPath.c_str(), &file);
	int hasfiles = (hFind != INVALID_HANDLE_VALUE);

	while (hasfiles)
	{
		LOG("Loading %s\n", file.cFileName);
		AnsiString name = ExtractFileName(file.cFileName);
	#if 0
		AnsiString filename = /*dir + */ file.cFileName;
		if(VerifyDll(filename, &dllinfo) == E_OK)
		{
			AddDll(dllinfo);
		}
		else
		{
			LOG("Library %s not loaded\n", filename.c_str());
		}
	#endif		
		pl_set_str(&modname, name.c_str());
		load_module_dynamic(NULL, &plDllPath, &modname);
		hasfiles = FindNextFile(hFind, &file);
	}
	FindClose(hFind);


	n = list_count(aucodec_list());
	LOG("Populated %u audio codec%s\n", n, 1==n?"":"s");
	if (0 == n) {
		DEBUG_WARNING("No audio-codec modules loaded!\n");
	}

	if (audioCodecsAvailable == false) {
		struct le *le;
		for (le = aucodec_list()->head; le; le=le->next) {
			struct aucodec *ac = (struct aucodec*)le->data;
			AnsiString tmp;
			tmp.sprintf("%s/%d/%d", ac->name, ac->srate, ac->ch);
            audioCodecs.push_back(tmp);
		}
		audioCodecsAvailable = true;
	}

	n = list_count(aufilt_list());
	LOG("Populated %u audio filter%s\n", n, 1==n?"":"s");	

	return 0;
}


static void app_close(void)
{
	ua_close();
	mod_close();
	libre_close();

	/* Check for memory leaks */
    LOG("Checking for memory leaks...\n");
	tmr_debug();
	mem_debug();
}

extern "C" void dialog_info_subscriber_close(void);
extern "C" void subscriber_close(void);

static void quit(int sig)
{
	if (!app.terminating) {
		/** \todo Closing dialog-info subscriber here shouldn't be necessary? */
		dialog_info_subscriber_close();
		/** \todo Closing presence subscriber here shouldn't be necessary? */
		subscriber_close();
		ua_stop_all(false);
		app.terminating = true;
		return;
	}

	LOG("forced exit - signal %d\n", sig);

	ua_stop_all(true);
}


extern "C" void signal_handler(int sig)
{
	LOG("terminated by signal %d\n", sig);
	quit(sig);
}


extern "C" int stderr_handler(const char *p, size_t size, void *arg)
{
	(void)arg;

	if (1 != fwrite(p, size, 1, stderr))
		return ENOMEM;

	return 0;
}

extern "C" void control_handler(void)
{
	if (app.terminating)
	{
		return;
	}
	else if (appQuit || appRestart)
	{
		//if (appQuit)
		//{
		//	app.terminating = true;
		//}
		quit(0);
	}
	
	int err;
	Command cmd;
	if (UA->GetCommand(cmd))
	{
    	return;
	}
	switch (cmd.type)
	{
	case Command::CALL:
		err = ua_connect(ua_cur(), &app.callp, NULL /*from*/,
			cmd.target.c_str(), NULL, VIDMODE_OFF, cmd.extraHeaderLines.c_str());
		if (err)
		{
			DEBUG_WARNING("connect failed: %m\n", err);
		}
		break;
	case Command::ANSWER:
		ua_answer(ua_cur(), app.callp, cmd.audioMod.c_str(), cmd.audioDev.c_str());
		break;
	case Command::TRANSFER:
		if (app.callp)
		{
			call_transfer(app.callp, cmd.target.c_str());
		}
		break;
	case Command::SEND_DIGIT:
		if (app.callp)
		{
			if (call_send_digit(app.callp, cmd.key) == 0)
			{
				call_send_digit(app.callp, 0x00);
			}
		}
		break;
	case Command::HOLD:
		if (app.callp)
		{
			call_hold(app.callp, cmd.bEnabled);
		}
		break;
	case Command::MUTE:
		if (app.callp)
		{
			struct audio *audio = call_audio(app.callp);
			audio_mute(audio, cmd.bEnabled);
		}
		break;
	case Command::HANGUP:
		if (app.callp)
		{
			ua_hangup(ua_cur(), app.callp, cmd.code, NULL);
			UA_CB->ChangeCallState(Callback::CALL_STATE_CLOSED, "", "", 0, -1, "", "", -1, "", "", "");
			app.callp = NULL;
		}
		if (app.paging_txp)
		{
			paging_tx_hangup(app.paging_txp);
			//mem_deref(app.paging_txp);
			//UA_CB->ChangePagingTxState(Callback::PAGING_TX_ENDED);
			//app.paging_txp = NULL;
		}
		break;
	case Command::SET_MSG_LOGGING:
		ua_log_messages(cmd.bEnabled);
		break;
	case Command::REREGISTER: {
		struct ua* ua = ua_cur();
		err |= ua_reregister(ua);
		break;
	}
	case Command::UNREGISTER: {
		struct ua* ua = ua_cur();
		ua_unregister(ua);
		break;
	}
	case Command::START_RING: {
		struct ua* ua = ua_cur();
		struct config * cfg = conf_config();
		//LOG("UaMain: START_RING\n");
		(void)ua_play_file(ua, cfg->audio.ring_mod, cfg->audio.ring_dev, cmd.target.c_str(), -1);
		break;
	}
	case Command::STOP_RING: {
		struct ua* ua = ua_cur();
		//LOG("UaMain: STOP_RING\n");
		ua_play_stop(ua);
		break;
	}
	case Command::RECORD: {
        recorder_start(cmd.target.c_str(), cmd.channels, static_cast<enum recorder_side>(cmd.recSide));
		break;
	}
	case Command::PAGING_TX: {
		struct paging_tx *tx;
		int err;
		struct sa addr;
		uint16_t port = 4000;
		int pos = cmd.target.Pos(":");
		AnsiString addr_part = cmd.target;
		if (pos > 0)
		{
			AnsiString asPort = cmd.target.SubString(pos + 1, cmd.target.Length() - pos);
			port = StrToIntDef(asPort, port);
			addr_part = cmd.target.SubString(1, pos - 1);
		}
		else
		{
			DEBUG_WARNING("PAGING_TX: no target port is set in configuration (%s)\n", cmd.target.c_str());
			break;
		}
		//err = net_inet_pton(cmd.target.c_str(), &addr);
		err = sa_set_str(&addr, addr_part.c_str(), port);
		if (err)
		{
			DEBUG_WARNING("PAGING_TX: failed to convert target text (%s) to network address\n", cmd.target.c_str());
			break;
		}
		const char* forced_src_mod = NULL;
		const char* forced_src_dev = NULL;
		if (cmd.pagingTxWaveFile != "")
		{
			forced_src_mod = "aufile";
            forced_src_dev = cmd.pagingTxWaveFile.c_str();
		}
		err = paging_tx_alloc(&tx, conf_config(), &addr, forced_src_mod, forced_src_dev, cmd.pagingTxCodec.c_str(), cmd.pagingTxPtime);
		if (err)
		{
			DEBUG_WARNING("paging_tx_alloc failed (%m)\n", err);
			break;
		}
		err = paging_tx_start(tx);
		if (err)
		{
			DEBUG_WARNING("paging_tx_start failed (%m)\n", err);
			mem_deref(tx);
		}
		else
		{
			app.paging_txp = tx;
			UA_CB->ChangePagingTxState(Callback::PAGING_TX_STARTED);			
		}
		break;
	}
	case Command::SWITCH_AUDIO_SOURCE: {
		struct audio* a = NULL;
		if (app.callp)
		{
			a = call_audio(app.callp);
		}
		else if (app.paging_txp)
		{
        	a = paging_audio(app.paging_txp);
		}
		if (a)
		{
			err = audio_set_source(a, cmd.audioMod.c_str(), cmd.audioDev.c_str());
			if (err) {
				DEBUG_WARNING("failed to set audio source (%m)\n", err);
				break;
			}
        }
		break;
	}
	case Command::SWITCH_AUDIO_PLAYER: {
		if (app.callp)
		{
			struct audio* a = call_audio(app.callp);
			err = audio_set_player(a, cmd.audioMod.c_str(), cmd.audioDev.c_str());
			if (err) {
				DEBUG_WARNING("failed to set audio output (%m)\n", err);
				break;
			}
		}
		break;
	}
	case Command::UPDATE_SOFTVOL_TX: {
		struct config * cfg = conf_config();
		cfg->audio.softvol_tx = cmd.softvol;
		break;
	}
	case Command::UPDATE_SOFTVOL_RX: {
		struct config * cfg = conf_config();	
		cfg->audio.softvol_rx = cmd.softvol;
		break;
	}
	default:
		assert(!"Unhandled command type");
		break;
	}
}

static bool app_terminated = false;

__fastcall Worker::Worker(bool CreateSuspended)
	: TThread(CreateSuspended)
{
}
//---------------------------------------------------------------------------
void __fastcall Worker::Execute()
{
	int err;
    play_set_path((Paths::GetProfileDir() + "\\").c_str());
	do
	{
		appRestart = false;
		err = app_init();
		if (err) {
			DEBUG_WARNING("app_init failed (%m)\n", err);
   			UA_CB->ChangeAppState(Callback::APP_INIT_FAILED);
			goto out;
		}
		UA_CB->ChangeAppState(Callback::APP_INITIALIZED);		

		err = app_start();
		if (err) {
			DEBUG_WARNING("app_start failed (%m)\n", err);
			UA_CB->ChangeAppState(Callback::APP_START_FAILED);
			goto out;
		}

		/* Main loop */
		//while (!app.terminating) {
			err = re_main(signal_handler, control_handler);
		//}

	 out:
		app_close();
		list_flush(contact_list());		

		if (err) {
			DEBUG_WARNING("main exit with error (%m)\n", err);
			ua_stop_all(true);			
		} else {
			LOG("main exit w/o error, waiting for restart or termination\n");
		}
		while (!appRestart && !appQuit && !Terminated)
		{
			Sleep(10);
		}
	} while (appRestart);
	//return err;
	app_terminated = true;
}
//---------------------------------------------------------------------------

int Ua::Start(void)
{
	thread = new Worker(false);
	thread->FreeOnTerminate = true;
	return 0;
}

void Ua::Restart(void)
{
	/** \todo ugly forced hangup */
	if (app.callp)
	{
		UA_CB->ChangeCallState(Callback::CALL_STATE_CLOSED, "", "", 0, -1, "", "", -1, "", "", "");
		app.callp = NULL;
	}
	if (app.paging_txp)
	{
		paging_tx_hangup(app.paging_txp);
	}
	appRestart = true;
}

void Ua::Quit(void)
{
	if (appQuit)
	{
    	return;
	}
	if (app.paging_txp)
	{
		paging_tx_hangup(app.paging_txp);
	}
	appQuit = true;
	while (!app_terminated)
	{
		Sleep(10);
		Application->ProcessMessages();
	}
}

int Ua::GetAudioCodecList(std::vector<AnsiString> &codecs)
{
	if (audioCodecsAvailable == false)
	{
		return -1;
	}
	codecs = audioCodecs;
	return 0;
}



