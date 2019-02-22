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
		UPDATE_SOFTVOL_RX
	} type;

	AnsiString target;
	AnsiString extraHeaderLines;	///< extra/custom header lines for outgoing calls
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
};

#endif
