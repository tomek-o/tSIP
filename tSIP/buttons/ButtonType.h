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

		TYPE_LIMITER
	};
	const char* TypeName(enum Type type);
	const char* TypeDescription(enum Type type);
};

#endif
