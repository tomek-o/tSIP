//---------------------------------------------------------------------------


#pragma hdrstop

#include "Calls.h"
#include "buttons/ProgrammableButtons.h"
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
	unsigned int nextUid = 1;
	std::map<unsigned int, Call> entries;
	Mutex mutex;

	Call previousCall;

	std::set<unsigned int> lineButtonIds;	
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
		if (nextUid == 0)
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

Call* Calls::GetCurrentCall(void)
{
	ScopedLock<Mutex> lock(mutex);
    int TODO__GET_CURRENT_CALL;
	if (!entries.empty())
		return &entries.begin()->second;
	return NULL;
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

void Calls::OnLineButtonClick(int id, TProgrammableButton* btn)
{
	ScopedLock<Mutex> lock(mutex);

}

void Calls::OnButtonConfigChange(void)
{
	ScopedLock<Mutex> lock(mutex);
	lineButtonIds.clear();
	for (unsigned int i=0; i<buttons.btnConf.size(); i++)
	{
		if (buttons.btnConf[i].type == Button::LINE)
		{
			lineButtonIds.insert(i);
		}
	}

	// check if any line button in use was removed, disconnect associated calls

    int TODO__MORE_FISHY_CASES_LIKE_SINGLE_BUTTON;	// + removing all buttons; + adding new buttons, switching from single call to multi-call

	std::map<unsigned int, Call>::iterator iter;
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

