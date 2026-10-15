//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "RecordingFile.h"
#include "Settings.h"
#include "Paths.h"
#include "common/TelecomUtils.h"
#include "common/base64.h"
#include <string>

//---------------------------------------------------------------------------

#pragma package(smart_init)

AnsiString RecordingFile::GetDir(void)
{
	AnsiString dir;
	if (appSettings.uaConf.recording.recDir == UaConf::RecordingCfg::RecDirRelative)
	{
		dir = Paths::GetProfileDir() + "\\recordings\\";
	}
	else
	{
		dir = appSettings.uaConf.recording.customRecDir.c_str();
		if (dir[dir.Length()] != '\\')
			dir += "\\";
	}
	if (!ForceDirectories(dir))
		return "";
	return dir;
}

AnsiString RecordingFile::MakeBaseName(bool incoming, AnsiString peerUri, AnsiString callUri)
{
	AnsiString dir = GetDir();
	if (dir == "")
		return "";

	AnsiString file = dir;
	file += FormatDateTime("yyyymmdd_hhnnss", Now());
	file += incoming ? "_1_" : "_0_";
	// let's limit URI/number length to some reasonable value
	AnsiString uri;
	if (incoming)
	{
		uri = ExtractNumberFromUri(peerUri);
	}
	else
	{
		uri = callUri;
	}
	uri = uri.SubString(1, 32);
	if (appSettings.uaConf.recording.noNumberB64Encoding == false)
	{
		std::string b64uri = base64_encode((unsigned char*)uri.c_str(), uri.Length(), BASE64_ALPHABET_FSAFE);
		file += b64uri.c_str();
	}
	else
	{
		/// \note There are more invalid characters (<>:"/\|?*) - file creation would fail for them
		AnsiString escaped = StringReplace(uri, "*", "A", TReplaceFlags() << rfReplaceAll);
		file += escaped;
	}
	return file;
}
