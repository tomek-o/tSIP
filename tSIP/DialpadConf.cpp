//---------------------------------------------------------------------------


#pragma hdrstop

#include "DialpadConf.h"
#include "common/StaticCheck.h"
#include <json/json.h>

//---------------------------------------------------------------------------

#pragma package(smart_init)

#define ARRAY_SIZE(a) ((sizeof(a))/(sizeof((a)[0])))

namespace
{
	unsigned int Rgb(unsigned int r, unsigned int g, unsigned int b)
	{
		return r | (g << 8) | (b << 16);
	}
	const char *meterColorNames[] = { "low", "mid", "high", "peak", "track", "thumb" };
}

DialpadConf::DialpadConf(void)
{
	meterColors[MC_LOW] = Rgb(40, 180, 70);
	meterColors[MC_MID] = Rgb(230, 190, 30);
	meterColors[MC_HIGH] = Rgb(220, 40, 40);
	meterColors[MC_PEAK] = Rgb(0, 0, 0);
	meterColors[MC_TRACK] = Rgb(90, 90, 100);
	meterColors[MC_THUMB] = Rgb(200, 200, 210);

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

void DialpadConf::ElementConf::toJson(Json::Value &jv) const
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
		width == right.width &&
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
	const Json::Value &jc = jv["meterColors"];
	if (jc.type() == Json::objectValue)
	{
		for (unsigned int i=0; i<MC_CNT; i++)
			jc.getUInt(meterColorNames[i], meterColors[i]);
	}
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

void DialpadConf::toJson(Json::Value &jv) const
{
	jv = Json::Value(Json::objectValue);
	Json::Value &jc = jv["meterColors"];
	jc = Json::Value(Json::objectValue);
	for (unsigned int i=0; i<MC_CNT; i++)
		jc[meterColorNames[i]] = meterColors[i];
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
	for (unsigned int i=0; i<MC_CNT; i++)
	{
		if (meterColors[i] != right.meterColors[i])
			return false;
	}

	return true;
}

