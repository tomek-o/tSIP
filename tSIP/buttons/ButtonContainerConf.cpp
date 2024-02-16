//---------------------------------------------------------------------------


#pragma hdrstop

#include "ButtonContainerConf.h"
#include <Graphics.hpp>
#include <json/json.h>

//---------------------------------------------------------------------------

#pragma package(smart_init)

ButtonContainerConf::ButtonContainerConf(void):
	backgroundColor(clBtnFace)
{
}


void ButtonContainerConf::FromJson(const Json::Value &jv)
{
	if (jv.type() != Json::objectValue)
		return;
	jv.getAString("backgroundImage", backgroundImage);
	jv.getInt("backgroundColor", backgroundColor);
}

void ButtonContainerConf::ToJson(Json::Value &jv) const
{
	jv = Json::Value(Json::objectValue);
	jv["backgroundImage"] = backgroundImage;
	jv["backgroundColor"] = backgroundColor;
}

