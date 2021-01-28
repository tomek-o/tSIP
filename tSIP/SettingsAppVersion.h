//---------------------------------------------------------------------------

#ifndef SettingsAppVersionH
#define SettingsAppVersionH
//---------------------------------------------------------------------------

namespace Json
{
	class Value;
}

struct SettingsAppVersion
{
	unsigned int FileVersionMS;
	unsigned int FileVersionLS;
	bool operator<(const SettingsAppVersion &other) const
	{
	   if (FileVersionMS < other.FileVersionMS)
	   {
		   return true;
	   }
	   else if (other.FileVersionMS == FileVersionMS && FileVersionLS < other.FileVersionLS)
	   {
		   return true;
	   }
	   else
	   {
		   return false;
	   }
	}
	SettingsAppVersion(void)
	{
		FileVersionMS = 0;
		FileVersionLS = 0;
	}
	void FromJson(const Json::Value &jv);
	void ToJson(Json::Value &jv);
	void FromAppExe(void);
};					

#endif
