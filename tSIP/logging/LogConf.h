#ifndef LogConfH
#define LogConfH

#include "common/FontConf.h"

namespace Json
{
	class Value;
}

struct LogConf
{
	bool logToFile;
	bool flush;
	enum {
		MIN_MAX_FILE_SIZE = 0,
		MAX_MAX_FILE_SIZE = 1000*1024*1024
	};
	enum {
		DEF_MAX_FILE_SIZE = 10*1024*1024
	};
	int maxFileSize;
	bool timestamps;
	enum {
		MAX_LOGROTATE = 5
	};
	enum {
		DEF_LOGROTATE = 1
	};
	unsigned int logRotate;
	enum {
		MIN_MAX_UI_LOG_LINES = 100,
		MAX_MAX_UI_LOG_LINES = 10000
	};
	unsigned int maxUiLogLines;

	FontConf consoleFont;
	unsigned int windowWidth, windowHeight;

	bool showWindowAtStartup;

	/// log window shows the list view instead of the rich edit
	bool listView;
	/// log levels: 0 SIP, 1 error, 2 warning, 3 info, 4 debug
	enum { LEVEL_COUNT = 5 };
	/// text color per level in the rich edit view, TColor values
	unsigned int richColors[LEVEL_COUNT];
	/// text color per level in the list view, TColor values
	unsigned int listTextColors[LEVEL_COUNT];
	/// background color per level in the list view, TColor values
	unsigned int listBackgrounds[LEVEL_COUNT];
	/// bit per level, set = shown in the log window
	unsigned int levelFilter;

	/// log audio buffer (aubuf) activity
	bool logAubuf;

	LogConf(void);

	void fromJson(const Json::Value &jv);
	void toJson(Json::Value &jv) const;

	bool operator==(const LogConf &right) const;
	bool operator!=(const LogConf &right) const
	{
		return !(*this == right);
	}
};

#endif
