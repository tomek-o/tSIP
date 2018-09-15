//---------------------------------------------------------------------------

#ifndef ContactsH
#define ContactsH
//---------------------------------------------------------------------------

#include "common/Observable.h"
#include <string>
#include <vector>
#include <System.hpp>

class Contacts: public Observable
{
public:
	struct Entry
	{
		AnsiString description;
		AnsiString company;
		AnsiString uri1;
		AnsiString uri2;
		AnsiString uri3;
		AnsiString note;

		bool operator==(const Entry& right) const {
			return (description == right.description &&
					company == right.company &&
					uri1 == right.uri1 &&
					uri2 == right.uri2 &&
					uri3 == right.uri3 &&
					note == right.note
					);
		}

		bool operator<(const Entry &other) const
		{
		   return description.UpperCase() < other.description.UpperCase();
		}

		AnsiString GetMainUri(void) const;
	};

private:
	std::vector<Entry> entries;
	AnsiString filename;
public:
	std::vector<Entry>& GetEntries(void)
	{
		return entries;
	}
	void SetFilename(AnsiString name)
	{
		filename = name;
	}
	int Read(void);
	int Write(void);
	void Update(void)
	{
		notifyObservers();
	}
	void Sort(void);
	Entry* GetEntry(AnsiString uri);
};

/** \brief Class of object passed to registered observers
*/
class ContactsNotifyArgument: public Argument
{
public:
	int dummy;
	//Contacts::Entry &entry;
};

#endif
