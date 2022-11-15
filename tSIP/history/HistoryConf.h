//---------------------------------------------------------------------------
#ifndef HistoryConfH
#define HistoryConfH
//---------------------------------------------------------------------------

#include <vector>

namespace Json
{
	class Value;
}

struct HistoryConf
{
	bool noStoreToFile;
	bool usePaiForDisplayIfAvailable;
	bool usePaiForDialIfAvailable;
	bool showHint;
	bool formatCallDurationAsHourMinSec;
	bool showCodecNameInHint;
	bool showLastCodeInHint;
	bool showLastReplyLineInHint;
	bool showRecordFileInHint;
	std::vector<int> listColumnWidths;

	HistoryConf(void);
	void fromJson(const Json::Value &jv);
	void toJson(Json::Value &jv);

	bool operator==(const HistoryConf &right) const;
	bool operator!=(const HistoryConf &right) const
	{
		return !(*this == right);
	}
};

#endif
