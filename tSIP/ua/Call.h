//---------------------------------------------------------------------------

#ifndef CallH
#define CallH
//---------------------------------------------------------------------------

#include <System.hpp>
#include <deque>

struct Call
{
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
	int state;				///< as in Callback::ua_state_e enum
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
		incoming(false),
		progress(false),
		connected(false),
		disconnecting(false),
		accessUrlMode(0),
		state(0),
		lastScode(0),
		recording(false),
		ringStarted(false),
		displayParentHandle(NULL)
	{}
	void reset(void);
};

#endif
