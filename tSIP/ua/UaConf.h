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
#include "VideoModules.h"
#include "baresip_dialog_info_direction.h"

#include <stdint.h>

namespace Json
{
	class Value;
}

struct SettingsAppVersion;

class UaConf
{
public:
	bool disableUa;

	struct Account
	{
		enum TRANSPORT_TYPE
		{
			TRANSPORT_UDP = 0,
			TRANSPORT_TCP,
			TRANSPORT_TLS,
			TRANSPORT_LIMITER
		} transport;
		std::string reg_server;
		std::string user;
		std::string display_name;
		std::string auth_user;
		std::string pwd;
		std::string cuser;
		int reg_expires;
		bool answer_any;
		int dtmf_tx_format;
		std::vector<std::string> audio_codecs;
		std::vector<std::string> video_codecs;

		bool hide_reg_server;	// hide in settings and not store in settings file
		bool hide_user;
		bool hide_display_name;
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

		std::string mediaenc;

		bool operator==(const Account& right) const {
			return (reg_server == right.reg_server &&
				user == right.user &&
				display_name == right.display_name &&
				auth_user == right.auth_user &&
				pwd == right.pwd &&
				cuser == right.cuser &&
				transport == right.transport &&
				reg_expires == right.reg_expires &&
				answer_any == right.answer_any &&
				dtmf_tx_format == right.dtmf_tx_format &&

				hide_reg_server == right.hide_reg_server &&
				hide_user == right.hide_user &&
				hide_display_name == right.hide_display_name &&
				hide_auth_user == right.hide_auth_user &&
				hide_pwd == right.hide_pwd &&
				hide_cuser == right.hide_cuser &&

				audio_codecs == right.audio_codecs &&
				video_codecs == right.video_codecs &&
				ptime == right.ptime &&
				stun_server == right.stun_server &&
				outbound1 == right.outbound1 &&
				outbound2 == right.outbound2 &&

				mediaenc == right.mediaenc
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
			hide_display_name(false),
			hide_auth_user(false),
			hide_pwd(false),
			hide_cuser(false),

			ptime(DEF_PTIME),

			mediaenc("")
		{
			audio_codecs.push_back("PCMU/8000/1");
			audio_codecs.push_back("PCMA/8000/1");

			video_codecs.push_back("H264/packetization-mode=0");
			video_codecs.push_back("H264/packetization-mode=1");
			//video_codecs.push_back("H263");
			video_codecs.push_back("MP4V-ES");
		}

		const char* const getTransportStr(void) const {
			switch (transport)
			{
			case TRANSPORT_UDP:
				return "udp";
			case TRANSPORT_TCP:
				return "tcp";
			case TRANSPORT_TLS:
				return "tls";
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
		int sub_dialog_info_expires;
		bool sub_presence;
		int sub_presence_expires;
		std::list<int> btnIds;

		/// not actual configuration - current state
		int dialog_info_state;
		enum dialog_info_direction direction;
		std::string remoteIdentity;
		std::string remoteIdentityDisplay;

		bool operator==(const Contact& right) const {
			return (
				description == right.description &&
				user == right.user &&
				sub_dialog_info == right.sub_dialog_info &&
				sub_dialog_info_expires == right.sub_dialog_info_expires &&
				sub_presence == right.sub_presence &&
				sub_presence_expires == right.sub_presence_expires
				);
		}
		bool operator!=(const Contact& right) const {
			return !(*this == right);
		}
		Contact():
			sub_dialog_info(false),
			sub_dialog_info_expires(600),
			sub_presence(false),
			sub_presence_expires(600),
			dialog_info_state(-1),
			direction(DIALOG_INFO_DIR_UNKNOWN)
		{
		}
	};
	std::vector<Contact> contacts;

	bool logMessages;
	bool logMessagesOnlyFirstLine;
	bool logAubuf;

	struct AudioCfg {
		std::string mod;       	/**< Audio source module            */
		std::string dev;		/**< Audio source device            */
		std::string wavefile;
		AudioCfg(void) {
			mod = AudioModules::winwave;
		}
		bool operator==(const AudioCfg& right) const {
			if (mod != right.mod)
				return false;
			if (dev != right.dev)
				return false;
			if (wavefile != right.wavefile)
				return false;
			return true;
		}
		bool operator!=(const AudioCfg& right) const {
			return !(*this == right);
		}
	} audioCfgSrc, audioCfgPlay, audioCfgAlert, audioCfgRing, audioCfgPlayIntercom;

	bool startAudioSourceAtCallStart;	// start audio source at the very beginning of the call (fighting with some delay on BT devices) 

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

	struct AudioGate {
		bool enabled;					/**< on/off */
		unsigned int closeThreshold;	/**< raw sample value */
		unsigned int holdMs;			/**< how long gate should be opened if signal is below close threshold */
		unsigned int attackMs;			/**< time to gate fully open (gain 0 -> 1 transition) in ms */
		unsigned int releaseMs;			/**< time to gete fully closed (gain 1 -> 0 transition) in ms */
		AudioGate (void):
			enabled(false),
			closeThreshold(1000),		/**< of 32767 */
			holdMs(1000),
			attackMs(50),
			releaseMs(1000)
		{}
		bool operator==(const AudioGate& right) const;
		bool operator!=(const AudioGate& right) const {
			return !(*this == right);
		}
	} audioGateTx;

	bool loopRingWithoutSilence;	

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
		unsigned int fileFormat;		///< \sa baresip_recorder.h
		unsigned int bitrate;
		enum RecStart {
			RecStartManual = 0,			///< start recording manually, e.g. using Lua API
			RecStartCallConfirmed,		///< recording starts on call confirmed
			RecStartCallEarly,			///< recording starts on call early media (SIP/183)

			RecStartLimiter
		} recStart;
		bool noNumberB64Encoding;		///< do not encode number/URI with filesystem-safe base64
		RecordingCfg(void):
			enabled(false),
			recDir(RecDirRelative),
			channels(1),
			side(0),
			fileFormat(0),
			bitrate(64000),
			recStart(RecStartCallConfirmed),
			noNumberB64Encoding(false)
		{}
		bool operator==(const RecordingCfg& right) const {
			return enabled == right.enabled &&
				recDir == right.recDir &&
				customRecDir == right.customRecDir &&
				channels == right.channels &&
				side == right.side &&
				recStart == right.recStart &&
				noNumberB64Encoding == right.noNumberB64Encoding
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

	struct Opus {
		bool stereo;
		bool spropStereo;

		bool setBitrate;
		uint32_t bitrate;

		bool setSamplerate;
		uint32_t samplerate;

		bool setCbr;
		bool cbr;

		bool setInbandFec;
		bool inbandFec;

		bool setDtx;
		bool dtx;

		bool mirror;
		uint32_t complexity;

		bool setApplication;
		int application;

		bool setPacketLoss;
		uint32_t packetLoss;

		bool operator==(const UaConf::Opus& right) const {
			if (
				stereo == right.stereo &&
				spropStereo == right.spropStereo &&
				setBitrate == right.setBitrate &&
				bitrate == right.bitrate &&
				setSamplerate == right.setSamplerate &&
				samplerate == right.samplerate &&
				setCbr == right.setCbr &&
				cbr == right.cbr &&
				setInbandFec == right.setInbandFec &&
				inbandFec == right.inbandFec &&
				setDtx == right.setDtx &&
				dtx == right.dtx &&
				mirror == right.mirror &&
				complexity == right.complexity &&
				setApplication == right.setApplication &&
				application == right.application &&
				setPacketLoss == right.setPacketLoss &&
				packetLoss == right.packetLoss
				)
				return true;
			return false;
		}
		bool operator!=(const UaConf::Opus& right) const {
			return !(*this == right);
		}
		Opus(void):
			stereo(true),
			spropStereo(true),
			setBitrate(false),
			bitrate(64000),
			setSamplerate(false),
			samplerate(48000),
			setCbr(false),
			cbr(false),
			setInbandFec(false),
			inbandFec(false),
			setDtx(false),
			dtx(false),
			mirror(false),
			complexity(10),
			setApplication(false),
			application(0),
			setPacketLoss(false),
			packetLoss(10)
		{
		}
	} opus;

	struct Zrtp {
		bool startParallel;
		Zrtp(void):
			startParallel(true)
		{}
		bool operator==(const Zrtp& right) const {
			if (
				startParallel == right.startParallel
				)
			{
				return true;
			}
			return false;
		}
		bool operator!=(const Zrtp& right) const {
			return !(*this == right);
		}
	} zrtp;

	struct Tls {
		std::string certificate;
		std::string caFile;
		bool useWindowsRootCaStore;
		bool verifyServer;
		Tls(void):
			useWindowsRootCaStore(true),
			verifyServer(true)
		{}
		bool operator==(const Tls& right) const {
			return (
				certificate == right.certificate &&
				caFile == right.caFile &&
				useWindowsRootCaStore == right.useWindowsRootCaStore &&
				verifyServer == right.verifyServer
			);
		}
		bool operator!=(const Tls& right) const {
			return !(*this == right);
		}
	} tls;

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

	struct Messages {
		int replyCode;
		std::string replyReason;
		bool doNotReply;
		Messages(void):
			replyCode(200),
			replyReason("OK"),
			doNotReply(false)
		{}
		bool operator==(const UaConf::Messages& right) const {
			if (replyCode == right.replyCode &&
				replyReason == right.replyReason &&
                doNotReply == right.doNotReply
				)
			{
				return true;
			}
			return false;
		}
		bool operator!=(const UaConf::Messages& right) const {
        	return !(*this == right);
		}
	} messages;

	bool autoAnswer;
	int autoAnswerCode;
	std::string autoAnswerReason;
	unsigned int autoAnswerDelayMin;
	unsigned int autoAnswerDelayMax;

	bool autoAnswerCallInfo;	///< auto-answer when Call-Info with answer-after is present in incoming INVITE
	unsigned int autoAnswerCallInfoDelayMin;	///< minimum delay when auto answering intercom/paging calls [ms]

	bool answerOnEventTalk;		///< answer when NOTIFY with "Event: talk" is received (CRM integration through asterisk AMI)

	bool handleOodRefer;		///< handle incoming out-of-dialog refer

	bool customUserAgent;
	std::string userAgent;

	struct Video {
		struct Device {
			std::string mod;       	/**< module */
			std::string dev;		/**< device */
			Device(void):
				mod(VideoModules::dshow)
			{
			}
			bool operator==(const Device& right) const {
				if (
					mod != right.mod ||
					dev != right.dev
				) {
                	return false;
				}
				return true;
			}
			bool operator!=(const Device& right) const {
				return !(*this == right);
			}
		} videoSource;

		unsigned int width, height;
		unsigned int bitrate;
		unsigned int fps;

		struct Selfview {
			bool enabled;
			bool pip;
			Selfview(void):
				enabled(true),
				pip(true)
			{}
			bool operator==(const Selfview& right) const {
				return (
					enabled == right.enabled &&
					pip == right.pip
				);
			}
			bool operator!=(const Selfview& right) const {
				return !(*this == right);
			}
		} selfview;

		struct Dshow {
			bool skipReadingBackMediaFormat;
			Dshow(void):
				skipReadingBackMediaFormat(false)
			{}
			bool operator==(const Dshow& right) const {
				return (
                	skipReadingBackMediaFormat == right.skipReadingBackMediaFormat
				);
			}
			bool operator!=(const Dshow& right) const {
            	return !(*this == right);
			}
		} dshow;

		Video(void):
			width(640),
			height(480),
			bitrate(512000),
			fps(25)
		{
		}

		bool operator==(const Video& right) const {
			return (
				videoSource == right.videoSource &&
				width == right.width &&
				height == right.height &&
				bitrate == right.bitrate &&
				fps == right.fps &&
				selfview == right.selfview &&
				dshow == right.dshow
			);
		}
		bool operator!=(const Video& right) const {
			return !(*this == right);
		}
	} video;

	UaConf() {
 		disableUa = false;
		aec = AEC_NONE;
		logMessages = false;
		logMessagesOnlyFirstLine = false;
		logAubuf = false;
		loopRingWithoutSilence = false;
		startAudioSourceAtCallStart = false;
		autoAnswer = false;
		autoAnswerCode = 200;
		autoAnswerReason = "OK";
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
		if (disableUa != right.disableUa)
			return false;
		if (loopRingWithoutSilence != right.loopRingWithoutSilence)
			return false;
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
		if (startAudioSourceAtCallStart != right.startAudioSourceAtCallStart)
			return false;
		if (recording != right.recording)
			return false;
		if (audioAgcRx != right.audioAgcRx)
			return false;
		if (audioGateTx != right.audioGateTx)
			return false;
		if (audioPortaudio != right.audioPortaudio)
			return false;
		if (audioPreprocTx != right.audioPreprocTx)
			return false;
		if (aec != right.aec)
			return false;
		if (webrtcAec != right.webrtcAec)
			return false;
		if (opus != right.opus)
			return false;
		if (zrtp != right.zrtp)
			return false;
		if (tls != right.tls)
			return false;
		if (local != right.local)
			return false;
		if (ifname != right.ifname)
			return false;
		if (avt != right.avt)
			return false;
		if (messages != right.messages)
			return false;
		if (customUserAgent != right.customUserAgent)
			return false;
		if (customUserAgent == true && (userAgent != right.userAgent))
			return false;
		if (video != right.video)
			return false;
		return true;
	}
	bool operator!=(const UaConf& right) const {
		return !(*this == right);
	}

	void fromJson(const Json::Value& uaConfJson, const struct SettingsAppVersion& settingsAppVersion);
	void toJson(Json::Value& uaConfJson) const;
};

#endif
