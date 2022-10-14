//---------------------------------------------------------------------------


#pragma hdrstop

#include "Contacts.h"
#include "ContactsXmlImport.h"
#include "common/TelecomUtils.h"
#include "common/Utilities.h"
#include <SysUtils.hpp>
#include <assert.h>
#include <algorithm>
#include <fstream> 
#include <json/json.h>

//---------------------------------------------------------------------------

#pragma package(smart_init)

Contacts::Contacts(void)
{
	memset(&ft, 0, sizeof(ft));
}

int Contacts::Read(void)
{
	assert(filename != "");
	Json::Value root;   // will contains the root value after parsing.
	Json::Reader reader;

	entries.clear();

	GetFileWriteTime(filename, &ft);

	try
	{
		std::ifstream ifs(filename.c_str());
		std::string strConfig((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
		ifs.close();
		bool parsingSuccessful = reader.parse( strConfig, root );
		if ( !parsingSuccessful )
		{
			notifyObservers();		
			return 2;
		}
	}
	catch(...)
	{
		notifyObservers();
		return 1;
	}

	const Json::Value &contacts = root["Contacts"];
	entries.clear();
	for (int i=0; i<contacts.size(); i++)
	{
		const Json::Value &contact = contacts[i];
		struct Entry entry;

		entry.description = contact.get("description", "").asString().c_str();
		entry.company = contact.get("company", "").asString().c_str();
		entry.uri1 = contact.get("uri", "").asString().c_str();
		entry.uri2 = contact.get("uri2", "").asString().c_str();
		entry.uri3 = contact.get("uri3", "").asString().c_str();
		entry.note = contact.get("note", "").asString().c_str();
		entry.file = contact.get("file", entry.file.c_str()).asString().c_str();

		entries.push_back(entry);
	}

	notifyObservers();

	return 0;
}

int Contacts::Write(void)
{
	assert(filename != "");
    Json::Value root;
	Json::StyledWriter writer;
	Json::Value &jvContacts = root["Contacts"];
	for (unsigned int i=0; i<entries.size(); i++)
	{
		Entry &entry = entries[i];
		Json::Value &jv = jvContacts[i];
		jv["uri"] = entry.uri1.c_str();
		jv["uri2"] = entry.uri2.c_str();
		jv["uri3"] = entry.uri3.c_str();
		jv["description"] = entry.description.c_str();
		jv["company"] = entry.company.c_str();
		jv["note"] = entry.note.c_str();
		jv["file"] = entry.file.c_str();
	}

	std::string outputConfig = writer.write( root );

	try
	{
		std::ofstream ofs(filename.c_str());
		ofs << outputConfig;
		ofs.close();
		GetFileWriteTime(filename, &ft);		
	}
	catch(...)
	{
    	return 1;
	}
		
	return 0;
}

int Contacts::ReadXml(AnsiString name)
{
	int status = ContactsXmlImport(name, entries);
	if (status == 0)
	{
        Sort();
		Update();
	}
	return status;
}

#pragma warn -8091	// incorrectly issued by BDS2006
void Contacts::Sort(void)
{
	std::stable_sort(entries.begin(), entries.end());
}

Contacts::Entry* Contacts::GetEntry(AnsiString uri)
{
	if (uri.Length() == 0)
		return NULL;
		
	for (unsigned int i=0; i<entries.size(); i++)
	{
		Entry &entry = entries[i];
		if (entry.uri1 == uri || entry.uri2 == uri || entry.uri3 == uri)
		{
			return &entry;
		}
	}
	uri = ExtractNumberFromUri(uri);
	if (uri != "")
	{
		for (unsigned int i=0; i<entries.size(); i++)
		{
			Entry &entry = entries[i];
			if (entry.uri1 == uri || entry.uri2 == uri || entry.uri3 == uri)
			{
				return &entry;
			}
		}
	}
	return NULL;
}

AnsiString Contacts::Entry::GetMainUri(void) const
{
	if (uri1 != "")
		return uri1;
	else if (uri2 != "")
		return uri2;
	else
		return uri3;
}
