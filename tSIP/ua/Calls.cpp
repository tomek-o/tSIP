//---------------------------------------------------------------------------


#pragma hdrstop

#include "Calls.h"
#include <map>

//---------------------------------------------------------------------------

#pragma package(smart_init)

namespace {
	int nextUid = 1;
	std::map<int, Call> entries;
}

Call* Calls::Alloc(void)
{
	Call newCall;
	bool colliding = false;
	do
	{
		newCall.uid = nextUid;
		nextUid++;
		if (nextUid <= 0)
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

Call* Calls::FindByUid(int uid)
{
	std::map<int, Call>::iterator iter;
	iter = entries.find(uid);
	if (iter == entries.end())
		return NULL;
	else
		return &iter->second;
}

Call* Calls::GetFirstCall(void)
{
	if (!entries.empty())
		return &entries.begin()->second;
	return NULL;
}

Call* Calls::GetCurrentCall(void)
{
    int TODO__GET_CURRENT_CALL;
	if (!entries.empty())
		return &entries.begin()->second;
	return NULL;
}

Call* Calls::GetCurrentCall(void)
{
    int TODO__GET_PREVIOUS_CALL;
	if (!entries.empty())
		return &entries.begin()->second;
	return NULL;
}

void Calls::Clear(void)
{
	entries.clear();
}

void Calls::RemoveByUid(int uid)
{
	entries.erase(uid);
}

std::vector<int> Calls::GetUids(void)
{
	std::vector<int> ret;
	std::map<int, Call>::iterator iter;
	for (iter = entries.begin(); iter != entries.end(); ++iter)
	{
		ret.push_back(iter->first);
	}
	return ret;
}



