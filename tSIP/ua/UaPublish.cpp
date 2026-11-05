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
const unsigned int RETRY_FIRST_MS = 30000;
const unsigned int RETRY_MAX_MS = 300000;
/** the reply (or the SIP transaction timeout, ~32 s) comes before; longer: the request is considered lost */
const unsigned int IN_FLIGHT_MAX_MS = 70000;

/** Time since a tick (GetTickCount, 32 bit): correct also when the counter wraps (every 49.7 days) */
unsigned int Elapsed(unsigned long long nowMs, unsigned long long thenMs)
{
	return static_cast<unsigned int>(nowMs - thenMs);
}
}

void Request(int accountId, bool open, const std::string &note)
{
	ScopedLock<Mutex> lock(mutex);
	Presence &p = presences[accountId];
	p.active = true;
	p.open = open;
	p.note = note;
	p.lastSentMs = 0;
	p.retryMs = 0;
	p.failures = 0;
}

void Withdraw(int accountId)
{
	ScopedLock<Mutex> lock(mutex);
	std::map<int, Presence>::iterator it = presences.find(accountId);
	if (it != presences.end())
	{
		it->second.active = false;
		it->second.lastSentMs = 0;
		it->second.retryMs = 0;
		it->second.failures = 0;
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
	{
		Presence &p = it->second;
		// replies of earlier requests did not come for that long: they are lost, not outstanding
		if (p.inFlight > 0 && p.lastSentMs != 0 && Elapsed(nowMs, p.lastSentMs) >= IN_FLIGHT_MAX_MS)
			p.inFlight = 0;
		p.inFlight++;
		p.lastSentMs = nowMs ? nowMs : 1;
	}
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
		if (p.inFlight > 0 && p.lastSentMs != 0 && Elapsed(nowMs, p.lastSentMs) < IN_FLIGHT_MAX_MS)
			continue;
		const unsigned long long waitMs = p.retryMs ? p.retryMs : refreshMs;
		if (p.lastSentMs == 0 || Elapsed(nowMs, p.lastSentMs) >= waitMs)
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
	if (p.inFlight > 0)
		p.inFlight--;
	const bool success = (scode >= 200 && scode < 300);
	if (!success)
	{
		const bool hadEtag = !p.etag.empty();
		// the next PUBLISH is sent without SIP-If-Match
		p.etag = "";
		if (!p.active)
			return;		// failed unpublish: not retried
		if (scode == 412 && hadEtag)
		{
			// stale SIP-ETag (the server lost the publication): publish again at once
			p.failures = 0;
			p.retryMs = 0;
			p.lastSentMs = 0;
			return;
		}
		// the failed request counts as sent: retried after a growing delay (30 s ... 5 min), not every tick
		if (p.failures < 8)
			p.failures++;
		unsigned int delayMs = RETRY_FIRST_MS << (p.failures - 1);
		if (delayMs > RETRY_MAX_MS)
			delayMs = RETRY_MAX_MS;
		p.retryMs = delayMs;
		// counted from the failure, not from sending (a timeout reply comes ~30 s after the request)
		p.lastSentMs = GetTickCount();
		if (p.lastSentMs == 0)
			p.lastSentMs = 1;
		return;
	}
	p.failures = 0;
	p.retryMs = 0;
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
