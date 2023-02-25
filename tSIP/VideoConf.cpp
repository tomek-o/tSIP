//---------------------------------------------------------------------------


#pragma hdrstop

#include "VideoConf.h"
#include "VideoModules.h"
#include <json/json.h>

//---------------------------------------------------------------------------

#pragma package(smart_init)

const char* VideoConf::getDisplayParentTypeName(enum VideoConf::DisplayParentType type)
{
	switch (type)
	{
		case DISPLAY_PARENT_NONE:
			return "none (separate window)";
		case DISPLAY_PARENT_BUTTON:
			return "programmable button";
		default:
			return "???";
	}
}

VideoConf::VideoConf(void):
	enabled(true),
	displayParentType(DISPLAY_PARENT_NONE),
	displayParentId(0)
{

}

void VideoConf::fromJson(const Json::Value &jv)
{
	jv.getBool("enabled", enabled);

	int tmp;
	tmp = jv.get("displayParentType", displayParentType).asInt();
	if (tmp >= 0 && tmp < DISPLAY_PARENT__LIMITER)
	{
		displayParentType = static_cast<DisplayParentType>(tmp);
	}
	jv.getInt("displayParentId", displayParentId);
}

void VideoConf::toJson(Json::Value &jv)
{
	jv["enabled"] = enabled;
	jv["displayParentType"] = displayParentType;
	jv["displayParentId"] = displayParentId;
}

bool VideoConf::operator==(const VideoConf &right) const
{
	return (
		enabled == right.enabled &&
		displayParentType == right.displayParentType &&
		displayParentId == right.displayParentId
	);
}
