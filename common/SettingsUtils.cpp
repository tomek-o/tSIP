//---------------------------------------------------------------------------


#pragma hdrstop

#include "SettingsUtils.h"
#include <json/json.h>
#include <SysUtils.hpp>
#include <stdio.h>
#include <fstream>

//---------------------------------------------------------------------------

#pragma package(smart_init)

using namespace SettingsUtils;

namespace
{

enum SettingsUtils::ReadStatus ReadFile(AnsiString fileName, Json::Value &root)
{
	Json::Reader reader;
	root = Json::Value();
	try
	{
		std::ifstream ifs(fileName.c_str(), std::ios::in | std::ios::binary);
		if (!ifs.is_open())
		{
			return READ_FILE_NOT_FOUND;
		}
		std::string strConfig((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
		ifs.close();
		if (strConfig.empty())
		{
			return READ_EMPTY_FILE;
		}
		bool parsingSuccessful = reader.parse(strConfig, root);
		if ( !parsingSuccessful )
		{
			return READ_PARSE_ERROR;
		}
		return READ_OK;
	}
	catch(...)
	{
		return READ_IO_ERROR;
	}
}


}

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

enum SettingsUtils::ReadStatus SettingsUtils::ReadFileOrBackup(AnsiString fileName, Json::Value &root)
{
	SettingsUtils::ReadStatus status;
	status = ReadFile(fileName, root);
	if (status != READ_OK)
	{
		status = ReadFile(GetBakFileName(fileName), root);
		if (status != READ_OK)
			return status;
		return READ_RECOVERED_FROM_BACKUP;
	}
	return status;
}


