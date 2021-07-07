//---------------------------------------------------------------------------

#ifndef ButtonTypeH
#define ButtonTypeH
//---------------------------------------------------------------------------
namespace Button
{
	enum Type
	{
		DISABLED = 0,
		SPEED_DIAL,
		BLF,
		DTMF,
		REDIAL,
		TRANSFER,
		HOLD,
		REREGISTER,
		MWI,
		MUTE,
		MUTE_RING,
		CONTACT_NOTE,
		HTTP_QUERY,
		PRESENCE,
		UNREGISTER,
		EXECUTE,
		PAGING_TX,
		SCRIPT,
		SIP_ACCESS_URL,
		SWITCH_AUDIO_SOURCE,
		SWITCH_AUDIO_PLAYER,
		HANGUP,		
		SHOW_SETTINGS,
		MINIMIZE,
		SHOW_LOG,
		EXIT,
		CONTACT_FILE,	// open file associated with contact / phonebook entry
		UA_RESTART,		// restart user agent
		SEND_TEXT_MESSAGE,	// send SIMPLE text message to recipient from button or from main URI "dial"		
		RECORD,
		RECORD_PAUSE,
		AUTO_ANSWER_DND,
		ZRTP_VERIFY_SAS,
		ZRTP_UNVERIFY_SAS,
		
		TYPE_LIMITER
	};
	const char* TypeName(enum Type type);
	const char* TypeDescription(enum Type type);
};

#endif
