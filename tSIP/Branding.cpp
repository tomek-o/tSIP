//---------------------------------------------------------------------------


#pragma hdrstop

#include "Branding.h"
#include "resource.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

AnsiString Branding::appName = "tSIP";
AnsiString Branding::appProto = "tsip";
bool Branding::recording = false;

void Branding::init(void)
{
	AnsiString text = LoadStr(ID_STR_RECORDING_FEATURE);
	if (text == "RECORDING_ENABLED")
		recording = true;
	text = LoadStr(ID_STR_APP_NAME);
	if (text != "")
		appName = text;
	text = LoadStr(ID_STR_APP_PROTO);
	if (text != "")
		appProto = text;
}

