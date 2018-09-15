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
	Fifo<Callback, 512> fifo;
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

void CallbackQueue::ChangeCallState(Callback::ua_state_e state, AnsiString caller, AnsiString caller_name, int scode, int answer_after, AnsiString alert_info, AnsiString access_url, int access_url_mode)
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

void CallbackQueue::ChangeDlgInfoState(int id, int state)
{
	ScopedLock<Mutex> lock(mutex);
	Callback *cb = fifo.getWriteable();
	if (!cb)
		return;
	cb->type = Callback::DLG_INFO_STATE;
	cb->contactId = id;
	cb->dlgInfoState = state;
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




