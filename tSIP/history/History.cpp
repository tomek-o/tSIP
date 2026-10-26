//---------------------------------------------------------------------------


#pragma hdrstop

#include "History.h"
#include "Branding.h"
#include <assert.h>
#include <algorithm>
#include <fstream> 
#include <json/json.h>

//---------------------------------------------------------------------------

#pragma package(smart_init)


void History::AddEntry(History::Entry& entry)
{
	std::deque<Entry>::iterator iter;
	for (iter = entries.begin(); iter != entries.end(); ++iter)
	{
		if (*iter == entry)
		{
			entries.erase(iter);
			break;
		}
	}
	assert(callbackGetContactName);
	entry.contactName = callbackGetContactName(entry.uri.c_str()).c_str();
	entry.paiContactName = callbackGetContactName(entry.paiUri.c_str()).c_str();
	entries.push_front(entry);
	if (entries.size() > maxEntries)
	{
		entries.pop_back();
	}
	notifyObservers();	
}

void History::RefreshContactNames(void)
{
	if (!callbackGetContactName)
		return;
	for (std::deque<Entry>::iterator iter = entries.begin(); iter != entries.end(); ++iter)
	{
		iter->contactName = callbackGetContactName(iter->uri.c_str()).c_str();
		iter->paiContactName = callbackGetContactName(iter->paiUri.c_str()).c_str();
	}
	notifyObservers();
}

void History::Clear(void)
{
	entries.clear();
	notifyObservers();
}

int History::Read(CallbackGetContactName callbackGetContactName)
{
	assert(filename != "");
	Json::Value root;   // will contains the root value after parsing.
	Json::Reader reader;

	assert(callbackGetContactName);
	this->callbackGetContactName = callbackGetContactName;

	try
	{
		std::ifstream ifs(filename.c_str());
		std::string strConfig((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
		ifs.close();
		bool parsingSuccessful = reader.parse( strConfig, root );
		if ( !parsingSuccessful )
		{
			return 2;
		}
	}
	catch(...)
	{
		return 1;
	}

	const Json::Value &callHistory = root["CallHistory"];
	entries.clear();
	for (int i=0; i<std::min(maxEntries, callHistory.size()); i++)
	{
		const Json::Value &call = callHistory[i];
		struct Entry entry;

		entry.uri = call.get("uri", "").asString().c_str();
		entry.peerName = call.get("peerName", "").asString().c_str();
		entry.contactName = callbackGetContactName(entry.uri.c_str()).c_str();

		entry.paiUri = call.get("paiUri", "").asString().c_str();
		entry.paiPeerName = call.get("paiPeerName", "").asString().c_str();
		entry.paiContactName = callbackGetContactName(entry.paiUri.c_str()).c_str();

		entry.incoming = call.get("incoming", "").asBool();
		entry.time = call.get("time", 0).asInt();

		entry.codecName = call.get("codecName", "").asString().c_str();

		call.getInt("lastScode", entry.lastScode);
		call.getInt("accountId", entry.accountId);
		call.getAString("lastReplyLine", entry.lastReplyLine);

		const Json::Value &ts = call["timestamp"];
		entry.timestamp.year = ts.get("year", 0).asInt();
		entry.timestamp.month = ts.get("month", 0).asInt();
		entry.timestamp.day = ts.get("day", 0).asInt();
		entry.timestamp.hour = ts.get("hour", 0).asInt();
		entry.timestamp.min = ts.get("min", 0).asInt();
		entry.timestamp.sec = ts.get("sec", 0).asInt();
		entry.timestamp.msec = ts.get("msec", 0).asInt();

		call.getAString("recordFile", entry.recordFile);

		call.getAString("reason", entry.reason);

		const Json::Value &jr = call["rtp"];
		if (jr.type() == Json::objectValue)
		{
			RtpSummary &r = entry.rtp;
			r.valid = true;
			r.txPackets = jr.get("txPackets", 0).asUInt();
			r.rxPackets = jr.get("rxPackets", 0).asUInt();
			r.txBytes = jr.get("txBytes", 0).asUInt();
			r.rxBytes = jr.get("rxBytes", 0).asUInt();
			r.rtcpValid = jr.get("rtcpValid", false).asBool();
			r.rtcpRxLost = jr.get("rtcpRxLost", 0).asInt();
			r.rtcpTxLost = jr.get("rtcpTxLost", 0).asInt();
			r.rtcpRxJitterUs = jr.get("rtcpRxJitterUs", 0).asUInt();
			r.rtcpTxJitterUs = jr.get("rtcpTxJitterUs", 0).asUInt();
			r.jbufValid = jr.get("jbufValid", false).asBool();
			r.jbufLate = jr.get("jbufLate", 0).asUInt();
			r.jbufLost = jr.get("jbufLost", 0).asUInt();
			r.jbufOos = jr.get("jbufOos", 0).asUInt();
			r.jbufDups = jr.get("jbufDups", 0).asUInt();
			r.jbufOverflow = jr.get("jbufOverflow", 0).asUInt();
			r.jbufUnderflow = jr.get("jbufUnderflow", 0).asUInt();
			r.jbufFlush = jr.get("jbufFlush", 0).asUInt();
			r.rxBufUnderrun = jr.get("rxBufUnderrun", 0).asUInt();
			r.rxBufOverrun = jr.get("rxBufOverrun", 0).asUInt();
			r.txBufUnderrun = jr.get("txBufUnderrun", 0).asUInt();
			r.txBufOverrun = jr.get("txBufOverrun", 0).asUInt();
			r.txBurst = jr.get("txBurst", 0).asUInt();
			r.txLate = jr.get("txLate", 0).asUInt();
			r.txMaxGapMs = jr.get("txMaxGapMs", 0).asUInt();
		}

		entries.push_back(entry);
	}

	notifyObservers();

	return 0;
}

int History::Write(void)
{
	assert(filename != "");
    Json::Value root;
	Json::StyledWriter writer;
	Json::Value &jCallHistory = root["CallHistory"];
	jCallHistory.resize(0);
	for (unsigned int i=0; i<entries.size(); i++)
	{
		Entry &entry = entries[i];
		Json::Value &jEntry = jCallHistory[i];

		jEntry["uri"] = entry.uri.c_str();
		jEntry["peerName"] = entry.peerName.c_str();

		jEntry["paiUri"] = entry.paiUri.c_str();
		jEntry["paiPeerName"] = entry.paiPeerName.c_str();

		jEntry["codecName"] = entry.codecName.c_str();

		jEntry["lastScode"] = entry.lastScode;
		jEntry["accountId"] = entry.accountId;
		jEntry["lastReplyLine"] = entry.lastReplyLine;

		jEntry["incoming"] = entry.incoming;
		jEntry["time"] = entry.time;
		jEntry["timestamp"]["year"] = entry.timestamp.year;
		jEntry["timestamp"]["month"] = entry.timestamp.month;
		jEntry["timestamp"]["day"] = entry.timestamp.day;
		jEntry["timestamp"]["hour"] = entry.timestamp.hour;
		jEntry["timestamp"]["min"] = entry.timestamp.min;
		jEntry["timestamp"]["sec"] = entry.timestamp.sec;
		jEntry["timestamp"]["msec"] = entry.timestamp.msec;
		if (Branding::recording)
		{
			jEntry["recordFile"] = entry.recordFile;
		}
		jEntry["reason"] = entry.reason;
		if (entry.rtp.valid)
		{
			const RtpSummary &r = entry.rtp;
			Json::Value &jr = jEntry["rtp"];
			jr["txPackets"] = r.txPackets;
			jr["rxPackets"] = r.rxPackets;
			jr["txBytes"] = r.txBytes;
			jr["rxBytes"] = r.rxBytes;
			if (r.rtcpValid)
			{
				jr["rtcpValid"] = true;
				jr["rtcpRxLost"] = r.rtcpRxLost;
				jr["rtcpTxLost"] = r.rtcpTxLost;
				jr["rtcpRxJitterUs"] = r.rtcpRxJitterUs;
				jr["rtcpTxJitterUs"] = r.rtcpTxJitterUs;
			}
			if (r.jbufValid)
			{
				jr["jbufValid"] = true;
				jr["jbufLate"] = r.jbufLate;
				jr["jbufLost"] = r.jbufLost;
				jr["jbufOos"] = r.jbufOos;
				jr["jbufDups"] = r.jbufDups;
				jr["jbufOverflow"] = r.jbufOverflow;
				jr["jbufUnderflow"] = r.jbufUnderflow;
				jr["jbufFlush"] = r.jbufFlush;
			}
			jr["rxBufUnderrun"] = r.rxBufUnderrun;
			jr["rxBufOverrun"] = r.rxBufOverrun;
			jr["txBufUnderrun"] = r.txBufUnderrun;
			jr["txBufOverrun"] = r.txBufOverrun;
			jr["txBurst"] = r.txBurst;
			jr["txLate"] = r.txLate;
			jr["txMaxGapMs"] = r.txMaxGapMs;
		}
	}

	std::string outputConfig = writer.write( root );

	try
	{
		std::ofstream ofs(filename.c_str());
		ofs << outputConfig;
		ofs.close();
	}
	catch(...)
	{
    	return 1;
	}
		
	return 0;
}


