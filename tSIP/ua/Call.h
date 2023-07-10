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
	Callback::ua_state_e state;
	int lastScode;			///< last SIP code (on disconnected)
	AnsiString lastReplyLine;
	AnsiString recordFile;
	bool recording;
	std::deque<char> dtmfRxQueue;
	bool ringStarted;
	AnsiString paiPeerUri;
	AnsiString paiPeerName;
	AnsiString codecName;
	void* displayParentHandle;
	unsigned int audioErrorCount;
	int btnId;
	bool hold;
	bool mute;

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
		ringStarted(false),
		displayParentHandle(NULL),
		audioErrorCount(0),
		btnId(-1),
		hold(false),
		mute(false),
		tmrAutoAnswer(NULL),
		autoAnswerCode(200),
		autoAnswerIntercom(false)
	{}
	~Call(void);
	void reset(void);

	void setHold(bool state);
	void setMute(bool state);
	AnsiString getPeerUri(void) const;
	AnsiString getPeerName(void) const;
	AnsiString getStateName(void) const;
	AnsiString getStateDescription(void) const;
};

#endif
