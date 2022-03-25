//---------------------------------------------------------------------------


#pragma hdrstop

#include "Branding.h"
#include "resource.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

AnsiString Branding::appName = "tSIP";
AnsiString Branding::appProto = "tsip";
AnsiString Branding::appUrl = "";
AnsiString Branding::fixedSettings = "";
bool Branding::recording = false;

namespace
{

bool initialized = false;

AnsiString GetRcDataAsString(unsigned int id)
{
	HRSRC rsrc = FindResource(HInstance, MAKEINTRESOURCE(id), RT_RCDATA);
	if(!rsrc)
	{
    	return "";
	}
	DWORD Size = SizeofResource(HInstance, rsrc);
	HGLOBAL MemoryHandle = LoadResource(HInstance, rsrc);

	if(MemoryHandle == NULL)
		return "";

	BYTE *MemPtr = (BYTE *)LockResource(MemoryHandle);

	AnsiString ret = AnsiString(reinterpret_cast<char*>(MemPtr), Size);

	return ret;
}

}

void Branding::init(void)
{
	if (initialized)
		return;
	AnsiString text = LoadStr(ID_STR_RECORDING_FEATURE);
	if (text == "RECORDING_ENABLED")
		recording = true;
	text = LoadStr(ID_STR_APP_NAME);
	if (text != "")
		appName = text;
	text = LoadStr(ID_STR_APP_PROTO);
	if (text != "")
		appProto = text;
	appUrl = LoadStr(ID_STR_APP_URL);
    fixedSettings = GetRcDataAsString(ID_RCDATA_FIXED_SETTINGS);
	initialized = true;
}

