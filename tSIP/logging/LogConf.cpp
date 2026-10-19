#pragma hdrstop

#include "LogConf.h"
#include <json/json.h>

#pragma package(smart_init)

namespace {

const char *const levelNames[LogConf::LEVEL_COUNT] = {"sip", "error", "warning", "info", "debug"};

void colorsFromJson(const Json::Value &jv, const char *key, unsigned int *colors)
{
	const Json::Value &jc = jv[key];
	for (int i=0; i<LogConf::LEVEL_COUNT; i++)
		jc.getUInt(levelNames[i], colors[i]);
}

void colorsToJson(Json::Value &jv, const char *key, const unsigned int *colors)
{
	Json::Value &jc = jv[key];
	for (int i=0; i<LogConf::LEVEL_COUNT; i++)
		jc[levelNames[i]] = colors[i];
}

bool sameColors(const unsigned int *a, const unsigned int *b)
{
	for (int i=0; i<LogConf::LEVEL_COUNT; i++)
		if (a[i] != b[i])
			return false;
	return true;
}

}

LogConf::LogConf(void):
	logToFile(false),
	flush(false),
	maxFileSize(DEF_MAX_FILE_SIZE),
	logRotate(DEF_LOGROTATE),
	maxUiLogLines(5000),
	windowWidth(630), windowHeight(400),
	showWindowAtStartup(false),
	viewMode(LOG_VIEW_TEXT),
	levelFilter(0x1F),
	richBackground(0xFFFFFF),
	logAubuf(false),
	moreShown(false)
{
	consoleFont.name = "Courier New";
	// TColor values (BGR): SIP navy, error red, warning dark yellow, info black, debug gray
	const unsigned int defaultText[LEVEL_COUNT] = {0x800000, 0x0000C8, 0x0078A0, 0x000000, 0x808080};
	for (int i=0; i<LEVEL_COUNT; i++)
	{
		richColors[i] = defaultText[i];
		listTextColors[i] = defaultText[i];
		listBackgrounds[i] = 0xFFFFFF;
	}
}

void LogConf::fromJson(const Json::Value &jv)
{
	if (jv.type() != Json::objectValue)
		return;
	jv.getBool("LogToFile", logToFile);
	jv.getBool("Flush", flush);
	jv.getIntInRange("MaxFileSize", maxFileSize, MIN_MAX_FILE_SIZE, MAX_MAX_FILE_SIZE);
	jv.getUIntInRange("MaxUiLogLines", maxUiLogLines, MIN_MAX_UI_LOG_LINES, MAX_MAX_UI_LOG_LINES);
	jv.getUIntInRange("LogRotate", logRotate, 0, MAX_LOGROTATE);
	jv.getUInt("WindowWidth", windowWidth);
	jv.getUInt("WindowHeight", windowHeight);
	jv.getBool("ShowWindowAtStartup", showWindowAtStartup);
	const Json::Value &jm = jv["LogViewMode"];
	if (jm.type() == Json::stringValue)
		viewMode = (jm.asString() == "List") ? LOG_VIEW_LIST : LOG_VIEW_TEXT;
	jv.getUInt("LevelFilter", levelFilter);
	jv.getUInt("RichBackground", richBackground);
	jv.getBool("LogAubuf", logAubuf);
	jv.getBool("MoreShown", moreShown);
	colorsFromJson(jv, "RichColors", richColors);
	colorsFromJson(jv, "ListTextColors", listTextColors);
	colorsFromJson(jv, "ListBackgrounds", listBackgrounds);
	consoleFont.fromJson(jv["ConsoleFont"]);
}

void LogConf::toJson(Json::Value &jv) const
{
	jv = Json::Value(Json::objectValue);
	jv["LogToFile"] = logToFile;
	jv["Flush"] = flush;
	jv["MaxFileSize"] = maxFileSize;
	jv["MaxUiLogLines"] = maxUiLogLines;
	jv["LogRotate"] = logRotate;
	jv["WindowWidth"] = windowWidth;
	jv["WindowHeight"] = windowHeight;
	jv["ShowWindowAtStartup"] = showWindowAtStartup;
	jv["LogViewMode"] = (viewMode == LOG_VIEW_LIST) ? "List" : "Text";
	jv["LevelFilter"] = levelFilter;
	jv["RichBackground"] = richBackground;
	jv["LogAubuf"] = logAubuf;
	jv["MoreShown"] = moreShown;
	colorsToJson(jv, "RichColors", richColors);
	colorsToJson(jv, "ListTextColors", listTextColors);
	colorsToJson(jv, "ListBackgrounds", listBackgrounds);
	consoleFont.toJson(jv["ConsoleFont"]);
}

bool LogConf::operator==(const LogConf &right) const
{
	return (
		sameColors(richColors, right.richColors) &&
		sameColors(listTextColors, right.listTextColors) &&
		sameColors(listBackgrounds, right.listBackgrounds) &&
		logToFile == right.logToFile &&
		flush == right.flush &&
		maxFileSize == right.maxFileSize &&
		maxUiLogLines == right.maxUiLogLines &&
		logRotate == right.logRotate &&
		windowWidth == right.windowWidth &&
		windowHeight == right.windowHeight &&
		showWindowAtStartup == right.showWindowAtStartup &&
		viewMode == right.viewMode &&
		levelFilter == right.levelFilter &&
		richBackground == right.richBackground &&
		logAubuf == right.logAubuf &&
		moreShown == right.moreShown &&
		consoleFont == right.consoleFont
	);
}
