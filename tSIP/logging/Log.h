/** \file
 *  \brief Global logging unit
 */

#ifndef LogH
#define LogH
#include <string>
#include <stdarg.h>
#include "common/singleton.h"

/** rief Log levels, tSIP's own convention (not tied to any library)
 *
 *  Used for filtering in the log window. Mapping of library levels is
 *  done where the library output is received.
 */
enum LogLevel
{
	LogLevelSip = 0,	///< SIP message traces
	LogLevelError,
	LogLevelWarning,
	LogLevelInfo,	///< default level of LOG()
	LogLevelDebug
};

/** \brief Global logger
*/
class CLog: public CSingleton<CLog>
{
public:
    /** \brief Log formatted text with optional timestamp */
	void log(const char *lpData, ...);
	/** rief Log formatted text with level and module name */
	void logL(int level, const char *module, const char *lpData, ...);
	/** \brief Log formatted text without timestamp */
	void logRaw(const char *lpData, ...);
	/** \brief Log already formatted text without timestamp
		\param text text, does not have to be NUL-terminated
		\param len number of bytes to log
		\note use instead of logRaw("%.*s", len, text) - avoids printf reading
			(or CodeGuard checking) not NUL-terminated buffer beyond len
	*/
	void logRawLen(const char *text, size_t len);
	/** \brief Select output log file
	 *
	 *  Empty file name disables logging to file.
	 *  \return zero on no error
	 */
	int SetFile(const std::string &file);
	void SetLogToFile(bool);
	void SetFlush(bool state);
	void SetMaxFileSize(unsigned int size);
	void SetTimestamps(bool enabled);
	/** Set total maximum number of files used for "logrotate" scheme (apart from main file).
		Equivalent of "rotate" parameter for logrotate - number of old files to keep.
		Old files are named like main file but with with ".x" added after file extension 
		\note If limit is decreased, old files exceeding it are not deleted
	*/
	void SetLogRotateCnt(unsigned int cnt);
	/** \brief Close log file */
	void Close(void);
	/** \brief OnLog callback declaration
	*/
	typedef void (__closure *CallbackLog)(int level, const char *module, const char *buf);
	/** \brief Callback called when new log line is added
	 *
	 *  If set, this callback is called when new log line is added. This can
	 *  be used to update user interface (i.e. add some items to TListView)
	 *  when log is written by various decoupled units (with no knowledge
	 *  about user interface)     .
	 */
	CallbackLog callbackLog;
private:
	CLog();
	/// Closes the log file - Destroy() would otherwise leave it open
	~CLog();
	friend CSingleton<CLog>;
	std::string sFile;
	bool bLogToFile;
	bool bFlush;
	unsigned int maxFileSize;
	/// tracked locally to avoid an ftell() syscall on every logged line
	unsigned long curFileSize;
	bool timestamps;
	unsigned int maxLogrotateCnt;
	void write(const char* buf, int size);
	void emit(int level, const char *module, const char *lpData, va_list ap);
};

/** \brief Macro to avoid unnecessary typing
*/
#define LOG CLog::Instance()->log
/** rief Log with level, e.g. LOGL(LogLevelWarning, "ua", "text %d
", x) */
#define LOGL CLog::Instance()->logL

#endif
