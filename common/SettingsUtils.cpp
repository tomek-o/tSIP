//---------------------------------------------------------------------------


#pragma hdrstop

#include "SettingsUtils.h"
#include <SysUtils.hpp>
#include <stdio.h>

//---------------------------------------------------------------------------

#pragma package(smart_init)

AnsiString SettingsUtils::GetNextFileName(AnsiString fileName)
{
	return fileName + ".next";
}

AnsiString SettingsUtils::GetBakFileName(AnsiString fileName)
{
	return fileName + ".bak";
}

int SettingsUtils::AtomicUpdateWithBackup(AnsiString fileName, const std::string &content)
{
	AnsiString tmpFile = GetNextFileName(fileName);
	AnsiString backupFile = GetBakFileName(fileName);

	FILE *fp = fopen(tmpFile.c_str(), "wb");
	if (fp == NULL)
	{
		return 1;
	}

	int ret = fwrite(content.data(), content.size(), 1, fp);
	fflush(fp);
	fclose(fp);
	if (ret != 1)
	{
		DeleteFile(tmpFile.c_str());
		return 1;
	}

	if (FileExists(fileName))
	{
		if (!ReplaceFile(fileName.c_str(), tmpFile.c_str(), backupFile.c_str(),
				REPLACEFILE_WRITE_THROUGH, NULL, NULL))
		{
			DeleteFile(tmpFile.c_str());
			return 1;
		}
	}
	else
	{
		if (!MoveFileEx(tmpFile.c_str(), fileName.c_str(), MOVEFILE_WRITE_THROUGH | MOVEFILE_COPY_ALLOWED))
		{
			DeleteFile(tmpFile.c_str());
			return 1;
		}
	}

	return 0;
}

