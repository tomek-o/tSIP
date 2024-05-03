//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "TrayNotifierConf.h"
#include <json/json.h>
#include <re_types.h>

//---------------------------------------------------------------------------

#pragma package(smart_init)

void TrayNotifierConf::ElementConf::fromJson(const Json::Value &jv)
{
	if (jv.type() != Json::objectValue)
		return;
	jv.getBool("visible", visible);
	jv.getInt("left", left);
	jv.getInt("top", top);
	jv.getUInt("width", width);
}

void TrayNotifierConf::ElementConf::toJson(Json::Value &jv) const
{
	jv = Json::Value(Json::objectValue);
	jv["visible"] = visible;
	jv["left"] = left;
	jv["top"] = top;
	jv["width"] = width;
}

TrayNotifierConf::TrayNotifierConf(void):
	iHeight(105),
	iWidth(213),
	iPosX(30),	// overriden later, depending on screen size
	iPosY(30),	// overriden later, depending on screen size
	showOnIncoming(false),
	skipIfMainWindowVisible(false),
	showOnOutgoing(false),
	hideWhenAnsweringCall(false),
	hideWhenAnsweringCallAutomatically(false),
	scalingPct(SCALING_DEF),
	doNotChangePosition(false)
{
	maxX = GetSystemMetrics(SM_CXSCREEN);
	/** \todo Ugly fixed taskbar margin */
	maxY = GetSystemMetrics(SM_CYSCREEN) - 32;
	iPosX = maxX - iWidth;
	iPosY = maxY - iHeight;

	elements.labelDescription.left = 8;
	elements.labelDescription.width = 191;
	elements.labelDescription.top = 8;

	elements.labelUri.left = 8;
	elements.labelUri.width = 191;
	elements.labelUri.top = 27;

	elements.btnAnswer.left = 8;
	elements.btnAnswer.width = 75;
	elements.btnAnswer.top = 50;

	elements.btnHangup.left = 124;
	elements.btnHangup.width = 75;
	elements.btnHangup.top = 50;
}

void TrayNotifierConf::fromJson(const Json::Value &jv)
{
	if (jv.type() != Json::objectValue)
		return;

	jv.getUInt("Width", iWidth);
	jv.getUInt("Height", iHeight);

	{
		int tmp = jv.get("PosX", iPosX).asInt();
		if (tmp >= 0 && tmp + iWidth <= maxX)
		{
			iPosX = tmp;
		}
	}
	{
		int tmp = jv.get("PosY", iPosY).asInt();
		if (tmp >= 0 && tmp + iHeight <= maxY)
		{
			iPosY = tmp;
		}
	}
	jv.getBool("ShowOnIncoming", showOnIncoming);
	jv.getBool("SkipIfMainWindowVisible", skipIfMainWindowVisible);
	jv.getBool("ShowOnOutgoing", showOnOutgoing);
	jv.getBool("HideWhenAnsweringCall", hideWhenAnsweringCall);
	jv.getBool("HideWhenAnsweringCallAutomatically", hideWhenAnsweringCallAutomatically);
	{
		int tmp = jv.get("ScalingPct", scalingPct).asInt();
		if (tmp >= SCALING_MIN && tmp <= SCALING_MAX) {
			scalingPct = tmp;
		}
	}
	jv.getBool("DoNotChangePosition", doNotChangePosition);


	const Json::Value &je = jv["elements"];
	if (jv.type() == Json::objectValue)
	{
		elements.labelDescription.fromJson(je["labelDescription"]);
		elements.labelUri.fromJson(je["labelUri"]);
		elements.btnAnswer.fromJson(je["btnAnswer"]);
		elements.btnHangup.fromJson(je["btnHangup"]);
	}
}

void TrayNotifierConf::toJson(Json::Value &jv) const
{
	jv = Json::Value(Json::objectValue);

	jv["PosX"] = iPosX;
	jv["PosY"] = iPosY;
	jv["Width"] = iWidth;
	jv["Height"] = iHeight;
	jv["ShowOnIncoming"] = showOnIncoming;
	jv["SkipIfMainWindowVisible"] = skipIfMainWindowVisible;
	jv["ShowOnOutgoing"] = showOnOutgoing;
	jv["HideWhenAnsweringCall"] = hideWhenAnsweringCall;
	jv["HideWhenAnsweringCallAutomatically"] = hideWhenAnsweringCallAutomatically;
	jv["ScalingPct"] = scalingPct;
	jv["DoNotChangePosition"] = doNotChangePosition;

	{
		Json::Value &je = jv["elements"];
		elements.labelDescription.toJson(je["labelDescription"]);
		elements.labelUri.toJson(je["labelUri"]);
		elements.btnAnswer.toJson(je["btnAnswer"]);
		elements.btnHangup.toJson(je["btnHangup"]);
	}
}



