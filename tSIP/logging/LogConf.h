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
