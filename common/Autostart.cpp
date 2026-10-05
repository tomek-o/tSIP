#include <vcl.h>
#pragma hdrstop

#include "Autostart.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#pragma package(smart_init)

namespace
{
	const char *RUN_KEY_PATH = "Software\\Microsoft\\Windows\\CurrentVersion\\Run";
}

bool Autostart::IsEnabled(const AnsiString &name, const AnsiString &command)
{
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, RUN_KEY_PATH, 0, KEY_READ, &hKey) != ERROR_SUCCESS)
		return false;

	char buffer[1024];
	DWORD size = sizeof(buffer) - 1;
	DWORD type = 0;
	LONG status = RegQueryValueEx(hKey, name.c_str(), NULL, &type, (BYTE*)buffer, &size);
	RegCloseKey(hKey);

	if (status != ERROR_SUCCESS || type != REG_SZ)
		return false;

	buffer[size] = '\0';
	return command == buffer;
}

bool Autostart::Enable(const AnsiString &name, const AnsiString &command)
{
	HKEY hKey;
	if (RegCreateKeyEx(HKEY_CURRENT_USER, RUN_KEY_PATH, 0, NULL, REG_OPTION_NON_VOLATILE,
			KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS)
		return false;

	LONG status = RegSetValueEx(hKey, name.c_str(), 0, REG_SZ,
		(const BYTE*)command.c_str(), command.Length() + 1);
	RegCloseKey(hKey);

	return status == ERROR_SUCCESS;
}

bool Autostart::Disable(const AnsiString &name)
{
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, RUN_KEY_PATH, 0, KEY_WRITE, &hKey) != ERROR_SUCCESS)
		return true;	// nothing to remove

	LONG status = RegDeleteValue(hKey, name.c_str());
	RegCloseKey(hKey);

	return status == ERROR_SUCCESS || status == ERROR_FILE_NOT_FOUND;
}
