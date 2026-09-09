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
	enum { MAX_ENTRIES_MIN = 20 };
	enum { MAX_ENTRIES_DEF = 1000 };
	enum { MAX_ENTRIES_MAX = 5000 };

	bool noStoreToFile;
	bool ignoreCallsCompletedElsewhere;
	bool usePaiForDisplayIfAvailable;
	bool usePaiForDialIfAvailable;
	bool showHint;
	bool formatCallDurationAsHourMinSec;
	bool showCodecNameInHint;
	bool showLastCodeInHint;
	bool showLastReplyLineInHint;
	bool showReasonInHint;
	bool showRecordFileInHint;
	unsigned int maxEntries;
	std::vector<int> listColumnWidths;

	HistoryConf(void);
	void fromJson(const Json::Value &jv);
	void toJson(Json::Value &jv) const;

	bool operator==(const HistoryConf &right) const;
	bool operator!=(const HistoryConf &right) const
	{
		return !(*this == right);
	}
};

#endif
