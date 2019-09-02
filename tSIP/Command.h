//---------------------------------------------------------------------------

#ifndef CommandH
#define CommandH
//---------------------------------------------------------------------------

#include <System.hpp>

class Command
{
public:
	enum Type
	{
		CALL = 0,
		ANSWER,
		SEND_DIGIT,
		TRANSFER,
		HOLD,
		HANGUP,
		SET_MSG_LOGGING,
		REREGISTER,
		UNREGISTER,
		MUTE,
		START_RING,			///< start playing ring tone / wave file
		STOP_RING,			///< stop playing ring wave file
		RECORD,
		PAGING_TX,
		SWITCH_AUDIO_SOURCE,///< change source audio device for current call (audioMod/audioDev)
		SWITCH_AUDIO_PLAYER,///<                                             (audioMod/audioDev)
		UPDATE_SOFTVOL_TX,
		UPDATE_SOFTVOL_RX,
		SEND_CUSTOM_REQUEST,

		TYPE__LIMITER
	} type;

	AnsiString target;
	AnsiString extraHeaderLines;	///< extra/custom header lines for outgoing calls and custom requests
	int accountId;
	int callId;
	int code;
	char key;
	bool bEnabled;
	AnsiString audioMod;
	AnsiString audioDev;
	AnsiString pagingTxWaveFile;	///< source wave file for paging TX; default audio source is used if empty
	AnsiString pagingTxCodec;
	unsigned int pagingTxPtime;
	unsigned int channels;
	unsigned int recSide;
	unsigned int softvol;

	int requestId;
	AnsiString method;				///< SEND_CUSTOM_REQUEST

	Command(void):
		type(TYPE__LIMITER),
		accountId(-1),
		callId(-1),
		code(-1),
		key('\0'),
		bEnabled(false),
		pagingTxPtime(0),
		channels(0),
		recSide(0),
		softvol(0),
		requestId(-1)
	{}
};

#endif
