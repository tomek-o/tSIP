/** \file
 *  \brief Global logging unit
 */

#include <vcl.h>
#pragma hdrstop 

#include "Log.h"

#include <stdio.h>
#include <time.h>
#include <sys\timeb.h>
#include <windows.h>
#include "common/StaticMutex.h"
#include "common/ScopedLock.h"

namespace {
	FILE *fout = NULL;
	/// guards fout; StaticMutex as it is still locked during shutdown
	StaticMutex mutex;

	/// number of '\n' in the buffer, each stored as CRLF in a text mode file
	unsigned int countNewlines(const char *buf, int size)
	{
		unsigned int cnt = 0;
		for (int i=0; i<size; i++)
		{
			if (buf[i] == '\n')
				cnt++;
		}
		return cnt;
	}

	/// size of an open stream, zero if it cannot be determined
	unsigned long streamSize(FILE *f)
	{
		if (f == NULL || fseek(f, 0, SEEK_END) != 0)
			return 0;
		long pos = ftell(f);
		return (pos > 0) ? (unsigned long)pos : 0;
	}
}

CLog::CLog()
{
	sFile = "";
	bLogToFile = true;
	bFlush = false;
	callbackLog = NULL;
	maxFileSize = 0;
	curFileSize = 0;
	timestamps = false;
	maxLogrotateCnt = 1;
};

CLog::~CLog()
{
	callbackLog = NULL;
	Close();
}

int CLog::SetFile(const std::string &file)
{
	ScopedLock<StaticMutex> lock(mutex);
	if (sFile == file)
	{
		return 0;
	}
	sFile = file;
	if (fout)
	{
		fclose(fout);
		fout = NULL;
	}
	curFileSize = 0;
	// empty name disables logging to file - that is not an error
	if (file == "")
		return 0;
	fout = fopen(sFile.c_str(),"at+");
	if (fout == NULL)
	{
		sFile = "";
		return -1;
	}
	curFileSize = streamSize(fout);
	return 0;
}

void CLog::Close(void)
{
	ScopedLock<StaticMutex> lock(mutex);
	if (fout)
		fclose(fout);
	fout = NULL;
	curFileSize = 0;
}

void CLog::SetLogToFile(bool state)
{
	ScopedLock<StaticMutex> lock(mutex);
	bLogToFile = state;
}

void CLog::SetFlush(bool state)
{
	ScopedLock<StaticMutex> lock(mutex);
	bFlush = state;
}

void CLog::SetMaxFileSize(unsigned int size)
{
	ScopedLock<StaticMutex> lock(mutex);
	maxFileSize = size;
}

void CLog::SetTimestamps(bool enabled)
{
	ScopedLock<StaticMutex> lock(mutex);
	timestamps = enabled;
}

void CLog::SetLogRotateCnt(unsigned int cnt)
{
	ScopedLock<StaticMutex> lock(mutex);
	maxLogrotateCnt = cnt;
}

void CLog::log(const char *lpData, ...)
{
	va_list ap;
	char buf[2048]; //determines max message length
	CallbackLog callback = NULL;

	// formatting works on the local buffer only - no need to hold the lock
	{
		int size;
		if (timestamps)
		{
			struct timeb timebuffer;
			ftime( &timebuffer );
			struct tm *tm_info = localtime(&timebuffer.time);
			size = tm_info ? strftime(buf, sizeof(buf), "%T", tm_info) : 0;
			int res = snprintf(buf+size, sizeof(buf)-size, ".%03hu ", timebuffer.millitm);
			buf[sizeof(buf)-1] = '\0';
			// RTL returns a negative value on truncation
			if (res < 0)
				size = (int)sizeof(buf) - 2;
			else
				size += res;
		}
		else
		{
			size = 0;
		}
		if (size > (int)sizeof(buf) - 2)
			size = (int)sizeof(buf) - 2;

		if ((int)sizeof(buf)-size-2 > 0)
		{
			va_start(ap, lpData);
			int res = vsnprintf(buf + size, sizeof(buf)-size-2, lpData, ap);
			va_end(ap);
			if (res < 0)
				size = (int)sizeof(buf) - 2;
			else
				size += res;
		}
		if (size > (int)sizeof(buf) - 2)
			size = (int)sizeof(buf) - 2;

		buf[size] = '\0';

		ScopedLock<StaticMutex> lock(mutex);
		write(buf, size);
		callback = callbackLog;
	}

	// called with the lock released: it may block on the log window mutex and
	// would otherwise hold up every other thread that logs
	if (callback)
		callback(buf);
}

void CLog::logRaw(const char *lpData, ...)
{
	va_list ap;
	char buf[2048]; //determines max message length
	CallbackLog callback = NULL;

	// formatting works on the local buffer only - no need to hold the lock
	{
		int size = 0;

		if ((int)sizeof(buf)-size-2 > 0)
		{
			va_start(ap, lpData);
			int res = vsnprintf(buf + size, sizeof(buf)-size-2, lpData, ap);
			va_end(ap);
			// RTL returns a negative value on truncation
			if (res < 0)
				size = (int)sizeof(buf) - 2;
			else
				size += res;
		}
		if (size > (int)sizeof(buf) - 2)
			size = (int)sizeof(buf) - 2;

		buf[size] = '\0';

		ScopedLock<StaticMutex> lock(mutex);
		write(buf, size);
		callback = callbackLog;
	}

	// see log(): the callback must not run with the lock held
	if (callback)
		callback(buf);
}

void CLog::write(const char* buf, int size)
{
	if (bLogToFile && fout)
	{
		// fwrite() with size=0 always returns 0, even on success
		if (size > 0 && fwrite(buf, size, 1, fout) != 1)
		{
			// bad stream (e.g. file removed while open) - recreate it
			fclose(fout);
			fout = fopen(sFile.c_str(), "at+");
			if (!fout)
			{
				sFile = "";
			}
			curFileSize = streamSize(fout);
			return;
		}
		// text mode stream: each line terminator is stored as CRLF
		curFileSize += size + countNewlines(buf, size);
		if (bFlush)
		{
			fflush(fout);
		}
		if (maxFileSize != 0 && curFileSize > maxFileSize)
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
			if (!fout)
			{
				sFile = "";
			}
			curFileSize = 0;
		}
	}
}
