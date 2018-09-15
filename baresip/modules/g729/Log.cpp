/** \file
 *  \brief Global logging unit
 */

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <sys\timeb.h>
#include "Log.h"

extern void Log(char* txt);

CLog::CLog()
{
};

void CLog::log(const char *lpData, ...)
{
	va_list ap;
	char buf[1024]; //determines max message length

    int size = 0;

	if ((int)sizeof(buf)-size-2 > 0)
	{
		va_start(ap, lpData);
		size += vsnprintf(buf + size, sizeof(buf)-size-2, lpData, ap);
		va_end(ap);
	}
	if (size > (int)sizeof(buf) - 2)
		size = sizeof(buf) - 2;
	buf[size] = '\n';
	buf[size+1] = 0;

	OutputDebugString(buf);
//	Log(buf);
}

