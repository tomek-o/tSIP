#pragma hdrstop

#include "Paths.h"
#include <Forms.hpp>

//---------------------------------------------------------------------------

#pragma package(smart_init)

using namespace Paths;


namespace
{

AnsiString asProfileDir;

}	// namespace


void Paths::SetProfileDir(AnsiString dir)
{
	dir = dir.Trim();
	if (dir != "")
	{
		if (dir[dir.Length()] == '\\')
		{
			dir = dir.SubString(1, dir.Length() - 1);
		}
	}
	asProfileDir = dir;
	if (asProfileDir == "")
	{
		asProfileDir = ExtractFileDir(Application->ExeName);
	}
}

AnsiString Paths::GetProfileDir(void)
{
	return asProfileDir;
}

AnsiString Paths::GetFullImgName(AnsiString name)
{
	AnsiString fname;
	fname.sprintf("%s\\img\\%s", asProfileDir.c_str(), name.c_str());
	return fname;
}

AnsiString Paths::GetConfig(void)
{
	return asProfileDir + "\\" + ChangeFileExt(ExtractFileName(Application->ExeName), ".json");
}


