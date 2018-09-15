/** \file
 *  \brief Global logging unit
 */

#ifndef LogH
#define LogH
#include <string>
#include "common/singleton.h"

/** \brief Log detail level
*/
enum E_LOGLEVEL
{
	E_LOG_NONE = 0,
	E_LOG_TRACE,
	E_LOG_ALL
};

/** \brief Global logger
*/
class CLog: public CSingleton<CLog>
{
public:
    /** \brief Log formatted text with timestamp */
	void log(char *lpData, ...);
	/** \brief Select output log file
	 *  \return zero on no error
	 */
	int SetFile(std::string);
	void SetLogToFile(bool);
	void SetFlush(bool state);
	void SetMaxFileSize(unsigned int size);
	/** Set total maximum number of files used for "logrotate" scheme (apart from main file).
		Equivalent of "rotate" parameter for logrotate - number of old files to keep.
		Old files are named like main file but with with ".x" added after file extension 
		\note If limit is decreased, old files exceeding it are not deleted
	*/
	void SetLogRotateCnt(unsigned int cnt);
	/** \brief Set log detail level / disable logging */
	void SetLevel(int);
	/** \brief Close log file */
	void Close(void);
	/** \brief OnLog callback declaration
	*/
	typedef void (__closure *CallbackLog)(char *buf);
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
	//~CLog() {};
	friend CSingleton<CLog>;
	std::string sFile;
	bool bLogToFile;
	bool bFlush;
	int iLogLevel;
	unsigned int maxFileSize;
	unsigned int maxLogrotateCnt;
};

/** \brief Macro to avoid unnecessary typing
*/
#define LOG CLog::Instance()->log

#endif
