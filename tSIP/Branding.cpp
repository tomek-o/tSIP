//---------------------------------------------------------------------------


#pragma hdrstop

#include "Branding.h"
#include "resource.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

AnsiString Branding::appName = "tSIP";
AnsiString Branding::appProto = "tsip";
AnsiString Branding::appUrl = "";
bool Branding::recording = false;

namespace
{
	bool initialized = false;
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

	initialized = true;
}

