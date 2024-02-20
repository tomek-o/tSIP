//---------------------------------------------------------------------------
#pragma hdrstop

#include "Calls.h"
#include "buttons/ProgrammableButtons.h"
#include "buttons/ProgrammableButton.h"
#include "Globals.h"
#include "Settings.h"
#include "ControlQueue.h"
#include "common/Mutex.h"
#include "common/ScopedLock.h"
#include "Log.h"
#include <map>
#include <set>

//---------------------------------------------------------------------------

#pragma package(smart_init)

namespace {
	/** \note Call UID = 0 is reserved/invalid/not used */
	unsigned int nextUid = Call::INVALID_UID + 1;
	std::map<unsigned int, Call> entries;
	Mutex mutex;

	Call previousCall;

	std::set<unsigned int> lineButtonIds;

	Call* FindCallFromLineButton(unsigned int btnId)
	{
		for (std::map<unsigned int, Call>::iterator iter = entries.begin(); iter != entries.end(); ++iter)
		{
			Call &call = iter->second;
			if (call.btnId == btnId)
				return &call;
		}
		return NULL;
	}

	unsigned int currentCallUid = Call::INVALID_UID;
}

Call* Calls::Alloc(void)
{
	ScopedLock<Mutex> lock(mutex);
	Call newCall;
	bool colliding = false;
	do
	{
		newCall.uid = nextUid;
		nextUid++;
		if (nextUid == Call::INVALID_UID)
		{
			nextUid = 1;
		}
		if (!entries.empty())
		{
			colliding = (entries.find(newCall.uid) != entries.end());
		}
	} while (colliding);

	entries[newCall.uid] = newCall;

	return &entries[newCall.uid];
}

Call* Calls::FindByUid(unsigned int uid)
{
	ScopedLock<Mutex> lock(mutex);
	std::map<unsigned int, Call>::iterator iter;
	iter = entries.find(uid);
	if (iter == entries.end())
		return NULL;
	else
		return &iter->second;
}

Call* Calls::FindByAutoAnswerTimer(Extctrls::TTimer *tmr)
{
	std::map<unsigned int, Call>::iterator iter;
	for (iter = entries.begin(); iter != entries.end(); ++iter)
	{
		Call &call = iter->second;
		if (call.tmrAutoAnswer == tmr)
		{
			return &call;
		}
	}
	return NULL;
}

Call* Calls::GetCurrentCall(void)
{
	ScopedLock<Mutex> lock(mutex);
	if (currentCallUid != Call::INVALID_UID)
	{
		Call *call = FindByUid(currentCallUid);
		return call;
	}
	return NULL;
}

unsigned int Calls::GetCurrentCallUid(void)
{
	ScopedLock<Mutex> lock(mutex);
	return currentCallUid;
}

int Calls::SetCurrentCallUid(unsigned int uid)
{
	ScopedLock<Mutex> lock(mutex);
	if (currentCallUid == uid)
		return 0;
	Call* call = FindByUid(uid);
	if (call == NULL)
		return -1;
	if (call->btnId >= 0)
	{
		TProgrammableButton *btn = buttons.GetBtn(call->btnId);
		OnLineButtonClick(call->btnId, btn);
	}
	return 0;
}

Call& Calls::GetPreviousCall(void)
{
	ScopedLock<Mutex> lock(mutex);
	return previousCall;
}

void Calls::SetPreviousCall(const Call &call)
{
	ScopedLock<Mutex> lock(mutex);
	previousCall = call;
}

void Calls::Clear(void)
{
	ScopedLock<Mutex> lock(mutex);
	entries.clear();
}

void Calls::RemoveByUid(unsigned int uid)
{
	ScopedLock<Mutex> lock(mutex);
	entries.erase(uid);
	// button "down" state is not changed
}

std::vector<unsigned int> Calls::GetUids(void)
{
	ScopedLock<Mutex> lock(mutex);
	std::vector<unsigned int> ret;
	std::map<unsigned int, Call>::iterator iter;
	for (iter = entries.begin(); iter != entries.end(); ++iter)
	{
		ret.push_back(iter->first);
	}
	return ret;
}

const std::map<unsigned int, Call> Calls::GetCalls(void)
{
	ScopedLock<Mutex> lock(mutex);
	return entries;
}

unsigned int Calls::Count(void)
{
	ScopedLock<Mutex> lock(mutex);
	return entries.size();
}

Recorder* Calls::FindRecorder(int recorderId)
{
	ScopedLock<Mutex> lock(mutex);
	std::map<unsigned int, Call>::iterator iter;
	for (iter = entries.begin(); iter != entries.end(); ++iter)
	{
		if (iter->second.recorder.id == recorderId)
		{
			return &iter->second.recorder;
		}
	}
	return NULL;
}

int Calls::AssignLineButton(Call *call, bool outgoing, int &btnId)
{
	if (lineButtonIds.empty())
	{
		// single-call configuration
		btnId = -1;
		if (entries.size() == 1)
		{
            currentCallUid = call->uid;
			return 0;	// no button assigned - none needed
		}
		else
		{
			// deny second call in single-call configuration
			return -1;
		}
	}
	else
	{
		if (outgoing)
		{
			// try button in "down" state first
			for (std::set<unsigned int>::iterator iter = lineButtonIds.begin(); iter != lineButtonIds.end(); ++iter)
			{
				unsigned int id = *iter;
				TProgrammableButton *btn = buttons.GetBtn(*iter);
				if (!btn || !btn->GetDown())
					continue;
				Call *callFromBtn = FindCallFromLineButton(id);
				if (callFromBtn == NULL)
				{
					call->btnId = id;
					btnId = call->btnId;
					currentCallUid = call->uid;
					call->ShowOnLineButton();
					return 0;
				}
			}
		}

		for (std::set<unsigned int>::iterator iter = lineButtonIds.begin(); iter != lineButtonIds.end(); ++iter)
		{
			unsigned int id = *iter;
			Call *callFromBtn = FindCallFromLineButton(id);
			if (callFromBtn == NULL)
			{
				call->btnId = id;
				btnId = call->btnId;
				if (Count() == 1)
				{
					TProgrammableButton *btn = buttons.GetBtn(*iter);
					if (btn)
					{
						for (std::set<unsigned int>::iterator iter2 = lineButtonIds.begin(); iter2 != lineButtonIds.end(); ++iter2)
						{
							TProgrammableButton *btn2 = buttons.GetBtn(*iter2);
							if (btn2 && btn2->GetDown())
							{
								btn2->SetDown(false);
							}
						}
						btn->SetDown(true);
					}
					currentCallUid = call->uid;
				}
				else
				{
					TProgrammableButton *btn = buttons.GetBtn(*iter);
					if (btn && btn->GetDown())
					{
						currentCallUid = call->uid;
					}
				}
				call->ShowOnLineButton();
				return 0;
			}
		}
		// failed to assign LINE button for the call
		return -1;
	}
}

void Calls::OnLineButtonClick(int id, TProgrammableButton* btn)
{
	ScopedLock<Mutex> lock(mutex);
	assert(lineButtonIds.find(id) != lineButtonIds.end());	// sanity check

	bool autoHold = appSettings.Calls.autoHoldWhenSwitchingCalls;

	for (std::set<unsigned int>::iterator iter = lineButtonIds.begin(); iter != lineButtonIds.end(); ++iter)
	{
		if (*iter == id)
			continue;
		TProgrammableButton *btn = buttons.GetBtn(*iter);
		if (btn && btn->GetDown())
		{
			btn->SetDown(false);
			if (autoHold)
			{
				Call *call = FindCallFromLineButton(*iter);
				if (call)
				{
					call->setHold(true);
				}
			}
		}
	}

	btn->SetDown(true);
	Call *call = FindCallFromLineButton(id);
	if (call)
	{
		currentCallUid = call->uid;
		if (autoHold)
			call->setHold(false);
	}
	else
	{
		currentCallUid = Call::INVALID_UID;
		buttons.UpdateBtnState(Button::HOLD, false);
		buttons.UpdateBtnState(Button::MUTE, false);
	}
}

void Calls::OnButtonConfigChange(void)
{
	ScopedLock<Mutex> lock(mutex);

    std::set<unsigned int> prevLineButtonIds = lineButtonIds;

	lineButtonIds.clear();
	for (unsigned int i=0; i<buttons.btnConf.size(); i++)
	{
		if (buttons.btnConf[i].type == Button::LINE)
		{
			lineButtonIds.insert(i);
		}
	}

	std::map<unsigned int, Call>::iterator iter;

	// disconnect current call if switching from single-call to multi-call configuration
	if (prevLineButtonIds.size() < 1 && lineButtonIds.size() > 1)
	{
		for (iter = entries.begin(); iter != entries.end(); ++iter)
		{
			Call &call = iter->second;
			call.disconnecting = true;
			UA->Hangup(call.uid);
		}
	}
	else
	{
		// check if any line button in use was removed, disconnect associated calls
		for (iter = entries.begin(); iter != entries.end(); ++iter)
		{
			Call &call = iter->second;
			if (call.btnId >= 0 && lineButtonIds.find(call.btnId) == lineButtonIds.end())
			{
				LOG("Disconnecting call %u because associated LINE button %u was removed\n", call.uid, call.btnId);
				call.disconnecting = true;
				UA->Hangup(call.uid);
			}
		}
	}
}

void Calls::SetHold(bool state)
{
	ScopedLock<Mutex> lock(mutex);
	for(std::map<unsigned int, Call>::iterator iter = entries.begin(); iter != entries.end(); ++iter)
	{
		iter->second.setHold(state);
	}
}
