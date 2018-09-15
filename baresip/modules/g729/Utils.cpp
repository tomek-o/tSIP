/** \file
*/

//---------------------------------------------------------------------------

#include "Utils.h"
#include <windows.h>
#include <assert.h>

//---------------------------------------------------------------------------


HMODULE Utils::GetCurrentModule(void)
{
    MEMORY_BASIC_INFORMATION mbi;
    static int dummy;
    VirtualQuery( &dummy, &mbi, sizeof(mbi) );
    return reinterpret_cast<HMODULE>(mbi.AllocationBase);
}

std::string Utils::GetDllPath(void)
{
	static char szPath [MAX_PATH + 1] = "";

	/** \note
	GetModuleFileName behavior:
	Windows XP/2000:  The string is truncated to
	nSize characters and is NOT null terminated if overflowed.
	*/
	GetModuleFileName (GetCurrentModule(), szPath, MAX_PATH);
	szPath[MAX_PATH] = '\0';

	return szPath;
}


std::string Utils::ReplaceFileExtension(std::string filename, std::string ext)
{
	unsigned int dot = filename.rfind('.');
	if (dot == std::string::npos)
		return "";
	unsigned int bslash = filename.rfind('\\');
	if (bslash != std::string::npos)
	{
		if (bslash > dot)
			return "";
	}
	std::string ret = filename.substr(0, dot) + ext;
	return ret;
}


