//---------------------------------------------------------------------------


#pragma hdrstop

#include "HistoryConf.h"
#include "common/StaticCheck.h"
#include <json/json.h>

//---------------------------------------------------------------------------

#pragma package(smart_init)

#define ARRAY_SIZE(a) ((sizeof(a))/(sizeof((a)[0])))

HistoryConf::HistoryConf(void):
			noStoreToFile(false),
			usePaiForDisplayIfAvailable(true),
			usePaiForDialIfAvailable(true),
			showHint(true),
			formatCallDurationAsHourMinSec(true),
			showCodecNameInHint(true),
			showLastCodeInHint(true),
			showLastReplyLineInHint(true),
			showRecordFileInHint(true)
{
}

void HistoryConf::fromJson(const Json::Value &jv)
{
	if (jv.type() != Json::objectValue)
		return;
	jv.getBool("NoStoreToFile", noStoreToFile);
	jv.getBool("UsePaiForDisplayIfAvailable", usePaiForDisplayIfAvailable);
	jv.getBool("UsePaiForDialIfAvailable", usePaiForDialIfAvailable);
	jv.getBool("ShowHint", showHint);
	jv.getBool("FormatCallDurationAsHourMinSec", formatCallDurationAsHourMinSec);
	jv.getBool("ShowCodecNameInHint", showCodecNameInHint);
	jv.getBool("ShowLastCodeInHint", showLastCodeInHint);
	jv.getBool("ShowLastReplyLineInHint", showLastReplyLineInHint);
	jv.getBool("ShowRecordFileInHint", showRecordFileInHint);
	{
		const Json::Value &jlcw = jv["ListColumnWidths"];
		if (jlcw.type() == Json::arrayValue)
		{
            listColumnWidths.clear();
			for (unsigned int i=0; i<jlcw.size(); i++)
			{
				listColumnWidths.push_back(jlcw[i].asInt());
			}
		}
	}
}

void HistoryConf::toJson(Json::Value &jv)
{
	jv = Json::Value(Json::objectValue);
	jv["NoStoreToFile"] = noStoreToFile;
	jv["UsePaiForDisplayIfAvailable"] = usePaiForDisplayIfAvailable;
	jv["UsePaiForDialIfAvailable"] = usePaiForDialIfAvailable;
	jv["ShowHint"] = showHint;
	jv["FormatCallDurationAsHourMinSec"] = formatCallDurationAsHourMinSec;
	jv["ShowCodecNameInHint"] = showCodecNameInHint;
	jv["ShowLastCodeInHint"] = showLastCodeInHint;
	jv["ShowLastReplyLineInHint"] = showLastReplyLineInHint;
	jv["ShowRecordFileInHint"] = showRecordFileInHint;

	Json::Value &jlcw = jv["ListColumnWidths"];
	jlcw.resize(0);
	for (unsigned int i=0; i<listColumnWidths.size(); i++)
	{
		jlcw.append(listColumnWidths[i]);
	}
}

bool HistoryConf::operator==(const HistoryConf &right) const
{
	return (
		noStoreToFile == right.noStoreToFile &&
		usePaiForDisplayIfAvailable == right.usePaiForDisplayIfAvailable &&
		usePaiForDialIfAvailable == right.usePaiForDialIfAvailable &&
		showHint == right.showHint &&
		formatCallDurationAsHourMinSec == right.formatCallDurationAsHourMinSec &&
		showCodecNameInHint == right.showCodecNameInHint &&
		showLastCodeInHint == right.showLastCodeInHint &&
		showLastReplyLineInHint == right.showLastReplyLineInHint &&
		showRecordFileInHint == right.showRecordFileInHint &&
		listColumnWidths == right.listColumnWidths
	);
}

