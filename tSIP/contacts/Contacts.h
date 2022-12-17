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
		AnsiString file;

		bool operator==(const Entry& right) const {
			return (description == right.description &&
					company == right.company &&
					uri1 == right.uri1 &&
					uri2 == right.uri2 &&
					uri3 == right.uri3 &&
					note == right.note &&
					file == right.file
					);
		}

		bool operator<(const Entry &other) const
		{
		   return (description.AnsiCompareIC(other.description) < 0);
		}

		AnsiString GetMainUri(void) const;
	};

private:
	std::vector<Entry> entries;
	AnsiString filename;
	FILETIME ft;
public:
	Contacts(void);
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
	int ReadXml(AnsiString name);
	void Update(void)
	{
		notifyObservers();
	}
	void Sort(void);
	Entry* GetEntry(AnsiString uri);
	FILETIME getFiletime(void)
	{
    	return ft;
	}
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
