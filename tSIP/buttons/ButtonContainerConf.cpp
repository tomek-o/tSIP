//---------------------------------------------------------------------------


#pragma hdrstop

#include "ButtonContainerConf.h"
#include <json/json.h>

//---------------------------------------------------------------------------

#pragma package(smart_init)

void ButtonContainerConf::FromJson(const Json::Value &jv)
{
	if (jv.type() != Json::objectValue)
		return;
	jv.getAString("backgroundImage", backgroundImage);
}

void ButtonContainerConf::ToJson(Json::Value &jv)
{
	jv = Json::Value(Json::objectValue);
	jv["backgroundImage"] = backgroundImage;
}

