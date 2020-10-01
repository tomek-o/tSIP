#pragma hdrstop

#include "Translate.h"
#include "Paths.h"
#include "Log.h"
#include <json/json.h>
#include <fstream>
#include <map>

#pragma package(smart_init)

namespace
{
	std::map<AnsiString, AnsiString> translations;
	std::map<void*, TranslationCb> translationCallbacks;
	AnsiString asDir;
	bool logMissingKeysFlag = false;

	/** Handle nested i18n translations recursively.
		When tested i18n-editor was converting existing keys with dot notation to nested objects.
		Other editor, i18n Manager was keeping existing dot notation in key names.
	*/
	void CollectTranslations(const Json::Value &jvalue, AnsiString prefix)
	{
		Json::Value::Members members( jvalue.getMemberNames() );
		for ( Json::Value::Members::iterator it = members.begin(); it != members.end(); ++it )
		{
			const char* name = (*it).c_str();
			const Json::Value &jv = jvalue[name];
			if (jv.type() == Json::stringValue)
			{
				AnsiString text = ::Utf8ToAnsi(jv.asCString());
				translations[prefix + name] = text;
			}
			else if (jv.type() == Json::objectValue)
			{
				CollectTranslations(jv, prefix + name + ".");
			}
		}
	}
}

std::vector<AnsiString> EnumerateTranslations(void)
{
    AnsiString dir = Paths::GetProfileDir();
	if (dir[dir.Length()] != '\\')
		dir += "\\";
	dir += "translations\\";
	asDir = dir;
	WIN32_FIND_DATA file;
	AnsiString asSrchPath = dir + "*";
	HANDLE hFind = FindFirstFile(asSrchPath.c_str(), &file);
	int hasfiles = (hFind != INVALID_HANDLE_VALUE);

    std::vector<AnsiString> ret;

	while (hasfiles)
	{
		AnsiString name = file.cFileName;
		if ((file.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && name != "." && name != "..")
		{
			AnsiString fullName = dir + name + "\\translations.json";
			if (FileExists(fullName))
			{
				ret.push_back(name);
			}
		}
		hasfiles = FindNextFile(hFind, &file);
	}
	FindClose(hFind);
	return ret;
}

int LoadTranslations(AnsiString name, bool logMissingKeys)
{
    logMissingKeysFlag = logMissingKeys;
	translations.clear();
	if (name == "")
	{
		return -1;
	}
	LOG("Translate: language = [%s]\n", name.c_str());

    AnsiString dir = Paths::GetProfileDir();
	if (dir[dir.Length()] != '\\')
		dir += "\\";
	AnsiString filename = dir + "translations\\" + name + "\\translations.json";

	Json::Value root;   // will contains the root value after parsing.
	Json::Reader reader;

	try
	{
		std::ifstream ifs(filename.c_str());
		std::string strTranslations((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
		ifs.close();
		bool parsingSuccessful = reader.parse( strTranslations, root );
		if ( !parsingSuccessful )
		{
			//notifyObservers();		
			return 2;
		}
		else
		{
			if (root.type() == Json::objectValue)
			{
				CollectTranslations(root, "");
			}
			std::map<void*, TranslationCb>::iterator iter;
			for (iter = translationCallbacks.begin(); iter != translationCallbacks.end(); ++iter)
			{
            	iter->second(iter->first);
			}
			return 0;
		}
	}
	catch(...)
	{
		//notifyObservers();
		return 1;
	}
}

void RegisterTranslationCb(void* obj, TranslationCb cb)
{
	translationCallbacks[obj] = cb;
	cb(obj);	// translate immediately
}

void UnregisterTranslationCb(void* obj)
{
	translationCallbacks.erase(obj);
}

void Translate(const char* key, AnsiString& text)
{
	std::map<AnsiString, AnsiString>::iterator iter;
	iter = translations.find(key);
	if (iter != translations.end())
	{
		text = iter->second;
	}
	else
	{
		if (logMissingKeysFlag)
		{
			LOG("Translate: missing key [%s]\n", key);
		}
	}
}

AnsiString Translate2(const char* key, AnsiString defaultText)
{
	std::map<AnsiString, AnsiString>::iterator iter;
	iter = translations.find(key);
	if (iter != translations.end())
	{
		return iter->second;
	}
	else
	{
		if (logMissingKeysFlag)
		{
			LOG("Translate: missing key [%s]\n", key);
		}
	}	
	return defaultText;
}


