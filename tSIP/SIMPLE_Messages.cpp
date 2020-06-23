//---------------------------------------------------------------------------


#pragma hdrstop

#include "SIMPLE_Messages.h"
#include "FormMessage.h"
#include "Log.h"
#include "Settings.h"
#include <set>

//---------------------------------------------------------------------------

#pragma package(smart_init)

using namespace SIMPLE_Messages;


namespace SIMPLE_Messages
{

std::set<TfrmMessage*> messageWindows;

void RegisterWindow(TfrmMessage *frmMessage)
{
	messageWindows.insert(frmMessage);
}

void UnregisterWindow(TfrmMessage *frmMessage)
{
	messageWindows.erase(frmMessage);
}

void CloseAllWindows(void)
{
	std::set<TfrmMessage*>::iterator iter;
	for (iter = messageWindows.begin(); iter != messageWindows.end(); )
	{
		TfrmMessage *frm = *(iter++);
		frm->Close();
	}
}

bool IsTargetMatching(AnsiString remote, AnsiString target)
{
	if (remote == target)
	{
		return true;
	}

	if (target.Pos("sip:") != 1)
	{
		if (appSettings.uaConf.accounts.size() > 0)
		{
			target = (AnsiString)"sip:" + target + "@" + appSettings.uaConf.accounts[0].reg_server.c_str();
		}
	}
	if (remote.Pos("sip:") != 1)
	{
		if (appSettings.uaConf.accounts.size() > 0)
		{
			remote = (AnsiString)"sip:" + remote + "@" + appSettings.uaConf.accounts[0].reg_server.c_str();
		}
	}

	if (remote == target)
	{
		return true;
	}

	return false;
}

TfrmMessage* FindForm(AnsiString target)
{
	std::set<TfrmMessage*>::iterator iter;
	for (iter = messageWindows.begin(); iter != messageWindows.end(); ++iter)
	{
		TfrmMessage *frmIter = *(iter);
		if (IsTargetMatching(target, frmIter->GetTarget()))
		{
			return frmIter;
		}
	}
	return NULL;
}

unsigned int CountIncomingForms(void)
{
	std::set<TfrmMessage*>::iterator iter;
	unsigned int incomingCount = 0;
	for (iter = messageWindows.begin(); iter != messageWindows.end(); ++iter)
	{
		TfrmMessage *frmIter = *(iter);
		if (frmIter->GetIncoming())
		{
			incomingCount++;
		}
	}
	return incomingCount;
}

void OnIncomingMessage(AnsiString caller, AnsiString contentType, AnsiString body)
{
	LOG("Received message from %s: ContentType %s, body [%s]\n", caller.c_str(), contentType.c_str(), body.c_str());
	std::set<TfrmMessage*>::iterator iter;
	TfrmMessage *frm = FindForm(caller);
	if (frm == NULL && CountIncomingForms() > 25)
	{
		LOG("Limit for MESSAGE windows for incoming transactions exceeded, ignoring incoming message\n");
		return;
	}
	if (frm == NULL)
	{
		frm = new TfrmMessage(NULL);
		frm->SetIncoming();
		frm->SetTarget(caller);
	}
	frm->AddIncomingMessage(contentType, body);
	frm->Show();
}

void OnMessageStatus(int requestUid, int requestError, int sipCode, AnsiString reason)
{
	LOG("MessageStatus: requestUid = %d, requestError = %d, sipCode = %d, reason = %s\n", requestUid, requestError, sipCode, reason.c_str());
	std::set<TfrmMessage*>::iterator iter;
	for (iter = messageWindows.begin(); iter != messageWindows.end(); ++iter)
	{
		TfrmMessage *frmIter = *(iter);
		if (frmIter->HandleMessageStatus(requestUid, requestError, sipCode, reason) == 0)
		{
			return;
		}
	}
}

void Send(AnsiString target, AnsiString text, bool sendImmediately)
{
	TfrmMessage *frm = FindForm(target);
	if (frm == NULL)
	{
		frm = new TfrmMessage(NULL);
		frm->SetTarget(target);
	}
	if (text != "")
	{
		frm->SetText(text);
	}
	frm->Show();
	if (sendImmediately)
	{
    	frm->_SendMsg();
	}
}

}	// namespace SIMPLE_Messages

