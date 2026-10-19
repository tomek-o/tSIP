//---------------------------------------------------------------------------

#pragma hdrstop

#include "Contacts.h"
#include "ContactsXmlImport.h"
#include "common/TelecomUtils.h"
#include "common/Utilities.h"
#include "Settings.h"
#include <SysUtils.hpp>
#include <assert.h>
#include <algorithm>
#include <fstream> 
#include <json/json.h>

//---------------------------------------------------------------------------

#pragma package(smart_init)

namespace
{
	/** \brief Quote a CSV field if it contains the separator, a quote, or a
	 *  newline, doubling any embedded quotes (RFC 4180 style, matching what
	 *  FormContactsCsvImport's parser understands).
	 */
	AnsiString CsvQuote(AnsiString field)
	{
		bool needsQuote = field.Pos(",") > 0 || field.Pos("\"") > 0 ||
			field.Pos("\r") > 0 || field.Pos("\n") > 0;
		if (!needsQuote)
			return field;

		AnsiString result = "\"";
		for (int i = 1; i <= field.Length(); i++)
		{
			if (field[i] == '"')
				result += "\"\"";
			else
				result += field[i];
		}
		result += "\"";
		return result;
	}
}

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
		entry.ring = contact.get("ring", "").asString().c_str();
		{
			const Json::Value &jvAccount = contact["account"];
			if (jvAccount.isInt())
				entry.accountId = jvAccount.asInt();
		}

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
		// written only when set, to not grow the file for every contact
		if (entry.ring != "")
		{
			jv["ring"] = entry.ring.c_str();
		}
		if (entry.accountId >= 0)
		{
			jv["account"] = entry.accountId;
		}
	}

	std::string outputConfig = writer.write( root );

	try
	{
		std::ofstream ofs(filename.c_str());
		if (!ofs.is_open())
		{
			return 1;
		}
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

int Contacts::WriteCsv(AnsiString fileName)
{
	try
	{
		std::ofstream ofs(fileName.c_str(), std::ios::out | std::ios::binary);
		if (!ofs.is_open())
		{
			return 1;
		}

		ofs << "Description,Number1,Number2,Number3,Company,Note\r\n";
		for (unsigned int i=0; i<entries.size(); i++)
		{
			Entry &entry = entries[i];
			ofs << CsvQuote(entry.description).c_str() << ","
			    << CsvQuote(entry.uri1).c_str() << ","
			    << CsvQuote(entry.uri2).c_str() << ","
			    << CsvQuote(entry.uri3).c_str() << ","
			    << CsvQuote(entry.company).c_str() << ","
			    << CsvQuote(entry.note).c_str() << "\r\n";
		}
		ofs.close();
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

namespace
{

/** Digits of a phone number and whether it has an international prefix (+CC).
    Spaces, dashes and other separators are ignored. */
bool PhoneDigits(const AnsiString &num, std::string &digits, bool &international)
{
	digits.clear();
	international = false;
	for (int i = 1; i <= num.Length(); i++)
	{
		const char c = num[i];
		if (c >= '0' && c <= '9')
			digits += c;
		else if (c == '+' && digits.empty() && !international)
			international = true;
	}
	return !digits.empty();
}

/** "+CC rest" (international) against "0 rest" (national): the country code has 1 to 3 digits,
    so every split is tried. E.g. +49 6241 123456789 matches 06241-123456789. */
bool InternationalMatchesNational(const std::string &intl, const std::string &national)
{
	if (national.size() < 2 || national[0] != '0')
		return false;
	const std::string rest = national.substr(1);
	for (size_t cc = 1; cc <= 3 && cc < intl.size(); cc++)
	{
		if (intl.size() - cc == rest.size() && intl.compare(cc, std::string::npos, rest) == 0)
			return true;
	}
	return false;
}

bool SameNumber(const std::string &a, bool aIntl, const std::string &b, bool bIntl)
{
	if (aIntl == bIntl)
		return a == b;
	return aIntl ? InternationalMatchesNational(a, b) : InternationalMatchesNational(b, a);
}

}	// namespace

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
	const AnsiString original = uri;
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
	// phone numbers in different notation: +CC with spaces (incoming) against national 0 with dashes (contact)
	std::string inDigits;
	bool inIntl = false;
	if (appSettings.Contacts.fuzzyNumberMatch && PhoneDigits(ExtractNumberFromUri(original), inDigits, inIntl))
	{
		for (unsigned int i=0; i<entries.size(); i++)
		{
			Entry &entry = entries[i];
			const AnsiString *uris[] = { &entry.uri1, &entry.uri2, &entry.uri3 };
			for (unsigned int u=0; u<3; u++)
			{
				std::string cDigits;
				bool cIntl = false;
				if (PhoneDigits(ExtractNumberFromUri(*uris[u]), cDigits, cIntl) &&
					SameNumber(inDigits, inIntl, cDigits, cIntl))
				{
					return &entry;
				}
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
