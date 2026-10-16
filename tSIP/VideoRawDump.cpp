#include "baresip_base_config.h"

#ifdef USE_VIDEO

//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "VideoRawDump.h"
#include "Log.h"
#include <re.h>
#include "baresip.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>

//---------------------------------------------------------------------------

#pragma package(smart_init)

namespace
{
	/** Protects the dump state: written from the UA thread (start / stop),
		read from the video decode path */
	CRITICAL_SECTION cs;
	FILE *dumpFile = NULL;		/**< raw H.264 output */
	void *dumpMp4 = NULL;		/**< MP4 output (baresip h264_mp4_*) */
	VideoRawDumpFormat dumpFormat = VRD_H264_RAW;
	bool dumpActive = false;
	/** Decoder state of the recorded video stream (compared by pointer only) */
	const void *dumpDec = NULL;
	/** Call UID of the recorded video stream (for StopForCall) */
	unsigned int dumpCallUid = 0;
	/** false after Start() until an access unit starting with SPS or IDR arrives */
	bool dumpSynced = false;

	/** Access unit starts with Annex-B start code followed by SPS (7) or IDR slice (5) */
	bool IsKeyAccessUnit(const uint8_t *buf, size_t len)
	{
		size_t i;
		if (len >= 4 && buf[0] == 0 && buf[1] == 0 && buf[2] == 0 && buf[3] == 1)
			i = 4;
		else if (len >= 3 && buf[0] == 0 && buf[1] == 0 && buf[2] == 1)
			i = 3;
		else
			return false;
		if (i >= len)
			return false;
		int type = buf[i] & 0x1F;
		return (type == 5 || type == 7);
	}

	/** Close output, caller must hold cs */
	void CloseLocked(void)
	{
		if (dumpFile)
		{
			fclose(dumpFile);
			dumpFile = NULL;
		}
		if (dumpMp4)
		{
			h264_mp4_close(dumpMp4);
			dumpMp4 = NULL;
		}
		dumpActive = false;
		dumpDec = NULL;
	}

	void OnAccessUnit(const void *dec, const uint8_t *buf, size_t len, void *arg)
	{
		(void)arg;
		EnterCriticalSection(&cs);
		if (dumpActive && dec == dumpDec)
		{
			if (!dumpSynced && IsKeyAccessUnit(buf, len))
				dumpSynced = true;
			if (dumpSynced)
			{
				bool failed;
				if (dumpFormat == VRD_MP4)
					failed = (h264_mp4_write(dumpMp4, buf, len) != 0);
				else
					failed = (fwrite(buf, 1, len, dumpFile) != len);
				if (failed)
				{
					LOG("Video raw dump: write error, stopping\n");
					CloseLocked();
				}
			}
		}
		LeaveCriticalSection(&cs);
	}
}

bool VideoRawDump::IsMp4Supported(void)
{
	return h264_mp4_supported();
}

VideoRawDumpFormat VideoRawDump::EffectiveFormat(unsigned int configured)
{
	if (configured == VRD_MP4 && IsMp4Supported())
		return VRD_MP4;
	return VRD_H264_RAW;
}

void VideoRawDump::Init(void)
{
	InitializeCriticalSection(&cs);
	avcodec_h264_set_au_dump(OnAccessUnit, NULL);
}

int VideoRawDump::Start(AnsiString fileName, const void *dec, unsigned int callUid,
			VideoRawDumpFormat format)
{
	if (!dec)
	{
		LOG("Video raw dump: no video decoder for this call (video not received yet)\n");
		return EINVAL;
	}

	FILE *f = NULL;
	void *mp4 = NULL;
	int err = 0;
	if (format == VRD_MP4)
	{
		if (!IsMp4Supported())
		{
			LOG("Video raw dump: MP4 is not supported by the FFmpeg libraries in use, use H.264 raw\n");
			return EPROTONOSUPPORT;
		}
		err = h264_mp4_open(&mp4, fileName.c_str());
	}
	else
	{
		f = fopen(fileName.c_str(), "wb");
		if (!f)
			err = errno ? errno : 1;
	}
	if (err)
	{
		LOG("Video raw dump: cannot open %s (%s)\n", fileName.c_str(), strerror(err));
		return err;
	}

	EnterCriticalSection(&cs);
	CloseLocked();
	dumpFile = f;
	dumpMp4 = mp4;
	dumpFormat = format;
	dumpDec = dec;
	dumpCallUid = callUid;
	dumpSynced = false;
	dumpActive = true;
	LeaveCriticalSection(&cs);
	LOG("Video raw dump: started, %s\n", fileName.c_str());
	return 0;
}

void VideoRawDump::Stop(void)
{
	EnterCriticalSection(&cs);
	if (dumpActive)
	{
		CloseLocked();
		LOG("Video raw dump: stopped\n");
	}
	LeaveCriticalSection(&cs);
}

void VideoRawDump::StopForCall(unsigned int callUid)
{
	EnterCriticalSection(&cs);
	if (dumpActive && dumpCallUid == callUid)
	{
		CloseLocked();
		LOG("Video raw dump: stopped (call ended)\n");
	}
	LeaveCriticalSection(&cs);
}

void VideoRawDump::Shutdown(void)
{
	// no new access units reach the dump after this; the critical section
	// is kept (the decode thread may still be leaving OnAccessUnit)
	avcodec_h264_set_au_dump(NULL, NULL);
	Stop();
}

bool VideoRawDump::IsActive(void)
{
	EnterCriticalSection(&cs);
	bool active = dumpActive;
	LeaveCriticalSection(&cs);
	return active;
}

#endif	// USE_VIDEO
