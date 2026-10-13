//---------------------------------------------------------------------------

#ifndef CallH
#define CallH
//---------------------------------------------------------------------------

#include "Recorder.h"
#include "Callback.h"

#include <System.hpp>
#include <deque>

namespace Extctrls
{
	class TTimer;
}

struct Call
{
	enum { INVALID_UID = 0 };

	unsigned int uid;
	bool incoming;
	bool progress;	// early media
	bool connected;
	bool disconnecting;		///< set to true if softphone is disconnecting (not 2nd party)
	AnsiString uri;
	AnsiString initialTarget;	///< used by ORIGINATE, otherwise same as uri
	AnsiString peerName;	///< = display name
	AnsiString accessUrl;
	int accessUrlMode;
	AnsiString initialRxInvite;
	TDateTime timestamp;
	TDateTime timeTalkStart;
private:
	Callback::ua_state_e state;
public:
	void SetState(Callback::ua_state_e state);
	void SetInitialState(Callback::ua_state_e state);
	Callback::ua_state_e GetState(void) const {
		return state;
	}
	void ShowOnLineButton(void) const;
	int lastScode;			///< last SIP code (on disconnected)
	AnsiString lastReplyLine;
	AnsiString recordFile;
	bool recording;
	std::deque<char> dtmfRxQueue;
	AnsiString paiPeerUri;
	AnsiString paiPeerName;
	AnsiString codecName;
	void* displayParentHandle;
	unsigned int audioErrorCount;
	/** \brief Extra header lines for outgoing INVITE of this call (added after header lines
		from settings); "Name: value" lines separated with CRLF; set by plugin (Call method) or
		by Lua (SetCallExtraHeaderLines, e.g. in "on making call" script) */
	AnsiString extraHeaderLines;
	/** \brief Do not disconnect call at end of audio file regardless of settings -
		file played on request (e.g. plugin PlayAudioFile), requester decides what next */
	bool keepCallAtAudioFileEnd;
	int btnId;
	bool hold;
	bool mute;
	AnsiString dialString;
	AnsiString sipReason;

	/** \note Owned (deleted in destructor) - copies of Call must not keep it,
		use Calls::GetCallsInfo() for snapshots of call list */
	Extctrls::TTimer *tmrAutoAnswer;
	int autoAnswerCode;
	bool autoAnswerIntercom;

	Recorder recorder;

	struct Zrtp {
		int sessionId;
		bool active;
		AnsiString sas;
		AnsiString cipher;
		bool verified;
		Zrtp(void):
			sessionId(-1),
			active(false),
			verified(false)
		{}
	} zrtp;

	Call(void):
		uid(0),
		incoming(false),
		progress(false),
		connected(false),
		disconnecting(false),
		accessUrlMode(0),
		state(Callback::CALL_STATE_CLOSED),
		lastScode(0),
		recording(false),
		displayParentHandle(NULL),
		audioErrorCount(0),
		keepCallAtAudioFileEnd(false),
		btnId(-1),
		hold(false),
		mute(false),
		sidetone(false),
		sidetoneLevel(50),
		tmrAutoAnswer(NULL),
		autoAnswerCode(200),
		autoAnswerIntercom(false)
	{}
	~Call(void);
	void reset(void);

	void setHold(bool state);
	void setMute(bool state);
	bool sidetone;					///< transmitted audio mixed into local playback
	unsigned int sidetoneLevel;		///< percent of transmitted audio level
	/** \brief Enable/disable sidetone; works only if call has audio (connected or early media)
		\return actual state */
	bool setSidetone(bool state, unsigned int level);
	AnsiString getPeerUri(void) const;
	AnsiString getPeerName(void) const;
	AnsiString getStateName(void) const;
	AnsiString getStateDescription(void) const;
	AnsiString getStateTranslatedName(void) const;
	AnsiString getStateTranslatedDescription(void) const;
};

#endif
