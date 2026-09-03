//---------------------------------------------------------------------------

#include <vcl.h>

#pragma hdrstop

#include "MessageHistory.h"
#include "Log.h"
#include "common/base64.h"
#include "Settings.h"

#include <fstream>
#include <map>
#include <json/json.h>

//---------------------------------------------------------------------------

#pragma package(smart_init)

namespace MessageHistory
{

namespace
{

AnsiString asDir;

/** \brief Reduce a contact identifier to a canonical form so that e.g.
 *  "300", "300@myserver" and "sip:300@myserver" are all treated as the
 *  same contact for history storage/lookup: strips an optional display
 *  name/angle brackets, an optional "sip:"/"sips:" scheme and any URI
 *  parameters, then keeps the user part before "@". If there is no user
 *  part (e.g. the target is just a bare host/IP, or "user" is empty as in
 *  "sip:@192.168.1.5"), the host (IP, IP:PORT or domain) is used instead.
 */
AnsiString NormalizeContactId(AnsiString target)
{
	AnsiString s = target.Trim();

	int lt = s.Pos("<");
	if (lt > 0)
	{
		int gt = s.Pos(">");
		if (gt > lt)
		{
			s = s.SubString(lt + 1, gt - lt - 1);
		}
	}

	if (s.Pos("sips:") == 1)
	{
		s = s.SubString(6, s.Length() - 5);
	}
	else if (s.Pos("sip:") == 1)
	{
		s = s.SubString(5, s.Length() - 4);
	}

	int semi = s.Pos(";");
	if (semi > 0)
	{
		s = s.SubString(1, semi - 1);
	}

	int at = s.Pos("@");
	AnsiString user, host;
	if (at > 0)
	{
		user = s.SubString(1, at - 1);
		host = s.SubString(at + 1, s.Length() - at);
	}
	else
	{
		user = s;
	}

	if (user != "")
	{
		return user;
	}
	if (host != "")
	{
		return host;
	}
	return s;
}

AnsiString GetFilePath(AnsiString target)
{
	AnsiString key = NormalizeContactId(target);
	std::string b64 = base64_encode((unsigned char*)key.c_str(), key.Length(), BASE64_ALPHABET_FSAFE);
	AnsiString encoded = b64.c_str();
	if (encoded.Length() > 200)
	{
		encoded = encoded.SubString(1, 200);
	}
	if (encoded.Length() == 0)
	{
		encoded = "_";
	}
	return asDir + "\\" + encoded + ".ndjson";
}

/** \brief Read every entry from the given contact's history file, oldest first.
 *  Only ever called once per contact (by GetCachedEntries()) - further
 *  access goes through the in-memory cache to avoid re-reading/re-parsing
 *  the whole file on every LoadLast()/LoadOlder() call (which would make
 *  paging through a long history an O(n^2) disk-and-parse operation).
 */
std::vector<Entry> ReadAllEntriesFromDisk(AnsiString target)
{
	std::vector<Entry> result;

	try
	{
		std::ifstream ifs(GetFilePath(target).c_str());
		if (ifs.is_open())
		{
			Json::Reader reader;
			std::string line;
			while (std::getline(ifs, line))
			{
				if (line.empty())
				{
					continue;
				}
				Json::Value root;
				if (!reader.parse(line, root))
				{
					continue;
				}
				Entry entry;
				entry.time = (time_t)root.get("time", 0).asInt();
				entry.incoming = root.get("incoming", false).asBool();
				entry.contentType = root.get("contentType", "text/plain").asString().c_str();
				entry.body = root.get("body", "").asString().c_str();

				result.push_back(entry);
			}
		}
	}
	catch (...)
	{
		LOG("MessageHistory: failed to load history for %s\n", target.c_str());
	}

	return result;
}

std::map<AnsiString, std::vector<Entry> > cache;	///< key: NormalizeContactId(target)

/** \brief Look up (populating from disk on first access) the cached, in-memory
 *  entry list for a contact. Kept in sync incrementally by Append() so that,
 *  after the first read, subsequent calls never touch disk again.
 */
std::vector<Entry>& GetCachedEntries(AnsiString target)
{
	AnsiString key = NormalizeContactId(target);
	std::map<AnsiString, std::vector<Entry> >::iterator it = cache.find(key);
	if (it == cache.end())
	{
		it = cache.insert(std::make_pair(key, ReadAllEntriesFromDisk(target))).first;
	}
	return it->second;
}

}	// namespace

void SetDir(AnsiString dir)
{
	asDir = dir;
	ForceDirectories(asDir);
	cache.clear();
}

void Append(AnsiString target, bool incoming, AnsiString contentType, AnsiString utf8Body)
{
	if (asDir == "" || !appSettings.uaConf.messages.historyEnabled)
	{
		return;
	}

	Entry entry;
	entry.time = time(NULL);
	entry.incoming = incoming;
	entry.contentType = contentType;
	entry.body = utf8Body;

	Json::Value jEntry;
	jEntry["time"] = (int)entry.time;
	jEntry["incoming"] = entry.incoming;
	jEntry["contentType"] = entry.contentType.c_str();
	jEntry["body"] = entry.body.c_str();

	Json::FastWriter writer;
	std::string line = writer.write(jEntry);	// FastWriter already appends a trailing '\n'

	try
	{
		std::ofstream ofs(GetFilePath(target).c_str(), std::ios::app | std::ios::binary);
		ofs << line;
		ofs.close();
		if (!ofs)
		{
			LOG("MessageHistory: failed to write message to history file for %s (disk full/inaccessible?)\n", target.c_str());
		}
	}
	catch (...)
	{
		LOG("MessageHistory: failed to append message to history file for %s\n", target.c_str());
	}

	/* keep the in-session view consistent even if the write above failed;
	 * the message was genuinely sent/received, only its persistence is in doubt */
	GetCachedEntries(target).push_back(entry);
}

std::vector<Entry> LoadLast(AnsiString target, unsigned int count)
{
	if (!appSettings.uaConf.messages.historyEnabled)
	{
		return std::vector<Entry>();
	}

	const std::vector<Entry> &all = GetCachedEntries(target);
	if (all.size() <= count)
	{
		return all;
	}
	return std::vector<Entry>(all.end() - count, all.end());
}

std::vector<Entry> LoadOlder(AnsiString target, unsigned int alreadyLoaded, unsigned int count)
{
	if (!appSettings.uaConf.messages.historyEnabled)
	{
		return std::vector<Entry>();
	}

	const std::vector<Entry> &all = GetCachedEntries(target);
	int total = (int)all.size();
	int endIdx = total - (int)alreadyLoaded;
	if (endIdx <= 0)
	{
		return std::vector<Entry>();
	}
	int startIdx = endIdx - (int)count;
	if (startIdx < 0)
	{
		startIdx = 0;
	}
	return std::vector<Entry>(all.begin() + startIdx, all.begin() + endIdx);
}

}	// namespace MessageHistory
