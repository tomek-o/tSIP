//---------------------------------------------------------------------------

#ifndef CallbackQueueH
#define CallbackQueueH
//---------------------------------------------------------------------------

#include "common/singleton.h"
#include "Callback.h"
#include <System.hpp>

class Callback;

class CallbackQueue: public CSingleton<CallbackQueue>
{
private:
	CallbackQueue();
	friend CSingleton<CallbackQueue>;
public:
	int GetCallback(Callback& cb);
	/** \param scode SIP code for closing call
	*/
	void ChangeCallState(Callback::ua_state_e state, AnsiString caller, AnsiString caller_name, int scode, int answer_after, AnsiString alert_info, AnsiString access_url, int access_url_mode, AnsiString pai_peer_uri, AnsiString pai_peer_name, AnsiString codec_name);
	void OnReinviteReceived(AnsiString caller, AnsiString caller_name, AnsiString pai_peer_uri, AnsiString pai_peer_name);
	void ChangeCallDtmfState(AnsiString dtmf, bool active);
	void ChangeRegState(int acc_id, Callback::reg_state_e state, const char *prm);
	void ChangeAppState(Callback::app_state_e state);
	void ChangeDlgInfoState(int id, int state, int direction, const char *remote_identity, const char *remote_identity_display);
	void ChangePresenceState(int id, int state, const char *note);
	void ChangeMwiState(int newMsg, int oldMsg);
	void ChangePagingTxState(Callback::paging_tx_state_e state);
	void NotifyEventTalk(void);
	void SetCallData(AnsiString initialRxInvite);
	void NotifyAudioError(void);
};

#define UA_CB CallbackQueue::Instance()

#endif
