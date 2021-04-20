//---------------------------------------------------------------------------


#pragma hdrstop

#include "DialpadConf.h"
#include "common/StaticCheck.h"
#include <json/json.h>

//---------------------------------------------------------------------------

#pragma package(smart_init)

#define ARRAY_SIZE(a) ((sizeof(a))/(sizeof((a)[0])))

DialpadConf::DialpadConf(void)
{
	{
		ElementConf &ec = elements[EL_ED_TRANSFER];
		ec.visible = true;
		ec.left = 3;
		ec.top = 181;
		ec.width = 136;
		ec.height = 24;
	}
	{
		ElementConf &ec = elements[EL_TRBAR_MIC_VOLUME];
		ec.visible = true;
		ec.left = 140;
		ec.top = 0;
		ec.width = 20;
		ec.height = 180;
	}
	{
		ElementConf &ec = elements[EL_BTN_RESET_MIC_VOLUME];
		ec.visible = true;
		ec.left = 143;
		ec.top = 181;
		ec.width = 18;
		ec.height = 24;
	}
	{
		ElementConf &ec = elements[EL_TRBAR_SPEAKER_VOLUME];
		ec.visible = true;
		ec.left = 160;
		ec.top = 0;
		ec.width = 20;
		ec.height = 180;
	}
	{
		ElementConf &ec = elements[EL_BTN_RESET_SPEAKER_VOLUME];
		ec.visible = true;
		ec.left = 165;
		ec.top = 181;
		ec.width = 18;
		ec.height = 24;
	}
}

void DialpadConf::ElementConf::fromJson(const Json::Value &jv)
{
	if (jv.type() != Json::objectValue)
		return;
	jv.getBool("visible", visible);
	jv.getInt("left", left);
	jv.getInt("top", top);
	jv.getUInt("width", width);
	jv.getUInt("height", height);
}

void DialpadConf::ElementConf::toJson(Json::Value &jv)
{
	jv = Json::Value(Json::objectValue);
	jv["visible"] = visible;
	jv["left"] = left;
	jv["top"] = top;
	jv["width"] = width;
	jv["height"] = height;
}

bool DialpadConf::ElementConf::operator==(const DialpadConf::ElementConf &right) const
{
	if (visible == right.visible &&
		left == right.left &&
		top == right.top &&
		width == right.top &&
		height == right.height)
	{
		return true;
	}
	return false;
}

void DialpadConf::fromJson(const Json::Value &jv)
{
	if (jv.type() != Json::objectValue)
		return;
	const Json::Value &je = jv["elements"];
	if (je.type() == Json::arrayValue)
	{
		for (unsigned int i=0; i<je.size(); i++) {
			if (i >= ARRAY_SIZE(elements))
				break;
			elements[i].fromJson(je[i]);
		}
	}
}

void DialpadConf::toJson(Json::Value &jv)
{
	jv = Json::Value(Json::objectValue);
	Json::Value &je = jv["elements"];
	for (unsigned int i=0; i<ARRAY_SIZE(elements); i++)
	{
		elements[i].toJson(je[i]);
	}
}

bool DialpadConf::operator==(const DialpadConf &right) const
{
	for (unsigned int i=0; i<ARRAY_SIZE(elements); i++)
	{
		if (elements[i] != right.elements[i])
			return false;
	}

	return true;
}

