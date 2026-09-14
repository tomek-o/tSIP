#pragma hdrstop

#include "LogConf.h"
#include <json/json.h>

#pragma package(smart_init)

LogConf::LogConf(void):
	logToFile(false),
	flush(false),
	maxFileSize(DEF_MAX_FILE_SIZE),
	timestamps(false),
	logRotate(DEF_LOGROTATE),
	maxUiLogLines(5000),
	windowWidth(630), windowHeight(400),
	showWindowAtStartup(false)
{
	consoleFont.name = "Courier New";
}

void LogConf::fromJson(const Json::Value &jv)
{
	if (jv.type() != Json::objectValue)
		return;
	jv.getBool("LogToFile", logToFile);
	jv.getBool("Flush", flush);
	jv.getIntInRange("MaxFileSize", maxFileSize, MIN_MAX_FILE_SIZE, MAX_MAX_FILE_SIZE);
	jv.getUIntInRange("MaxUiLogLines", maxUiLogLines, MIN_MAX_UI_LOG_LINES, MAX_MAX_UI_LOG_LINES);
	jv.getBool("Timestamps", timestamps);
	jv.getUIntInRange("LogRotate", logRotate, 0, MAX_LOGROTATE);
	jv.getUInt("WindowWidth", windowWidth);
	jv.getUInt("WindowHeight", windowHeight);
	jv.getBool("ShowWindowAtStartup", showWindowAtStartup);
	consoleFont.fromJson(jv["ConsoleFont"]);
}

void LogConf::toJson(Json::Value &jv) const
{
	jv = Json::Value(Json::objectValue);
	jv["LogToFile"] = logToFile;
	jv["Flush"] = flush;
	jv["MaxFileSize"] = maxFileSize;
	jv["MaxUiLogLines"] = maxUiLogLines;
	jv["Timestamps"] = timestamps;
	jv["LogRotate"] = logRotate;
	jv["WindowWidth"] = windowWidth;
	jv["WindowHeight"] = windowHeight;
	jv["ShowWindowAtStartup"] = showWindowAtStartup;
	consoleFont.toJson(jv["ConsoleFont"]);
}

bool LogConf::operator==(const LogConf &right) const
{
	return (
		logToFile == right.logToFile &&
		flush == right.flush &&
		maxFileSize == right.maxFileSize &&
		maxUiLogLines == right.maxUiLogLines &&
		timestamps == right.timestamps &&
		logRotate == right.logRotate &&
		windowWidth == right.windowWidth &&
		windowHeight == right.windowHeight &&
		showWindowAtStartup == right.showWindowAtStartup &&
		consoleFont == right.consoleFont
	);
}

