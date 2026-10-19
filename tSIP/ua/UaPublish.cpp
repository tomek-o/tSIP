//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "UaPublish.h"
#include "common/Mutex.h"
#include "common/ScopedLock.h"
#include <map>

//---------------------------------------------------------------------------

#pragma package(smart_init)

namespace UaPublish
{

namespace
{
Mutex mutex;
std::map<int, Presence> presences;
}

void Request(int accountId, bool open, const std::string &note)
{
	ScopedLock<Mutex> lock(mutex);
	Presence &p = presences[accountId];
	p.active = true;
	p.open = open;
	p.note = note;
	p.lastSentMs = 0;
}

void Withdraw(int accountId)
{
	ScopedLock<Mutex> lock(mutex);
	std::map<int, Presence>::iterator it = presences.find(accountId);
	if (it != presences.end())
	{
		it->second.active = false;
		it->second.lastSentMs = 0;
	}
}

bool Get(int accountId, Presence &out)
{
	ScopedLock<Mutex> lock(mutex);
	std::map<int, Presence>::const_iterator it = presences.find(accountId);
	if (it == presences.end())
		return false;
	out = it->second;
	return true;
}

void SentAt(int accountId, unsigned long long nowMs)
{
	ScopedLock<Mutex> lock(mutex);
	std::map<int, Presence>::iterator it = presences.find(accountId);
	if (it != presences.end())
		it->second.lastSentMs = nowMs ? nowMs : 1;
}

std::vector<int> Due(unsigned long long nowMs)
{
	ScopedLock<Mutex> lock(mutex);
	std::vector<int> due;
	for (std::map<int, Presence>::const_iterator it = presences.begin(); it != presences.end(); ++it)
	{
		const Presence &p = it->second;
		if (!p.active)
			continue;
		// refresh at 5/6 of the granted expiry, so the publication does not lapse
		const unsigned long long refreshMs = static_cast<unsigned long long>(p.expires) * 1000 * 5 / 6;
		if (p.lastSentMs == 0 || nowMs - p.lastSentMs >= refreshMs)
			due.push_back(it->first);
	}
	return due;
}

void Reply(int accountId, int scode, const std::string &etag, unsigned int expires)
{
	ScopedLock<Mutex> lock(mutex);
	std::map<int, Presence>::iterator it = presences.find(accountId);
	if (it == presences.end())
		return;
	Presence &p = it->second;
	const bool success = (scode >= 200 && scode < 300);
	if (!success)
	{
		// the next PUBLISH is sent without SIP-If-Match
		// the failed request counts as sent: retried at the next refresh interval, not every tick
		p.etag = "";
		return;
	}
	if (!p.active)
	{
		// unpublished: nothing to replace any more
		p.etag = "";
		return;
	}
	if (!etag.empty())
		p.etag = etag;
	if (expires > 0)
		p.expires = expires;
}

}
