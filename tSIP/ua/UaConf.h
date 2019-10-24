//---------------------------------------------------------------------------

#ifndef UaConfH
#define UaConfH
//---------------------------------------------------------------------------

#include <string>
#include <vector>
#include <list>
#include <assert.h>
#include <mem.h>

#include "AudioModules.h"

class UaConf
{
public:
	struct Account
	{
		enum TRANSPORT_TYPE
		{
			TRANSPORT_UDP = 0,
			TRANSPORT_TCP,
			TRANSPORT_LIMITER
		} transport;
		std::string reg_server;
		std::string user;
		std::string auth_user;
		std::string pwd;
		std::string cuser;
		int reg_expires;
		bool answer_any;
		int dtmf_tx_format;
		std::vector<std::string> audio_codecs;

		bool hide_reg_server;	// hide in settings and not store in settings file
		bool hide_user;
		bool hide_auth_user;
		bool hide_pwd;
		bool hide_cuser;

        enum {
			MIN_PTIME = 1,
			DEF_PTIME = 20,
			MAX_PTIME = 200
        };
		int ptime;

		std::string stun_server;
		std::string outbound1;
		std::string outbound2;

		bool operator==(const Account& right) const {
			return (reg_server == right.reg_server &&
				user == right.user &&
				auth_user == right.auth_user &&
				pwd == right.pwd &&
				cuser == right.cuser &&
				transport == right.transport &&
				reg_expires == right.reg_expires &&
				answer_any == right.answer_any &&
				dtmf_tx_format == right.dtmf_tx_format &&

				hide_reg_server == right.hide_reg_server &&
				hide_user == right.hide_user &&
				hide_auth_user == right.hide_auth_user &&
				hide_pwd == right.hide_pwd &&
				hide_cuser == right.hide_cuser &&

				audio_codecs == right.audio_codecs &&
				ptime == right.ptime &&
				stun_server == right.stun_server &&
				outbound1 == right.outbound1 &&
				outbound2 == right.outbound2
				);
		}
		bool operator!=(const Account& right) const {
			return !(*this == right);
		}
		Account():
			transport(TRANSPORT_UDP),
			reg_server("127.0.0.1:5060"),
			user("3001"),
			pwd("3001"),
			cuser(""),
			reg_expires(0),	// no registration in default configuration
			answer_any(false),
			dtmf_tx_format(0),

			hide_reg_server(false),
			hide_user(false),
			hide_auth_user(false),
			hide_pwd(false),
			hide_cuser(false),

			ptime(DEF_PTIME)
		{
			audio_codecs.push_back("PCMU/8000/1");
			audio_codecs.push_back("PCMA/8000/1");
		}
		const char* const getTransportStr(void) const {
			switch (transport)
			{
			case TRANSPORT_UDP:
				return "udp";
			case TRANSPORT_TCP:
				return "tcp";
			default:
				assert(!"Unhandled cfg transport type");
				return "???";
			}
		}
		bool isAnySettingHidden(void) const {
			return
				hide_reg_server ||
				hide_user ||
				hide_auth_user ||
				hide_pwd ||
				hide_cuser;
		}
	};
	std::vector<Account> accounts;

	enum { CONTACTS_CNT = 10 };
	struct Contact {
		std::string description;
		std::string user;
		bool sub_dialog_info;
		bool sub_presence;
		std::list<int> btnIds;
		int dialog_info_state;	///< not actual configuration - current state
		bool operator==(const Contact& right) const {
			return (
				description == right.description &&
				user == right.user &&
				sub_dialog_info == right.sub_dialog_info &&
				sub_presence == right.sub_presence
				);
		}
		bool operator!=(const Contact& right) const {
			return !(*this == right);
		}
		Contact():
			sub_dialog_info(false),
			sub_presence(false),
			dialog_info_state(-1)
		{
		}
	};
	std::vector<Contact> contacts;

	bool logMessages;

	struct AudioCfg {
		enum { MAX_MOD_LENGTH = 32 };
		char mod[MAX_MOD_LENGTH];       /**< Audio source module            */
		char dev[128];      /**< Audio source device            */
		char wavefile[512];
		AudioCfg(void) {
			//mod[0] = '\0';
			strcpy(mod, AudioModules::winwave);
			dev[0] = '\0';
		}
		bool operator==(const AudioCfg& right) const {
			if (strcmp(mod, right.mod))
				return false;
			if (strcmp(dev, right.dev))
				return false;
			if (strcmp(wavefile, right.wavefile))
				return false;
			return true;
		}
		bool operator!=(const AudioCfg& right) const {
			return !(*this == right);
		}
	} audioCfgSrc, audioCfgPlay, audioCfgAlert, audioCfgRing, audioCfgPlayIntercom;

	struct AudioPortaudio {
		double inSuggestedLatency;
		double outSuggestedLatency;
		AudioPortaudio(void):
			inSuggestedLatency(0.1),
			outSuggestedLatency(0.1)
		{}
		bool operator==(const AudioPortaudio& right) const;
		bool operator!=(const AudioPortaudio& right) const {
        	return !(*this == right);
		}
	} audioPortaudio;

	struct AudioSoftVol {
		unsigned int tx;
		unsigned int rx;
		AudioSoftVol(void):
			tx(128),
			rx(128)
		{}
	} audioSoftVol;

	struct AudioAgc {
		bool enabled;		/**< on/off switch */
		unsigned int target;/**< amplitude that AGC should regulate to */
		double maxGain;		/**< maximum allowed gain */
		double attackRate;	/**<  */
		double releaseRate;
		AudioAgc (void):
			enabled(false),
			target(20000),	/**< of 32767 */
			maxGain(6.0),
			attackRate(0.01),
			releaseRate(0.01)
		{}
		bool operator==(const AudioAgc& right) const;
		bool operator!=(const AudioAgc& right) const {
        	return !(*this == right);
		}
	} audioAgcRx;

	struct RecordingCfg {
		bool enabled;
		enum RecDir {
			RecDirRelative = 0,
			RecDirCustom,
			
			RecDirLimiter
		} recDir;
		std::string customRecDir;
		enum { CHANNELS_MIN = 1, CHANNELS_MAX = 2 };
		unsigned int channels;
		unsigned int side;				///< which party to record in single channel mode, \sa baresip.h: enum recorder_side; 0 = both parties mixed
		enum RecStart {
			RecStartManual = 0,			///< start recording manually, e.g. using Lua API
			RecStartCallConfirmed,		///< recording starts on call confirmed
			RecStartCallEarly,			///< recording starts on call early media (SIP/183)

			RecStartLimiter
		} recStart;
		RecordingCfg(void):
			enabled(false),
			recDir(RecDirRelative),
			channels(1),
			side(0),
			recStart(RecStartCallConfirmed)
		{}
		bool operator==(const RecordingCfg& right) const {
			return enabled == right.enabled &&
				recDir == right.recDir &&
				customRecDir == right.customRecDir &&
				channels == right.channels &&
				side == right.side &&
				recStart == right.recStart
				;
		}
		bool operator!=(const RecordingCfg& right) const {
			return !(*this == right);
		}
	} recording;

	struct AudioPreproc {
		bool enabled;
		bool denoiseEnabled;
		bool agcEnabled;
		bool vadEnabled;
		bool dereverbEnabled;
		int agcLevel;
		enum { AGC_LEVEL_DEFAULT = 8000 };
		AudioPreproc(void):
			enabled(false),
			denoiseEnabled(true),
			agcEnabled(false),
			vadEnabled(false),
			dereverbEnabled(false),
			agcLevel(AGC_LEVEL_DEFAULT)
		{
		}
		bool operator==(const AudioPreproc& right) const {
			return enabled == right.enabled &&
				denoiseEnabled == right.denoiseEnabled &&
				agcEnabled == right.agcEnabled &&
				vadEnabled == right.vadEnabled &&
				dereverbEnabled == right.dereverbEnabled &&
				agcLevel == right.agcLevel
				;
		}
		bool operator!=(const AudioPreproc& right) const {
			return !(*this == right);
		}
	} audioPreprocTx;

	enum Aec {
		AEC_NONE = 0,
		AEC_SPEEX,
		AEC_WEBRTC,
		AEC_LIMIT
	} aec;

	struct WebrtcAec {
		int msInSndCardBuf;
		int skew;
		bool operator==(const UaConf::WebrtcAec& right) const {
			if (msInSndCardBuf == right.msInSndCardBuf &&
				skew == right.skew)
				return true;
			return false;
		}
		bool operator!=(const UaConf::WebrtcAec& right) const {
			return !(*this == right);
		}
		WebrtcAec(void):
			msInSndCardBuf(120),
			skew(0)
		{
		}
	} webrtcAec;

	std::string local;
	std::string ifname;	///< baresip config_net.ifname

	struct Avt {
		unsigned int portMin;
		unsigned int portMax;
		unsigned int jbufDelayMin;
		unsigned int jbufDelayMax;
		unsigned int rtpTimeout;
		enum { DEF_PORT_MIN = 1024 };
		enum { DEF_PORT_MAX = 49152 };
		enum { DEF_JBUF_DELAY_MIN = 5 };
		enum { DEF_JBUF_DELAY_MAX = 10 };
		enum { JBUF_DELAY_CFG_LIMIT = 1000 };
		enum { DEF_RTP_TIMEOUT = 0 };
		Avt(void):
			portMin(DEF_PORT_MIN),
			portMax(DEF_PORT_MAX),
			jbufDelayMin(DEF_JBUF_DELAY_MIN),
			jbufDelayMax(DEF_JBUF_DELAY_MAX),
			rtpTimeout(DEF_RTP_TIMEOUT)
		{
		}
		int ValidatePorts(void) {
			if (portMax - portMin < 2 || portMin == 0 || portMax > 65535) {
				portMin = DEF_PORT_MIN;
				portMax = DEF_PORT_MAX;
				return -1;
			}
			return 0;
		}
		int ValidateJbufDelay(void) {
			if (jbufDelayMin > JBUF_DELAY_CFG_LIMIT || jbufDelayMax > JBUF_DELAY_CFG_LIMIT ||
				jbufDelayMin >= jbufDelayMax) {
				jbufDelayMin = DEF_JBUF_DELAY_MIN;
				jbufDelayMax = DEF_JBUF_DELAY_MAX;
				return -1;
			}
			return 0;
		}
		int ValidateRtpTimeout(void) {
			if (rtpTimeout > 36000) {
				rtpTimeout = DEF_RTP_TIMEOUT;
				return -1;
			}
			return 0;
        }
		int Validate(void) {
			int ret = 0;
			ret += ValidatePorts();
			ret += ValidateJbufDelay();
			ret += ValidateRtpTimeout();
			return ret;
		}
		bool operator==(const UaConf::Avt& right) const {
			if (portMin == right.portMin &&
				portMax == right.portMax &&
				jbufDelayMin == right.jbufDelayMin &&
				jbufDelayMax == right.jbufDelayMax &&
				rtpTimeout == right.rtpTimeout
				)
			{
				return true;
			}
			return false;
		}
		bool operator!=(const UaConf::Avt& right) const {
			return !(*this == right);
		}
	} avt;	// audio/video transport

	bool autoAnswer;
	int autoAnswerCode;
	unsigned int autoAnswerDelayMin;
	unsigned int autoAnswerDelayMax;

	bool autoAnswerCallInfo;	///< auto-answer when Call-Info with answer-after is present in incoming INVITE
	unsigned int autoAnswerCallInfoDelayMin;	///< minimum delay when auto answering intercom/paging calls [ms]

	bool answerOnEventTalk;		///< answer when NOTIFY with "Event: talk" is received (CRM integration through asterisk AMI)

	bool handleOodRefer;		///< handle incoming out-of-dialog refer

	bool customUserAgent;
	std::string userAgent;

	UaConf() {
		aec = AEC_NONE;
		logMessages = false;
		autoAnswer = false;
		autoAnswerCode = 200;
		autoAnswerDelayMin = 0;
		autoAnswerDelayMax = 0;
		autoAnswerCallInfo = false;
		autoAnswerCallInfoDelayMin = 0;
		answerOnEventTalk = false;
		handleOodRefer = false;
		customUserAgent = false;
		contacts.resize(CONTACTS_CNT);
	}

	bool operator==(const UaConf& right) const {
		if (accounts.size() != right.accounts.size())
			return false;
		for (int i=0; i<accounts.size(); i++)
		{
			if (accounts[i] != right.accounts[i])
			{
				return false;
			}
		}
		if (contacts.size() != right.contacts.size())
			return false;
		for (int i=0; i<contacts.size(); i++)
		{
			if (contacts[i] != right.contacts[i])
			{
				return false;
			}
		}
	#if 1
		if (audioCfgSrc != right.audioCfgSrc)
			return false;
		if (audioCfgPlay != right.audioCfgPlay)
			return false;
		if (audioCfgAlert != right.audioCfgAlert)
			return false;
		if (audioCfgRing != right.audioCfgRing)
			return false;
	#endif
		if (recording != right.recording)
			return false;
		if (audioAgcRx != right.audioAgcRx)
			return false;
		if (audioPortaudio != right.audioPortaudio)
			return false;
		if (audioPreprocTx != right.audioPreprocTx)
			return false;
		if (aec != right.aec)
			return false;
		if (local != right.local)
			return false;
		if (ifname != right.ifname)
			return false;
		if (avt != right.avt)
			return false;
		if (customUserAgent != right.customUserAgent)
			return false;
		if (customUserAgent == true && (userAgent != right.userAgent))
			return false;
		return true;
	}
	bool operator!=(const UaConf& right) const {
		return !(*this == right);
	}
};

#endif
