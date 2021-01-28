//---------------------------------------------------------------------------


#pragma hdrstop

#include "SpeedDialStatus.h"
#include "common/ScopedLock.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

SpeedDialStatus speedDialStatus;

void SpeedDialStatus::ClearEntries(void)
{
	newEntries.clear();
}

void SpeedDialStatus::AddEntry(enum EntryType type, AnsiString msg)
{
	struct Entry entry;
	entry.type = type;
	entry.msg = msg;
	newEntries.push_back(entry);
}

void SpeedDialStatus::Update(void)
{
	ScopedLock<Mutex> lock(mutex);
	entries = newEntries;
	notifyObservers();
}

const std::vector<struct SpeedDialStatus::Entry>& SpeedDialStatus::GetEntries(void)
{
	ScopedLock<Mutex> lock(mutex);
	return entries;
}

int SpeedDialStatus::GetMaximumLevel(void)
{
	ScopedLock<Mutex> lock(mutex);
	int level = -1;
	for (unsigned int i=0; i<entries.size(); i++)
	{
		if (level < entries[i].type)
		{
			level = entries[i].type;
		}
	}
	return level;
}


