/** \file
 *  \brief Global logging unit
 */

#include <vcl.h>
#pragma hdrstop 

#include <stdio.h>
#include <iostream>
#include <fstream>

#include <stdio.h>
#include <time.h>
#include <sys\timeb.h>
#include "Log.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <SysUtils.hpp>
#include "common/Mutex.h"
#include "common/ScopedLock.h"

namespace {
	FILE *fout = NULL;
	Mutex mutex;
}

CLog::CLog()
{
	sFile = "";
	iLogLevel = 0;
	bLogToFile = true;
	bFlush = false;
	callbackLog = NULL;
	maxFileSize = 0;
	timestamps = false;
	maxLogrotateCnt = 1;
};

int CLog::SetFile(const std::string &file)
{
	ScopedLock<Mutex> lock(mutex);
	sFile = file;
	iLogLevel = E_LOG_NONE;
	if (fout)
	{
		fclose(fout);
		fout = NULL;
	}
	if (file == "")
		return -1;
	fout = fopen(sFile.c_str(),"at+");
	if (fout == NULL)
	{
		sFile = "";	
		return -1;
	}
	return 0;
}

void CLog::Close(void)
{
	ScopedLock<Mutex> lock(mutex);
	if (fout)
		fclose(fout);
	fout = NULL;
}

void CLog::SetLogToFile(bool state)
{
	bLogToFile = state;
}

void CLog::SetFlush(bool state)
{
    bFlush = state;
}

void CLog::SetMaxFileSize(unsigned int size)
{
    maxFileSize = size;
}

void CLog::SetTimestamps(bool enabled)
{
	timestamps = enabled;
}

void CLog::SetLogRotateCnt(unsigned int cnt)
{
    maxLogrotateCnt = cnt;
}

void CLog::SetLevel(int level)
{
	iLogLevel = level;
}

void CLog::log(char *lpData, ...)
{
	ScopedLock<Mutex> lock(mutex);
	va_list ap;
	char buf[2048]; //determines max message length

	int size;
	if (timestamps)
	{
		struct timeb timebuffer;
		ftime( &timebuffer );
		size = strftime(buf, sizeof(buf), "%T", localtime(&timebuffer.time));
		int res = snprintf(buf+size, sizeof(buf)-size, ".%03hu ", timebuffer.millitm);
		buf[sizeof(buf)-1] = '\0';
		size += res;
	}
	else
	{
		size = 0;
	}

	if ((int)sizeof(buf)-size-2 > 0)
	{
		va_start(ap, lpData);
		size += vsnprintf(buf + size, sizeof(buf)-size-2, lpData, ap);
		va_end(ap);
	}
	if (size > (int)sizeof(buf) - 2)
		size = sizeof(buf) - 2;

	buf[size] = '\0';

	if (bLogToFile && fout)
	{
		fwrite(buf, size, 1, fout);
		if (bFlush)
		{
            fflush(fout);
		}
		if (maxFileSize != 0)
		{
			int size = ftell(fout);
			if (size > maxFileSize)
			{
				fclose(fout);
				if (maxLogrotateCnt > 0)
				{
					/*
					Renaming (in reverse order, base log file -> file.log.1 as last):
						file.log   -> file.log.1
						file.log.1 -> file.log.2
						file.log.2 -> file.log.3
						etc.
					*/
					for (unsigned int i=maxLogrotateCnt; i>=2; i--)
					{
						AnsiString fileN, fileNminus1;
						fileN.sprintf("%s.%u", sFile.c_str(), i);
						fileNminus1.sprintf("%s.%u", sFile.c_str(), i-1);
						DeleteFile(fileN);
						RenameFile(fileNminus1, fileN);
					}
					AnsiString file1;
					file1.sprintf("%s.1", sFile.c_str());
					DeleteFile(file1);
					RenameFile(sFile.c_str(), file1);
				}
				// truncate				
				fout = fopen(sFile.c_str(),"wt+");								
            }
        }
	}

	if (callbackLog)
		callbackLog(buf);
}

