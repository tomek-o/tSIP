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
#include "NetInterfaces.h"
#include "Paths.h"
#include <re.h>
#include <rem.h>
#include "baresip.h"
#include "Log.h"
#include "common/Utilities.h"
#include "common/StaticCheck.h"
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

	std::vector<AnsiString> videoCodecs;
	volatile bool videoCodecsAvailable = false;

	/** Escaping text to be added as display name
		Rules:
		\ -> \\
		" -> \"
	*/
	AnsiString EscapeDisplayName(AnsiString dn)
	{
		AnsiString ret;
		for (int i=1; i<=dn.Length(); i++)
		{
			char c = dn[i];
			if (c == '\\')
			{
				ret += '\\';
			}
			else if (c == '"')
			{
				ret += '\\';
			}
			ret += c;
		}
		return ret;
	}
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
				ua_hangup(ua_cur(), call, 486, "Busy Here");
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

			if (appSettings.uaConf.startAudioSourceAtCallStart)
			{
				call_start_audio_extra_source(call);
			}

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
		if (appSettings.uaConf.startAudioSourceAtCallStart)
		{
			call_start_audio_extra_source(call);
		}
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

static void custom_req_response_handler(int err, const struct sip_msg *msg, void *arg)
{
	int requestUid = reinterpret_cast<int>(arg);
	(void)arg;
	AnsiString replyText;

	if (err) {
		DEBUG_WARNING("custom request uid %d reply error: %m\n", requestUid, err);
	} else {
		if (msg->scode < 200) {
			DEBUG_WARNING("Reply for custom request uid %d: scode = %d\n", requestUid, msg->scode);
		} else 	if (msg->scode < 300) {
		#if 0		
			mbuf_set_pos(msg->mb, 0);
			DEBUG_WARNING("----- Reply for custom request uid %d to %r -----\n%b",
				 requestUid,
				 &(msg->to.auri), mbuf_buf(msg->mb),
				 mbuf_get_left(msg->mb));
		#else
			DEBUG_WARNING("Reply for custom request uid %d: scode = %d\n", requestUid, msg->scode);
		#endif
		} else {
			DEBUG_WARNING("%r: custom request %d failed: %u %r\n", &(msg->to.auri), requestUid,
				msg->scode, &msg->reason);
		}
		// copy whole reply text
		mbuf_set_pos(msg->mb, 0);
		int length = mbuf_get_left(msg->mb);
		char *ptr = (char*)mbuf_buf(msg->mb);
		replyText.SetLength(length);
		memcpy(&replyText[1], ptr, length);
	}

	UA_CB->NotifyCustomRequestStatus(requestUid, err, msg?msg->scode:0, replyText);	
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

extern "C" int dialog_info_handler(int id, const struct dialog_data *ddata, unsigned int ddata_cnt)
{
	UA_CB->ChangeDlgInfoState(id, ddata, ddata_cnt);
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

static void simple_message_recv_handler(const struct pl *peer,
	const struct pl *ctype, struct mbuf *body, void *arg,
    int *reply_code, const char** reply_reason, int *do_not_reply)
{
	LOG("message received\n");
	AnsiString caller, contentType, asBody;
	caller.SetLength(peer->l);
	memcpy(&caller[1], peer->p, peer->l);
	contentType.SetLength(ctype->l);
	memcpy(&contentType[1], ctype->p, ctype->l);

	//mbuf_set_pos(body, 0);
	int length = mbuf_get_left(body);
	char *ptr = (char*)mbuf_buf(body);
	asBody.SetLength(length);
	memcpy(&asBody[1], ptr, length);

	struct config * cfg = conf_config();
	*reply_code = cfg->messages.reply_code;
	*reply_reason = cfg->messages.reply_reason;
	*do_not_reply = cfg->messages.do_not_reply;

	UA_CB->OnMessageReceived(caller, contentType, asBody);
}

static void simple_message_response_handler(int err, const struct sip_msg *msg, void *resp_callback_arg)
{
	int requestId = reinterpret_cast<int>(resp_callback_arg);
#if 0
	if (err) {
		DEBUG_WARNING("MESSAGE %d response handler: error = %d\n", requestId, err);
	} else {
		if (msg->scode >= 300) {
			DEBUG_WARNING("MESSAGE %d response: code %u, reason: %r\n", requestId, msg->scode, &msg->reason);
		}
	}
#endif
	AnsiString reason;
	if (msg && (msg->reason.l > 0))
	{
		//reason.sprintf("%.*s", msg->reason.l, msg->reason.p);
		reason.SetLength(msg->reason.l + 1);
		memcpy(&reason[1], msg->reason.p, msg->reason.l);
		reason[msg->reason.l + 1] = '\0';
	}
	UA_CB->OnMessageStatus(requestId, err, msg?msg->scode:0, reason);
}

static void recorder_state_handler(struct recorder_st *recorder, enum recorder_state state) {
	DEBUG_WARNING("recorder %p state changed to %d\n", recorder, static_cast<int>(state));
	STATIC_CHECK(RECORDER_STATE_IDLE == Callback::RECORDER_STATE_IDLE, EnumMismatch);
	STATIC_CHECK(RECORDER_STATE_ACTIVE == Callback::RECORDER_STATE_ACTIVE, EnumMismatch);
	STATIC_CHECK(RECORDER_STATE_PAUSED == Callback::RECORDER_STATE_PAUSED, EnumMismatch);
	UA_CB->ChangeRecorderState(0, static_cast<Callback::rec_state_e>(state));	
}

static void zrtp_state_handler(int session_id, struct zrtp_st *st)
{
	//DEBUG_WARNING("ZRTP state changed: session_id %d, active = %d, sas [%s], cipher [%s], verified = %d\n",
	//	session_id, static_cast<int>(st->active), st->sas, st->cipher, static_cast<int>(st->verified));
	Callback::Zrtp zrtp;
	zrtp.sessionId = session_id;
	zrtp.active = st->active;
	zrtp.sas = st->sas;
	zrtp.cipher = st->cipher;
	zrtp.verified = st->verified;
	UA_CB->ChangeEncryptionState(zrtp);
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

	strncpyz(cfg->audio.src_mod, appSettings.uaConf.audioCfgSrc.mod.c_str(), sizeof(cfg->audio.src_mod));
	if (appSettings.uaConf.audioCfgSrc.mod != AudioModules::aufile &&
		appSettings.uaConf.audioCfgSrc.mod != AudioModules::aufileMm)
	{
		strncpyz(cfg->audio.src_dev, appSettings.uaConf.audioCfgSrc.dev.c_str(), sizeof(cfg->audio.src_dev));
	}
	else
	{
        // aufile / aufile_mm
		AnsiString fileFull;
		fileFull.sprintf("%s\\%s", Paths::GetProfileDir().c_str(), appSettings.uaConf.audioCfgSrc.wavefile.c_str());
		strncpyz(cfg->audio.src_dev, fileFull.c_str(), sizeof(cfg->audio.src_dev));
	}
	strncpyz(cfg->audio.play_mod, appSettings.uaConf.audioCfgPlay.mod.c_str(), sizeof(cfg->audio.play_mod));
	strncpyz(cfg->audio.play_dev, appSettings.uaConf.audioCfgPlay.dev.c_str(), sizeof(cfg->audio.play_dev));
	strncpyz(cfg->audio.alert_mod, appSettings.uaConf.audioCfgAlert.mod.c_str(), sizeof(cfg->audio.alert_mod));
	strncpyz(cfg->audio.alert_dev, appSettings.uaConf.audioCfgAlert.dev.c_str(), sizeof(cfg->audio.alert_dev));
	strncpyz(cfg->audio.ring_mod, appSettings.uaConf.audioCfgRing.mod.c_str(), sizeof(cfg->audio.ring_mod));
	strncpyz(cfg->audio.ring_dev, appSettings.uaConf.audioCfgRing.dev.c_str(), sizeof(cfg->audio.ring_dev));

	cfg->aec = (config::e_aec)appSettings.uaConf.aec;

	cfg->webrtc.msInSndCardBuf = appSettings.uaConf.webrtcAec.msInSndCardBuf;
	cfg->webrtc.skew = appSettings.uaConf.webrtcAec.skew;

	{
		const UaConf::Opus &opus = appSettings.uaConf.opus;
		cfg->opus.stereo = opus.stereo;
		cfg->opus.sprop_stereo = opus.spropStereo;
		cfg->opus.set_bitrate = opus.setBitrate;
		cfg->opus.bitrate = opus.bitrate;
		cfg->opus.set_samplerate = opus.samplerate;
		cfg->opus.set_cbr = opus.setCbr;
		cfg->opus.cbr = opus.cbr;
		cfg->opus.set_inband_fec = opus.setInbandFec;
		cfg->opus.inband_fec = opus.inbandFec;
		cfg->opus.set_dtx = opus.setDtx;
		cfg->opus.dtx = opus.dtx;
		cfg->opus.mirror = opus.mirror;
		cfg->opus.complexity = opus.complexity;
		cfg->opus.set_application = opus.setApplication;
		cfg->opus.application = static_cast<e_opus_application>(opus.application);
		cfg->opus.set_packet_loss = opus.setPacketLoss;
		cfg->opus.packet_loss = opus.packetLoss;
	}

	{
		const UaConf::Zrtp &zrtp = appSettings.uaConf.zrtp;
		cfg->zrtp.start_parallel = zrtp.startParallel;
		strncpyz(cfg->zrtp.zid_filename, (Paths::GetProfileDir() + "\\zrtp.zid").c_str(), sizeof(cfg->zrtp.zid_filename));
	}

	{
		const UaConf::Tls &tls = appSettings.uaConf.tls;
		if (tls.certificate != "")
		{
			strncpyz(cfg->sip.cert, (Paths::GetProfileDir() + "\\certificates\\" + tls.certificate.c_str()).c_str(), sizeof(cfg->sip.cert));
		}
		if (tls.caFile != "")
		{
			strncpyz(cfg->sip.cafile, (Paths::GetProfileDir() + "\\certificates\\" + tls.caFile.c_str()).c_str(), sizeof(cfg->sip.cafile));
		}
		cfg->sip.use_windows_root_ca_store = tls.useWindowsRootCaStore;
		cfg->sip.verify_server = tls.verifyServer;
	}

	strncpyz(cfg->sip.local, appSettings.uaConf.local.c_str(), sizeof(cfg->sip.local));
	if (appSettings.uaConf.ifname.size() > 0) {
		strncpyz(cfg->net.ifname, appSettings.uaConf.ifname.c_str(), sizeof(cfg->net.ifname));
	} else {
		// try to avoid VirtualBox interface
		cfg->net.ifname[0] = '\0';
		std::vector<NetInterface> interfaces;
		GetNetInterfaces(interfaces);
		if (!interfaces.empty()) {
			for (unsigned int i=0; i<interfaces.size(); i++) {
				const NetInterface &netIf = interfaces[i];
				if (netIf.ip == "0.0.0.0" || strncmp(netIf.ip.c_str(), "169.154.", strlen("169.154.")) == 0) {
					continue;
				}
			#if 0
				if (netIf.ip == "192.168.56.1") {	// VirtualBox default IP
					continue;
				}
			#endif
				AnsiString driverName = GetNetAdapterDriverName(netIf.name);
				if (driverName.Pos("VirtualBox") > 0 ||
					driverName.Pos("VMware") > 0 ||
					driverName.Pos("Hyper-V") > 0 ||
					driverName.Pos("Virtual Ethernet") > 0) {
					continue;
				}
				DEBUG_WARNING("No adapter specified in config, network adapter [%s] seems to be valid to bind\n", driverName.c_str());
				strncpyz(cfg->net.ifname, netIf.name.c_str(), sizeof(cfg->net.ifname));
				break;
			}
		}
	}

	net_debug(&pf_log, NULL);

	cfg->avt.rtp_ports.min = appSettings.uaConf.avt.portMin;
	cfg->avt.rtp_ports.max = appSettings.uaConf.avt.portMax;
	cfg->avt.jbuf_del.min = appSettings.uaConf.avt.jbufDelayMin;
	cfg->avt.jbuf_del.max = appSettings.uaConf.avt.jbufDelayMax;
	cfg->avt.rtp_timeout = appSettings.uaConf.avt.rtpTimeout;

	cfg->messages.reply_code = appSettings.uaConf.messages.replyCode;
	strncpyz(cfg->messages.reply_reason, appSettings.uaConf.messages.replyReason.c_str(), sizeof(cfg->messages.reply_reason));
	cfg->messages.do_not_reply = appSettings.uaConf.messages.doNotReply;

	cfg->recording.enabled = appSettings.uaConf.recording.enabled;

	cfg->audio_preproc_tx.enabled = appSettings.uaConf.audioPreprocTx.enabled;
	cfg->audio_preproc_tx.denoise_enabled = appSettings.uaConf.audioPreprocTx.denoiseEnabled;
	cfg->audio_preproc_tx.agc_enabled = appSettings.uaConf.audioPreprocTx.agcEnabled;
	cfg->audio_preproc_tx.vad_enabled = appSettings.uaConf.audioPreprocTx.vadEnabled;
	cfg->audio_preproc_tx.dereverb_enabled = appSettings.uaConf.audioPreprocTx.dereverbEnabled;
	cfg->audio_preproc_tx.agc_level = appSettings.uaConf.audioPreprocTx.agcLevel;

	cfg->audio.agc_rx.enabled = appSettings.uaConf.audioAgcRx.enabled;
	cfg->audio.agc_rx.target = appSettings.uaConf.audioAgcRx.target;
	cfg->audio.agc_rx.max_gain = appSettings.uaConf.audioAgcRx.maxGain;
	cfg->audio.agc_rx.attack_rate = appSettings.uaConf.audioAgcRx.attackRate;
	cfg->audio.agc_rx.release_rate = appSettings.uaConf.audioAgcRx.releaseRate;

	cfg->audio.gate_tx.enabled = appSettings.uaConf.audioGateTx.enabled;
	cfg->audio.gate_tx.close_threshold = appSettings.uaConf.audioGateTx.closeThreshold;
	cfg->audio.gate_tx.hold_ms = appSettings.uaConf.audioGateTx.holdMs;
	cfg->audio.gate_tx.attack_ms = appSettings.uaConf.audioGateTx.attackMs;
	cfg->audio.gate_tx.release_ms = appSettings.uaConf.audioGateTx.releaseMs;

	cfg->audio.portaudioInSuggestedLatency = appSettings.uaConf.audioPortaudio.inSuggestedLatency;
	cfg->audio.portaudioOutSuggestedLatency = appSettings.uaConf.audioPortaudio.outSuggestedLatency;

	cfg->audio.loop_ring_without_silence = appSettings.uaConf.loopRingWithoutSilence;
#ifdef USE_VIDEO
	strncpyz(cfg->video.src_mod, appSettings.uaConf.video.videoSource.mod.c_str(), sizeof(cfg->video.src_mod));
	strncpyz(cfg->video.src_dev, appSettings.uaConf.video.videoSource.dev.c_str(), sizeof(cfg->video.src_dev));
	cfg->video.width = appSettings.uaConf.video.width;
	cfg->video.height = appSettings.uaConf.video.height;
	cfg->video.bitrate = appSettings.uaConf.video.bitrate;
	cfg->video.fps = appSettings.uaConf.video.fps;
	cfg->video.selfview.enabled = appSettings.uaConf.video.selfview.enabled;
	cfg->video.selfview.pip = appSettings.uaConf.video.selfview.pip;
	cfg->video.dshow.skip_reading_back_media_format = appSettings.uaConf.video.dshow.skipReadingBackMediaFormat;
#endif
	configure();

	/* Initialise User Agents */
	AnsiString uaName;
	if (appSettings.uaConf.customUserAgent == false) {
		uaName.sprintf("%s %s", Branding::appName.c_str(), GetFileVer(Application->ExeName).c_str());
	} else {
		uaName = appSettings.uaConf.userAgent.c_str();
    }

	err = ua_init(uaName.c_str(), true, true, true, false);
	if (err)
		return err;

	ua_log_messages(appSettings.uaConf.logMessages, appSettings.uaConf.logMessagesOnlyFirstLine);

	aubuf_debug_enable(appSettings.uaConf.logAubuf);

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

	err = message_init(simple_message_recv_handler, simple_message_response_handler, NULL);
	if (err != 0) {
    	LOG("Failed to register handler for MESSAGE RX (err = %d)!\n", err);
	}

	recorder_init(recorder_state_handler);

	baresip_zrtp_init(zrtp_state_handler);

	return 0;
}


static int app_start(void)
{
	int n;
	struct pl modname;

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

	app.n_uas = 0;
	for (int i=0; i<appSettings.uaConf.accounts.size(); i++)
	{
		AnsiString addr;
		UaConf::Account &acc = appSettings.uaConf.accounts[i];

		if (acc.display_name != "")
		{
			AnsiString escapedDisplayName = EscapeDisplayName(acc.display_name.c_str());
			addr.cat_printf("\"%s\" ", escapedDisplayName.c_str());
		}

		addr.cat_printf("<sip:%s@%s;transport=%s>;regint=%d", //;sipnat=outbound",
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

		if (acc.mediaenc != "")
		{
        	addr.cat_printf(";mediaenc=%s", acc.mediaenc.c_str());
		}

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

		{
			addr.cat_printf(";video_codecs=");
			for (unsigned int i=0; i<acc.video_codecs.size(); i++)
			{
				addr.cat_printf("%s", acc.video_codecs[i].c_str());
				if (i < acc.video_codecs.size() - 1)
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
		else
		{
			DEBUG_WARNING("Failed to add account #%d\n", i); 
		}
	}
	if (!app.n_uas)
	{
		DEBUG_WARNING("No valid SIP account found - check your config\n");
		return ENOENT;
	}

	if (appSettings.uaConf.accounts.size() != 1)
	{
		DEBUG_WARNING("Multiple accounts not handled!\n");
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
				addr.cat_printf(";dlginfo=p2p;dlginfo_expires=%d", contact.sub_dialog_info_expires);
			}
			if (contact.sub_presence)
			{
				addr.cat_printf(";presence=p2p;presence_expires=%d", contact.sub_presence_expires);
			}
			pl pl_addr;
			pl_set_str(&pl_addr, addr.c_str());
			contact_add(NULL, &pl_addr, i, dialog_info_handler, presence_handler);
		}
	}

	// contact list must be initialized here
	pl_set_str(&modname, "dialog-info");
	load_module2(NULL, &modname);

	// contact list must be initialized here
	pl_set_str(&modname, "presence");
	load_module2(NULL, &modname);

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

	if (videoCodecsAvailable == false) {
		struct le *le;
		for (le = vidcodec_list()->head; le; le=le->next) {
			struct vidcodec *vc = (struct vidcodec*)le->data;
			AnsiString tmp;
			if (vc->variant) {
				tmp.sprintf("%s/%s", vc->name, vc->variant);
			} else {
            	tmp = vc->name;
			}
			videoCodecs.push_back(tmp);
		}
		videoCodecsAvailable = true;
	}

	n = list_count(vidfilt_list());
	LOG("Populated %u video filter%s\n", n, 1==n?"":"s");

	return 0;
}


static void app_close(void)
{
    message_close();
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
			cmd.target.c_str(), NULL,
			cmd.video?VIDMODE_ON:VIDMODE_OFF, cmd.vidispParentHandle,
			cmd.extraHeaderLines.c_str());
		if (err)
		{
			DEBUG_WARNING("connect failed: %m\n", err);
			UA_CB->ChangeCallState(Callback::CALL_STATE_CLOSED, "", "", 0, -1, "", "", -1, "", "", "");
		}
		break;
	case Command::ANSWER:
        LOG("Answering...\n");
		err = ua_answer(ua_cur(), app.callp, cmd.audioMod.c_str(), cmd.audioDev.c_str(), cmd.video?VIDMODE_ON:VIDMODE_OFF, cmd.vidispParentHandle);
		if (err)
		{
        	DEBUG_WARNING("ua_answer failed: %m\n", err);
		}
		else
		{
        	DEBUG_WARNING("Answered\n");
		}
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
			ua_hangup(ua_cur(), app.callp, cmd.code, cmd.reason.c_str());
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
		ua_log_messages(cmd.bEnabled, cmd.bParam);
		break;
	case Command::SET_AUBUF_LOGGING:
		aubuf_debug_enable(cmd.bEnabled);
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
		(void)ua_play_file(ua, cfg->audio.ring_mod, cfg->audio.ring_dev, cmd.target.c_str(), -1, cfg->audio.loop_ring_without_silence);
		break;
	}
	case Command::PLAY_STOP: {
		struct ua* ua = ua_cur();
		//LOG("UaMain: PLAY_STOP\n");
		ua_play_stop(ua);
		break;
	}
	case Command::START_RING2: {
		struct ua* ua = ua_cur();
		struct config * cfg = conf_config();
		//LOG("UaMain: START_RING2\n");
		(void)ua_play_file2(ua, cfg->audio.ring_mod, cfg->audio.ring_dev, cmd.target.c_str());
		break;
	}
	case Command::RECORD: {
		recorder_start(cmd.target.c_str(), cmd.channels,
			static_cast<enum recorder_side>(cmd.recSide),\
			static_cast<enum recorder_file_format>(cmd.recFileFormat),
			cmd.bitrate
		);
		break;
	}
	case Command::RECORD_PAUSE: {
        recorder_pause();
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
	case Command::SEND_CUSTOM_REQUEST: {
		DEBUG_WARNING("SEND_CUSTOM_REQUEST, uid = %d, method = %s\n", cmd.requestId, cmd.method.c_str());
		err = sip_req_send(ua_cur(), cmd.method.c_str(), cmd.target.c_str(),
					custom_req_response_handler, reinterpret_cast<void*>(cmd.requestId),
					"%s", cmd.extraHeaderLines.c_str());
		if (err != 0) {
			DEBUG_WARNING("Failed send custom request (%m)\n", err);
			UA_CB->NotifyCustomRequestStatus(cmd.requestId, err, 0, "");
		}
		break;
	}
	case Command::SEND_MESSAGE: {
		err = message_send(ua_cur(), cmd.target.c_str(), cmd.text.c_str(), (void*)cmd.requestId);
		DEBUG_WARNING("Sending message to %s: status = %d\n", cmd.target.c_str(), err);
		break;
	}
	case Command::GENERATE_TONES: {
		if (app.callp) {
			for (unsigned int i=0; i<ARRAY_SIZE(cmd.tones); i++) {
				const struct Command::Tone &tone = cmd.tones[i];
				if (tone.amplitude > 0.000001f)
				{
					call_start_tone(app.callp, i, tone.amplitude, tone.frequency);
				}
				else
				{
					call_stop_tone(app.callp, i);
				}
			}
		}
		break;
	}
	case Command::ZRTP_VERIFY_SAS: {
    	baresip_zrtp_verify_sas(1, cmd.bParam);
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

		err = app_start();
		if (err) {
			DEBUG_WARNING("app_start failed (%m)\n", err);
			UA_CB->ChangeAppState(Callback::APP_START_FAILED);
			goto out;
		}

		UA_CB->ChangeAppState(Callback::APP_STARTED);

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

unsigned int Ua::GetAudioRxSignalLevel(void)
{
	return softvol_get_rx_level();
}

int Ua::GetVideoCodecList(std::vector<AnsiString> &codecs)
{
	if (videoCodecsAvailable == false)
	{
		return -1;
	}
	codecs = videoCodecs;
	return 0;
}




