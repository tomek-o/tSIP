#include <vcl.h>
#pragma hdrstop

#include "SettingsAppVersion.h"
#include "common\Utilities.h"
#include <json/json.h>

void SettingsAppVersion::FromJson(const Json::Value &jv)
{
	if (jv.type() == Json::objectValue)
	{
		jv.getUInt("FileVersionMS", FileVersionMS);
		jv.getUInt("FileVersionLS", FileVersionLS);
	}
}

void SettingsAppVersion::ToJson(Json::Value &jv)
{
	jv["FileVersionMS"] = FileVersionMS;
	jv["FileVersionLS"] = FileVersionLS;

	AnsiString asVer;
	asVer.sprintf("%d.%02d.%02d.%02d", HIWORD(FileVersionMS), LOWORD(FileVersionMS), HIWORD(FileVersionLS), LOWORD(FileVersionLS));
	jv["FileVersionText"] = asVer;
}

void SettingsAppVersion::FromAppExe(void)
{
	GetFileVer(Application->ExeName, FileVersionMS, FileVersionLS);
}


