//---------------------------------------------------------------------------

#ifndef SpeedDialStatusH
#define SpeedDialStatusH
//---------------------------------------------------------------------------
#include "common/Observable.h"
#include <windows.h>
#include "common/Mutex.h"
#include <vector>
#include <System.hpp>

class SpeedDialStatus: public Observable
{
public:
	enum EntryType {
		INFO = 0,
		WARN,
		ERR
	};
	void ClearEntries(void);
	void AddEntry(enum EntryType type, AnsiString msg);
	void Update(void);
	int GetMaximumLevel(void);

	struct Entry {
		enum EntryType type;
		AnsiString msg;
	};
	const std::vector<struct Entry>& GetEntries(void);
private:
	Mutex mutex;
	std::vector<struct Entry> entries;
	std::vector<struct Entry> newEntries;
};

/** \brief Class of object passed to registered observers
*/
class SpeedDialStatusNotifyArgument: public Argument
{
public:
	int dummy;
};

extern SpeedDialStatus speedDialStatus;

#endif
