//---------------------------------------------------------------------------

#ifndef HistoryH
#define HistoryH
//---------------------------------------------------------------------------

#include "common/Observable.h"
#include <string>
#include <deque>
#include <System.hpp>

class History: public Observable
{
public:
	enum { CALL_HISTORY_LIMIT = 1000 };
	struct Entry
	{
		struct Timestamp
		{
			unsigned short year, month, day, hour, min, sec, msec;
			Timestamp(void):
				year(0), month(0), day(0), hour(0), min(0), sec(0), msec(0)
			{
			}
			bool operator==(const Timestamp& right) const {
				return (
					year == right.year &&
					month == right.month &&
					day == right.day &&
					hour == right.hour &&
					min == right.min &&
					sec == right.sec &&
					msec == right.msec
				);
			}
		} timestamp;
		AnsiString uri;
		AnsiString peerName;	///< display name
		AnsiString contactName;	///< name associated with contact; not stored in file, cached only after resolving

		AnsiString paiUri;
		AnsiString paiPeerName;
		AnsiString paiContactName;

		bool incoming;
		int time;	///< call time in seconds (starting from CONFIRMED state)

		AnsiString codecName;
		int lastScode;				///< last SIP code from received reply (on disconnected)
		AnsiString lastReplyLine;

		AnsiString recordFile;

		AnsiString reason;

		Entry(void):
			incoming(false),
			time(0),
			lastScode(0)
		{}

		bool operator==(const Entry& right) const {
			return (
				uri == right.uri &&
				paiUri == right.paiUri &&
				incoming == right.incoming &&
				timestamp == right.timestamp
				);
		}
	};

	History(void):
		callbackGetContactName(NULL),
		maxEntries(CALL_HISTORY_LIMIT)
	{
	}
	void AddEntry(Entry& entry);
	typedef AnsiString (__closure *CallbackGetContactName)(AnsiString uri);
private:
	std::deque<Entry> entries;
	AnsiString filename;
	CallbackGetContactName callbackGetContactName;
	unsigned int maxEntries;
public:
	const std::deque<Entry>& GetEntries(void) const
	{
		return entries;
	}
	void SetContactName(int id, AnsiString name)
	{
        entries[id].contactName = name.c_str();
    }
	void SetFilename(AnsiString name)
	{
		filename = name;
	}
	/** \brief Set maximum number of stored history entries; older entries beyond this
		count are dropped from AddEntry() and from Read() (a lowered limit takes effect
		only for the currently loaded/added entries, not retroactively rewriting the file
		until the next Write())
	*/
	void SetMaxEntries(unsigned int max)
	{
		maxEntries = max;
	}
	void Clear(void);
	int Read(CallbackGetContactName callbackGetContactName);
	int Write(void);
};

/** \brief Class of object passed to registered observers
*/
class HistoryNotifyArgument: public Argument
{
public:
	int dummy;
	//History::Entry &entry;
};

#endif
