//---------------------------------------------------------------------------


#pragma hdrstop

#include "CallbackQueue.h"
#include "Callback.h"
#include "common/Mutex.h"
#include "common/ScopedLock.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

#include "common/fifo.h"
#include "Callback.h"

namespace
{
	Fifo<Callback, 1024> fifo;
	Mutex mutex;
}

CallbackQueue::CallbackQueue()
{

}

int CallbackQueue::GetCallback(Callback& cb)
{
	ScopedLock<Mutex> lock(mutex);
	Callback *tmpcb = fifo.getReadable();
	if (tmpcb == NULL)
	{
		return 1;
	}
	cb = *tmpcb;
	fifo.pop();
	return 0;
}

void CallbackQueue::SetCallData(AnsiString initialRxInvite)
{
	ScopedLock<Mutex> lock(mutex);
	Callback *cb = fifo.getWriteable();
	if (!cb)
		return;
	cb->type = Callback::SET_CALL_DATA;
	cb->initialRxInvite = initialRxInvite;
	fifo.push();
}

void CallbackQueue::ChangeCallState(Callback::ua_state_e state, AnsiString caller, AnsiString caller_name, int scode, int answer_after, AnsiString alert_info, AnsiString access_url, int access_url_mode, AnsiString pai_peer_uri, AnsiString pai_peer_name, AnsiString codec_name)
{
	ScopedLock<Mutex> lock(mutex);
	Callback *cb = fifo.getWriteable();
	if (!cb)
		return;
	cb->type = Callback::CALL_STATE;
	cb->state = state;
	cb->caller = caller;
	cb->callerName = caller_name;
	cb->scode = scode;
	cb->callAnswerAfter = answer_after;
	cb->alertInfo = alert_info;
	cb->accessUrl = access_url;
	cb->accessUrlMode = access_url_mode;
	cb->paiPeerUri = pai_peer_uri;
	cb->paiPeerName = pai_peer_name;
	cb->codecName = codec_name;
	fifo.push();
}

void CallbackQueue::OnReinviteReceived(AnsiString caller, AnsiString caller_name, AnsiString pai_peer_uri, AnsiString pai_peer_name)
{
	ScopedLock<Mutex> lock(mutex);
	Callback *cb = fifo.getWriteable();
	if (!cb)
		return;
	cb->type = Callback::CALL_REINVITE_RECEIVED;
	cb->caller = caller;
	cb->callerName = caller_name;
	cb->paiPeerUri = pai_peer_uri;
	cb->paiPeerName = pai_peer_name;
	fifo.push();
}

void CallbackQueue::ChangeCallDtmfState(AnsiString dtmf, bool active)
{
	ScopedLock<Mutex> lock(mutex);
	Callback *cb = fifo.getWriteable();
	if (!cb)
		return;
	cb->type = Callback::CALL_DTMF_STATE;
	cb->dtmf = dtmf;
	cb->dtmfActive = active;
	fifo.push();
}

void CallbackQueue::ChangeRecorderState(int id, Callback::rec_state_e state)
{
	ScopedLock<Mutex> lock(mutex);
	Callback *cb = fifo.getWriteable();
	if (!cb)
		return;
	cb->type = Callback::RECORDER_STATE;
	cb->recorderId = id;
	cb->rec_state = state;
	fifo.push();
}

void CallbackQueue::ChangeEncryptionState(const Callback::Zrtp &state)
{
	ScopedLock<Mutex> lock(mutex);
	Callback *cb = fifo.getWriteable();
	if (!cb)
		return;
	cb->type = Callback::ENCRYPTION_STATE;
	cb->zrtp = state;
	fifo.push();
}


void CallbackQueue::ChangeRegState(int acc_id, Callback::reg_state_e state, const char *prm)
{
	ScopedLock<Mutex> lock(mutex);
	Callback *cb = fifo.getWriteable();
	if (!cb)
		return;
	cb->type = Callback::REG_STATE;
	cb->reg_state = state;
	cb->accountId = acc_id;
	cb->prm = prm ? prm : "";
	fifo.push();
}

void CallbackQueue::ChangeAppState(Callback::app_state_e state)
{
	ScopedLock<Mutex> lock(mutex);
	Callback *cb = fifo.getWriteable();
	if (!cb)
		return;
	cb->type = Callback::APP_STATE;
	cb->app_state = state;
	fifo.push();
}

void CallbackQueue::ChangeDlgInfoState(int id, const struct dialog_data *ddata, unsigned int ddata_cnt)
{
	ScopedLock<Mutex> lock(mutex);
	Callback *cb = fifo.getWriteable();
	if (!cb)
		return;
	cb->type = Callback::DLG_INFO_STATE;
	cb->contactId = id;
	if (ddata_cnt > sizeof(cb->ddata)/sizeof(cb->ddata[0]))
		ddata_cnt = sizeof(cb->ddata)/sizeof(cb->ddata[0]);
	cb->ddata_cnt = ddata_cnt;
	memcpy(cb->ddata, ddata, ddata_cnt * sizeof(ddata[0]));
	fifo.push();
}

void CallbackQueue::ChangePresenceState(int id, int state, const char* note)
{
	ScopedLock<Mutex> lock(mutex);
	Callback *cb = fifo.getWriteable();
	if (!cb)
		return;
	cb->type = Callback::PRESENCE_STATE;
	cb->contactId = id;
	cb->presenceState = state;
	cb->presenceNote = note;
	fifo.push();
}

void CallbackQueue::ChangeMwiState(int newMsg, int oldMsg)
{
	ScopedLock<Mutex> lock(mutex);
	Callback *cb = fifo.getWriteable();
	if (!cb)
		return;
	cb->type = Callback::MWI_STATE;
	cb->mwiNewMsg = newMsg;
	cb->mwiOldMsg = oldMsg;
	fifo.push();
}

void CallbackQueue::ChangePagingTxState(Callback::paging_tx_state_e state)
{
	ScopedLock<Mutex> lock(mutex);
	Callback *cb = fifo.getWriteable();
	if (!cb)
		return;
	cb->type = Callback::PAGING_TX_STATE;
	cb->paging_tx_state = state;
	fifo.push();
}

void CallbackQueue::NotifyEventTalk(void)
{
	ScopedLock<Mutex> lock(mutex);
	Callback *cb = fifo.getWriteable();
	if (!cb)
		return;
	cb->type = Callback::EVENT_TALK;
	fifo.push();
}

void CallbackQueue::NotifyAudioError(void)
{
	ScopedLock<Mutex> lock(mutex);
	Callback *cb = fifo.getWriteable();
	if (!cb)
		return;
	cb->type = Callback::AUDIO_ERROR;
	fifo.push();
}


void CallbackQueue::NotifyCustomRequestStatus(int requestUid, int requestError, int sipStatusCode, AnsiString requestReplyText)
{
	ScopedLock<Mutex> lock(mutex);
	Callback *cb = fifo.getWriteable();
	if (!cb)
		return;

	cb->type = Callback::CUSTOM_REQUEST_STATUS;
	cb->requestUid = requestUid;
	cb->requestError = requestError;
	cb->scode = sipStatusCode;
	cb->requestReplyText = requestReplyText;

	fifo.push();
}

void CallbackQueue::OnMessageReceived(AnsiString caller, AnsiString contentType, AnsiString body)
{
	ScopedLock<Mutex> lock(mutex);
	Callback *cb = fifo.getWriteable();
	if (!cb)
		return;
		
	cb->type = Callback::SIMPLE_MESSAGE;
	cb->caller = caller;
	cb->contentType = contentType;
	cb->body = body;

	fifo.push();
}

void CallbackQueue::OnMessageStatus(int requestUid, int requestError, int sipStatusCode, AnsiString reason)
{
	ScopedLock<Mutex> lock(mutex);
	Callback *cb = fifo.getWriteable();
	if (!cb)
		return;

	cb->type = Callback::SIMPLE_MESSAGE_STATUS;
	cb->requestUid = requestUid;
	cb->requestError = requestError;
	cb->scode = sipStatusCode;
	cb->reason = reason;

	fifo.push();
}





