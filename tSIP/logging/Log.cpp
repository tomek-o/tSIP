/** \file
 *  \brief Global logging unit
 */

#include <vcl.h>
#pragma hdrstop 

#include "Log.h"

#include <stdio.h>
#include <string.h>
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

void CLog::SetLogRotateCnt(unsigned int cnt)
{
	ScopedLock<StaticMutex> lock(mutex);
	maxLogrotateCnt = cnt;
}

void CLog::log(const char *lpData, ...)
{
	va_list ap;
	va_start(ap, lpData);
	emit(LogLevelInfo, "", lpData, ap);
	va_end(ap);
}

void CLog::logL(int level, const char *module, const char *lpData, ...)
{
	va_list ap;
	va_start(ap, lpData);
	emit(level, module, lpData, ap);
	va_end(ap);
}

void CLog::emit(int level, const char *module, const char *lpData, va_list ap)
{
	(void)module;
	char buf[2048]; //determines max message length
	CallbackLog callback = NULL;

	// formatting works on the local buffer only - no need to hold the lock
	{
		int size;
		{
			// line format: <level> <day> <HH:MM:SS.mmm> <text>, the module name is not written
			struct timeb timebuffer;
			ftime( &timebuffer );
			const struct tm *tm_info = localtime(&timebuffer.time);
			int res = snprintf(buf, sizeof(buf), "%d %02d %02d:%02d:%02d.%03hu ",
				level,
				tm_info ? tm_info->tm_mday : 0,
				tm_info ? tm_info->tm_hour : 0,
				tm_info ? tm_info->tm_min : 0,
				tm_info ? tm_info->tm_sec : 0,
				timebuffer.millitm);
			buf[sizeof(buf)-1] = '\0';
			// RTL returns a negative value on truncation
			if (res < 0)
				size = (int)sizeof(buf) - 2;
			else
				size = res;
		}
		if (size > (int)sizeof(buf) - 2)
			size = (int)sizeof(buf) - 2;

		if ((int)sizeof(buf)-size-2 > 0)
		{
			int res = vsnprintf(buf + size, sizeof(buf)-size-2, lpData, ap);
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
		callback(level, module, buf);
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
		callback(LogLevelInfo, "", buf);
}

void CLog::logRawLen(const char *text, size_t len)
{
	char buf[2048]; //determines max message length, same as logRaw()
	CallbackLog callback = NULL;

	if (text == NULL)
		return;

	{
		int size = (len > sizeof(buf) - 2) ? (int)sizeof(buf) - 2 : (int)len;
		memcpy(buf, text, size);
		buf[size] = '\0';

		ScopedLock<StaticMutex> lock(mutex);
		write(buf, size);
		callback = callbackLog;
	}

	// see log(): the callback must not run with the lock held
	if (callback)
		callback(LogLevelInfo, "", buf);
}

void CLog::write(const char* buf, int size)
{
	if (bLogToFile && fout)
	{
		// text mode stream: each LF is stored as CRLF, so a CR already in front of it is dropped
		// (SIP messages contain CRLF already; without this they would be stored as CR CR LF).
		// The buffer is copied only when such a pair exists.
		std::string text;
		const char *out = buf;
		int outSize = size;
		bool hasCrlf = false;
		for (int i = 0; i + 1 < size && !hasCrlf; i++)
			hasCrlf = (buf[i] == '\r' && buf[i + 1] == '\n');
		if (hasCrlf)
		{
			text.reserve(size);
			for (int i = 0; i < size; i++)
			{
				if (buf[i] == '\r' && i + 1 < size && buf[i + 1] == '\n')
					continue;
				text += buf[i];
			}
			out = text.data();
			outSize = (int)text.size();
		}
		// fwrite() with size=0 always returns 0, even on success
		if (outSize > 0 && fwrite(out, outSize, 1, fout) != 1)
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
		curFileSize += outSize + countNewlines(out, outSize);
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
