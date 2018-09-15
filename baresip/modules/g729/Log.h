/** \file
 *  \brief Global logging unit
 */

#ifndef LogH
#define LogH
#include <string>
#include "singleton.h"

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
	void log(const char *lpData, ...);
private:
	CLog();
	//~CLog() {};
	friend class CSingleton<CLog>;
};

/** \brief Macro to avoid unnecessary typing
*/
#define LOG CLog::Instance()->log

#endif
