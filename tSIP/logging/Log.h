/** \file
 *  \brief Global logging unit
 */

#ifndef LogH
#define LogH
#include <string>
#include "common/singleton.h"

/** \brief Global logger
*/
class CLog: public CSingleton<CLog>
{
public:
    /** \brief Log formatted text with optional timestamp */
	void log(const char *lpData, ...);
	/** \brief Log formatted text without timestamp */
	void logRaw(const char *lpData, ...);
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
	typedef void (__closure *CallbackLog)(const char *buf);
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
};

/** \brief Macro to avoid unnecessary typing
*/
#define LOG CLog::Instance()->log

#endif
